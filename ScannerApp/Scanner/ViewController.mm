/*
  This file is part of the Structure SDK.
  Copyright © 2015 Occipital, Inc. All rights reserved.
  http://structure.io
*/
#import <CommonCrypto/CommonDigest.h>

#include <sstream>
#import "ViewController.h"
#import "ViewController+Camera.h"
#import "ViewController+Sensor.h"
#import "ViewController+OpenGL.h"
#import "PersistentStore.h"
#import "Reachability.h"
#import "EAGLView.h"
#import "Utils/DeviceUID.h"

#include <cmath>

// Needed to determine platform string
#include <sys/types.h>
#include <sys/sysctl.h>
#include <tgmath.h>


NSUInteger g_uploadedCount = 0;
NSUInteger g_fileCount = 0;
NSInteger g_numCurrentUploads = 0;
unsigned long long g_uploadedByteCount = 0;
unsigned long long g_totalUploadBytes = 0;

#pragma mark - Utilities

namespace // anonymous namespace for local functions.
{
    BOOL isIpadAir2()
    {
        const char* kernelStringName = "hw.machine";
        NSString* deviceModel;
        {
            size_t size;
            sysctlbyname(kernelStringName, NULL, &size, NULL, 0); // Get the size first
            
            char *stringNullTerminated = (char*)malloc(size);
            sysctlbyname(kernelStringName, stringNullTerminated, &size, NULL, 0); // Now, get the string itself
            
            deviceModel = [NSString stringWithUTF8String:stringNullTerminated];
            free(stringNullTerminated);
        }
        
        if ([deviceModel isEqualToString:@"iPad5,3"]) return YES; // Wi-Fi
        if ([deviceModel isEqualToString:@"iPad5,4"]) return YES; // Wi-Fi + LTE
        return NO;
    }
    
    BOOL getDefaultHighResolutionSettingForCurrentDevice()
    {
        // iPad Air 2 can handle 30 FPS high-resolution, so enable it by default.
        if (isIpadAir2())
            return TRUE;
        
        // Older devices can only handle 15 FPS high-resolution, so keep it disabled by default
        // to avoid showing a low framerate.
        return FALSE;
    }
} // anonymous

#pragma mark - ViewController Setup

@implementation ViewController

- (void)dealloc
{
    [self.avCaptureSession stopRunning];
    
    if ([EAGLContext currentContext] == _display.context)
    {
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [self setupGL];
    
    [self setupUserInterface];
    
    [self setupIMU];
    
    [self setupCameraFile];
    
    [self setupStructureSensor];
    
    // Later, we’ll set this true if we have a device-specific calibration
    _useColorCamera = [STSensorController approximateCalibrationGuaranteedForDevice];
    
    _renderDepthOverlay = true;
    
    // Make sure we get notified when the app becomes active to start/restore the sensor state if necessary.
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(appDidBecomeActive)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
    
    self.scanButton.hidden = YES;
    self.doneButton.hidden = YES;
    self.resetButton.hidden = YES;
    [[self progressView] setHidden:YES];
    [[self uploadErrorIndicator] setHidden:YES];
    [[self scanButton] setHidden:YES];
    [[self tableView] setHidden:YES];
    self.tooCloseLabel.hidden = YES;
    self.losingTrackingLabel.hidden = YES;

    self.uploadErrorDescription.hidden = YES;

    blinkCounter = 0;
    
    _uploadQueue = [[NSOperationQueue alloc] init];
    _verifyQueue = [[NSOperationQueue alloc] init];
    
    [_uploadQueue setMaxConcurrentOperationCount:1];
    [_verifyQueue setMaxConcurrentOperationCount:3];
    
    [self loadOptions];
    
    /*
     NSString* documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask,YES) objectAtIndex:0];
    //get files
    NSArray* directoryContent = [self getSortedFilesFromFolder:documentPath]; //[[NSFileManager defaultManager] contentsOfDirectoryAtPath:documentPath error:NULL];
    myScanData = [[NSMutableArray alloc]initWithArray:directoryContent];
     */
    myScanData = [[NSMutableArray alloc] init];
    
    sceneTypes = [[NSMutableArray alloc] initWithObjects:
                  @"Please Select A Scene Type",
                  @"Dining Room",
                  @"Office",
                  @"Classroom",
                  @"Bedroom / Hotel",
                  @"Living room / Lounge",
                  @"Kitchen",
                  @"Bookstore / Library",
                  @"Bathroom",
                  @"Conference Room",
                  @"Misc.", nil];
    
    corners = [[NSMutableArray alloc] init];
    
    NSLog(@"View did load");
    
    // Set up tableView
    self.tableView.allowsMultipleSelectionDuringEditing = NO;
    
    // Set default color resolution
    [[self colorResControl] setSelectedSegmentIndex:1];
    [self updateColorRes:self.colorResControl.selectedSegmentIndex];
    
    [[self numCornersProgressView] setHidden:YES];
    self.numCornersProgressView.transform = CGAffineTransformMakeScale(3.0f, 25.0f);
    
    numPrevFramesTracked = sizeof(numPreviousCorners) / sizeof(int);
    for(int i = 0; i < numPrevFramesTracked; i++) {
        numPreviousCorners[i] = 0;
    }
    
    self.changeExposure.titleLabel.textAlignment = NSTextAlignmentCenter;
    self.exposureTable.hidden = YES;

    

    exposureLevels = [[NSArray alloc] initWithObjects:@"auto (low light)", @"2", @"5", @"10", @"15", @"20", nil];
    exposureValue = [exposureLevels objectAtIndex:0];
    self.AWBLabel.hidden = NO;
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    // The framebuffer will only be really ready with its final size after the view appears.
    [(EAGLView *)self.view setFramebuffer];
    
    [self setupGLViewport];

    [self updateAppStatusMessage];
    
    // We will connect to the sensor when we receive appDidBecomeActive.
}

- (void)appDidBecomeActive
{    
    if ([self currentStateNeedsSensor])
    {
        [self connectToStructureSensorAndStartStreaming];
    }
    
    // Abort the current scan if we were still scanning before going into background since we
    // are not likely to recover well.
    if (_slamState.scannerState == ScannerStateScanning)
    {
        [self resetButtonPressed:self];
    }
    
    [self changeExposureValue];
    //NSLog(@"Resolution: ");
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    
    [self respondToMemoryWarning];
}

- (void)setupUserInterface
{
    // Make sure the status bar is hidden.
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationSlide];
    
    // Fully transparent message label, initially.
    self.appStatusMessageLabel.alpha = 0;
    
    // Make sure the label is on top of everything else.
    self.appStatusMessageLabel.layer.zPosition = 100;
    
    //make progress bars thicker
    CGAffineTransform transform = CGAffineTransformMakeScale(1.0f, 10.0f);
    self.progressView.transform = transform;
    self.memProgressView.transform = transform;
    
    [self changeExposureValue];
}

// Make sure the status bar is disabled (iOS 7+)
- (BOOL)prefersStatusBarHidden
{
    return YES;
}

-(double)getPercentUsedDiskspace {
    uint64_t totalSpace = 0;
    uint64_t totalFreeSpace = 0;
    NSError *error = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSDictionary *dictionary = [[NSFileManager defaultManager] attributesOfFileSystemForPath:[paths lastObject] error: &error];
    
    if (dictionary) {
        NSNumber *fileSystemSizeInBytes = [dictionary objectForKey: NSFileSystemSize];
        NSNumber *freeFileSystemSizeInBytes = [dictionary objectForKey:NSFileSystemFreeSize];
        totalSpace = [fileSystemSizeInBytes unsignedLongLongValue];
        totalFreeSpace = [freeFileSystemSizeInBytes unsignedLongLongValue];
        //NSLog(@"Memory Capacity of %llu MiB with %llu MiB Free memory available.", ((totalSpace/1024ll)/1024ll), ((totalFreeSpace/1024ll)/1024ll)); // TMI for log
    } else {
        NSLog(@"Error Obtaining System Memory Info: Domain = %@, Code = %ld", [error domain], (long)[error code]);
    }
    
    //return totalFreeSpace;
    double precentUsed = 1 - (double)totalFreeSpace/(double)totalSpace;
    //NSLog(@"percent used memory = %f", precentUsed); // TMI for log
    return precentUsed;
}

