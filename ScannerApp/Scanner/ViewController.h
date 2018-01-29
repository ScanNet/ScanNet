/*
  This file is part of the Structure SDK.
  Copyright © 2015 Occipital, Inc. All rights reserved.
  http://structure.io
*/

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMotion/CoreMotion.h>
#define HAS_LIBCXX
#import <Structure/Structure.h>
#import "GPUImage.h"
                                  
#include <string>
#include <vector>

#import "Config.h"


struct Options //TODO get rid of mesh view/tracking params
{
    // Whether we should use depth aligned to the color viewpoint when Structure Sensor was calibrated.
    // This setting may get overwritten to false if no color camera can be used.
    
    bool useHardwareRegisteredDepth = false;
    
    // Focus position for the color camera (between 0 and 1). Must remain fixed one depth streaming
    // has started when using hardware registered depth.
    const float lensPosition = 0.75f;
    
    unsigned int colorEncodeBitrate = 5000;
    
    //meta-data (res, intrinsics)
    unsigned int colorWidth = 640;
    unsigned int colorHeight = 480;
    unsigned int depthWidth = 640;
    unsigned int depthHeight = 480;
    float colorFocalX = 578.0f; float colorFocalY = 578.0f; float colorCenterX = 320.0f; float colorCenterY = 240.0f; //default for VGA
    float depthFocalX = 570.5f; float depthFocalY = 570.5f; float depthCenterX = 320.0f; float depthCenterY = 240.0f; //default for VGA
    bool useHalfResColor = false;
 
    float colorToDepthExtrinsics[16];
    
    std::string sceneLabel = "";
    
    std::string deviceId = "";
    std::string deviceName = "";
    
    std::string specifiedSceneType = "";

    std::string userName = "";
};

enum ScannerState
{
    // scene label - wait for label (can't can until have one)
    ScannerStateSceneLabelling = 0,
    
    // before scanning (let user toggle awb, find good first frame)
    ScannerStateCubePlacement,
    
    // Scanning (scan!)
    ScannerStateScanning,
    
    // View all uploads
    ScannerStateViewing,
    
    NumStates
};

// SLAM-related members.
struct SlamData
{
    SlamData ()
    : initialized (false)
    , scannerState (ScannerStateCubePlacement)
    {}
    
    BOOL initialized;
    BOOL showingMemoryWarning = false;
    
    NSTimeInterval prevFrameTimeStamp = -1.0;
    
    ScannerState scannerState;
};

struct AppStatus
{
    NSString* const pleaseConnectSensorMessage = @"Please connect Structure Sensor.";
    NSString* const pleaseChargeSensorMessage = @"Please charge Structure Sensor.";
    NSString* const needColorCameraAccessMessage = @"This app requires camera access to capture color.\nAllow access by going to Settings → Privacy → Camera.";
    NSString* const needIntrinsicsMessage = @"No intrinsics received. Please restart the app.";
    
    enum SensorStatus
    {
        SensorStatusOk,
        SensorStatusNeedsUserToConnect,
        SensorStatusNeedsUserToCharge,
        SensorStatusNeedsIntrinsics,
    };
    
    // Structure Sensor status.
    SensorStatus sensorStatus = SensorStatusOk;
    
    // Whether iOS camera access was granted by the user.
    bool colorCameraIsAuthorized = true;
    
    // Whether there is currently a message to show.
    bool needsDisplayOfStatusMessage = false;
    
    // Flag to disable entirely status message display.
    bool statusMessageDisabled = false;
};

// Display related members.
struct DisplayData
{
    DisplayData ()
    {
    }
    
    ~DisplayData ()
    {
        if (lumaTexture)
        {
            CFRelease (lumaTexture);
            lumaTexture = NULL;
        }
        
        if (chromaTexture)
        {
            CFRelease (chromaTexture);
            lumaTexture = NULL;
        }
        
        if (videoTextureCache)
        {
            CFRelease(videoTextureCache);
            videoTextureCache = NULL;
        }
        
        /*
        if (corners)
        {
            corners = NULL;
        }*/
    }
    
    // OpenGL context.
    EAGLContext *context;
    
    // OpenGL Texture reference for y images.
    CVOpenGLESTextureRef lumaTexture;
    
    // OpenGL Texture reference for color images.
    CVOpenGLESTextureRef chromaTexture;
    
    // OpenGL Texture cache for the color camera.
    CVOpenGLESTextureCacheRef videoTextureCache;
    
    // Shader to render a GL texture as a simple quad.
    STGLTextureShaderYCbCr *yCbCrTextureShader;
    STGLTextureShaderRGBA *rgbaTextureShader;
    STGLTextureShaderGray *grayTextureShader;
    
    GLuint depthAsRgbaTexture;
    GLuint grayTexture;
    
    //uint8_t* cornerBuffer;

    // Renders the volume boundaries as a cube.
    //STCubeRenderer *cubeRenderer;
    
    // OpenGL viewport.
    GLfloat viewport[4];
    
    // OpenGL projection matrix for the color camera.
    GLKMatrix4 colorCameraGLProjectionMatrix = GLKMatrix4Identity;
    
    // OpenGL projection matrix for the depth camera.
    GLKMatrix4 depthCameraGLProjectionMatrix = GLKMatrix4Identity;
};

