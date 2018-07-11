#!/usr/bin/python
#
# Adapted from: https://github.com/mcordts/cityscapesScripts
#
# The evaluation script for instance-level semantic labeling.
# We use this script to evaluate your approach on the test set.
# You can use the script to evaluate on the validation set.
#
# usage: evalInstanceLevelSemanticLabeling.py --gt_path [gtPath] --pred_path [predictionPath]
#
# To run this script, make sure that your results contain text files
# (one for each test set image) with the content:
#   relPathPrediction1 labelIDPrediction1 confidencePrediction1
#   relPathPrediction2 labelIDPrediction2 confidencePrediction2
#   relPathPrediction3 labelIDPrediction3 confidencePrediction3
#   ...
#
# - The given paths "relPathPrediction" point to images that contain
# binary masks for the described predictions, where any non-zero is
# part of the predicted instance. The paths must not contain spaces,
# must be relative to the root directory and must point to locations 
# within the root directory.
# - The label IDs "labelIDPrediction" specify the class of that mask
# - The field "confidencePrediction" is a float value that assigns a
# confidence score to the mask.
#
# Note that this tool creates a file named "gtInstances.json" during its
# first run. This file helps to speed up computation and should be deleted
# whenever anything changes in the ground truth annotations or anything
# goes wrong.

# python imports
import math
import os, sys, argparse
import fnmatch
from copy import deepcopy
import json

try:
    import numpy as np
except:
    print "Failed to import numpy package."
    sys.exit(-1)
try:
    from PIL import Image
except:
    print "Please install the module 'Pillow' for image processing, e.g."
    print "pip install pillow"
    sys.exit(-1)

from instances2dict import instances2dict


parser = argparse.ArgumentParser()
parser.add_argument('--gt_path', required=True, help='path to gt files')
parser.add_argument('--pred_path', required=True, help='path to result files')
parser.add_argument('--output_file', default='', help='output file (default pred_path/semantic_instance.txt')
opt = parser.parse_args()
if not opt.output_file:
    opt.output_file = os.path.join(opt.pred_path, 'semantic_instance.txt')


######################
# Parameters
######################

CLASS_LABELS = ['cabinet', 'bed', 'chair', 'sofa', 'table', 'door', 'window', 'bookshelf', 'picture', 'counter', 'desk', 'curtain', 'refrigerator', 'shower curtain', 'toilet', 'sink', 'bathtub', 'otherfurniture']
VALID_CLASS_IDS = np.array([3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 24, 28, 33, 34, 36, 39])
ID2LABEL = {}
for i in range(len(CLASS_LABELS)):
    ID2LABEL[VALID_CLASS_IDS[i]] = CLASS_LABELS[i]

# overlaps for evaluation
opt.overlaps           = np.arange(0.5,1.,0.05)
# minimum region size for evaluation [pixels]
opt.minRegionSizes     = np.array( [ 100 , 1000 , 1000 ] )
# distance thresholds [m]
opt.distanceThs        = np.array( [  float('inf') , 100 , 50 ] )
# distance confidences
opt.distanceConfs      = np.array( [ -float('inf') , 0.5 , 0.5 ] )

opt.gtInstancesFile    = os.path.join(os.path.dirname(os.path.realpath(__file__)),'gtInstances.json')


# Print an error message and quit
def printError(message, user_fault=False):
    print('ERROR: ' + str(message))
    if user_fault:
      sys.exit(2)
    sys.exit(-1)

# Returns the directory name for the given filename, e.g.
# fileName = "/foo/bar/foobar.txt"
# return value is "bar"
# Not much error checking though
def getDirectory(fileName):
    dirName = os.path.dirname(fileName)
    return os.path.basename(dirName)

# Make sure that the given path exists
def ensurePath(path):
    if not path:
        return
    if not os.path.isdir(path):
        os.makedirs(path)

# Write a dictionary as json file
def writeDict2JSON(dictName, fileName):
    with open(fileName, 'w') as f:
        f.write(json.dumps(dictName, default=lambda o: o.__dict__, sort_keys=True, indent=4))