- (BOOL)isConnectedToWIFI
{
    Reachability* reachability = [Reachability reachabilityWithHostName:@"171.67.76.38"];//[Reachability reachabilityForInternetConnection];
    [reachability startNotifier];
    NetworkStatus status = [reachability currentReachabilityStatus];
    if (status == ReachableViaWiFi) {
        //NSLog(@"Reachable via wifi (enable upload)"); // TMI for log
        return true;
    }
    //NSLog(@"Not reachable via wifi (disable upload)"); // TMI for log
    return false;
}

- (void)enterSceneLabellingState
{
    NSLog(@"[enterSceneLabellingState]");
    [self loadSceneLabel];
    
    if ([self isConnectedToWIFI]) self.uploadButton.hidden = NO; //allow uploads from here
    else self.uploadButton.hidden = YES;
    
    // disable scanning
    if (isNotEmpty(self.sceneLabelField.text)) //&& !_options.specifiedSceneType.empty())
        self.scanButton.hidden = NO;
    else                                       self.scanButton.hidden = YES;
    self.doneButton.hidden = YES;
    self.resetButton.hidden = YES;
    self.toggleDepthButton.hidden = NO;
    self.viewScans.hidden = NO;
    
    // Cannot be lost //TODO remove tracking lost stuff
    _trackingLostLabel.hidden = YES;
    
    self.sceneLabelFieldView.hidden = NO;
    self.memProgressFieldView.hidden = NO;
    
    //update memory
    float percentUsedMem = (float)[self getPercentUsedDiskspace];
    [[self memProgressView] setProgress:percentUsedMem];
    
    _slamState.scannerState = ScannerStateSceneLabelling;
    
    [self updateIdleTimer];
}

- (void)enterCubePlacementState
{
    NSLog(@"[enterCubePlacementState]");
    
    if ([self isConnectedToWIFI]) self.uploadButton.hidden = NO; //allow uploads from here
    else self.uploadButton.hidden = YES;
    
    // Switch to the Scan button.
    self.scanButton.hidden = NO;
    self.doneButton.hidden = YES;
    self.resetButton.hidden = YES;
    self.toggleDepthButton.hidden = NO;
    // Cannot be lost in cube placement mode.
    _trackingLostLabel.hidden = YES;
    
    // can still edit this here
    self.sceneLabelFieldView.hidden = NO;
    self.memProgressFieldView.hidden = NO;
    
    [self setColorCameraParametersForInit];
    
    _slamState.scannerState = ScannerStateCubePlacement;
    
    [self updateIdleTimer];
}

- (void)enterScanningState
{
    PersistentStore::set("sceneLabel", _options.sceneLabel.c_str());
    PersistentStore::set("userName", _options.userName.c_str());
    
    self.sceneLabelFieldView.hidden = YES;
    self.memProgressFieldView.hidden = YES;
    
    [self startTrackingIMU];
    [self startScanningAndOpen];
    self.uploadButton.hidden = YES;
    
    self.optionsView.hidden = YES;
    self.viewScans.hidden = YES;
    
    // Switch to the Done button.
    self.scanButton.hidden = YES;
    self.doneButton.hidden = NO;
    self.resetButton.hidden = YES;//NO;
    self.uploadErrorIndicator.hidden = YES;
    self.toggleDepthButton.hidden = NO;
    
    self.AWBLabel.hidden = YES;
    self.AWBSwitch.hidden = YES;
    self.changeExposure.hidden = YES;

#ifdef DEBUG_MODE
    self.uploadErrorDescription.hidden = YES;
#endif
    
    // Tell the mapper if we have a support plane so that it can optimize for it.
    //[_slamState.mapper setHasSupportPlane:_slamState.cameraPoseInitializer.hasSupportPlane];
    
    //_slamState.tracker.initialCameraPose = _slamState.cameraPoseInitializer.cameraPose;
    
    _slamState.scannerState = ScannerStateScanning;
    
    // We will lock exposure during scanning to ensure better coloring.
    //[self setColorCameraParametersForScanning];
}

- (void)enterViewingState
{
    //view locally stored scans
    
    // Cannot be lost in view mode.
    [self hideTrackingErrorMessage];
    
    self.sceneLabelFieldView.hidden = YES;
    self.memProgressFieldView.hidden = YES;
    
    // Hide the Scan/Done/Reset button.
    self.scanButton.hidden = YES;
    self.doneButton.hidden = YES;
    self.resetButton.hidden = YES;
    self.uploadErrorIndicator.hidden = YES;
    self.toggleDepthButton.hidden = YES;
#ifdef DEBUG_MODE
    self.uploadErrorDescription.hidden = YES;
#endif
    //TODO CHECK THIS PART HERE
    [_sensorController stopStreaming];
    
    if (_useColorCamera)
        [self stopColorCamera];
    
   //TODO IMPLEMENT VIEW PART HERE
    
    _slamState.scannerState = ScannerStateViewing;
    
    [self updateIdleTimer];
}

namespace { // anonymous namespace for utility function.
    
    float keepInRange(float value, float minValue, float maxValue)
    {
        if (isnan (value))
            return minValue;
        
        if (value > maxValue)
            return maxValue;
        
        if (value < minValue)
            return minValue;
        
        return value;
    }
    
}
/*
- (void)adjustVolumeSize:(GLKVector3)volumeSize
{
    // Make sure the volume size remains between 10 centimeters and 10 meters.
    volumeSize.x = keepInRange (volumeSize.x, 0.1, 10.f);
    volumeSize.y = keepInRange (volumeSize.y, 0.1, 10.f);
    volumeSize.z = keepInRange (volumeSize.z, 0.1, 10.f);
    
    _slamState.mapper.volumeSizeInMeters = volumeSize;
    
    _slamState.cameraPoseInitializer.volumeSizeInMeters = volumeSize;
    [_display.cubeRenderer adjustCubeSize:_slamState.mapper.volumeSizeInMeters
                         volumeResolution:_slamState.mapper.volumeResolution];
}
*/
#pragma mark -  Structure Sensor Management

-(BOOL)currentStateNeedsSensor
{
    switch (_slamState.scannerState)
    {
        // Initialization and scanning need the sensor.
        case ScannerStateSceneLabelling:
        case ScannerStateCubePlacement:
        case ScannerStateScanning:
            return TRUE;
            
        // Other states don't need the sensor.
        default:
            return FALSE;
    }
}

#pragma mark - IMU

- (void)setupIMU
{
    g_imuLock = [NSLock new];
    // 60 FPS is responsive enough for motion events.
    // Real fps is ~53 (for device motion events
    const float fps = 60.0;
    _motionManager = [[CMMotionManager alloc] init];
    _motionManager.deviceMotionUpdateInterval = 1.0/fps;
    
    // Limiting the concurrent ops to 1 is a simple way to force serial execution
    _imuQueue = [[NSOperationQueue alloc] init];
    [_imuQueue setMaxConcurrentOperationCount:1];
    
}

- (void)setupCameraFile
{
    // Set up .camera File
    g_cameraLock = [NSLock new];
}

- (void)startTrackingIMU
{
    __weak ViewController *weakSelf = self;
    CMDeviceMotionHandler dmHandler = ^(CMDeviceMotion *motion, NSError *error)
    {
        // Could be nil if the self is released before the callback happens.
        if (weakSelf) {
            [weakSelf processDeviceMotion:motion withError:error];
        }
    };
    
    //double currentTime = [[NSDate date] timeIntervalSince1970];
    //fwrite((uint8_t *) &currentTime, 8, 1, g_imuFile);
    g_numIMUmeasurements = 0;

    
    [_motionManager startDeviceMotionUpdatesToQueue:_imuQueue withHandler:dmHandler];
}

- (void)stopTrackingIMU
{
    [_motionManager stopDeviceMotionUpdates];
}

