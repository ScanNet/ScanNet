#!/usr/bin/env python
#
# Compute aggregated annotation statistics

import argparse
import collections
import copy
import csv
import json
import sys
import os
import logging
import traceback

import util

# logger
FORMAT = '%(asctime)-15s [%(levelname)s] %(message)s'
logging.basicConfig(format=FORMAT)
log = logging.getLogger('compute_annotation_stats')
log.setLevel(logging.INFO)


SEGS_FILE = "${name}_vh_clean_2.0.010000.segs.json"
ANNS_FILE = "${name}.aggregation.json"


def computeStatistics(json, allLabels=None, allCategories=None):
    # 'annotatedVertices': 0,
    # 'unannotatedVertices': 0,
    # 'annotatedSegments': 0,
    # 'unannotatedSegments': 0,
    # 'totalSegments': 0,
    # 'totalVertices': 0,
    # 'percentComplete': 0
    # 'objects': 0
    # 'labels': 0
    vertToSeg = json.get('segIndices')  # Mapping of vertex to segment index
    segToVerts = {}
    for vert, seg in enumerate(vertToSeg):
        if seg in segToVerts:
            segToVerts[seg].append(vert)
        else:
            segToVerts[seg] = [vert]
    segGroups = json.get('segGroups')    # Array of segment groups
    stats = collections.Counter({
        'totalVertices': len(vertToSeg),
        'totalSegments': len(segToVerts),
        'annotatedVertices': 0,
        'annotatedSegments': 0,
        'segmentGroups': len(segGroups)
    })
    labels = collections.Counter()
    categories = collections.Counter()
    objectIds = collections.Counter()
    labeledObjectIds = collections.Counter()
    annSegs = collections.Counter()
    for segGroup in segGroups:
        segments = segGroup.get('segments')
        label = segGroup.get('label')
        if label != 'unknown' and label != '':
            annSegs.update(segments)
            labels.update({segGroup.get('label'): 1})
            labeledObjectIds.update({segGroup.get('objectId'): 1})
            lparts = label.split(':')
            category = lparts[0]
            categories.update({category: 1})
        objectIds.update({segGroup.get('objectId'): 1})
    if allLabels is not None:
        allLabels.update(labels)
    if allCategories is not None:
        allCategories.update(categories)
    stats.update({'annotatedSegments': len(annSegs)})
    for seg in list(annSegs):
        if seg in segToVerts:
            stats.update({'annotatedVertices': len(segToVerts[seg])})
    stats.update({
        'unannotatedVertices': len(vertToSeg) - stats.get('annotatedVertices'),
        'unannotatedSegments': len(segToVerts) - stats.get('annotatedSegments'),
        'objects': len(objectIds),
        'labeledObjects': len(labeledObjectIds),
        'labels': len(labels),
        'categories': len(categories),
        'percentObjectLabeled': 100*len(labeledObjectIds)/len(objectIds) if len(objectIds) > 0 else 0,
        'percentComplete': 100*stats.get('annotatedVertices')/stats.get('totalVertices') if stats.get('totalVertices') > 0 else 0
       })
    return stats


def loadAnnotations(segmentsFilename, annotationFilename):
    with open(segmentsFilename) as segmentsFile:
        segments = json.load(segmentsFile)
    segmentGroups = {'segGroups': []}
    if util.is_non_zero_file(annotationFilename):
        try:
            with open(annotationFilename) as annotationFile:
                segmentGroups = json.load(annotationFile)                
        except:
            log.error('Error loading annotation file ' + annotationFilename)
            traceback.print_exc()
    # merge the two
    merged = segments.copy()
    merged.update(segmentGroups)
    return merged


def loadAllAnnotations(annotationFilename):
    with open(annotationFilename) as annotationFile:
        annotations = json.load(annotationFile)
    annotationsByModel = {}
    for ann in annotations:
        modelId = ann.get('modelId')
        if modelId in annotationsByModel:
            annotationsByModel[modelId].append(ann)
        else:
            annotationsByModel[modelId] = [ann]
    return annotationsByModel


def convertAnnotations(annsByModel):
    m = {}
    copyFields = ['workerId', 'annId', 'objectId', 'label', 'id']
    for modelId,anns in annsByModel.iteritems():
        segGroups = []
        for ann in anns:
            sg = copy.copy(ann.get('segments'))
            for f in copyFields:
                sg[f] = ann.get(f)
            segGroups.append(sg)
        parts = modelId.split('.')
        m[parts[1]] = {
            'sceneId': modelId,
            'segGroups': segGroups
        }
    return m