# Read prediction info
# imgFile, predId, confidence
def readPredInfo(predInfoFileName):
    predInfo = {}
    if (not os.path.isfile(predInfoFileName)):
        printError("Infofile '{}' for the predictions not found.".format(predInfoFileName))
    abs_pred_path = os.path.abspath(opt.pred_path)
    with open(predInfoFileName, 'r') as f:
        for line in f:
            splittedLine         = line.split(" ")
            if len(splittedLine) != 3:
                printError( "Invalid prediction file. Expected content: relPathPrediction1 labelIDPrediction1 confidencePrediction1", user_fault=True )
            if os.path.isabs(splittedLine[0]):
                printError( "Invalid prediction file. First entry in each line must be a relative path.", user_fault=True )

            filename             = os.path.join( os.path.dirname(predInfoFileName),splittedLine[0] )
            filename             = os.path.abspath( filename )

            # check if that file is actually somewhere within the prediction root
            if os.path.commonprefix( [filename,abs_pred_path] ) != abs_pred_path:
                printError( "Predicted mask {} in prediction text file {} points outside of prediction path.".format(filename,predInfoFileName), user_fault=True )

            imageInfo            = {}
            imageInfo["labelID"] = int(float(splittedLine[1]))
            imageInfo["conf"]    = float(splittedLine[2])
            predInfo[filename]   = imageInfo

    return predInfo

# Routine to read ground truth image
def readGTImage(gtImageFileName):
    return Image.open(gtImageFileName)

# either read or compute a dictionary of all ground truth instances
def getGtInstances(groundTruthList):
    gtInstances = {}
    # if there is a global statistics json, then load it
    if (os.path.isfile(opt.gtInstancesFile)):
        print "Loading ground truth instances from JSON."
        with open(opt.gtInstancesFile) as json_file:
            gtInstances = json.load(json_file)
    # otherwise create it
    else:
        print "Creating ground truth instances from png files."
        gtInstances = instances2dict(groundTruthList, CLASS_LABELS, VALID_CLASS_IDS)
        writeDict2JSON(gtInstances, opt.gtInstancesFile)

    return gtInstances

# Filter instances, ignore labels without instances
def filterGtInstances(singleImageInstances):
    instanceDict = {}
    for labelName in singleImageInstances:
        if not labelName in CLASS_LABELS:
            continue
        instanceDict[labelName] = singleImageInstances[labelName]
    return instanceDict

# match ground truth instances with predicted instances
def matchGtWithPreds(predictionList,groundTruthList,gtInstances):
    matches = {}
    print "Matching {} pairs of images...".format(len(predictionList))

    count = 0
    for (pred,gt) in zip(predictionList,groundTruthList):
        # key for dicts
        dictKey = os.path.abspath(gt)

        # Read input files
        gtImage  = readGTImage(gt)
        predInfo = readPredInfo(pred)

        # Get and filter ground truth instances
        unfilteredInstances = gtInstances[ dictKey ]
        curGtInstancesOrig  = filterGtInstances(unfilteredInstances)

        # Try to assign all predictions
        (curGtInstances,curPredInstances) = assignGt2Preds(curGtInstancesOrig, gtImage, predInfo)

        # append to global dict
        matches[ dictKey ] = {}
        matches[ dictKey ]["groundTruth"] = curGtInstances
        matches[ dictKey ]["prediction"]  = curPredInstances

        count += 1
        sys.stdout.write("\rImages Processed: {}".format(count))
        sys.stdout.flush()
    print ""

    return matches

