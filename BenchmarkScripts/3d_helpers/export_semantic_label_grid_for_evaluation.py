# Example script to output evaluation format given a volumetric grid of predictions.
# Input:
#   - prediction grid as path to .npy np array
#   - world2grid transform as path to .npy 4x4 np array
#   - path to the corresponding *_vh_clean_2.ply mesh
#   - output file to write evaluation format .txt file
#
# example usage: export_semantic_label_grid_for_evaluation.py --grid_file [path to predicted grid] --world2grid_file [path to world to grid] --output_file [output file] --mesh_file [path to corresponding mesh file]

# python imports
import math
import os, sys, argparse
import inspect

try:
    import numpy as np
except:
    print "Failed to import numpy package."
    sys.exit(-1)

currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0,parentdir)
import util_3d

parser = argparse.ArgumentParser()
parser.add_argument('--grid_file', required=True, help='path to predicted grid as np array')
parser.add_argument('--world2grid_file', required=True, help='path to world2grid matrix as np array')
parser.add_argument('--output_file', required=True, help='output file')
parser.add_argument('--mesh_file', required=True, help='path to scene*_vh_clean_2.ply')
opt = parser.parse_args()


# find label/instance ids per mesh vertex by looking up each mesh vertex in the grid of predicted ids
def export(prediction_grid, world2grid, mesh_vertices, output_file):
    grid_dim = prediction_grid.shape
    grid_dim = np.flip(np.array(grid_dim), axis=0)
    num_verts = mesh_vertices.shape[0]
    ids = np.zeros(shape=(num_verts), dtype=np.uint32)
    vertices_grid = util_3d.transform_points(world2grid, mesh_vertices)
    for i in range(num_verts):
        v = vertices_grid[i]
        grid_coord = np.floor(v)
        # clamp to grid bounds
        grid_coord = np.clip(grid_coord, np.zeros((3)), grid_dim-1).astype(np.int32)
        ids[i] = prediction_grid[grid_coord[2], grid_coord[1], grid_coord[0]]
    util_3d.export_ids(output_file, ids)


def main():
    prediction_grid = np.load(opt.grid_file)
    world2grid = np.load(opt.world2grid_file)
    assert len(prediction_grid.shape) == 3 and world2grid.shape == (4,4)
    mesh_vertices = util_3d.read_mesh_vertices(opt.mesh_file)
    export(prediction_grid, world2grid, mesh_vertices, opt.output_file)


if __name__ == '__main__':
    main()
