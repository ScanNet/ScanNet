# Evaluates scene type classification task
# Input:
#   - path to .txt prediction file
#   - path to .txt ground truth file
#   - output file to write results to
# Note that only the valid classes are used for evaluation,
# i.e., any ground truth label not in the valid label set
# is ignored in the evaluation.
#
# example usage: evaluate_scene_type.py --pred_file [path to prediction file] --gt_file [path to gt file] --output_file [output file]

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
parser.add_argument('--pred_file', required=True, help='path to directory of predicted grids and world2grids as np arrays')
parser.add_argument('--gt_file', required=True, help='path to gt files')
parser.add_argument('--output_file', default='', help='output file [default: scene_type_evaluation.txt]')
opt = parser.parse_args()

if opt.output_file == '':
    opt.output_file = 'scene_type_evaluation.txt'


CLASS_LABELS = ['apartment', 'bathroom', 'bedroom / hotel', 'bookstore / library', 'conference room', 'copy/mail room', 'hallway', 'kitchen', 'laundry room', 'living_room / lounge', 'office', 'storage / basement / garage', 'misc.']
VALID_CLASS_IDS = np.array([1, 2, 3, 4, 8, 9, 13, 14, 15, 16, 18, 20, 21])
UNKNOWN_ID = np.max(VALID_CLASS_IDS + 1)


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


def get_acc(label_id, confusion):
    if not label_id in VALID_CLASS_IDS:
        return float('nan')
    # #true positives
    tp = np.longlong(confusion[label_id, label_id])
    # #false negatives
    fn = np.longlong(confusion[label_id, :].sum()) - tp
    denom = (tp + fn)
    if denom == 0:
        return float('nan')
    return (float(tp) / denom, tp, denom)


def write_result_file(confusion, ious, accs, filename):
    with open(filename, 'w') as f:
        f.write('iou scores\n')
        for i in range(len(VALID_CLASS_IDS)):
            label_id = VALID_CLASS_IDS[i]
            label_name = CLASS_LABELS[i]
            iou = ious[label_name][0]
            f.write('{0:<32s}({1:<2d}): {2:>5.3f}\n'.format(label_name, label_id, iou))
        f.write('recall scores\n')
        for i in range(len(VALID_CLASS_IDS)):
            label_id = VALID_CLASS_IDS[i]
            label_name = CLASS_LABELS[i]
            acc = accs[label_name][0]
            f.write('{0:<32s}({1:<2d}): {2:>5.3f}\n'.format(label_name, label_id, acc))
        f.write('\nconfusion matrix\n')
        f.write('\t\t\t')
        for i in range(len(VALID_CLASS_IDS)):
            #f.write('\t{0:<32s}({1:<2d})'.format(CLASS_LABELS[i], VALID_CLASS_IDS[i]))
            f.write('{0:<8d}'.format(VALID_CLASS_IDS[i]))
        f.write('\n')
        for r in range(len(VALID_CLASS_IDS)):
            f.write('{0:<32s}({1:<2d})'.format(CLASS_LABELS[r], VALID_CLASS_IDS[r]))
            for c in range(len(VALID_CLASS_IDS)):
                f.write('\t{0:>5.3f}'.format(confusion[VALID_CLASS_IDS[r],VALID_CLASS_IDS[c]]))
            f.write('\n')
    print 'wrote results to', filename


def evaluate(pred_file, gt_file, output_file):
    max_id = UNKNOWN_ID + 1
    confusion = np.zeros((max_id+1, max_id+1), dtype=np.ulonglong)

    predictions = {}
    gt = {}
    try:
        lines = open(pred_file).read().splitlines()
    except Exception, e:
        util.print_error('unable to load ' + pred_file + ': ' + str(e))
    for line in lines:
        parts = line.split(' ')
        if len(parts) != 2 or not util.represents_int(parts[1]):
            util.print_error('Prediction file must have lines of format [%s %d] for scan name and scene type, respectively')
        predictions[parts[0]] = int(parts[1])
    try:
        lines = open(gt_file).read().splitlines()
    except Exception, e:
        util.print_error('unable to load ' + gt_file + ': ' + str(e))
    for line in lines:
        parts = line.split(' ')
        if len(parts) != 2 or not util.represents_int(parts[1]):
            util.print_error('Ground truth file must have lines of format [%s %d] for scan name and scene type, respectively')
        gt[parts[0]] = int(parts[1])
    # sanity checks
    if len(predictions) != len(gt):
        util.print_error('number of predicted scans (%d) does not match number of ground truth scans (%d)' % (len(predictions), len(gt)))
    print 'evaluating', len(predictions), 'scans...'
    for gt_scan,gt_type in gt.iteritems():
        if gt_type not in VALID_CLASS_IDS:
            continue
        if gt_scan not in predictions:
            util.print_error('prediction file does not contain gt scan %s' % gt_scan)
        pred_type = predictions[gt_scan]
        if pred_type not in VALID_CLASS_IDS:
            pred_type = UNKNOWN_ID
        confusion[gt_type][pred_type] += 1

    class_ious = {}
    class_accs = {}
    for i in range(len(VALID_CLASS_IDS)):
        label_name = CLASS_LABELS[i]
        label_id = VALID_CLASS_IDS[i]
        class_ious[label_name] = get_iou(label_id, confusion)
        class_accs[label_name] = get_acc(label_id, confusion)
    # print
    print 'classes          IoU'
    print '----------------------------'
    for i in range(len(VALID_CLASS_IDS)):
        label_name = CLASS_LABELS[i]
        #print('{{0:<32s}: 1:>5.3f}'.format(label_name, class_ious[label_name][0]))
        print('{0:<32s}: {1:>5.3f}   ({2:>6d}/{3:<6d})'.format(label_name, class_ious[label_name][0], class_ious[label_name][1], class_ious[label_name][2]))
    print ''
    print 'classes          recall'
    print '----------------------------'
    for i in range(len(VALID_CLASS_IDS)):
        label_name = CLASS_LABELS[i]
        print('{0:<32s}: {1:>5.3f}   ({2:>6d}/{3:<6d})'.format(label_name, class_accs[label_name][0], class_accs[label_name][1], class_accs[label_name][2]))
    write_result_file(confusion, class_ious, class_accs, output_file)


def main():
    # evaluate
    evaluate(opt.pred_file, opt.gt_file, opt.output_file)


if __name__ == '__main__':
    main()