- (void)processDeviceMotion:(CMDeviceMotion *)motion withError:(NSError *)error
{
    if (error != nil)
    {
        NSLog(@"processDeviceMotion error: %@", error);
        return;
    }

    
    //----- acceleration
    double accX = motion.userAcceleration.x;
    double accY = motion.userAcceleration.y;
    double accZ = motion.userAcceleration.z;
    
    //----- gravity
    double gravX = motion.gravity.x;
    double gravY = motion.gravity.y;
    double gravZ = motion.gravity.z;
    
    if ((accX == 0 && accY == 0 && accZ == 0) || (gravX == 0 && gravY == 0 && gravZ == 0)) return; // no valid acc/grav data
    
    //NSLog(@"IMU");
    [g_imuLock lock];
    double currentTime = [[NSProcessInfo processInfo] systemUptime];//[[NSDate date] timeIntervalSince1970];
    fwrite((uint8_t *) &currentTime, 8, 1, g_imuFile);
    
    // Non-portable, but should work in most cases.
    // Assumes host's representation of floating points (IEE 754) and host's endian-ness (little-endian)

    //----- rotation rate
    double rotX = motion.rotationRate.x;
    double rotY = motion.rotationRate.y;
    double rotZ = motion.rotationRate.z;

    fwrite((uint8_t *) &rotX, 8, 1, g_imuFile);
    fwrite((uint8_t *) &rotY, 8, 1, g_imuFile);
    fwrite((uint8_t *) &rotZ, 8, 1, g_imuFile);
    
    //----- acceleration
    fwrite((uint8_t *) &accX, 8, 1, g_imuFile);
    fwrite((uint8_t *) &accY, 8, 1, g_imuFile);
    fwrite((uint8_t *) &accZ, 8, 1, g_imuFile);
    
    //----- magnetometer
    double magX = motion.magneticField.field.x;
    double magY = motion.magneticField.field.y;
    double magZ = motion.magneticField.field.z;
    
    fwrite((uint8_t *) &magX, 8, 1, g_imuFile);
    fwrite((uint8_t *) &magY, 8, 1, g_imuFile);
    fwrite((uint8_t *) &magZ, 8, 1, g_imuFile);
    
    //----- attitude
    CMAttitude *attitude = motion.attitude;
    double roll = attitude.roll;
    double pitch = attitude.pitch;
    double yaw = attitude.yaw;
    
    fwrite((uint8_t *) &roll, 8, 1, g_imuFile);
    fwrite((uint8_t *) &pitch, 8, 1, g_imuFile);
    fwrite((uint8_t *) &yaw, 8, 1, g_imuFile);
    
    //----- gravity
    fwrite((uint8_t *) &gravX, 8, 1, g_imuFile);
    fwrite((uint8_t *) &gravY, 8, 1, g_imuFile);
    fwrite((uint8_t *) &gravZ, 8, 1, g_imuFile);
    
    fflush(g_imuFile);
    g_numIMUmeasurements++;
    [g_imuLock unlock];
}

/*
- (void)processDeviceMotion:(CMDeviceMotion *)motion withError:(NSError *)error
{
    if (_slamState.scannerState == ScannerStateCubePlacement)
    {
        // Update our gravity vector, it will be used by the cube placement initializer.
        _lastGravity = GLKVector3Make (motion.gravity.x, motion.gravity.y, motion.gravity.z);
    }
    
    if (_slamState.scannerState == ScannerStateCubePlacement || _slamState.scannerState == ScannerStateScanning)
    {
        // The tracker is more robust to fast moves if we feed it with motion data.
        [_slamState.tracker updateCameraPoseWithMotion:motion];
    }
}
*/
#pragma mark - UI Callbacks

- (IBAction)indexChanged:(UISegmentedControl *)sender {
    [self updateColorRes:self.colorResControl.selectedSegmentIndex];
    [self changeExposureValue];
}

- (IBAction)scanButtonPressed:(id)sender
{
    if (_options.userName.empty() || _options.sceneLabel.empty() || _options.specifiedSceneType.empty())
    {
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Incomplete metadata"
                                                        message:@"You must fill out a user id, scene name, and select scene type before scanning."
                                                       delegate:nil
                                              cancelButtonTitle:@"OK"
                                              otherButtonTitles:nil];
        [alert show];
    }
    else
    {
        [self changeExposureValue];
        self.exposureTable.hidden = YES;
        self.numCornersProgressView.hidden = NO;
        [self enterScanningState];
    }
}

- (IBAction)resetButtonPressed:(id)sender
{
    _slamState.prevFrameTimeStamp = -1.0;
    [self enterSceneLabellingState];
}

- (IBAction)doneButtonPressed:(id)sender
{
    self.numCornersProgressView.hidden = YES;

    [self cleanUp];
    [self enterSceneLabellingState];
    [[self tooCloseLabel] setHidden:YES];
    self.losingTrackingLabel.hidden = YES;
    self.AWBSwitch.hidden = NO;
    self.AWBLabel.hidden = NO;
    self.changeExposure.hidden = NO;
}

- (IBAction)viewScansPressed:(id)sender {
#ifdef DEBUG_MODE
    self.uploadErrorDescription.hidden = YES;
#endif
    if(self.tableView.hidden)
    {
        [myScanData removeAllObjects];
        
        NSString* documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        //get files
        NSArray* allScanFiles = [self getSortedFilesFromFolder:documentPath]; //[[NSMutableArray alloc]initWithArray:directoryContent];
        
        NSMutableArray* scanNamesShort = [[NSMutableArray alloc] init];
        
        //NSLog(@"Num Files: %d", [myScanData count]);
        /*
        if([allScanFiles count] == 0) // uploaded
        {
            [myScanData removeAllObjects];
        }*/
        
        for(int i = 0; i < [myScanData count]; i++)
        {
            NSArray *fileNameDivided = [[myScanData objectAtIndex:i] componentsSeparatedByString:@"\n\t"]; // JCJCJC
            NSString* shortName = [fileNameDivided objectAtIndex:0];
            [scanNamesShort addObject:shortName];
        }
        
        g_fileCount = [allScanFiles count];
        
        for (unsigned count = 0; count < g_fileCount; count++) {
            NSString *fileName = [[allScanFiles objectAtIndex:count] objectForKey:@"path"];
            NSArray *fileNameDivided = [fileName componentsSeparatedByString:@"."];
            NSString *scanName = fileNameDivided.firstObject;
            NSString *shortScanName = scanName;

            NSUInteger scanIndex = [scanNamesShort indexOfObject:shortScanName];
            
            if(scanIndex == NSNotFound) // New Scan
            {
                scanName = [scanName stringByAppendingString:@"\n\t"]; // JCJCJC
                scanName = [scanName stringByAppendingString:[fileNameDivided objectAtIndex:1]];
                scanName = [scanName stringByAppendingString:@" "];
                [myScanData addObject:scanName];
                [scanNamesShort addObject:shortScanName];
                //NSLog(@"%d", [scanNamesShort count]);
            }
            else
            {
                scanName = [myScanData objectAtIndex:scanIndex];
                
                NSMutableArray* scanNameSplit = [[NSMutableArray alloc] initWithArray:[scanName componentsSeparatedByString:@"\n\t"]]; // JCJCJC
                NSString* scanExtensions = [scanNameSplit objectAtIndex:1];

                if(![scanExtensions containsString:[fileNameDivided objectAtIndex:1]])
                {
                    scanExtensions = [scanExtensions stringByAppendingString:[fileNameDivided objectAtIndex:1]];
                    scanExtensions = [scanExtensions stringByAppendingString:@" "];
                    [scanNameSplit replaceObjectAtIndex:1 withObject:scanExtensions];
                    
                    
                    scanName = [scanNameSplit componentsJoinedByString:@"\n\t"];//JCJCJC
                    [myScanData replaceObjectAtIndex:scanIndex withObject:scanName];
                }
            }
            
            if([[fileNameDivided objectAtIndex:1] isEqualToString: @"txt"])
            {
                NSMutableArray* scanNameSplit = [[NSMutableArray alloc] initWithArray:[scanName componentsSeparatedByString:@"\n\t"]]; // JCJCJC
                
                if([scanNameSplit count] <= 2)
                {
                    NSString* filePath = [documentPath stringByAppendingPathComponent:fileName];
                    NSError *error = nil;
                    NSString* fileContents =[[NSString alloc] initWithContentsOfFile:filePath encoding: NSASCIIStringEncoding error: &error];
                    
                    NSArray *fileContentsSplit = [fileContents componentsSeparatedByString:@"\n"];
                    for(unsigned count = 0; count < [fileContentsSplit count]; count++)
                    {
                        NSString *lineOfFile = [fileContentsSplit objectAtIndex:count];
                        NSArray *lineSplit   = [lineOfFile componentsSeparatedByString:@" = "];
                        
                        if( [[lineSplit objectAtIndex:0] isEqualToString:@"sceneLabel"] ||
                            [[lineSplit objectAtIndex:0] isEqualToString:@"numDepthFrames"] ||
                            [[lineSplit objectAtIndex:0] isEqualToString:@"numColorFrames"] )
                        {
                            const static int maxLength = 35;
                            if(lineOfFile.length > maxLength) {
                                NSString* lastChar = [lineOfFile substringFromIndex:[lineOfFile length] - 1];
                                lineOfFile = [lineOfFile substringToIndex:maxLength - 4];
                                lineOfFile = [lineOfFile stringByAppendingString:@"..."];
                                lineOfFile = [lineOfFile stringByAppendingString:lastChar];
                            }
                            //NSLog(lineOfFile);
                             
                            [scanNameSplit addObject:lineOfFile];
                        }
                    }
                    
                    scanName = [scanNameSplit componentsJoinedByString:@"\n\t"]; //JCJCJC
                    [myScanData replaceObjectAtIndex:scanIndex withObject:scanName];
                }
            }
        }
        
        for(int i = [myScanData count] - 1; i >= 0; i--) // Remove scans with only jpg files
        {
            NSString* longName = [myScanData objectAtIndex:i];
            NSArray* splitScanName = [longName componentsSeparatedByString:@"\n\t"];
            NSString* extensions = [splitScanName objectAtIndex:1];
            
            if( [extensions containsString:@"jpg"] &&
               ![extensions containsString:@"depth"] &&
               ![extensions containsString:@"imu"] &&
               ![extensions containsString:@"h264"] &&
               ![extensions containsString:@"txt"] &&
               ![extensions containsString:@"camera"]) // Contains only jpg file
            {
                [myScanData removeObjectAtIndex:i];
                NSString* fileName = [documentPath stringByAppendingPathComponent:[splitScanName objectAtIndex:0]];
                fileName = [fileName stringByAppendingString:@".jpg"];
                
                [[NSFileManager defaultManager] removeItemAtPath:fileName error:nil];
                //NSLog(@"Removed file: %@", fileName);
            }
        }
    }
    
    //myScanData = [[NSMutableArray alloc]initWithArray:directoryContent];
    [[self tableView] setHidden:(!self.tableView.hidden)];
    [self.tableView reloadData];

}