# For a given frame, assign all predicted instances to ground truth instances
def assignGt2Preds(gtInstancesOrig, gtImage, predInfo):
    # In this method, we create two lists
    #  - predInstances: contains all predictions and their associated gt
    #  - gtInstances:   contains all gt instances and their associated predictions
    predInstances    = {}
    predInstCount    = 0

    # Create a prediction array for each class
    for label in CLASS_LABELS:
        predInstances[label] = []

    # We already know about the gt instances
    # Add the matching information array
    gtInstances = deepcopy(gtInstancesOrig)
    for label in gtInstances:
        for gt in gtInstances[label]:
            gt["matchedPred"] = []

    # Make the gt a numpy array
    gtNp = np.array(gtImage)

    # Get a mask of void labels in the groundtruth
    voidLabelIDList = []
    for labelid in VALID_CLASS_IDS:
        voidLabelIDList.append(labelid)
    boolVoid = np.in1d(gtNp, voidLabelIDList).reshape(gtNp.shape)

    # Loop through all prediction masks
    for predImageFile in predInfo:
        # Additional prediction info
        labelID  = predInfo[predImageFile]["labelID"]
        predConf = predInfo[predImageFile]["conf"]

        # maybe we are not interested in that label
        if not int(labelID) in ID2LABEL:
            continue

        # label name
        labelName = ID2LABEL[int(labelID)]

        # Read the mask
        predImage = Image.open(predImageFile)
        predImage = predImage.convert("L")
        predNp    = np.array(predImage)

        # make the image really binary, i.e. everything non-zero is part of the prediction
        boolPredInst   = predNp != 0
        predPixelCount = np.count_nonzero( boolPredInst )

        # skip if actually empty
        if not predPixelCount:
            continue

        # The information we want to collect for this instance
        predInstance = {}
        predInstance["imgName"]          = predImageFile
        predInstance["predID"]           = predInstCount
        predInstance["labelID"]          = int(labelID)
        predInstance["pixelCount"]       = predPixelCount
        predInstance["confidence"]       = predConf
        # Determine the number of pixels overlapping void
        predInstance["voidIntersection"] = np.count_nonzero( np.logical_and(boolVoid, boolPredInst) )

        # A list of all overlapping ground truth instances
        matchedGt = []

        # Loop through all ground truth instances with matching label
        # This list contains all ground truth instances that distinguish groups
        # We do not know, if a certain instance is actually a single object or a group
        # e.g. car or cargroup
        # However, for now we treat both the same and do the rest later
        for (gtNum,gtInstance) in enumerate(gtInstancesOrig[labelName]):

            intersection = np.count_nonzero( np.logical_and( gtNp == gtInstance["instID"] , boolPredInst) )

            # If they intersect add them as matches to both dicts
            if (intersection > 0):
                gtCopy   = gtInstance.copy()
                predCopy = predInstance.copy()

                # let the two know their intersection
                gtCopy["intersection"]   = intersection
                predCopy["intersection"] = intersection

                # append ground truth to matches
                matchedGt.append(gtCopy)
                # append prediction to ground truth instance
                gtInstances[labelName][gtNum]["matchedPred"].append(predCopy)

        predInstance["matchedGt"] = matchedGt
        predInstCount += 1
        predInstances[labelName].append(predInstance)

    return (gtInstances,predInstances)


