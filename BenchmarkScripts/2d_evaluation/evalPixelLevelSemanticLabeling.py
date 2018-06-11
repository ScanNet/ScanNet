#!/usr/bin/python
#
# Adapted from: https://github.com/mcordts/cityscapesScripts
#
# The evaluation script for pixel-level semantic labeling.
# We use this script to evaluate your approach on the test set.
# You can use the script to evaluate on the validation set.
#
# usage: evalPixelLevelSemanticLabeling.py --gt_path [gtPath] --pred_path [predictionPath]
#
# Note that the script is a lot faster, if you enable cython support.
# WARNING: Cython only tested for Ubuntu 64bit OS.
# To enable cython, run
# setup.py build_ext --inplace
#
# To run this script, make sure that your results are images,
# where pixels encode the class IDs as defined in labels.py.
# Note that the regular ID is used, not the train ID.
# Further note that many classes are ignored from evaluation.
# Thus, authors are not expected to predict these classes and all
# pixels with a ground truth label that is ignored are ignored in
# evaluation.

# python imports
import os, sys, argparse
import math
import platform
import fnmatch

try:
    import numpy as np
except:
    print("Failed to import numpy package.")
    sys.exit(-1)
try:
    from PIL import Image
except:
    print("Please install the module 'Pillow' for image processing, e.g.")
    print("pip install pillow")
    sys.exit(-1)
try:
    from itertools import izip
except ImportError:
    izip = zip

# C Support
# Enable the cython support for faster evaluation
# Only tested for Ubuntu 64bit OS
CSUPPORT = True
# Check if C-Support is available for better performance
if CSUPPORT:
    try:
        import addToConfusionMatrix
    except:
        CSUPPORT = False

parser = argparse.ArgumentParser()
parser.add_argument('--gt_path', required=True, help='path to gt files')
parser.add_argument('--pred_path', required=True, help='path to result files')
parser.add_argument('--output_file', default='', help='output file (default pred_path/semantic_label.txt')
opt = parser.parse_args()
if not opt.output_file:
    opt.output_file = os.path.join(opt.pred_path, 'semantic_label.txt')



CLASS_LABELS = ['wall', 'floor', 'cabinet', 'bed', 'chair', 'sofa', 'table', 'door', 'window', 'bookshelf', 'picture', 'counter', 'desk', 'curtain', 'refrigerator', 'shower curtain', 'toilet', 'sink', 'bathtub', 'otherfurniture']
VALID_CLASS_IDS = np.array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 24, 28, 33, 34, 36, 39])
ALL_CLASS_IDS = np.arange(40 + 1)

#########################
# Methods
#########################

# Print an error message and quit
def printError(message, user_fault=False):
    print('ERROR: ' + str(message))
    if user_fault:
      sys.exit(2)
    sys.exit(-1)


# Generate empty confusion matrix and create list of relevant labels
def generateMatrix():
    # generate for all labels, regardless of being ignored
    max_id = np.max(ALL_CLASS_IDS)
    return np.zeros(shape=(max_id+1, max_id+1),dtype=np.ulonglong)


# Get absolute or normalized value from field in confusion matrix.
def getMatrixFieldValue(confMatrix, i, j, normalized=True):
    if normalized:
        rowSum = confMatrix[i].sum()
        if (rowSum == 0):
            return float('nan')
        return float(confMatrix[i][j]) / rowSum
    else:
        return confMatrix[i][j]

# Calculate and return IOU score for a particular label
def getIouScoreForLabel(label, confMatrix):
    if not label in VALID_CLASS_IDS:
        return float('nan')

    # the number of true positive pixels for this label
    # the entry on the diagonal of the confusion matrix
    tp = np.longlong(confMatrix[label,label])

    # the number of false negative pixels for this label
    # the row sum of the matching row in the confusion matrix
    # minus the diagonal entry
    fn = np.longlong(confMatrix[label,:].sum()) - tp

    # the number of false positive pixels for this labels
    # Only pixels that are not on a pixel with ground truth label that is ignored
    # The column sum of the corresponding column in the confusion matrix
    # without the ignored rows and without the actual label of interest
    notIgnored = [l for l in VALID_CLASS_IDS if not l==label]
    fp = np.longlong(confMatrix[notIgnored,label].sum())

    # the denominator of the IOU score
    denom = (tp + fp + fn)
    if denom == 0:
        return float('nan')

    # return IOU
    return float(tp) / denom

# Calculate prior for a particular class id.
def getPrior(label, confMatrix):
    return float(confMatrix[label,:].sum()) / confMatrix.sum()

# Get average of scores.
# Only computes the average over valid entries.
def getScoreAverage(scoreList):
    validScores = 0
    scoreSum    = 0.0
    for score in scoreList:
        if not math.isnan(scoreList[score]):
            validScores += 1
            scoreSum += scoreList[score]
    if validScores == 0:
        return float('nan')
    return scoreSum / validScores

# Print intersection-over-union scores for all classes.
def printClassScores(scoreList):
    print 'classes          IoU'
    print '----------------------------'
    for i in range(len(VALID_CLASS_IDS)):
        label = VALID_CLASS_IDS[i]
        labelName = CLASS_LABELS[i]
        iouStr = "{0:>5.3f}".format(scoreList[labelName])
        print ("{0:<14s}: ".format(labelName) + iouStr)