-(IBAction)changeExposurePressed:(id)sender
{
    self.exposureTable.hidden = !self.exposureTable.hidden;
}
///* JCJCJC
//#pragma mark - Table View Data source
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if(tableView == self.exposureTable)
        return [exposureLevels count];
    else
        return [myScanData count];
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    if(tableView == self.exposureTable)
        return @"Exposure Duration (ms):";
    else {
        NSString *headerText = @"Previous Scans(";
        NSString *numPreviousScans = [NSString stringWithFormat: @"%ld", (long)[myScanData count]];
        headerText = [headerText stringByAppendingString:numPreviousScans];
        headerText = [headerText stringByAppendingString:@")"];
        return headerText;
    }
}

- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(nonnull NSIndexPath *)indexPath
{
    return YES;
}

#ifdef DEBUG_MODE
-(NSArray *)tableView:(UITableView *)tableView editActionsForRowAtIndexPath:(nonnull NSIndexPath *)indexPath
{
    if(tableView == self.exposureTable) {
        return @[];
    }
    else {
        UITableViewRowAction *uploadAction = [UITableViewRowAction rowActionWithStyle:UITableViewRowActionStyleDefault title:@"Upload" handler:^(UITableViewRowAction *action, NSIndexPath * indexPath) {
            
            [tableView setHidden:YES];
            
            [tableView deselectRowAtIndexPath:indexPath animated:YES];
            UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
            
            NSString* cellText = cell.textLabel.text;
            NSArray* textInRows = [cellText componentsSeparatedByString:@"\n\t"];
            
            NSString* scanName = [textInRows objectAtIndex:0];
            NSString* extensionsString = [textInRows objectAtIndex:1];
            NSMutableArray* extensions = [[extensionsString componentsSeparatedByString:@" "] mutableCopy];
            
            NSURL *uploadPath = [NSURL URLWithString:@SERVER UPLOAD];
            
            
            NSString* documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
            //NSLog(@"document path: %@", documentPath);
            //get files
            NSArray* directoryContent = [self getSortedFilesFromFolder:documentPath]; //[[NSFileManager defaultManager] contentsOfDirectoryAtPath:documentPath error:NULL];
            
            unsigned long totalFileCount = 0;
            g_uploadedCount = 0;
            g_totalUploadBytes = 0;
            g_uploadedByteCount = 0;
            
            
            
            //remove jpg files from consideration
            NSError *error = nil;
            NSArray* filesArray = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:documentPath error:&error];
            NSPredicate *predicate = [NSPredicate predicateWithFormat:@"SELF EndsWith '.jpg'"];
            filesArray = [filesArray filteredArrayUsingPredicate:predicate];
            
            NSPredicate* fileWithScanNamePred = [NSPredicate predicateWithFormat:@"SELF CONTAINS %s", scanName];
            filesArray = [filesArray filteredArrayUsingPredicate:predicate];

            
            totalFileCount = [directoryContent count];

            //NSLog(@"Scan Name: %@", scanName);
            
            //NSPredicate *predicate2 = [NSPredicate predicateWithFormat:@"SELF BeginsWith %@", scanName];
            //filesArray = [filesArray filteredArrayUsingPredicate:predicate2];
            
            /*
            for(int i = 0; i < [filesArray count]; i++) {
                NSLog(@"%@", [filesArray objectAtIndex:i]);
            }*/

            unsigned long jpgCount = [filesArray count];
            if (jpgCount > g_fileCount) {
                NSLog(@"ERROR: #jpg files = %lu > #files = %lu!", jpgCount, g_fileCount);
            }
            
            g_fileCount = totalFileCount - jpgCount;
            int numFiles = 0;
            if (g_fileCount == 0) {
                NSLog(@"no files to upload!");
                return;
            }
            
            for (unsigned count = 0; count < [directoryContent count]; count++) {
                NSString *fileName = [[directoryContent objectAtIndex:count] objectForKey:@"path"];
                NSString *absFileName = [documentPath stringByAppendingPathComponent:fileName];
                
                if([fileName containsString:scanName] && ![fileName containsString:@".jpg"])
                {
                    //NSLog(@"File Name: %@", scanName);
                    g_totalUploadBytes += [[[NSFileManager defaultManager] attributesOfItemAtPath:absFileName
                                                                                        error:nil] fileSize];
                    numFiles++;
                }
            }
            
            g_fileCount = numFiles;
            
            
            NSURLSessionConfiguration *defaultConfigObject = [NSURLSessionConfiguration defaultSessionConfiguration];
            defaultConfigObject.HTTPMaximumConnectionsPerHost = 5;
            defaultConfigObject.timeoutIntervalForRequest = 5000.0;
            
            //if([fileName containsString:@".depth"])
            
            const int maxTime = 10000;
            int scaledTime = 1000 + 1000 * g_fileCount;
            
            defaultConfigObject.timeoutIntervalForResource = (scaledTime > maxTime) ? maxTime : scaledTime;
            
            defaultSession = [NSURLSession sessionWithConfiguration:defaultConfigObject
                                                           delegate:self
                                                      delegateQueue:[NSOperationQueue mainQueue]];
            
            
            dispatch_async(dispatch_get_main_queue(), ^{
                // Reset progress
                [[self progressView] setHidden:NO];
                [[self progressView] setProgress:0.0 animated:YES];
                // Hide scan button
                [[self scanButton] setHidden:YES];
                // Hide upload button
                [[self uploadButton] setHidden:YES];
                // Hide color res
                [[self optionsView] setHidden:YES];
                // Hide error indicator, if any
                [[self uploadErrorIndicator] setHidden:YES];
                [[self sceneLabelFieldView] setHidden:YES];
    #ifdef DEBUG_MODE
                [[self uploadErrorDescription] setHidden:YES];
    #endif
                [[self viewScans] setHidden:YES];
            });
            
            for (unsigned count = 0; count < totalFileCount; count++) {
                
                NSString *fileName = [[directoryContent objectAtIndex:count] objectForKey:@"path"];
                if(![fileName containsString:@".jpg"] && [fileName containsString:scanName])
                {
                    NSLog(@"File %d: %@", count + 1, [[directoryContent objectAtIndex:count] objectForKey:@"path"]);
                    /*
                    while(g_numCurrentUploads > 4) {
                        [NSThread sleepForTimeInterval:30];
                    }
                    
                    g_numCurrentUploads++;
                     */
                    //NSLog(@"File Name Before Upload: %@", fileName);
                    [self uploadFile:documentPath
                            fileName:fileName
                          uploadPath:uploadPath];
                }
            }
        }];
        uploadAction.backgroundColor = [UIColor blueColor];
        
        
        UITableViewRowAction *deleteAction = [UITableViewRowAction rowActionWithStyle:UITableViewRowActionStyleDefault title:@"Delete" handler:^(UITableViewRowAction *action, NSIndexPath *indexPath) {
            NSString* scanName = [myScanData objectAtIndex:indexPath.row];
            [myScanData removeObjectAtIndex:indexPath.row];
            [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
            
            NSArray* splitScanName = [scanName componentsSeparatedByString:@"\n\t"];
            NSString* shortScanName = [splitScanName objectAtIndex:0];
            NSString* fileExtensions = [splitScanName objectAtIndex:1];
            NSArray* splitFileExtensions = [fileExtensions componentsSeparatedByString:@" "];
            
            NSString* documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
            NSString* fileNameWithoutFileType = [documentPath stringByAppendingPathComponent:shortScanName];
            fileNameWithoutFileType = [fileNameWithoutFileType stringByAppendingString:@"."];
            
            for (int i = 0; i < [splitFileExtensions count]; i++)
            {
                if([[splitFileExtensions objectAtIndex:i]  isEqualToString: @"jpg"] || [[splitFileExtensions objectAtIndex:i] length] < 3)
                    continue;
                
                NSString* fileName =
                [fileNameWithoutFileType stringByAppendingString:[splitFileExtensions objectAtIndex:i]];
                
                NSError *error = nil;
                
                [[NSFileManager defaultManager] removeItemAtPath:fileName error:&error];
            }
        }];
        
        deleteAction.backgroundColor = [UIColor redColor];
        
        return @[deleteAction, uploadAction];
    }
}
#endif

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(nonnull NSIndexPath *)indexPath
{
#ifndef DEBUG_MODE
    if(editingStyle == UITableViewCellEditingStyleDelete)
    {
        NSString* scanName = [myScanData objectAtIndex:indexPath.row];
        [myScanData removeObjectAtIndex:indexPath.row];
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
        
        NSArray* splitScanName = [scanName componentsSeparatedByString:@"\n\t"];
        NSString* shortScanName = [splitScanName objectAtIndex:0];
        NSString* fileExtensions = [splitScanName objectAtIndex:1];
        NSArray* splitFileExtensions = [fileExtensions componentsSeparatedByString:@" "];
        
        NSString* documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
        NSString* fileNameWithoutFileType = [documentPath stringByAppendingPathComponent:shortScanName];
        fileNameWithoutFileType = [fileNameWithoutFileType stringByAppendingString:@"."];
        
        for (int i = 0; i < [splitFileExtensions count]; i++)
        {
            NSString* fileName =
                [fileNameWithoutFileType stringByAppendingString:[splitFileExtensions objectAtIndex:i]];
            
            NSError *error = nil;
            
            [[NSFileManager defaultManager] removeItemAtPath:fileName error:&error];
        }
    }
#endif
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *cellIdentifier = @"cellID";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:
                             cellIdentifier];
    
    if (cell == nil) {
        cell = [[UITableViewCell alloc]initWithStyle:
                UITableViewCellStyleDefault reuseIdentifier:cellIdentifier];

        cell.textLabel.lineBreakMode = NSLineBreakByTruncatingMiddle;
        
    }
    
    
    if(tableView == self.exposureTable) {
        NSString *stringForCell = [exposureLevels objectAtIndex:indexPath.row];
        [cell.textLabel setText:stringForCell];
        cell.textLabel.numberOfLines = 0;
        cell.textLabel.adjustsFontSizeToFitWidth = YES;
        cell.textLabel.textAlignment = NSTextAlignmentCenter;
        return cell;
    }
    else {
        NSString *stringForCell = [myScanData objectAtIndex:indexPath.row];
        [cell.textLabel setText:stringForCell];
        cell.textLabel.numberOfLines = 10;
        cell.textLabel.adjustsFontSizeToFitWidth = YES;
        
        // Create image
        if([stringForCell containsString:@"jpg"])
        {
            NSString* scanName = [myScanData objectAtIndex:indexPath.row];
            NSArray* splitScanName = [scanName componentsSeparatedByString:@"\n\t"];
            NSString* shortScanName = [splitScanName objectAtIndex:0];
            
            NSString* documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
            NSString* filePath = [documentPath stringByAppendingPathComponent:shortScanName];
            filePath = [filePath stringByAppendingString:@".jpg"];
            cell.imageView.image = [UIImage imageWithContentsOfFile:filePath];
        }
        
        return cell;
    }

}

