import numpy as np
from scipy import spatial
import argparse
import time

""" Usage:

python nearest_neighbor.py --query_feats query_feat.txt --database_feats shapenet_feat.txt --database_models shapenet_name.txt --output query_output.txt

"""

parser = argparse.ArgumentParser()
parser.add_argument('--query_feats', help='Query feat txt, each line is feat for one shape.')
parser.add_argument('--database_feats', help='Each line is feat vector for a model.')
parser.add_argument('--database_models', help='Each line is obj path for the model.')
parser.add_argument('--output', help='Output filename, line j includes topk database model paths (separated by space) to the query j')
parser.add_argument('--topk', type=int, default=3, help='Output topk database models.')
parser.add_argument('--max_num_database', default=0, help='max num database models to search, 0=all')
FLAGS = parser.parse_args()


starttime = time.time()

max_num_models = int(FLAGS.max_num_database)


# Load features, each row is a feature
feats = np.loadtxt(FLAGS.database_feats, delimiter=',')
# Load database model names
model_names = [x.rstrip() for x in open(FLAGS.database_models)]
# Load query features
query_feats = np.loadtxt(FLAGS.query_feats, delimiter=',')

print len(model_names)
print feats.shape
print query_feats.shape

if max_num_models > 0:
    del model_names[max_num_models:]
    feats = np.resize(feats, (max_num_models, 512))

print len(model_names)
print feats.shape

loadtime = time.time()
print 'load time = ', (loadtime - starttime), ' seconds'
print '#search models = ', feats.shape[0]
print len(model_names)

# Compute distances..
dists = spatial.distance.cdist(query_feats, feats)

disttime = time.time()
print 'dist time = ', (disttime - loadtime), ' seconds'

# Get nearest neighbor for each obj
fout = open(FLAGS.output, 'w')
for i in range(dists.shape[0]):
    neighbor_idxs = np.argsort(dists[i, :])
    print "Query %d's nearest neighbor is %s with distance %f" \
            % (i, model_names[neighbor_idxs[0]], dists[i, neighbor_idxs[0]])
    model_list = [model_names[neighbor_idxs[k]] for k in range(FLAGS.topk)]
    fout.write(' '.join(model_list) + '\n')
fout.close()

nntime = time.time()
print 'nn time = ', (nntime - disttime), ' seconds'

