# Evaluates semantic label task
# Input:
#   - path to .txt prediction files
#   - path to .txt ground truth files
#   - output file to write results to
# Note that only the valid classes are used for evaluation,
# i.e., any ground truth label not in the valid label set
# is ignored in the evaluation.
#
# example usage: evaluate_semantic_label.py --scan_path [path to scan data] --output_file [output file]

# python imports
import math
import os, sys, argparse
import inspect

try:
    import numpy as np
except:
    print "Failed to import numpy package."
    sys.exit(-1)
try:
    from itertools import izip
except ImportError:
    izip = zip

currentdir = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
parentdir = os.path.dirname(currentdir)
sys.path.insert(0,parentdir)
import util
import util_3d

parser = argparse.ArgumentParser()
parser.add_argument('--pred_path', required=True, help='path to directory of predicted .txt files')
parser.add_argument('--gt_path', required=True, help='path to gt files')
parser.add_argument('--output_file', default='', help='output file [default: pred_path/semantic_label_evaluation.txt]')
opt = parser.parse_args()

if opt.output_file == '':
    opt.output_file = os.path.join(opt.pred_path, 'semantic_label_evaluation.txt')


CLASS_LABELS = ['wall', 'floor', 'cabinet', 'bed', 'chair', 'sofa', 'table', 'door', 'window', 'bookshelf', 'picture', 'counter', 'desk', 'curtain', 'refrigerator', 'shower curtain', 'toilet', 'sink', 'bathtub', 'otherfurniture']
VALID_CLASS_IDS = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 24, 28, 33, 34, 36, 39])
UNKNOWN_ID = np.max(VALID_CLASS_IDS) + 1


def evaluate_scan(pred_file, gt_file, confusion):
    try:
        pred_ids = util_3d.load_ids(pred_file)
    except Exception, e:
        util.print_error('unable to load ' + pred_file + ': ' + str(e))
    try:
        gt_ids = util_3d.load_ids(gt_file)
    except Exception, e:
        util.print_error('unable to load ' + gt_file + ': ' + str(e))
    # sanity checks
    if not pred_ids.shape == gt_ids.shape:
        util.print_error('%s: number of predicted values does not match number of vertices' % pred_file, user_fault=True)
    for (gt_val,pred_val) in izip(gt_ids.flatten(),pred_ids.flatten()):
        if gt_val not in VALID_CLASS_IDS:
            continue
        if pred_val not in VALID_CLASS_IDS:
            pred_val = UNKNOWN_ID
        confusion[gt_val][pred_val] += 1


def get_iou(label_id, confusion):
    if not label_id in VALID_CLASS_IDS:
        return float('nan')
    # #true positives
    tp = np.longlong(confusion[label_id, label_id])
    # #false negatives
    fn = np.longlong(confusion[label_id, :].sum()) - tp
    # #false positives
    not_ignored = [l for l in VALID_CLASS_IDS if not l == label_id]
    fp = np.longlong(confusion[not_ignored, label_id].sum())

    denom = (tp + fp + fn)
    if denom == 0:
        return float('nan')
    return (float(tp) / denom, tp, denom)


def write_result_file(confusion, ious, filename):
    with open(filename, 'w') as f:
        f.write('iou scores\n')
        for i in range(len(VALID_CLASS_IDS)):
            label_id = VALID_CLASS_IDS[i]
            label_name = CLASS_LABELS[i]
            iou = ious[label_name][0]
            f.write('{0:<14s}({1:<2d}): {2:>5.3f}\n'.format(label_name, label_id, iou))
        f.write('\nconfusion matrix\n')
        f.write('\t\t\t')
        for i in range(len(VALID_CLASS_IDS)):
            #f.write('\t{0:<14s}({1:<2d})'.format(CLASS_LABELS[i], VALID_CLASS_IDS[i]))
            f.write('{0:<8d}'.format(VALID_CLASS_IDS[i]))
        f.write('\n')
        for r in range(len(VALID_CLASS_IDS)):
            f.write('{0:<14s}({1:<2d})'.format(CLASS_LABELS[r], VALID_CLASS_IDS[r]))
            for c in range(len(VALID_CLASS_IDS)):
                f.write('\t{0:>5.3f}'.format(confusion[VALID_CLASS_IDS[r],VALID_CLASS_IDS[c]]))
            f.write('\n')
    print 'wrote results to', filename


def evaluate(pred_files, gt_files, output_file):
    max_id = UNKNOWN_ID
    confusion = np.zeros((max_id+1, max_id+1), dtype=np.ulonglong)

    print 'evaluating', len(pred_files), 'scans...'
    for i in range(len(pred_files)):
        evaluate_scan(pred_files[i], gt_files[i], confusion)
        sys.stdout.write("\rscans processed: {}".format(i+1))
        sys.stdout.flush()
    print ''

    class_ious = {}
    for i in range(len(VALID_CLASS_IDS)):
        label_name = CLASS_LABELS[i]
        label_id = VALID_CLASS_IDS[i]
        class_ious[label_name] = get_iou(label_id, confusion)
    # print
    print 'classes          IoU'
    print '----------------------------'
    for i in range(len(VALID_CLASS_IDS)):
        label_name = CLASS_LABELS[i]
        #print('{{0:<14s}: 1:>5.3f}'.format(label_name, class_ious[label_name][0]))
        print('{0:<14s}: {1:>5.3f}   ({2:>6d}/{3:<6d})'.format(label_name, class_ious[label_name][0], class_ious[label_name][1], class_ious[label_name][2]))
    write_result_file(confusion, class_ious, output_file)


def main():
    pred_files = [f for f in os.listdir(opt.pred_path) if f.endswith('.txt') and f != 'semantic_label_evaluation.txt']
    gt_files = []
    if len(pred_files) == 0:
        util.print_error('No result files found.', user_fault=True)
    for i in range(len(pred_files)):
        gt_file = os.path.join(opt.gt_path, pred_files[i])
        if not os.path.isfile(gt_file):
            util.print_error('Result file {} does not match any gt file'.format(pred_files[i]), user_fault=True)
        gt_files.append(gt_file)
        pred_files[i] = os.path.join(opt.pred_path, pred_files[i])

    # evaluate
    evaluate(pred_files, gt_files, opt.output_file)


if __name__ == '__main__':
    main()