- (void)setColorCameraParametersForInit
{
    NSError *error;
    
    [self.videoDevice lockForConfiguration:&error];
    
    /*
     // Auto-exposure
     if ([self.videoDevice isExposureModeSupported:AVCaptureExposureModeContinuousAutoExposure])
     [self.videoDevice setExposureMode:AVCaptureExposureModeContinuousAutoExposure];
     */
    // Auto-white balance.
    if ([self.videoDevice isWhiteBalanceModeSupported:AVCaptureWhiteBalanceModeContinuousAutoWhiteBalance])
        [self.videoDevice setWhiteBalanceMode:AVCaptureWhiteBalanceModeContinuousAutoWhiteBalance];
    
    [self.videoDevice unlockForConfiguration];
    
}

- (void)setColorCameraParametersForScanning
{
    NSError *error;
    
    [self.videoDevice lockForConfiguration:&error];
    
    /*
     // Exposure locked to its current value.
     if ([self.videoDevice isExposureModeSupported:AVCaptureExposureModeLocked])
     [self.videoDevice setExposureMode:AVCaptureExposureModeLocked];
     */
    // White balance locked to its current value.
    if ([self.videoDevice isWhiteBalanceModeSupported:AVCaptureWhiteBalanceModeLocked])
        [self.videoDevice setWhiteBalanceMode:AVCaptureWhiteBalanceModeLocked];
    
    [self.videoDevice unlockForConfiguration];
}