# Save results.
def write_result_file(conf, scores, filename):
    _SPLITTER = ','
    with open(filename, 'w') as f:
        f.write('iou scores\n')
        for i in range(len(VALID_CLASS_IDS)):
            label = VALID_CLASS_IDS[i]
            label_name = CLASS_LABELS[i]
            iou = scores[label_name]
            f.write('{0:<14s}({1:<2d}): {2:>5.3f}\n'.format(label_name, label, iou))
        f.write('\nconfusion matrix\n')
        for i in range(len(VALID_CLASS_IDS)):
            f.write('\t{0:<14s}({1:<2d})'.format(CLASS_LABELS[i], VALID_CLASS_IDS[i]))
        for r in range(len(VALID_CLASS_IDS)):
            f.write('{0:<14s}({1:<2d})'.format(CLASS_LABELS[r], VALID_CLASS_IDS[r]))
            for c in range(len(VALID_CLASS_IDS)):
                f.write('\t{0:>5.3f}'.format(conf[r,c]))
            f.write('\n')
    print 'wrote results to', filename


# Evaluate image lists pairwise.
def evaluateImgLists(predictionImgList, groundTruthImgList, outputFile):
    if len(predictionImgList) != len(groundTruthImgList):
        printError("List of images for prediction and groundtruth are not of equal size.", user_fault=True)
    confMatrix    = generateMatrix()
    perImageStats = {}
    nbPixels      = 0

    print 'Evaluating', len(predictionImgList), 'pairs of images...'

    # Evaluate all pairs of images and save them into a matrix
    for i in range(len(predictionImgList)):
        predictionImgFileName = predictionImgList[i]
        groundTruthImgFileName = groundTruthImgList[i]
        #print "Evaluate ", predictionImgFileName, "<>", groundTruthImgFileName
        nbPixels += evaluatePair(predictionImgFileName, groundTruthImgFileName, confMatrix, perImageStats)

        # sanity check
        if confMatrix.sum() != nbPixels:
            printError('Number of analyzed pixels and entries in confusion matrix disagree: confMatrix {}, pixels {}'.format(confMatrix.sum(),nbPixels))

        sys.stdout.write("\rImages Processed: {}".format(i+1))
        sys.stdout.flush()
    print ""

    # sanity check
    if confMatrix.sum() != nbPixels:
        printError('Number of analyzed pixels and entries in confusion matrix disagree: contMatrix {}, pixels {}'.format(confMatrix.sum(),nbPixels))

    # Calculate IOU scores on class level from matrix
    classScoreList = {}
    for i in range(len(VALID_CLASS_IDS)):
        labelName = CLASS_LABELS[i]
        label = VALID_CLASS_IDS[i]
        classScoreList[labelName] = getIouScoreForLabel(label, confMatrix)

    # Print IOU scores
    printClassScores(classScoreList)
    iouAvgStr  = "{avg:5.3f}".format(avg=getScoreAverage(classScoreList))
    print "--------------------------------"
    print "Score Average : " + iouAvgStr
    print "--------------------------------"
    print ""

    # write result file
    write_result_file(confMatrix, classScoreList, outputFile)

# Main evaluation method. Evaluates pairs of prediction and ground truth
# images which are passed as arguments.
def evaluatePair(predictionImgFileName, groundTruthImgFileName, confMatrix, perImageStats):
    # Loading all resources for evaluation.
    try:
        predictionImg = Image.open(predictionImgFileName)
        predictionNp  = np.array(predictionImg)
    except Exception, e:
        printError("Unable to load " + predictionImgFileName + ": " + str(e))
    try:
        groundTruthImg = Image.open(groundTruthImgFileName)
        groundTruthNp = np.array(groundTruthImg)
    except Exception, e:
        printError("Unable to load " + groundTruthImgFileName + ": " + str(e))

    # Check for equal image sizes
    if not (predictionImg.size[0] == groundTruthImg.size[0] or predictionImg.size[0] == 640 and predictionImg.size[1] == 480):
        printError("Invalid image size for " + predictionImgFileName, user_fault=True)
    if ( len(predictionNp.shape) != 2 ):
        printError("Predicted image has multiple channels.", user_fault=True)

    # resize for evaluation 
    predictionImg = predictionImg.resize((640, 480), Image.NEAREST)
    predictionNp  = np.array(predictionImg)
    groundTruthImg = groundTruthImg.resize((640, 480), Image.NEAREST)
    groundTruthNp = np.array(groundTruthImg)
    imgWidth  = predictionImg.size[0]
    imgHeight = predictionImg.size[1]
    nbPixels  = imgWidth*imgHeight
    # Evaluate images
    if (CSUPPORT):
        # using cython
        confMatrix = addToConfusionMatrix.cEvaluatePair(predictionNp, groundTruthNp, confMatrix, VALID_CLASS_IDS.tolist())
    else:
        # the slower python way
        for (groundTruthImgPixel,predictionImgPixel) in izip(groundTruthImg.getdata(),predictionImg.getdata()):
            if (not groundTruthImgPixel in VALID_CLASS_IDS):
                printError("Unknown label with id {:}".format(groundTruthImgPixel))

            confMatrix[groundTruthImgPixel][predictionImgPixel] += 1
    return nbPixels

# The main method
def main():

    pred_files = os.listdir(opt.pred_path)
    gt_files = []
    if len(pred_files) == 0:
        printError("No result files found.", user_fault=True)
    for i in range(len(pred_files)):
        gt_file = os.path.join(opt.gt_path, pred_files[i])
        if not os.path.isfile(gt_file):
            printError("Result file {} does not match any gt file".format(pred_files[i]), user_fault=True)
        gt_files.append(gt_file)
        pred_files[i] = os.path.join(opt.pred_path, pred_files[i])

    # evaluate
    evaluateImgLists(pred_files, gt_files, opt.output_file)

    return

# call the main method
if __name__ == "__main__":
    main()