@interface ViewController : UIViewController <STBackgroundTaskDelegate, //MeshViewDelegate,
UIPopoverControllerDelegate, UIGestureRecognizerDelegate, NSURLSessionTaskDelegate, UITableViewDelegate, UITableViewDataSource, UIPickerViewDataSource, UIPickerViewDelegate>
{
    // Structure Sensor controller.
    STSensorController *_sensorController;
    STStreamConfig _structureStreamConfig;
    
    SlamData _slamState;
    
    Options _options;
    
    // Manages the app status messages.
    AppStatus _appStatus;
    
    DisplayData _display;
    
    // Most recent gravity vector from IMU.
    GLKVector3 _lastGravity;
    
    // Scale of the scanning volume.
    //PinchScaleState _volumeScale;
    
    // IMU handling.
    CMMotionManager *_motionManager;
    NSOperationQueue *_imuQueue;
    NSOperationQueue *_uploadQueue;
    NSOperationQueue *_verifyQueue;
    
    STDepthToRgba *_depthAsRgbaVisualizer;
    
    bool _useColorCamera;
    bool _renderDepthOverlay;
    
    FILE *g_imuFile;
    NSLock* g_imuLock;
    unsigned int g_numIMUmeasurements;
    
    FILE *g_cameraFile;
    NSLock* g_cameraLock;
    
    std::vector<NSTimeInterval> _colorTimestamps; // NSTimeInterval : double
    std::vector<NSTimeInterval> _depthTimestamps; // NSTimeInterval : double
    
    NSTimer *uploadErrorTimer;
    NSUInteger blinkCounter;
    
    
    NSMutableArray *myScanData;
    
    NSMutableArray *sceneTypes;
    
    NSString* specifiedSceneType;
    
    NSMutableArray *corners;
    
    //For Upload Task
    NSURLSession *defaultSession;
    
    int numPreviousCorners[12];
    int numPrevFramesTracked;
    
    NSArray *exposureLevels;
    
    NSString* exposureValue;
}

@property (weak, nonatomic) IBOutlet UIButton *toggleDepthButton;

@property (weak, nonatomic) IBOutlet UIButton *uploadButton;
@property (weak, nonatomic) IBOutlet UITextField *sceneLabelField;
@property (weak, nonatomic) IBOutlet UIView *sceneLabelFieldView;
@property (weak, nonatomic) IBOutlet UIProgressView *memProgressView;
@property (weak, nonatomic) IBOutlet UIView *memProgressFieldView;
@property (weak, nonatomic) IBOutlet UITextField *userNameField;


@property (weak, nonatomic) IBOutlet UIView *optionsView;
@property (weak, nonatomic) IBOutlet UISegmentedControl *colorResControl;
@property (nonatomic, retain) AVCaptureSession *avCaptureSession;
@property (nonatomic, retain) AVCaptureDevice *videoDevice;

@property (weak, nonatomic) IBOutlet UILabel *appStatusMessageLabel;
@property (weak, nonatomic) IBOutlet UIButton *scanButton;
@property (weak, nonatomic) IBOutlet UIButton *resetButton;
@property (weak, nonatomic) IBOutlet UIButton *doneButton;
@property (weak, nonatomic) IBOutlet UILabel *trackingLostLabel;

@property (weak, nonatomic) IBOutlet UIProgressView *progressView;
@property (weak, nonatomic) IBOutlet UILabel *uploadErrorIndicator;

@property (weak, nonatomic) IBOutlet UIButton *viewScans;
@property (weak, nonatomic) IBOutlet UITableView *exposureTable;


@property (nonatomic, retain) IBOutlet UITableView *tableView;
@property (weak, nonatomic) IBOutlet UILabel *tooCloseLabel;
@property (weak, nonatomic) IBOutlet UIProgressView *numCornersProgressView;

@property (weak, nonatomic) IBOutlet UIButton *changeExposure;
@property (weak, nonatomic) IBOutlet UILabel *AWBLabel;


//@property (weak, nonatomic) IBOutlet UILabel *percentTooClose;
@property (weak, nonatomic) IBOutlet UILabel *losingTrackingLabel;
@property (weak, nonatomic) IBOutlet UIPickerView *pickSceneType;
@property (weak, nonatomic) IBOutlet UILabel *uploadErrorDescription;

@property (weak, nonatomic) IBOutlet UISwitch *AWBSwitch;
- (IBAction)awbSwitchChanged:(id)sender;
- (IBAction)indexChanged:(UISegmentedControl *)sender;
//- (IBAction)enableNewTrackerSwitchChanged:(id)sender;
- (IBAction)scanButtonPressed:(id)sender;
- (IBAction)resetButtonPressed:(id)sender;
- (IBAction)doneButtonPressed:(id)sender;
- (IBAction)uploadButtonPressed:(id)sender;
- (IBAction)sceneLabelFieldEdited:(id)sender;
- (IBAction)toggleDepthButtonPressed:(id)sender;
- (IBAction)viewScansPressed:(id)sender;
- (IBAction)userNameFieldEdited:(id)sender;

- (IBAction)changeExposurePressed:(id)sender;


//- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section;

//- (UITableView *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath;

- (BOOL)isConnectedToWIFI;

- (void)enterSceneLabellingState;
- (void)enterCubePlacementState;
- (void)enterScanningState;
- (void)enterViewingState;
- (void)updateAppStatusMessage;
- (BOOL)currentStateNeedsSensor;
- (void)updateIdleTimer;
- (void)showTrackingMessage:(NSString*)message;
- (void)hideTrackingErrorMessage;

- (void)updateColorRes:(NSInteger)index;
- (void)loadOptions;
- (void)loadSceneLabel;

- (void)cleanUpAndReset;
- (void)blinkUploadErrorIndicator;

@end