- (void)changeExposureValue
{
    if([exposureValue  isEqual: [exposureLevels objectAtIndex:0]]) {
        NSLog(@"Auto mode selected");
        NSError *error;
        
        [self.videoDevice lockForConfiguration:&error];
        
        
        // Auto-exposure
        if ([self.videoDevice isExposureModeSupported:AVCaptureExposureModeContinuousAutoExposure]) {
            [self.videoDevice setExposureMode:AVCaptureExposureModeContinuousAutoExposure];
        }
        
        [self.videoDevice unlockForConfiguration];
        
        return;
    }
    else {
        NSInteger duration = [exposureValue integerValue];
        NSLog(@"Duration: %ld", (long)duration);
        
        NSError *error;
        
        [self.videoDevice lockForConfiguration:&error];
        
        // Exposure locked to its current value.
        if ([self.videoDevice isExposureModeSupported:AVCaptureExposureModeLocked])
            [self.videoDevice setExposureModeCustomWithDuration:CMTimeMake(duration, 1000) ISO:200 completionHandler:nil];
        
        
        [self.videoDevice unlockForConfiguration];
    }
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath: (NSIndexPath *)indexPath
{
    if(tableView == self.tableView)
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
    
    if(tableView == self.exposureTable) {
        UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
        exposureValue = cell.textLabel.text;
        [self changeExposureValue];
    }
}


-(NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

-(NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    return [sceneTypes count];
}

-(NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    return [sceneTypes objectAtIndex:row];
}

-(void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
#ifdef DEBUG_MODE
    if(row != 0)
        _options.specifiedSceneType = [[sceneTypes objectAtIndex:row] UTF8String];
#else
    _options.specifiedSceneType = [[sceneTypes objectAtIndex:row] UTF8String];
#endif
}
/*********************************************************************/

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
   didSendBodyData:(int64_t)bytesSent
    totalBytesSent:(int64_t)totalBytesSent
totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend
{
    g_uploadedByteCount += bytesSent;
    //NSLog(@"g_uploadedByteCount: %llu g_totalUploadBytes: %llu", g_uploadedByteCount, g_totalUploadBytes);
    dispatch_async(dispatch_get_main_queue(), ^{
        float progress = g_uploadedByteCount / (double) g_totalUploadBytes;
        [[self progressView] setProgress:progress animated:YES];
    });
}

// Really not following ObjC naming conventions here...
- (void)uploadFile:(NSString *)dataDir
          fileName:(NSString *)fileName
        uploadPath:(NSURL *)uploadPath
{
    //NSLog(@"Upload url: %@, file: %@", uploadPath, fileName);
    NSString *absFileName = [dataDir stringByAppendingPathComponent:fileName];
    //NSLog(@"absolute filename: %@", absFileName);
    
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:uploadPath];
    [request setHTTPMethod:@"PUT"];
    [request addValue:fileName forHTTPHeaderField:@"FILE_NAME"];
    [request addValue:@"application/ipad_scanner_data" forHTTPHeaderField:@"Content-Type"];

    NSString *fileSize = [NSString stringWithFormat:@"%llu",
                                   [[[NSFileManager defaultManager] attributesOfItemAtPath:absFileName error:nil] fileSize]];
    NSLog(@"************ File Size: %@", fileSize);
    [request addValue:fileSize forHTTPHeaderField:@"Content-Length"];
    
    NSInputStream *stream = [[NSInputStream alloc] initWithFileAtPath:absFileName];
    [request setHTTPBodyStream:stream];
    
    NSURLSessionDataTask *uploadTask = [defaultSession dataTaskWithRequest:request
                                                  completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                                                      NSLog(@"Upload file %@ Response: %@, error: %@", fileName, response, error);
                                                    
                                                      g_numCurrentUploads--;
                                                      
                                                      if(response == NULL) {
                                                          NSLog(@"Response is NULL");
                                                          self.uploadErrorDescription.text =
                                                          [NSString stringWithFormat:@"Response is NULL"];
                                                          self.uploadErrorDescription.hidden = false;
                                                      }
                                                      else {
                                                          NSError *serializeError = nil;
                                                          NSDictionary *json = [NSJSONSerialization JSONObjectWithData:data options:kNilOptions error:&serializeError];
                                                          
                                                          NSLog(@"json count: %lu, key: %@, value: %@", [json count], [json allKeys], [json allValues]);
                                                          NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) response;
                                                          
                                                          NSInteger statusCode = [httpResponse statusCode];
                                                          if([json valueForKey:@"message"] == nil)
                                                          {
                                                              [NSString stringWithFormat:@"Status Code: %lu No Message in json", statusCode];
                                                              NSLog(@"No message in json");
                                                          }
                                                          else {
                                                              self.uploadErrorDescription.text =
                                                              [NSString stringWithFormat:@"Status Code: %lu Message: %@", statusCode, [json valueForKey:@"message"]];
                                                          }
                                                          self.uploadErrorDescription.hidden = NO;
                                                          
                                                          if (error == nil && statusCode == 200)
                                                          {
                                                              //[[self uploadErrorDescription] setHidden:YES];
                                                              [self verifyFile:dataDir
                                                                      fileName:fileName
                                                                    verifyPath:@SERVER VERIFY];
                                                          }
                                                          else
                                                          {
                                                              NSLog(@"ERROR: %@", [error localizedDescription]);
                                                              NSLog(@"Status Code: %ld", (long)statusCode);
                                                              [self blinkUploadErrorIndicator];
                                                              
                                                              
                                                              NSLog(@"***********************");
                                                          }
                                                      }
                                                      
                                                      
                                                      
                                                      /*
                                                      NSArray *allKeys = [jsonData allKeys];
                                                      for(int i = 0; i < [allKeys count]; i++) {
                                                          NSLog(@"%d", i);
                                                          NSLog(@"Data Type: %@", NSStringFromClass([jsonData objectForKey:[allKeys objectAtIndex:i]]));
                                                      }*/
                                                    
                                                                                                            //NSLog(@"Data Size: %lu", [data length]);
                                                      dispatch_async(dispatch_get_main_queue(), ^{
                                                          g_uploadedCount += 1;
                                                          if (g_uploadedCount == g_fileCount)
                                                          {
                                                              g_fileCount = 0;
                                                              [[self progressView] setHidden:YES];
                                                              [[self scanButton] setHidden:NO];
                                                              [[self uploadButton] setHidden:NO];
                                                              [[self optionsView] setHidden:NO];
                                                              [[self sceneLabelFieldView] setHidden:NO];
                                                              [[self viewScans] setHidden:NO];

                                                              //update memory (not exactly correct since verify may not have finished)
                                                              float percentUsedMem = (float)[self getPercentUsedDiskspace];
                                                              [[self memProgressView] setProgress:percentUsedMem];
                                                          }
                                                      });
                                                  }];
    
    NSBlockOperation *op = [NSBlockOperation blockOperationWithBlock:^{
        [uploadTask resume];
    }];
    
    [_uploadQueue addOperation:op];
    //[_uploadQueue add]
    
    return;
}

- (void) verifyFile:(NSString *)dataDir
           fileName:(NSString *)fileName
         verifyPath:(NSString *)verifyPath
{
    NSString *absFileName = [dataDir stringByAppendingPathComponent:fileName];
    NSString *checksum = calculateChecksum(absFileName);
    NSLog(@"File: %@ Hash: %@", fileName, checksum);
    
    NSString *query = [NSString stringWithFormat:@"?filename=%@&checksum=%@",
                                fileName,
                                checksum];
    NSURLComponents *verifyUrl = [[NSURLComponents alloc] initWithString:[verifyPath stringByAppendingString:query]];
    
    
    //NSLog(@"absFileName: %@, verify url: %@", fileName, verifyUrl);
    
    NSURLSessionDataTask *task = [[NSURLSession sharedSession] dataTaskWithURL:[verifyUrl URL]
                                                             completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                                                                 NSLog(@"Verify file %@ Response: %@, error: %@", fileName, response, error);

                                                                 NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *) response;
                                                                 NSInteger statusCode = [httpResponse statusCode];

                                                                if (error == nil && statusCode == 200)
                                                                {
                                                                    NSFileManager *fileManager = [NSFileManager defaultManager];
                                                                    [fileManager removeItemAtPath:absFileName error:nil];
                                                                    NSLog(@"Removed file %@", absFileName);
                                                                }
                                                            }];
    
    NSBlockOperation *op = [NSBlockOperation blockOperationWithBlock:^{
        [task resume];
    }];
    
    [_verifyQueue addOperation:op];
}

NSString *calculateChecksum(NSString *fileName)
{
    FILE *fp = fopen([fileName UTF8String], "rb");
    uint8_t buf[READ_CHUNK_SIZE];
    
    CC_MD5_CTX md5;
    CC_MD5_Init(&md5);

    size_t read_size = READ_CHUNK_SIZE;
    if (fp != NULL)
    {
        while (read_size == READ_CHUNK_SIZE)
        {
            read_size = fread(buf, sizeof(uint8_t), READ_CHUNK_SIZE, fp);
            CC_MD5_Update(&md5, buf, read_size);
        }
    }
    else
    {
        NSLog(@"Unable to read file: %@", fileName);
        return nil;
    }
    
    uint8_t digest[CC_MD5_DIGEST_LENGTH];
    CC_MD5_Final(digest, &md5);
    
    NSMutableString *output = [NSMutableString stringWithCapacity:CC_MD5_DIGEST_LENGTH * 2];
    for(int i = 0; i < CC_MD5_DIGEST_LENGTH; i++)
        [output appendFormat:@"%02x", digest[i]];

    return output;
}

-(NSArray*)getSortedFilesFromFolder: (NSString*)folderPath
{
    NSError *error = nil;
    NSArray* filesArray = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:folderPath error:&error];
    //NSPredicate *predicate = [NSPredicate predicateWithFormat:@"SELF EndsWith '.depth'"]; //just search for the .depth files
    //filesArray = [filesArray filteredArrayUsingPredicate:predicate];
    
    //collect files/properties
    NSMutableArray* filesAndProperties = [NSMutableArray arrayWithCapacity:[filesArray count]];
    for(NSString* file in filesArray) {
        if (![file isEqualToString:@".DS_Store"]) {
            NSString *filePath = [folderPath stringByAppendingPathComponent:file];
            NSDictionary* properties = [[NSFileManager defaultManager] attributesOfItemAtPath:filePath error:&error];
            NSDate* modDate = [properties objectForKey:NSFileModificationDate];
            [filesAndProperties addObject:[NSDictionary dictionaryWithObjectsAndKeys:file, @"path", modDate, @"lastModDate", nil]];
        }
    }
    //NSLog(@"%lu files and properties", [filesAndProperties count]);
    //sort by date (most recent last)
    NSArray* sortedFiles = [filesAndProperties sortedArrayUsingComparator:^(id path1, id path2)
                                 {
                                     NSComparisonResult comp = [[path1 objectForKey:@"lastModDate"] compare:[path2 objectForKey:@"lastModDate"]];
                                     return comp;
                                 }];
    return sortedFiles;
}