def evaluateMatches(matches):
    # In the end, we need two vectors for each class and for each overlap
    # The first vector (y_true) is binary and is 1, where the ground truth says true,
    # and is 0 otherwise.
    # The second vector (y_score) is float [0...1] and represents the confidence of
    # the prediction.
    #
    # We represent the following cases as:
    #                                       | y_true |   y_score
    #   gt instance with matched prediction |    1   | confidence
    #   gt instance w/o  matched prediction |    1   |     0.0
    #          false positive prediction    |    0   | confidence
    #
    # The current implementation makes only sense for an overlap threshold >= 0.5,
    # since only then, a single prediction can either be ignored or matched, but
    # never both. Further, it can never match to two gt instances.
    # For matching, we vary the overlap and do the following steps:
    #   1.) remove all predictions that satisfy the overlap criterion with an ignore region (either void or *group)
    #   2.) remove matches that do not satisfy the overlap
    #   3.) mark non-matched predictions as false positive

    # AP
    overlaps  = opt.overlaps
    # region size
    minRegionSizes = opt.minRegionSizes
    # distance thresholds
    distThs   = opt.distanceThs
    # distance confidences
    distConfs = opt.distanceConfs
    # only keep the first, if distances are not available
    #if not opt.distanceAvailable:
    minRegionSizes = [ minRegionSizes[0] ]
    distThs        = [ distThs       [0] ]
    distConfs      = [ distConfs     [0] ]

    # last three must be of same size
    if len(distThs) != len(minRegionSizes):
        printError("Number of distance thresholds and region sizes different")
    if len(distThs) != len(distConfs):
        printError("Number of distance thresholds and confidences different")

    # Here we hold the results
    # First dimension is class, second overlap
    ap = np.zeros( (len(distThs) , len(CLASS_LABELS) , len(overlaps)) , np.float )

    for dI,(minRegionSize,distanceTh,distanceConf) in enumerate(zip(minRegionSizes,distThs,distConfs)):
        for (oI,overlapTh) in enumerate(overlaps):
            for (lI,labelName) in enumerate(CLASS_LABELS):
                y_true   = np.empty( 0 )
                y_score  = np.empty( 0 )
                # count hard false negatives
                hardFns  = 0
                # found at least one gt and predicted instance?
                haveGt   = False
                havePred = False

                for img in matches:
                    predInstances = matches[img]["prediction" ][labelName]
                    gtInstances   = matches[img]["groundTruth"][labelName]
                    # filter groups in ground truth
                    gtInstances   = [ gt for gt in gtInstances if gt["instID"]>=1000 and gt["pixelCount"]>=minRegionSize and gt["medDist"]<=distanceTh and gt["distConf"]>=distanceConf ]

                    if gtInstances:
                        haveGt = True
                    if predInstances:
                        havePred = True

                    curTrue  = np.ones ( len(gtInstances) )
                    curScore = np.ones ( len(gtInstances) ) * (-float("inf"))
                    curMatch = np.zeros( len(gtInstances) , dtype=np.bool )

                    # collect matches
                    for (gtI,gt) in enumerate(gtInstances):
                        foundMatch = False
                        for pred in gt["matchedPred"]:
                            overlap = float(pred["intersection"]) / (gt["pixelCount"]+pred["pixelCount"]-pred["intersection"])
                            if overlap > overlapTh:
                                # the score
                                confidence = pred["confidence"]

                                # if we already hat a prediction for this groundtruth
                                # the prediction with the lower score is automatically a false positive
                                if curMatch[gtI]:
                                    maxScore = max( curScore[gtI] , confidence )
                                    minScore = min( curScore[gtI] , confidence )
                                    curScore[gtI] = maxScore
                                    # append false positive
                                    curTrue  = np.append(curTrue,0)
                                    curScore = np.append(curScore,minScore)
                                    curMatch = np.append(curMatch,True)
                                # otherwise set score
                                else:
                                    foundMatch = True
                                    curMatch[gtI] = True
                                    curScore[gtI] = confidence

                        if not foundMatch:
                            hardFns += 1

                    # remove non-matched ground truth instances
                    curTrue  = curTrue [ curMatch==True ]
                    curScore = curScore[ curMatch==True ]

                    # collect non-matched predictions as false positive
                    for pred in predInstances:
                        foundGt = False
                        for gt in pred["matchedGt"]:
                            overlap = float(gt["intersection"]) / (gt["pixelCount"]+pred["pixelCount"]-gt["intersection"])
                            if overlap > overlapTh:
                                foundGt = True
                                break
                        if not foundGt:
                            # collect number of void and *group pixels
                            nbIgnorePixels = pred["voidIntersection"]
                            for gt in pred["matchedGt"]:
                                # group?
                                if gt["instID"] < 1000:
                                    nbIgnorePixels += gt["intersection"]
                                # small ground truth instances
                                if gt["pixelCount"] < minRegionSize or gt["medDist"]>distanceTh or gt["distConf"]<distanceConf:
                                    nbIgnorePixels += gt["intersection"]
                            proportionIgnore = float(nbIgnorePixels)/pred["pixelCount"]
                            # if not ignored
                            # append false positive
                            if proportionIgnore <= overlapTh:
                                curTrue = np.append(curTrue,0)
                                confidence = pred["confidence"]
                                curScore = np.append(curScore,confidence)

                    # append to overall results
                    y_true  = np.append(y_true,curTrue)
                    y_score = np.append(y_score,curScore)

                # compute the average precision
                if haveGt and havePred:
                    # compute precision recall curve first

                    # sorting and cumsum
                    scoreArgSort      = np.argsort(y_score)
                    yScoreSorted      = y_score[scoreArgSort]
                    yTrueSorted       = y_true[scoreArgSort]
                    yTrueSortedCumsum = np.cumsum(yTrueSorted)

                    # unique thresholds
                    (thresholds,uniqueIndices) = np.unique( yScoreSorted , return_index=True )

                    # since we need to add an artificial point to the precision-recall curve
                    # increase its length by 1
                    nbPrecRecall = len(uniqueIndices) + 1

                    # prepare precision recall
                    nbExamples     = len(yScoreSorted)
                    nbTrueExamples = yTrueSortedCumsum[-1]
                    precision      = np.zeros(nbPrecRecall)
                    recall         = np.zeros(nbPrecRecall)

                    # deal with the first point
                    # only thing we need to do, is to append a zero to the cumsum at the end.
                    # an index of -1 uses that zero then
                    yTrueSortedCumsum = np.append( yTrueSortedCumsum , 0 )

                    # deal with remaining
                    for idxRes,idxScores in enumerate(uniqueIndices):
                        cumSum = yTrueSortedCumsum[idxScores-1]
                        tp = nbTrueExamples - cumSum
                        fp = nbExamples     - idxScores - tp
                        fn = cumSum + hardFns
                        p  = float(tp)/(tp+fp)
                        r  = float(tp)/(tp+fn)
                        precision[idxRes] = p
                        recall   [idxRes] = r

                    # first point in curve is artificial
                    precision[-1] = 1.
                    recall   [-1] = 0.

                    # compute average of precision-recall curve
                    # integration is performed via zero order, or equivalently step-wise integration
                    # first compute the widths of each step:
                    # use a convolution with appropriate kernel, manually deal with the boundaries first
                    recallForConv = np.copy(recall)
                    recallForConv = np.append( recallForConv[0] , recallForConv )
                    recallForConv = np.append( recallForConv    , 0.            )

                    stepWidths = np.convolve(recallForConv,[-0.5,0,0.5],'valid')

                    # integrate is now simply a dot product
                    apCurrent = np.dot( precision , stepWidths )

                elif haveGt:
                    apCurrent = 0.0
                else:
                    apCurrent = float('nan')
                ap[dI,lI,oI] = apCurrent

    return ap

