import os, sys
import json

try:
    import numpy as np
except:
    print "Failed to import numpy package."
    sys.exit(-1)

try:
    from plyfile import PlyData, PlyElement
except:
    print "Please install the module 'plyfile' for PLY i/o, e.g."
    print "pip install plyfile"
    sys.exit(-1)

import util


# matrix: 4x4 np array
# points Nx3 np array
def transform_points(matrix, points):
    assert len(points.shape) == 2 and points.shape[1] == 3
    num_points = points.shape[0]
    p = np.concatenate([points, np.ones((num_points, 1))], axis=1)
    p = np.matmul(matrix, np.transpose(p))
    p = np.transpose(p)
    p[:,:3] /= p[:,3,None]
    return p[:,:3]


def export_ids(filename, ids):
    with open(filename, 'w') as f:
        for id in ids:
            f.write('%d\n' % id)


def load_ids(filename):
    ids = open(filename).read().splitlines()
    ids = np.array(ids, dtype=np.int64)
    return ids


def read_mesh_vertices(filename):
    assert os.path.isfile(filename)
    with open(filename, 'rb') as f:
        plydata = PlyData.read(f)
        num_verts = plydata['vertex'].count
        vertices = np.zeros(shape=[num_verts, 3], dtype=np.float32)
        vertices[:,0] = plydata['vertex'].data['x']
        vertices[:,1] = plydata['vertex'].data['y']
        vertices[:,2] = plydata['vertex'].data['z']
    return vertices


# export 3d instance labels for instance evaluation
def export_instance_ids_for_eval(filename, label_ids, instance_ids):
    assert label_ids.shape[0] == instance_ids.shape[0]
    output_mask_path_relative = 'pred_mask'
    name = os.path.splitext(os.path.basename(filename))[0]
    output_mask_path = os.path.join(os.path.dirname(filename), output_mask_path_relative)
    if not os.path.isdir(output_mask_path):
        os.mkdir(output_mask_path)
    insts = np.unique(instance_ids)
    zero_mask = np.zeros(shape=(instance_ids.shape[0]), dtype=np.int32)
    with open(filename, 'w') as f:
        for idx, inst_id in enumerate(insts):
            if inst_id == 0:  # 0 -> no instance for this vertex
                continue
            output_mask_file = os.path.join(output_mask_path_relative, name + '_' + str(idx) + '.txt')
            loc = np.where(instance_ids == inst_id)
            label_id = label_ids[loc[0][0]]
            f.write('%s %d %f\n' % (output_mask_file, label_id, 1.0))
            # write mask
            mask = np.copy(zero_mask)
            mask[loc[0]] = 1
            export_ids(output_mask_file, mask)


# ------------ Instance Utils ------------ #

class Instance(object):
    instance_id = 0
    label_id = 0
    vert_count = 0
    med_dist = -1
    dist_conf = 0.0

    def __init__(self, mesh_vert_instances, instance_id):
        if (instance_id == -1):
            return
        self.instance_id     = int(instance_id)
        self.label_id    = int(self.get_label_id(instance_id))
        self.vert_count = int(self.get_instance_verts(mesh_vert_instances, instance_id))

    def get_label_id(self, instance_id):
        return int(instance_id // 1000)

    def get_instance_verts(self, mesh_vert_instances, instance_id):
        return (mesh_vert_instances == instance_id).sum()

    def to_json(self):
        return json.dumps(self, default=lambda o: o.__dict__, sort_keys=True, indent=4)

    def to_dict(self):
        dict = {}
        dict["instance_id"] = self.instance_id
        dict["label_id"]    = self.label_id
        dict["vert_count"]  = self.vert_count
        dict["med_dist"]    = self.med_dist
        dict["dist_conf"]   = self.dist_conf
        return dict

    def from_json(self, data):
        self.instance_id     = int(data["instance_id"])
        self.label_id        = int(data["label_id"])
        self.vert_count      = int(data["vert_count"])
        if ("med_dist" in data):
            self.med_dist    = float(data["med_dist"])
            self.dist_conf   = float(data["dist_conf"])

    def __str__(self):
        return "("+str(self.instance_id)+")"

def read_instance_prediction_file(filename, pred_path):
    lines = open(filename).read().splitlines()
    instance_info = {}
    abs_pred_path = os.path.abspath(pred_path)
    for line in lines:
        parts = line.split(' ')
        if len(parts) != 3:
            util.print_error('invalid instance prediction file. Expected (per line): [rel path prediction] [label id prediction] [confidence prediction]')
        if os.path.isabs(parts[0]):
            util.print_error('invalid instance prediction file. First entry in line must be a relative path')
        mask_file = os.path.join(os.path.dirname(filename), parts[0])
        mask_file = os.path.abspath(mask_file)
        # check that mask_file lives inside prediction path
        if os.path.commonprefix([mask_file, abs_pred_path]) != abs_pred_path:
            util.print_error('predicted mask {} in prediction text file {} points outside of prediction path.'.format(mask_file,filename))

        info            = {}
        info["label_id"] = int(float(parts[1]))
        info["conf"]    = float(parts[2])
        instance_info[mask_file]  = info
    return instance_info


def get_instances(ids, class_ids, class_labels, id2label):
    instances = {}
    for label in class_labels:
        instances[label] = []
    instance_ids = np.unique(ids)
    for id in instance_ids:
        if id == 0:
            continue
        inst = Instance(ids, id)
        if inst.label_id in class_ids:
            instances[id2label[inst.label_id]].append(inst.to_dict())
    return instances
            