- (IBAction)uploadButtonPressed:(id)sender {
    NSURL *uploadPath = [NSURL URLWithString:@SERVER UPLOAD];
    
    
    NSString* documentPath = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
    //NSLog(@"document path: %@", documentPath);
    //get files
    NSArray* directoryContent = [self getSortedFilesFromFolder:documentPath]; //[[NSFileManager defaultManager] contentsOfDirectoryAtPath:documentPath error:NULL];
    
    unsigned long totalFileCount = 0;
    g_uploadedCount = 0;
    g_totalUploadBytes = 0;
    g_uploadedByteCount = 0;
    totalFileCount = [directoryContent count];
    //remove jpg files from consideration
    NSError *error = nil;
    NSArray* filesArray = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:documentPath error:&error];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"SELF EndsWith '.jpg'"];
    filesArray = [filesArray filteredArrayUsingPredicate:predicate];
    unsigned long jpgCount = [filesArray count];
    if (jpgCount > g_fileCount) {
        NSLog(@"ERROR: #jpg files = %lu > #files = %lu!", jpgCount, g_fileCount);
    }
    g_fileCount = totalFileCount - jpgCount;
    if (g_fileCount == 0) {
        NSLog(@"no files to upload!");
        return;
    }
    
    for (unsigned count = 0; count < g_fileCount; count++) {
        NSString *fileName = [[directoryContent objectAtIndex:count] objectForKey:@"path"];
        NSString *absFileName = [documentPath stringByAppendingPathComponent:fileName];
        
        g_totalUploadBytes += [[[NSFileManager defaultManager] attributesOfItemAtPath:absFileName
                                                                                error:nil] fileSize];
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        // Reset progress
        [[self progressView] setHidden:NO];
        [[self progressView] setProgress:0.0 animated:YES];
        // Hide scan button
        [[self scanButton] setHidden:YES];
        // Hide upload button
        [[self uploadButton] setHidden:YES];
        // Hide color res
        [[self optionsView] setHidden:YES];
        // Hide error indicator, if any
        [[self uploadErrorIndicator] setHidden:YES];
        [[self sceneLabelFieldView] setHidden:YES];
#ifdef DEBUG_MODE
        [[self uploadErrorDescription] setHidden:YES];
#endif
        [[self viewScans] setHidden:YES];
    });
    
    g_numCurrentUploads = 0;
    
    NSURLSessionConfiguration *defaultConfigObject = [NSURLSessionConfiguration defaultSessionConfiguration];
    defaultConfigObject.HTTPMaximumConnectionsPerHost = 5;
    
    
    //if([fileName containsString:@".depth"])
    
    const int maxTime = 10000;
    int scaledTime = 1000 + 1000 * g_fileCount;
    
    defaultConfigObject.timeoutIntervalForResource = (scaledTime > maxTime) ? maxTime : scaledTime;
    defaultConfigObject.timeoutIntervalForRequest = defaultConfigObject.timeoutIntervalForResource;
    defaultSession = [NSURLSession sessionWithConfiguration:defaultConfigObject
                                                                 delegate:self
                                                            delegateQueue:[NSOperationQueue mainQueue]];
    
    for (unsigned count = 0; count < totalFileCount; count++) {
        NSLog(@"File %d: %@", count + 1, [[directoryContent objectAtIndex:count] objectForKey:@"path"]);
        
        NSString *fileName = [[directoryContent objectAtIndex:count] objectForKey:@"path"];
        if(![fileName containsString:@".jpg"])
        {
            g_numCurrentUploads++;
            NSLog(@"Num Current Uploads: %ld", (long)g_numCurrentUploads);

            [self uploadFile:documentPath
                    fileName:fileName
                  uploadPath:uploadPath];
        }
    }
    
}

//+(BOOL)isNotEmpty:(NSString*)string {
BOOL isNotEmpty(NSString* string) {
    if (string == nil) return false;
    if (![[string stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]] length])
        return false; //string is all whitespace
    if ([string length] == 0) return false;
    return true;
}

- (IBAction)sceneLabelFieldEdited:(id)sender {
    if (isNotEmpty(self.sceneLabelField.text)) {
        _options.sceneLabel = [self.sceneLabelField.text UTF8String];
        
        NSLog(@"entered scene label: %s", _options.sceneLabel.c_str());
        [self enterCubePlacementState];
    }
    else {
        NSLog(@"cannot use empty string %@ for scene label, reverting to scene labelling state", self.sceneLabelField.text);
        [self enterSceneLabellingState];
    }
    
    [self changeExposureValue];
}

- (IBAction)userNameFieldEdited:(id)sender {
    if (isNotEmpty(self.userNameField.text)) {
        _options.userName = [self.userNameField.text UTF8String];
        NSLog(@"entered user name: %s", _options.userName.c_str());
    }
}

- (IBAction)toggleDepthButtonPressed:(id)sender {
    _renderDepthOverlay = !_renderDepthOverlay;
}

- (IBAction)awbSwitchChanged:(id)sender {
    if (_slamState.scannerState == ScannerStateScanning)
        NSLog(@"warning: cannot toggle auto-exp/wb during scanning");
    else {
        if (self.AWBSwitch.isOn) {
            [self setColorCameraParametersForInit];
            NSLog(@"auto-exp/wb set to ON");
        }
        else {
            [self setColorCameraParametersForScanning];
            NSLog(@"auto-exp/wb set to OFF");
        }
    }
    
    [self changeExposureValue];
}

// Manages whether we can let the application sleep.
-(void)updateIdleTimer
{
    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];

//    if ([self isStructureConnectedAndCharged] && [self currentStateNeedsSensor])
//    {
//        // Do not let the application sleep if we are currently using the sensor data.
//        [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
//    }
//    else
//    {
//        // Let the application sleep if we are only viewing the mesh or if no sensors are connected.
//        [[UIApplication sharedApplication] setIdleTimerDisabled:NO];
//    }
}

- (void)showTrackingMessage:(NSString*)message
{
    self.trackingLostLabel.text = message;
    self.trackingLostLabel.hidden = NO;
}

- (void)hideTrackingErrorMessage
{
    self.trackingLostLabel.hidden = YES;
}

- (void)showAppStatusMessage:(NSString *)msg
{
    _appStatus.needsDisplayOfStatusMessage = true;
    [self.view.layer removeAllAnimations];
    
    [self.appStatusMessageLabel setText:msg];
    [self.appStatusMessageLabel setHidden:NO];
    
    // Progressively show the message label.
    [self.view setUserInteractionEnabled:false];
    [UIView animateWithDuration:0.5f animations:^{
        self.appStatusMessageLabel.alpha = 1.0f;
    }completion:nil];
}

- (void)hideAppStatusMessage
{
    if (!_appStatus.needsDisplayOfStatusMessage)
        return;
    
    _appStatus.needsDisplayOfStatusMessage = false;
    [self.view.layer removeAllAnimations];
    
    __weak ViewController *weakSelf = self;
    [UIView animateWithDuration:0.5f
                     animations:^{
                         weakSelf.appStatusMessageLabel.alpha = 0.0f;
                     }
                     completion:^(BOOL finished) {
                         // If nobody called showAppStatusMessage before the end of the animation, do not hide it.
                         if (!_appStatus.needsDisplayOfStatusMessage)
                         {
                             // Could be nil if the self is released before the callback happens.
                             if (weakSelf) {
                                 [weakSelf.appStatusMessageLabel setHidden:YES];
                                 [weakSelf.view setUserInteractionEnabled:true];
                             }
                         }
     }];
}

-(void)updateAppStatusMessage
{
    // Skip everything if we should not show app status messages (e.g. in viewing state).
    if (_appStatus.statusMessageDisabled)
    {
        [self hideAppStatusMessage];
        return;
    }
    
    // First show sensor issues, if any.
    switch (_appStatus.sensorStatus)
    {
        case AppStatus::SensorStatusOk:
        {
            break;
        }
            
        case AppStatus::SensorStatusNeedsUserToConnect:
        {
            [self showAppStatusMessage:_appStatus.pleaseConnectSensorMessage];

            return;
        }
            
        case AppStatus::SensorStatusNeedsUserToCharge:
        {
            [self showAppStatusMessage:_appStatus.pleaseChargeSensorMessage];
            return;
        }
            
        case AppStatus::SensorStatusNeedsIntrinsics:
        {
            [self showAppStatusMessage:_appStatus.needIntrinsicsMessage];
            return;
        }
    }
    
    // Then show color camera permission issues, if any.
    if (!_appStatus.colorCameraIsAuthorized)
    {
        [self showAppStatusMessage:_appStatus.needColorCameraAccessMessage];
        return;
    }

    // If we reach this point, no status to show.
    [self hideAppStatusMessage];
}