def loadSegmentsAndCombineAnnotations(segmentsFilename, segmentGroups=None):
    with open(segmentsFilename) as segmentsFile:
        #print 'Reading ' + segmentsFilename
        segments = json.load(segmentsFile)
    if segmentGroups is None:
        segmentGroups = {'segGroups': []}
    # merge the two
    merged = segments.copy()
    merged.update(segmentGroups)
    return merged


def saveCsv(data, csvfile):
    # Index and output
    keys = data[0].keys()
    keys.sort()
    keys.remove('id')
    fieldnames = ['id']
    fieldnames.extend(keys)
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames, extrasaction='ignore')
    writer.writeheader()
    for v in data:
        writer.writerow(v)


def saveJson(data, file):
    json.dump(data, file, indent=1, separators=(',', ': '))


def saveOutput(format, data, file):
    if format == 'json':
        saveJson(data, file)
    else:
        saveCsv(data, file)


def saveCounts(data, file):
    writer = csv.writer(file)
    writer.writerow(['label','count'])
    for label,count in data.most_common():
        writer.writerow([label,str(count)])


# Process directory with annotations
def processDir(args):
    dirname = args.get('input')
    allLabels = collections.Counter()
    allCategories = collections.Counter()
    allStats = []
    for root, dirs, files in os.walk(dirname):
        for name in dirs:
            dir = os.path.join(root, name)
            segsFile = os.path.join(dir, SEGS_FILE.replace("${name}", name))
            annsFile = os.path.join(dir, ANNS_FILE.replace("${name}", name))
            # log.info('segsFile=%s,annsFile=%s', segsFile, annsFile)
            if util.is_non_zero_file(segsFile):
                anns = loadAnnotations(segsFile, annsFile)
                stats = computeStatistics(anns, allLabels, allCategories)
                entry = {'id': name}
                entry.update(stats)
                allStats.append(entry)
                #print json.dumps(entry)
    labelsFile = args.get('labels')
    if labelsFile is not None:
        with open(labelsFile, 'wb') as labelsOut:
            saveCounts(allLabels, labelsOut)
    categoriesFile = args.get('categories')
    if categoriesFile is not None:
        with open(categoriesFile, 'wb') as categoriesOut:
            saveCounts(allCategories, categoriesOut)
    if len(allStats) > 0:
        if args.get('output'):
            with open(args.get('output'), 'wb') as outfile:
                saveOutput(args.get('format'), allStats, outfile)
        else:
            saveOutput(args.get('format'), allStats, sys.stdout)
            sys.stdout.flush()


def processFile(args):
    # File of annotations
    annsFilename = args.get('annotations')
    # Directory of segmentations
    dirname = args.get('input')

    rawAnnsByModel = loadAllAnnotations(annsFilename)
    annsByModel = convertAnnotations(rawAnnsByModel)
    allStats = []
    # Compute percentage of annotations
    for root, dirs, files in os.walk(dirname):
        for name in dirs:
            dir = os.path.join(root, name)
            segsFile = os.path.join(dir, SEGS_FILE.replace("${name}", name))
            if util.is_non_zero_file(segsFile):
                try:
                    anns = loadSegmentsAndCombineAnnotations(segsFile, annsByModel.get(name))
                    stats = computeStatistics(anns)
                    entry = {'id': name}
                    entry.update(stats)
                    allStats.append(entry)
                    # print json.dumps(entry)
                except:
                    log.error('Invalid segmentation file for ' + name)
                    traceback.print_exc()
            else:
                log.warn('No segmentation file for ' + name)
    if len(allStats) > 0:
        if args.get('output'):
            with open(args.get('output'), 'wb') as outfile:
                saveOutput(args.get('format'), allStats, outfile)
        else:
            saveOutput(args.get('format'), allStats, sys.stdout)
            sys.stdout.flush()


def main():
    parser = argparse.ArgumentParser(description='Compute annotation statistics!')
    parser.add_argument('input', help='Directory with annotation json')
    parser.add_argument('-a', '--annotations', dest='annotations',
                        help='Json file with all annotations', action='store')
    parser.add_argument('--labels', dest='labels', help='Output labels', action='store')
    parser.add_argument('--categories', dest='categories', help='Output categories', action='store')
    parser.add_argument('-f', '--format', dest='format', default='csv',
                        help='Output format', choices=['json', 'csv'], action='store')
    parser.add_argument('output', nargs='?')
    args = parser.parse_args()
    if args.annotations is not None:
        processFile(vars(args))
    else:
        processDir(vars(args))


if __name__ == "__main__":
    main()
