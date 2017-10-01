#!/usr/bin/env python
# NOTE: remember to source setpath.sh in mitsuba dir or specify MITSUBA_DIR env variable
import argparse
import logging
import multiprocessing
import os
import sys
# add path for the Python extension module
MITSUBA_DIR = os.path.join(os.environ['MITSUBA_DIR'], '')
sys.path.append(MITSUBA_DIR + os.sep + 'python' + os.sep + '2.7')
# Ensure python can find Mitsuba core libraries
os.environ['PATH'] = MITSUBA_DIR + os.pathsep + os.environ['PATH']
import mitsuba
from mitsuba.core import *
from mitsuba.render import Scene, RenderQueue, RenderJob

# set up logger
FORMAT = '%(asctime)-15s [%(levelname)s] %(message)s'
logging.basicConfig(format=FORMAT)
log = logging.getLogger('mts-render')
log.setLevel(logging.INFO)


def render(args):
    # Start up the scheduling system
    scheduler = Scheduler.getInstance()
    queue = RenderQueue()
    for i in range(0, multiprocessing.cpu_count()):
        scheduler.registerWorker(LocalWorker(i, 'wrk%i' % i))
    scheduler.start()

    # create globals
    pmgr = PluginManager.getInstance()
    integrator = pmgr.create({'type': args.get('integrator')})
    emitter = pmgr.create({'type': 'constant'})
    sensor = pmgr.create({
        'type': 'perspective',
        'film': {
            'type': 'ldrfilm',
            'width': args.get('width'),
            'height':args.get('height'),
            'pixelFormat': 'rgba',
            'exposure': args.get('exposure'),
            'banner': False
        },
        'sampler': {
            'type': 'ldsampler',
            'sampleCount': args.get('samples')
        },
        'fov': 45.0#,
        #'toWorld': Transform.lookAt(Point(0, 0, 1), Point(0, 0, 0), Vector(0, 1, 0))
    })

    # add mesh to scene
    meshfile = args.get('mesh')
    mesh = pmgr.create({
        'type': 'ply',
        'filename': meshfile,
        'bsdf': {'type' : 'diffuse', 'reflectance' : {'type': 'vertexcolors'}}
    })
    scene = Scene()
    scene.addChild(integrator)
    scene.addChild(emitter)
    scene.addChild(sensor)
    scene.addChild(mesh)
    scene.configure()
    scene.initialize()  # needed to force build of kd-tree

    # compute camera position
    aabb = mesh.getAABB()
    log.info(aabb)
    bsphere = aabb.getBSphere()
    camTarget = bsphere.center
    camTranslate = Transform.translate(Vector(camTarget))
    camUnTranslate = Transform.translate(-Vector(camTarget))
    UP = args.get('world_up')
    bsphere_mult = args.get('bsphere_mult')
    camOff_vec = args.get('camera_offset')
    camOffset = bsphere_mult * bsphere.radius * normalize(camOff_vec + UP)
    camOrigin = camTarget + camOffset
    camUp = args.get('camera_up')

    if args.get('render_turntable'):  # render turntable sequence
        angleDelta = args.get('frames_per_degree')
        stepSize = 1
        for i in range(0, 360 / angleDelta, stepSize):
            rotationCur = camTranslate * Transform.rotate(UP, i*angleDelta) * camUnTranslate;
            trafoCur = Transform.lookAt(rotationCur * camOrigin, camTarget, rotationCur * camUp)
            sensor.setWorldTransform(trafoCur)
            scene.setDestinationFile('{0}_%03i.png'.format(meshfile) % i)
            job = RenderJob('job_%i' % i, scene, queue)
            job.start()
            queue.waitLeft(0)
        # convert to video
        os.system('ffmpeg -y -pix_fmt yuv420p -r 30 -i {0}_%03d.png -vcodec libx264 {0}.mp4'.format(meshfile))
    else:  # render single view
        rotationCurr = Transform.rotate(UP, args.get('theta'))
        transformCurr = Transform.lookAt(rotationCurr * camOrigin, camTarget, rotationCurr * camUp)
        sensor.setWorldTransform(transformCurr)
        if args.get('outfile') is None:
            scene.setDestinationFile(os.path.splitext(meshfile)[0] + '.png')
        else:
            scene.setDestinationFile(args.get('outfile'))
        job = RenderJob('job', scene, queue)
        job.start()
        queue.waitLeft(0)

    queue.join()


def vec3(str):
    if str == 'x':
        return Vector(1, 0, 0)
    elif str == 'y':
        return Vector(0, 1, 0)
    elif str == 'z':
        return Vector(0, 0, 1)
    else:
        v = str.split(',')
        return Vector(float(v[0]), float(v[1]), float(v[2]))


def nvec3(str):
    return normalize(vec3(str))


def main():
    scriptpath = os.path.dirname(os.path.realpath(__file__))
    parser = argparse.ArgumentParser(description='Render PLY file using Mitsuba')
    parser.add_argument('-o', dest='outfile', action='store',
                        help='Output file')
    parser.add_argument('--cameras', dest='cameras', action='store',
                        help='Cameras file to render')
    parser.add_argument('--integrator', dest='integrator', action='store',
                        default='path', choices=['path','mlt','vpl','ao'],
                        help='Exposure value e, scales radiance by 2^e')
    parser.add_argument('--samples', dest='samples', action='store',
                        default=16, type=int,
                        help='Number of integrator samples per pixel')
    parser.add_argument('--width', dest='width', action='store',
                        default=640, type=int,
                        help='Image width')
    parser.add_argument('--height', dest='height', action='store',
                        default=480, type=int,
                        help='Image height')
    parser.add_argument('--frames_per_degree', dest='frames_per_degree', action='store',
                        default=5, type=int,
                        help='Image height')
    parser.add_argument('--theta', dest='theta', action='store',
                        default=0, type=float,
                        help='Azimuth angle to render from')
    parser.add_argument('--exposure', dest='exposure', action='store',
                        default=1.0, type=float,
                        help='Exposure value e, scales radiance by 2^e')
    parser.add_argument('--world_up', dest='world_up', action='store',
                        default=Vector(0, 0, 1), type=nvec3,
                        help='World up vector')
    parser.add_argument('--camera_up', dest='camera_up', action='store',
                        default=Vector(0, 1, 0), type=nvec3,
                        help='Camera image plane up vector')
    parser.add_argument('--camera_offset', dest='camera_offset', action='store',
                        default=Vector(0, 0, 0), type=nvec3,
                        help='Camera position offset vector to be added with one unit of world_up')
    parser.add_argument('--bsphere_mult', dest='bsphere_mult', action='store',
                        default=2.5, type=float,
                        help='Camera image plane up vector')
    parser.add_argument('--render_turntable', dest='render_turntable', action='store_true',
                        default=False,
                        help='Whether to render a turntable animation')
    parser.add_argument('mesh')
    args = parser.parse_args()
    render(vars(args))


if __name__ == "__main__":
    main()