- (void) respondToMemoryWarning
{
    switch( _slamState.scannerState )
    {
        //case ScannerStateViewing: //not much to do here
        //{
        //    break;
        //}
        case ScannerStateScanning:
        {
            if( !_slamState.showingMemoryWarning )
            {
                _slamState.showingMemoryWarning = true;
                
                UIAlertController *alertCtrl= [UIAlertController alertControllerWithTitle:@"Memory Low"
                                                                                  message:@"Scanning will be stopped to avoid loss."
                                                                           preferredStyle:UIAlertControllerStyleAlert];
                
                UIAlertAction* okAction = [UIAlertAction actionWithTitle:@"OK"
                                                                   style:UIAlertActionStyleDefault
                                                                 handler:^(UIAlertAction *action)
                                           {
                                               _slamState.showingMemoryWarning = false;
                                               //[self enterViewingState];
                                               [self cleanUp];
                                               [self enterSceneLabellingState];
                                           }];
                
                
                [alertCtrl addAction:okAction];
                
                // show the alert
                [self presentViewController:alertCtrl animated:YES completion:nil];
            }
            
            break;
        }
        default:
        {
            // not much we can do here
        }
    }
}

- (void) updateColorRes:(NSInteger)index
{
    switch (self.colorResControl.selectedSegmentIndex)
    {
        case 0:
        {
            _options.colorWidth = 640;
            _options.colorHeight = 480;
            //_options.colorFocal = 578.0f;   //default intrinsics for VGA (these will instead be accessed from the frames)
            //_options.colorCenterX = 320.0f;
            //_options.colorCenterY = 240.0f;
            _options.colorEncodeBitrate = 5000;
            _options.useHalfResColor = false;
            break;
        }
        case 1:
        {
            _options.colorWidth = 2592;
            _options.colorHeight = 1936;
            //_options.colorFocal = 578.0f * 4.05f;   //default intrinsics for hi-res (these will instead be accessed from the frames)
            //_options.colorCenterX = 320.0f * 4.05f;
            //_options.colorCenterY = 240.0f * 4.05f;
            _options.colorEncodeBitrate = 15000;
            _options.useHalfResColor = true;
            break;
        }
        default:
            break;
            
    }
    //if (_options.colorWidth != oldWidth || _options.colorHeight != oldHeight) {
        //NSLog(@"[updateColorRes] try to set color res to %d %d", _options.colorWidth, _options.colorHeight); // TMI for log
        if (self.avCaptureSession)
        {
            [self stopColorCamera];
            if (_useColorCamera)
                [self startColorCamera];
            else
                NSLog(@"[updateColorRes] not using color camera!");
        }
        else {
            //NSLog(@"[updateColorRes] No avCaptureSession!"); // TMI for log
        }
        
        // Force a scan reset since we cannot changing the image resolution during the scan is not
        // supported by STColorizer.
        [self resetButtonPressed:self.resetButton];
    //}
    
    std::stringstream ss;
    ss << self.colorResControl.selectedSegmentIndex;
    PersistentStore::set("colorResControl", ss.str().c_str());
}

- (void) loadOptions
{
    NSLog(@"[loadoptions]");
    
    //uid
    _options.deviceId = PersistentStore::get("uid");
    NSString* name = [[UIDevice currentDevice] name];
    _options.deviceName = [name UTF8String];
    if (_options.deviceId.empty()) {
        NSString* s = [DeviceUID uid];
        _options.deviceId = [s UTF8String];
        NSLog(@"creating uid: %s", _options.deviceId.c_str());
        PersistentStore::set("uid", _options.deviceId.c_str());
    }
    else {
        NSLog(@"loaded uid: %s", _options.deviceId.c_str()); //unecessary log
    }
    
    //scene label
    //[self loadSceneLabel];
    
    //colorres
    int colorRes = PersistentStore::getAsInt("colorResControl");
    //NSLog(@"colorRes from persistent store: %d", colorRes);
    if (colorRes != -1)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            self.colorResControl.selectedSegmentIndex = colorRes;
            [self updateColorRes:colorRes];
        });
    }
}

- (void) loadSceneLabel
{
    NSLog(@"[loadSceneLabel]");
    
    //uid
    _options.sceneLabel = PersistentStore::get("sceneLabel");
    NSString* sceneStr = [NSString stringWithUTF8String:_options.sceneLabel.c_str()];
    _options.userName = PersistentStore::get("userName");
    NSString* userStr = [NSString stringWithUTF8String:_options.userName.c_str()];
    
    //NSLog(@"persistent store label: %@", sceneStr);
    
    if (isNotEmpty(sceneStr)) { //look for last numeric
        NSScanner* scanner = [NSScanner scannerWithString:sceneStr];
        NSCharacterSet* numbers = [NSCharacterSet characterSetWithCharactersInString:@"0123456789"];
        NSInteger num; bool foundLastNum = false; NSString* prefix = @"";
        while (!scanner.atEnd) {
            NSString* nonNumString, *numString;
            [scanner scanUpToCharactersFromSet:numbers intoString:&nonNumString];
            if (nonNumString != nil) prefix = [prefix stringByAppendingString:nonNumString];
            if (scanner.atEnd) break;
            [scanner scanCharactersFromSet:numbers intoString:&numString];
            if (numString == nil) break;
            if (scanner.atEnd) {
                num = numString.integerValue;
                foundLastNum = true;
                break;
            }
            else {
                prefix = [prefix stringByAppendingString:numString];
            }
        }
        if (foundLastNum) {
            sceneStr = [NSString stringWithFormat:@"%@%ld", prefix, num+1];
            //NSLog(@"found last num %ld, new scene label: %@", num, sceneStr); // TMI for log
        }
        else {
            sceneStr = [NSString stringWithFormat:@"%@1", sceneStr];
            //NSLog(@"no last num, new scene label: %@", sceneStr);
        }
        _options.sceneLabel = [sceneStr UTF8String];
    }
    else {
        NSLog(@"empty scene label: %@ | %s", sceneStr, _options.sceneLabel.c_str());
    }
    
    NSLog(@"scene label: %@", sceneStr);
    
    self.sceneLabelField.text = sceneStr;
    self.userNameField.text = userStr;
}

- (void)cleanUpAndReset
{
    [self cleanUp];
    
    [self setColorCameraParametersForInit];
    
    _slamState.scannerState = ScannerStateCubePlacement;
    
    [self updateIdleTimer];
}

- (void)cleanUp
{
    //write out the file
    [self stopTrackingIMU];
    [self stopScanningAndWrite];
    
    if ([self isConnectedToWIFI])
    {
        self.uploadButton.hidden = NO; //allow uploads from here
    }
    else
    {
        self.uploadButton.hidden = YES;
    }
    
    [self hideTrackingErrorMessage];
    
    _appStatus.statusMessageDisabled = true;
    [self updateAppStatusMessage];
    
    // Hide the Scan/Done/Reset button.
    self.scanButton.hidden = NO;
    self.doneButton.hidden = YES;
    self.resetButton.hidden = YES;
    self.optionsView.hidden = NO;
    self.sceneLabelFieldView.hidden = NO;
    self.uploadErrorIndicator.hidden = YES;
#ifdef DEBUG_MODE
    self.uploadErrorDescription.hidden = YES;
#endif
    [[self scanButton] setHidden:NO];
}

- (void)blinkUploadErrorIndicator
{
    [[self uploadErrorIndicator] setHidden:![[self uploadErrorIndicator] isHidden]];
    
    if (blinkCounter == 0)
    {
        [uploadErrorTimer invalidate];
        dispatch_async(dispatch_get_main_queue(), ^{
            uploadErrorTimer = [NSTimer scheduledTimerWithTimeInterval:0.5
                                                                target:self
                                selector:@selector(blinkUploadErrorIndicator)
                                                              userInfo:nil
                                                               repeats:YES];
        });
        blinkCounter++;
    }
    else if (blinkCounter == 20)
    {
        [uploadErrorTimer invalidate];
        blinkCounter = 0;
        [[self uploadErrorIndicator] setHidden:NO];
    }
    else
    {
        blinkCounter++;
    }
}

@end
