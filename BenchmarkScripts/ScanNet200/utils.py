import os
import numpy as np
from plyfile import PlyData, PlyElement
import pandas as pd

from scannet200_constants import *

def read_plymesh(filepath):
    """Read ply file and return it as numpy array. Returns None if emtpy."""
    with open(filepath, 'rb') as f:
        plydata = PlyData.read(f)
    if plydata.elements:
        vertices = pd.DataFrame(plydata['vertex'].data).values
        faces = np.array([f[0] for f in plydata["face"].data])
        return vertices, faces

def save_plymesh(vertices, faces, filename, verbose=True, with_label=True):
    """Save an RGB point cloud as a PLY file.

    Args:
      points_3d: Nx6 matrix where points_3d[:, :3] are the XYZ coordinates and points_3d[:, 4:] are
          the RGB values. If Nx3 matrix, save all points with [128, 128, 128] (gray) color.
    """
    assert vertices.ndim == 2
    if with_label:
        if vertices.shape[1] == 7:
            python_types = (float, float, float, int, int, int, int)
            npy_types = [('x', 'f4'), ('y', 'f4'), ('z', 'f4'), ('red', 'u1'), ('green', 'u1'),
                         ('blue', 'u1'), ('label', 'u4')]

        if vertices.shape[1] == 8:
            python_types = (float, float, float, int, int, int, int, int)
            npy_types = [('x', 'f4'), ('y', 'f4'), ('z', 'f4'), ('red', 'u1'), ('green', 'u1'),
                         ('blue', 'u1'), ('label', 'u4'), ('instance_id', 'u4')]

    else:
        if vertices.shape[1] == 3:
            gray_concat = np.tile(np.array([128], dtype=np.uint8), (vertices.shape[0], 3))
            vertices = np.hstack((vertices, gray_concat))
        elif vertices.shape[1] == 6:
            python_types = (float, float, float, int, int, int)
            npy_types = [('x', 'f4'), ('y', 'f4'), ('z', 'f4'), ('red', 'u1'), ('green', 'u1'),
                         ('blue', 'u1')]
        else:
            pass

    vertices_list = []
    for row_idx in range(vertices.shape[0]):
        cur_point = vertices[row_idx]
        vertices_list.append(tuple(dtype(point) for dtype, point in zip(python_types, cur_point)))
    vertices_array = np.array(vertices_list, dtype=npy_types)
    elements = [PlyElement.describe(vertices_array, 'vertex')]

    if faces is not None:
        faces_array = np.empty(len(faces), dtype=[('vertex_indices', 'i4', (3,))])
        faces_array['vertex_indices'] = faces
        elements += [PlyElement.describe(faces_array, 'face')]

    # Write
    PlyData(elements).write(filename)

    if verbose is True:
        print('Saved point cloud to: %s' % filename)


# Map the raw category id to the point cloud
def point_indices_from_group(points, seg_indices, group, labels_pd, CLASS_IDs):
    group_segments = np.array(group['segments'])
    label = group['label']

    # Map the category name to id
    label_ids = labels_pd[labels_pd['raw_category'] == label]['id']
    label_id = int(label_ids.iloc[0]) if len(label_ids) > 0 else 0

    # Only store for the valid categories
    if not label_id in CLASS_IDs:
        label_id = 0

    # get points, where segment indices (points labelled with segment ids) are in the group segment list
    point_IDs = np.where(np.isin(seg_indices, group_segments))

    return points[point_IDs], point_IDs[0], label_id


# Uncomment out if mesh voxelization is required
# import trimesh
# from trimesh.voxel import creation
# from sklearn.neighbors import KDTree
# import MinkowskiEngine as ME


# VOXELIZE the scene from sampling on the mesh directly instead of vertices
def voxelize_pointcloud(points, colors, labels, instances, faces, voxel_size=0.2):

    # voxelize mesh first and determine closest labels with KDTree search
    trimesh_scene_mesh = trimesh.Trimesh(vertices=points, faces=faces)
    voxel_grid = creation.voxelize(trimesh_scene_mesh, voxel_size)
    voxel_cloud = np.asarray(voxel_grid.points)
    orig_tree = KDTree(points, leaf_size=8)
    _, voxel_pc_matches = orig_tree.query(voxel_cloud, k=1)
    voxel_pc_matches = voxel_pc_matches.flatten()

    # match colors to voxel ids
    points = points[voxel_pc_matches] / voxel_size
    colors = colors[voxel_pc_matches]
    labels = labels[voxel_pc_matches]
    instances = instances[voxel_pc_matches]

    # Voxelize scene
    quantized_scene, scene_inds = ME.utils.sparse_quantize(points, return_index=True)
    quantized_scene_colors = colors[scene_inds]
    quantized_labels = labels[scene_inds]
    quantized_instances = instances[scene_inds]

    return quantized_scene, quantized_scene_colors, quantized_labels, quantized_instances
