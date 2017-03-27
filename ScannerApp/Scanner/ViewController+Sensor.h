/*
  This file is part of the Structure SDK.
  Copyright © 2015 Occipital, Inc. All rights reserved.
  http://structure.io
*/

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#define HAS_LIBCXX

#import "ViewController.h"
#import <Structure/Structure.h>

@interface ViewController (Sensor) <STSensorControllerDelegate>

- (STSensorControllerInitStatus)connectToStructureSensorAndStartStreaming;
- (void)setupStructureSensor;
- (void)startScanningAndOpen;
- (void)stopScanningAndWrite;
- (BOOL)isStructureConnectedAndCharged;

- (void)cleanUpFileWriting;

@end