def computeAverages(aps):
    # max distance index
    dInf  = np.argmax( opt.distanceThs )
    d50m  = np.where( np.isclose( opt.distanceThs ,  50. ) )
    d100m = np.where( np.isclose( opt.distanceThs , 100. ) )
    o50   = np.where(np.isclose(opt.overlaps,0.5  ))

    avgDict = {}
    avgDict["allAp"]       = np.nanmean(aps[ dInf,:,:  ])
    avgDict["allAp50%"]    = np.nanmean(aps[ dInf,:,o50])

    avgDict["classes"]  = {}
    for (lI,labelName) in enumerate(CLASS_LABELS):
        avgDict["classes"][labelName]             = {}
        avgDict["classes"][labelName]["ap"]       = np.average(aps[ dInf,lI,  :])
        avgDict["classes"][labelName]["ap50%"]    = np.average(aps[ dInf,lI,o50])

    return avgDict

def printResults(avgDict):
    sep     = ""  #(","         if opt.csv       else ")
    col1    = ":" #(":"         if not opt.csv   else ")
    lineLen = 50

    print ""
    #if not args.csv:
    #    print "#"*lineLen)
    print "#"*lineLen
    line  = ""
    line += "{:<15}".format("what"      ) + sep + col1
    line += "{:>15}".format("AP"        ) + sep
    line += "{:>15}".format("AP_50%"    ) + sep
    print line
    #if not args.csv:
    #    print "#"*lineLen)
    print "#"*lineLen

    for (lI,labelName) in enumerate(CLASS_LABELS):
        apAvg  = avgDict["classes"][labelName]["ap"]
        ap50o  = avgDict["classes"][labelName]["ap50%"]
        line  = "{:<15}".format(labelName) + sep + col1
        line += sep + "{:>15.3f}".format(apAvg ) + sep
        line += sep + "{:>15.3f}".format(ap50o ) + sep
        print line

    allApAvg  = avgDict["allAp"]
    allAp50o  = avgDict["allAp50%"]

    #if not args.csv:
    #    print "-"*lineLen)
    print "-"*lineLen
    line  = "{:<15}".format("average") + sep + col1 
    line += "{:>15.3f}".format(allApAvg)  + sep 
    line += "{:>15.3f}".format(allAp50o)  + sep
    print line
    print ""

def prepareJSONDataForResults(avgDict, aps):
    JSONData = {}
    JSONData["averages"] = avgDict
    JSONData["overlaps"] = opt.overlaps.tolist()
    JSONData["minRegionSizes"]      = opt.minRegionSizes.tolist()
    JSONData["distanceThresholds"]  = opt.distanceThs.tolist()
    JSONData["minStereoDensities"]  = opt.distanceConfs.tolist()
    JSONData["instLabels"] = CLASS_LABELS
    JSONData["resultApMatrix"] = aps.tolist()

    return JSONData

def writeResultToFile(result, output_file):
    _SPLITTER = ','
    with open(output_file, 'w') as f:
        f.write(_SPLITTER.join(['class', 'class id', 'ap', 'ap50']) + '\n')
        for i in range(len(VALID_CLASS_IDS)):
            class_name = CLASS_LABELS[i]
            class_id = VALID_CLASS_IDS[i]
            ap = result["averages"]["classes"][class_name]["ap"]
            ap50 = result["averages"]["classes"][class_name]["ap50%"]
            f.write(_SPLITTER.join([str(x) for x in [class_name, class_id, ap, ap50]]) + '\n')

# Work through image list
def evaluateImgLists(predictionList, groundTruthList):
    # get dictionary of all ground truth instances
    gtInstances = getGtInstances(groundTruthList)
    # match predictions and ground truth
    matches = matchGtWithPreds(predictionList,groundTruthList,gtInstances)
    #writeDict2JSON(matches,"matches.json)
    # evaluate matches
    apScores = evaluateMatches(matches)
    # averages
    avgDict = computeAverages(apScores)
    # result dict
    resDict = prepareJSONDataForResults(avgDict, apScores)
    #if args.JSONOutput:
    # create output folder if necessary
    path = os.path.dirname(opt.output_file)
    ensurePath(path)
    # Write APs to JSON
    #writeDict2JSON(resDict, opt.output_file)
    writeResultToFile(resDict, opt.output_file)

    printResults(avgDict)

    return resDict

# The main method
def main(argv):

    #pred_files = [os.path.join(opt.pred_path, file) for file in os.listdir(opt.pred_path) if not os.path.isdir(os.path.join(opt.pred_path, file)) and file.endswith('.txt')]
    pred_files = []
    # find all the text files recursively
    for root, dirs, files in os.walk(opt.pred_path):
        for file in files:
            if file.endswith(".txt"):
                 pred_files.append(os.path.join(root, file))
    gt_files = []
    if len(pred_files) == 0:
        printError("No result files found.", user_fault=True)
    for i in range(len(pred_files)):
        gt_file = os.path.join(opt.gt_path, os.path.splitext(os.path.basename(pred_files[i]))[0] + '.png')
        if not os.path.isfile(gt_file):
            printError("Result file {} does not match any gt file".format(pred_files[i]), user_fault=True)
        gt_files.append(gt_file)
    #print pred_files
    #print gt_files

    # print some info for user
    print "Note that this tool uses the file '{}' to cache the ground truth instances.".format(opt.gtInstancesFile)
    print "If anything goes wrong, or if you change the ground truth, please delete the file."

    # evaluate
    evaluateImgLists(pred_files, gt_files)

    return

# call the main method
if __name__ == "__main__":
    main(sys.argv[1:])
