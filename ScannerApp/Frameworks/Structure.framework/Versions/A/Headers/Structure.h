/*
    This file is part of the Structure SDK.
    Copyright © 2015 Occipital, Inc. All rights reserved.
    http://structure.io
*/

#pragma once

#include <stdint.h>
#include <stdlib.h>
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <GLKit/GLKit.h>

# if !defined(__cplusplus) && !defined (HAS_LIBCXX)
#   error "Structure framework requires the C++ runtime. See Structure SDK Reference."
# endif

// Make sure the deployment target is higher or equal to iOS 8.
#if defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && (__IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_8_0)
#   error This version of Structure SDK only supports iOS 8 or higher.
#endif

//------------------------------------------------------------------------------
#pragma mark - Sensor Controller Types

/** Indicates the initialization status of the Structure Sensor.
 
See also:

- [STSensorController initializeSensorConnection]
*/
typedef NS_ENUM(NSInteger, STSensorControllerInitStatus)
{
    /** Indicates controller uninitialized because the sensor was not found. */
    STSensorControllerInitStatusSensorNotFound      = 0,
    /** Indicates controller initialization succeeded. */
    STSensorControllerInitStatusSuccess             = 1,
    /** Indicates controller was previously initialized. */
    STSensorControllerInitStatusAlreadyInitialized  = 2,
    /** Indicates controller uninitialized because sensor is waking up. */
    STSensorControllerInitStatusSensorIsWakingUp    = 3,
    /** Indicates controller uninitialized because of a failure to open the connection. */
    STSensorControllerInitStatusOpenFailed          = 4,
};

/** Streaming Interruption Reason
 
See also:
 
- [STSensorControllerDelegate sensorDidStopStreaming:]
*/
typedef NS_ENUM(NSInteger, STSensorControllerDidStopStreamingReason)
{
    /// Sensor stops streaming because of iOS app will resign active. This can occur when apps are sent to the background, during fast app switching, or when the notification/control center appears.
    STSensorControllerDidStopStreamingReasonAppWillResignActive = 0
};

/**
Constants indicating the Structure Sensor streaming configuration.

See also:

- [STSensorController startStreamingWithOptions:error:]
*/
typedef NS_ENUM(NSInteger, STStreamConfig)
{
    /// return -1 when the stream has not been calibrated
    STStreamConfigInvalid = -1,
    
    // 30 FPS modes
    
    /// QVGA depth at 30 FPS.
    STStreamConfigDepth320x240 = 0,

    /// QVGA depth at 30 FPS, aligned to the color camera.
    STStreamConfigRegisteredDepth320x240,

    /// QVGA depth and infrared at 30 FPS.
    STStreamConfigDepth320x240AndInfrared320x248,

    /// QVGA infrared at 30 FPS.
    STStreamConfigInfrared320x248,

    /// VGA depth at 30 FPS.
    STStreamConfigDepth640x480,

    /// VGA infrared at 30 FPS.
    STStreamConfigInfrared640x488,

    /// VGA depth and infrared at 30 FPS.
    STStreamConfigDepth640x480AndInfrared640x488,

    /// VGA depth at 30 FPS, aligned to the color camera.
    STStreamConfigRegisteredDepth640x480,
    
    /// QVGA depth at 60 FPS. Note: frame sync is not supported with this mode.
    STStreamConfigDepth320x240_60FPS
};

/** Frame Sync Configuration.

Constants indicating whether the driver should attempt to synchronize depth or infrared frames with color frames from AVFoundation.

When frame sync is active (i.e. **not** equal to `STFrameSyncOff`), one of the following methods is used in lieu of [STSensorControllerDelegate sensorDidOutputDepthFrame:], depending on the selected configuration:

- [STSensorControllerDelegate sensorDidOutputSynchronizedDepthFrame:andColorFrame:]
- [STSensorControllerDelegate sensorDidOutputSynchronizedInfraredFrame:andColorFrame:]

You must then repeatedly call frameSyncNewColorBuffer: from the AVFoundation video capture delegate methods. Otherwise, the sensor controller delegate methods will never deliver any frames. This is simply because synchronized frames cannot be delivered if there are no color frames to synchronize.

@note Frame sync of depth+infrared+RGB and 60 FPS depth are not currently supported.
@note For frame sync to be effective, the AVCaptureDevice must be configured to have a min and max frame rate of 30fps.
*/
typedef NS_ENUM(NSInteger, STFrameSyncConfig)
{
    /// Default mode, frame sync is off.
    STFrameSyncOff = 0,
    
    /// Frame sync between AVFoundation video frame and depth frame.
    STFrameSyncDepthAndRgb,
    
    /// Frame sync between AVFoundation video frame and infrared frame.
    STFrameSyncInfraredAndRgb
};

/** Sensor Calibration Type

Calibration types indicate whether a Structure Sensor + iOS device combination has no calibration, approximate calibration, or a device specific calibration from Calibrator.app.
 
See also:

- [STSensorController calibrationType]
 */
typedef NS_ENUM(NSInteger, STCalibrationType)
{
    /// There is no calibration for Structure Sensor + iOS device combination.
    STCalibrationTypeNone = 0,
    
    /// There exists an approximate calibration Structure Sensor + iOS device combination.
    STCalibrationTypeApproximate,
    
    /// There exists a device specific calibration from Calibrator.app of this Structure Sensor + iOS device combination.
    STCalibrationTypeDeviceSpecific,
};

//------------------------------------------------------------------------------

// Dictionary keys for `[STSensorController startStreamingWithOptions:error:]`.
extern NSString* const kSTStreamConfigKey;
extern NSString* const kSTFrameSyncConfigKey;
extern NSString* const kSTHoleFilterConfigKey;
extern NSString* const kSTHighGainConfigKey;
extern NSString* const kSTColorCameraFixedLensPositionKey;

//------------------------------------------------------------------------------
# pragma mark - ST Frame types

/** Color Frame
Formatted color data for the color image captured
*/
@class STColorFrame;

/** Depth Frame.
 
 STDepthFrame is the raw depth frame object for frames streaming from Structure Sensor.
 */
@interface STDepthFrame : NSObject

/// Frame width.
@property (readonly, nonatomic) int width;

/// Frame height.
@property (readonly, nonatomic) int height;

/// Contiguous chunk of `width * height` 16bit shift values.
@property (readonly, nonatomic) uint16_t *shiftData;

/// Contiguous chunk of `width * height` float depth values in millimeters.
@property (readonly, nonatomic) float *depthInMillimeters;

/// Return a version of the depth frame downsampled once.
@property (readonly, nonatomic) STDepthFrame *halfResolutionDepthFrame;

/// Capture timestamp in seconds since the iOS device boot (same clock as CoreMotion and AVCapture).
@property (readonly, nonatomic) NSTimeInterval timestamp;

/** Return a version of the depth frame aligned to the color camera viewpoint.
@param colorFrame By providing an STColorFrame a corresponding STDepthFrame will be returned
@warning Do not use the returned image after the original image has been released.
 */
- (STDepthFrame *)registeredToColorFrame:(STColorFrame*)colorFrame;

/** OpenGL projection matrix representing Structure Sensor virtual camera.
 
 This matrix can be used to render a scene by simulating the same camera properties as the Structure Sensor depth camera.
 
 @return A projection matrix.
 */
- (GLKMatrix4)glProjectionMatrix;

/**
 Get the rigid body transformation (RBT) between the iOS color camera and the depth image viewpoint.
 
 When using an un-registered mode, this transform is the same as [STSensorController colorCameraPoseInSensorCoordinateFrame:]. However, when using a registered depth mode, the depth frame is already aligned to the color camera viewpoint and thus this transform is identity.
 
 The RBT represents the world motion of the RGB camera w.r.t. the Structure Sensor stream viewpoint. The coordinate frame is right handed: X right, Y down, Z out. Equivalently, this matrix can transform a 3D point expressed in the iOS color camera coordinate system to the Structure Sensor depth stream coordinate system.
 
 @param matrix4x4 This output parameter should be a pointer to 16 floating point values in _column_ major order. This is the default ordering of GLKMatrix4.
 */
- (void)colorCameraPoseInDepthCoordinateFrame:(float *)matrix4x4;

@end

/** Infrared Frame.
 
 STInfraredFrame is the raw infrared frame object for frames streaming from Structure Sensor.
 */
@interface STInfraredFrame : NSObject

/// Contiguous chunk of `width * height` pixels.
@property (readwrite, nonatomic) uint16_t *data;

/// Frame width.
@property (readwrite, nonatomic) int width;

/// Frame height.
@property (readwrite, nonatomic) int height;

/** Capture timestamp in seconds since the iOS device boot (same clock as CoreMotion and AVCapture). */
@property (readwrite, nonatomic) NSTimeInterval timestamp;

@end

/** Color Frame.
 
 STColorFrame represents a frame from a color camera, captured from the device which Structure Sensor is attached to.
 */
@interface STColorFrame : NSObject

/// The buffer that AVFoundation created to store the raw image data.
@property (nonatomic, readonly) CMSampleBufferRef sampleBuffer;

/// Frame width.
@property (readwrite, nonatomic) int width;

/// Frame height.
@property (readwrite, nonatomic) int height;

/** Capture timestamp in seconds since the iOS device boot (same clock as CoreMotion and AVCapture). */
@property (readwrite, nonatomic) NSTimeInterval timestamp;

/** Return a version of the color frame downsampled once. */
@property (readonly, nonatomic) STColorFrame* halfResolutionColorFrame;

/** OpenGL projection matrix representing an iOS virtual color camera.
 
 This matrix can be used to render a scene by simulating the same camera properties as the iOS color camera.
 
 @return A projection matrix.
 */
- (GLKMatrix4)glProjectionMatrix;

@end

//------------------------------------------------------------------------------
# pragma mark - Sensor Controller Delegate

/** Sensor Controller Delegate
 
 The interface that your application-specific class must implement in order to receive sensor controller callbacks.
 
 @warning When creating a new application implementing a sensor controller delegate, the main `Info.plist` needs to contain an additional key "`Supported external accessory protocols`", with the following array of values:
 
 - `io.structure.control`
 - `io.structure.depth`
 - `io.structure.infrared`
 
 Without this modification to the plist, the app will not be able to connect to the sensor. All sample apps have this key/value array.
 
 See also <[STSensorController sharedController]> & <[STSensorController delegate]>.
 
 @note Delegate Registration Example
 
 [ STSensorController sharedController ].delegate = self;
 */
@protocol STSensorControllerDelegate <NSObject>

/// @name Connection Status

/// Notifies the delegate that the controller established a successful connection to the sensor.
- (void)sensorDidConnect;

/// Notifies the delegate that the sensor was disconnected from the controller.
- (void)sensorDidDisconnect;

/** Notifies the delegate that the sensor stopped streaming frames to the controller.

@param reason The reason why the stream was stopped. See: STSensorControllerDidStopStreamingReason.
*/
- (void)sensorDidStopStreaming:(STSensorControllerDidStopStreamingReason)reason;

/// @name Power Management

/// Notifies the delegate that the sensor has left low power mode.
- (void)sensorDidLeaveLowPowerMode;

/// Notifies the delegate that the sensor needs charging.
- (void)sensorBatteryNeedsCharging;

@optional

/// @name Colorless Frames

/** Notifies the delegate that the sensor made a new depth frame available to the controller. If the data is needed beyond the scope of this method, the depthFrame or its data must be copied by the receiver, e.g. using [STDepthFrame copy].

@param depthFrame The new depth frame.
*/
- (void)sensorDidOutputDepthFrame:(STDepthFrame *)depthFrame;

/** Notifies the delegate that the sensor made a new infrared frame available to the controller. If the data is needed beyond the scope of this method, the irFrame and its data must be copied by the receiver.

@param irFrame The new infrared frame.
*/
- (void)sensorDidOutputInfraredFrame:(STInfraredFrame *)irFrame;

/// @name Color-synchronized Frames

/** Notifies the delegate that the sensor made a new pair of depth and color frames available to the controller.

Frame sync methods will only be used if `kSTFrameSyncConfigKey` has been configured in [STSensorController startStreamingWithOptions:error:] to sync frames .
Also, data will only be delivered if frameSyncNewColorBuffer: is called every time a new sample buffer is available. The driver needs the CMSampleBuffers in order to return them through these methods.
If the STDepthFrame is needed beyond the scope of this function, the data must be copied by the receiver, e.g. using [STDepthFrame copy].
 
See also:

- <[STSensorController startStreamingWithOptions:error:]>
- <[STSensorController frameSyncNewColorBuffer:]>

@param depthFrame The new STDepthFrame depth frame.
@param colorFrame The new STColorFrame color buffer.
*/
- (void)sensorDidOutputSynchronizedDepthFrame:(STDepthFrame *)depthFrame
                               andColorFrame:(STColorFrame *)colorFrame;

/** Notifies the delegate that the sensor made a new pair of synchronized infrared and color frames available to the controller.

Frame sync methods will only be used if `kSTFrameSyncConfigKey` has been configured in [STSensorController startStreamingWithOptions:error:] to sync frames .
Also, data will only be delivered if frameSyncNewColorBuffer: is called every time a new sample buffer is available. The driver needs the CMSampleBuffers in order to return them through these methods.
If the STInfraredFrame is needed beyond the scope of this function, the data must be copied by the receiver.

See also:

- <[STSensorController startStreamingWithOptions:error:]>
- <[STSensorController frameSyncNewColorBuffer:]>

@param irFrame The new STInfraredFrame infrared frame.
@param colorFrame The STColorFrame new color buffer.
*/
- (void)sensorDidOutputSynchronizedInfraredFrame:(STInfraredFrame *)irFrame
                                  andColorFrame:(STColorFrame *)colorFrame;

/// @name Power Management

/// Notifies the delegate that the sensor has entered low power mode, this currently does nothing and is reserved for future usage.
- (void)sensorDidEnterLowPowerMode;

@end

//------------------------------------------------------------------------------
# pragma mark - Sensor Controller

/** The sensor controller is the central point that manages all the interactions between the sensor and your application-specific delegate class.

Its only instance is available through the sharedController method.

Your custom delegate object can be registered using its delegate property.

See also:

- <STSensorControllerDelegate>
*/
@interface STSensorController : NSObject

/// @name Controller Setup

/**
The STSensorController singleton.

Use it to register your application-specific STSensorControllerDelegate delegate.
*/
+ (STSensorController *)sharedController;

/**
The STSensorControllerDelegate delegate.

It will receive all notifications from the sensor, as well as raw stream data.
*/
@property(nonatomic, assign) id<STSensorControllerDelegate> delegate;

/**
Attempt to connect to the Structure Sensor.

@return Connection has succeeded only if the STSensorControllerInitStatus return value is either one of:

- STSensorControllerInitStatusSuccess
- STSensorControllerInitStatusAlreadyInitialized

@note Many methods (including startStreamingWithOptions:error:) will have no effect until this method succeeds at initializing the sensor.
*/
- (STSensorControllerInitStatus)initializeSensorConnection;

/**
This will begin streaming data from the sensor and delivering data using the delegate methods.

Here is some sample code to get registered QVGA depth with depth/color frame sync:

    NSError* error;
    BOOL success = [sensorController startStreamingWithOptions:@{kSTStreamConfigKey: @(STStreamConfigRegisteredDepth320x240),
                                                                 kSTFrameSyncConfigKey: @(STFrameSyncDepthAndRgb)} 
                                                         error:&error];

Here is some sample code to get VGA infrared images with high gain:

    NSError* error;
    BOOL success = [sensorController startStreamingWithOptions:@{kSTStreamConfigKey: @(STStreamConfigInfrared640x488),
                                                                 kSTHighGainConfigKey: @YES}
                                                         error:&error];
 
@param options Specifies the sensor configuration to use. The valid dictionary keys are:

- `kSTStreamConfigKey`: STStreamConfig value to specify the desired streaming configuration. Required.
- `kSTFrameSyncConfigKey`: STFrameSyncConfig value to configure frame sync with color frames from AVFoundation. Default is `STFrameSyncOff`.
- `kSTHoleFilterConfigKey`: boolean value to enable hole filtering. Default is `@YES` if the stream configuration includes depth. The depth filter enables a dilation of depth values that has the effect of filling holes. Note: setting it to `@YES` if the stream configuration does not include depth is invalid.
- `kSTHighGainConfigKey`: boolean value to enable high gain mode. Default is `@NO`. When set to `@YES` the sensor gain will be increased, causing better performance in dark or far away objects at the expense of some bright nearby objects. See also: setHighGainEnabled:.
- `kSTColorCameraFixedLensPositionKey`: float value to specify the lens position of the color camera. Required when using a registered depth mode. Note that the focus of the color camera has to remain fixed to this value when using a registered depth mode.

@param error will contain detailed information if the provided options are incorrect.

@return `TRUE` if the streaming options combination is valid, `FALSE` otherwise, filling in `error` with a detailed message.
*/
- (BOOL)startStreamingWithOptions:(NSDictionary *)options error:(NSError * __autoreleasing *)error;

/**
Stop streaming data from the sensor.

After this method is called, pending frames may still be delivered.
*/
- (void)stopStreaming;

/** Give the driver a color frame that will be used to synchronize shutters between the iOS camera and the Structure Sensor camera.

When receiving the CMSampleBufferRef from AVFoundation, you should only call this one method and do no other processing.
When a synchronized frame is found, one of the optional synchronized STSensorController delegate methods will be called, at which point you can then process/render the sampleBuffer.
@param sampleBuffer The new color buffer
*/
- (void)frameSyncNewColorBuffer:(CMSampleBufferRef)sampleBuffer;

/// @name Sensor Status

/// Checks whether the controlled sensor is connected.
- (BOOL)isConnected;

/// Checks whether the controlled sensor is in low-power mode.
- (BOOL)isLowPower;

/// Returns an integer in 0..100 representing the battery charge.
- (int)getBatteryChargePercentage;

/// @name Sensor Information

/// Returns the name of the controlled sensor, or nil if no sensor is connected.
- (NSString *)getName;

/// Returns the serial number of the controlled sensor, or nil if no sensor is connected.
- (NSString *)getSerialNumber;

/// Returns the firmware revision of the controlled sensor, or nil if no sensor is connected.
- (NSString *)getFirmwareRevision;

/// Returns the hardware revision of the controlled sensor, or nil if no sensor is connected.
- (NSString *)getHardwareRevision;

/** Launches Calibrator.app or prompts the user to install it.
 
 An option should be presented to the user when the sensor doesn't have a calibrationType with value 
 `CalibrationTypeDeviceSpecific`, the iOS device is supported by the Calibrator app, and registered depth is needed.
*/
+ (BOOL)launchCalibratorAppOrGoToAppStore;

/** Returns a boolean indicating whether an (at least) approximate depth-color calibration will be available when the sensor is connected to the current device.

This can be used to decide whether it is relevant to show color-specific UI elements before a sensor is connected.
 */
+ (BOOL)approximateCalibrationGuaranteedForDevice;

/** Returns the type of the current depth-color calibration as a STCalibrationType enumeration value.

This value can change depending on the actual device-sensor combination in use.
*/
- (STCalibrationType)calibrationType;

/// @name Advanced Setup

/** Enable or disable high sensor gain after the stream was started.

This method can dynamically overwrite the `kSTHighGainConfigKey` option specified when startStreamingWithOptions:error: was called.
 
@param enabled When set to `YES`, the sensor gain will be increased, causing better performance in dark or far away objects at the expense of some bright nearby objects.
*/
- (void)setHighGainEnabled:(BOOL)enabled;

/** Retrieve the current 4x4 transformation in _column_ major order between the iOS color camera and the Structure Sensor camera.

@param matrix4x4 This output parameter should be a pointer to 16 floating point values in _column_ major order. This is the default ordering of GLKMatrix4.
*/
- (void)colorCameraPoseInSensorCoordinateFrame:(float *)matrix4x4;

@end

//------------------------------------------------------------------------------
# pragma mark - STGLTextureShaderRGBA

/// Helper class to render a simple OpenGL ES quad with 2D texture in RGBA format.
@interface STGLTextureShaderRGBA : NSObject

/// Enable the underlying shader program.
- (void)useShaderProgram;

/// Render the texture on a fullscreen quad using the given GL_TEXTUREX unit.
/// @param textureUnit A given native OpenGL texture unit to render.
- (void)renderTexture:(GLint)textureUnit;

@end

//------------------------------------------------------------------------------
# pragma mark - STGLTextureShaderYCbCr

/// Helper class to render a simple OpenGL ES quad with a 2D texture in YCbCr format.
@interface STGLTextureShaderYCbCr : NSObject

/// Enable the underlying shader program.
- (void)useShaderProgram;

/// Render the texture on a fullscreen quad using the given GL_TEXTUREX unit.
/// @param lumaTextureUnit A given native OpenGL luminance texture unit to render.
/// @param chromaTextureUnit A given native OpenGL chroma texture unit to render.
- (void)renderWithLumaTexture:(GLint)lumaTextureUnit chromaTexture:(GLint)chromaTextureUnit;

@end

//------------------------------------------------------------------------------
# pragma mark - STGLTextureShaderGray

/// Helper class to render a simple OpenGL ES quad with a 2D single channel texture.
@interface STGLTextureShaderGray : NSObject

/// Enable the underlying shader program.
- (void)useShaderProgram;

/// Render the texture on a fullscreen quad using the given GL_TEXTUREX unit.
/// @param lumaTextureUnit A given native OpenGL texture unit to render.
- (void)renderWithLumaTexture:(GLint)lumaTextureUnit;

@end

//------------------------------------------------------------------------------
# pragma mark - STDepthToRgba

/// Required dictionary key to specify the converter strategy.
extern NSString* const kSTDepthToRgbaStrategyKey;

/**
Constants indicating the depth to color conversion strategy of STDepthToRgba.

See also:
 
- [STDepthToRgba initWithOptions:error:]
*/
typedef NS_ENUM(NSInteger, STDepthToRgbaStrategy)
{
    /// Linear mapping using a color gradient – pure red encodes the minimal depth, and pure blue encodes the farthest possible depth.
    STDepthToRgbaStrategyRedToBlueGradient = 0,
    
    /// Linear mapping from closest to farthest depth as a grayscale intensity.
    STDepthToRgbaStrategyGray,
};

/// Helper class to convert float depth data to RGB values for better visualization.
@interface STDepthToRgba : NSObject

/// Pointer to the RGBA values.
@property (nonatomic, readonly) uint8_t *rgbaBuffer;

/// Width of the output RGBA image.
@property (nonatomic, readonly) int width;

/// Height of the output RGBA image.
@property (nonatomic, readonly) int height;

/** Initialize with required parameters.
 @param options The option dictionary. The valid keys are:
 
 - `kSTDepthToRgbaStrategyKey`: STDepthToRgbaStrategy value to specify the conversion strategy to use. Required.
 
 @param error will contain detailed information if the provided options are incorrect.
 */
- (instancetype)initWithOptions:(NSDictionary *)options error:(NSError* __autoreleasing *)error;


/// Convert the given depth frame to RGBA. The returned pointer is the same as the one accessed through `rgbaBuffer`.
/// @param frame The depth frame.
- (uint8_t *)convertDepthFrameToRgba:(STDepthFrame *)frame;

@end

//------------------------------------------------------------------------------
# pragma mark - STWirelessLog

/**
Wireless logging utility.

This class gives the ability to wirelessly send log messages to a remote console.

It is very useful when the sensor is occupying the lightning port.
*/
@interface STWirelessLog : NSObject

/** 
Redirects the standard and error outputs to a TCP connection.

Messages sent to the `stdout` and `stderr` (such as `NSLog`, `std::cout`, `std::cerr`, `printf`) will be sent to the given IPv4 address on the specified port.

In order to receive these messages on a remote machine, you can, for instance, use the *netcat* command-line utility (available by default on Mac OS X). Simply run in a terminal: `nc -lk <port>`

@note If the connection fails, the returned error will be non-`nil` and no output will be transmitted.
 
@note Only one connection can be active.
@param ipv4Address The IPv4 address of the remote console.
@param port The port of the remote console.
@param error will contain detailed information if the provided options are incorrect.
*/
+ (void)broadcastLogsToWirelessConsoleAtAddress:(NSString *)ipv4Address usingPort:(int)port error:(NSError **)error ;

@end

//------------------------------------------------------------------------------
# pragma mark - STBackgroundTaskDelegate

/**
The interface that your class can implement to receive STBackgroundTask callbacks.

See also:

- <[STBackgroundTask delegate]>
*/

@class STBackgroundTask;

@protocol STBackgroundTaskDelegate <NSObject>
@optional

/** Reports progress in the background task.
@param sender The STBackgroundTask that reports the progress.
@param progress is a floating-point value from 0.0 (Not Started) to 1.0 (Complete).
*/
- (void)backgroundTask:(STBackgroundTask*)sender didUpdateProgress:(double)progress;

@end

/**
Handle to control a task running asynchronously in a background queue.

See also:

- <[STMesh newDecimateTaskWithMesh:numFaces:completionHandler:]>
- <[STMesh newFillHolesTaskWithMesh:completionHandler:]>
- <[STColorizer newColorizeTaskWithMesh:scene:keyframes:completionHandler:options:error:]>
- STBackgroundTaskDelegate
*/

//------------------------------------------------------------------------------
# pragma mark - STBackgroundTask

@interface STBackgroundTask : NSObject

/// Start the execution of the task asynchronously, in a background queue.
- (void)start;

/**
Cancels the background task if possible.
 
@note If the operation is already near completion, the completion handler may still be called successfully.
*/
- (void)cancel;

/// Synchronously wait until the task finishes its execution.
- (void)waitUntilCompletion;

/// Whether the task was canceled. You can check this in the completion handler to make sure the task was not canceled right after it finished.
@property(atomic, readonly) BOOL isCancelled;

/// By setting a STBackgroundTaskDelegate delegate to an STBackgroundTask, you can receive progress updates.
@property(atomic, assign) id<STBackgroundTaskDelegate> delegate;

@end

//------------------------------------------------------------------------------
# pragma mark - STErrorCode

/// Constant to identify the Structure Framework error domain.
extern NSString* const StructureSDKErrorDomain;

/** Error codes that specify an error. These may appear in NSError objects returned by various Structure Framework methods.

`NSString* const StructureSDKErrorDomain;`

is the constant to identify the Structure Framework error domain.
*/
typedef NS_ENUM(NSInteger, STErrorCode)
{
    /// The operation could not be completed because one or more keys in the options dictionary were not recognized.
    STErrorOptionNotRecognized                             = 0,
    
    /// The operation could not be completed because one or more values in the options dictionary were invalid.
    STErrorOptionInvalidValue                              = 1,
    
    /// The operation could not be completed because one or more required values in the options dictionary were missing.
    STErrorOptionMissingValue                              = 2,
    
    /// The operation could not be completed because the options dictionary contained one or more non-dynamic properties that could not be updated.
    STErrorOptionCannotBeUpdated                           = 3,
    
    /// The operation could not be completed because parameters passed to the SDK method contains non-valid values.
    STErrorInvalidValue                                    = 10,
    
    /// STCameraPoseInitializer tried to initialize a camera pose using without a depth frame.
    STErrorCameraPoseInitializerDepthFrameMissing          = 20,
    
    /// No such file was found.
    STErrorFileNoSuchFile                                  = 30,
    
    /// The operation could not be completed because file output path is invalid.
    STErrorFileWriteInvalidFileName                        = 31,
    
    /// `STTracker` lost tracking.
    STErrorTrackerLostTrack                                = 40,
    
    /// `STTracker` is not initialized yet.
    STErrorTrackerNotInitialized                           = 41,
    
    /// `STTracker` doesn't support the input color sample buffer format.
    STErrorTrackerColorSampleBufferFormatNotSupported      = 42,        // Deprecated, throws Exception instead.
    
    /// `STTracker` doesn't have color sample buffer and cannot continue tracking.
    STErrorTrackerColorSampleBufferMissing                 = 43,        // Deprecated, throws Exception instead.
    
    /// `STTracker` detected the color sample buffer exposure has changed.
    STErrorTrackerColorExposureTimeChanged                 = 44,
    
    /// `STTracker` doesn't have device motion and cannot continue tracking.
    STErrorTrackerDeviceMotionMissing                      = 45,
    
    /// `STTracker` doesn't have live triangle mesh to proceed tracking against model. Make sure `setLiveTriangleMeshEnabled` is enabled.
    STErrorTrackerTrackAgainstModelWithoutLiveTriangleMesh = 46,

    /// `STTracker` could not return a high quality camera pose estimate.
    STErrorTrackerPoorQuality                              = 47,

    /// The STMesh operation could not be completed because it contains an empty mesh.
    STErrorMeshEmpty                                       = 60,
    
    /// The STMesh operation could not be completed because it was cancelled.
    STErrorMeshTaskCancelled                               = 61,
    
    /// The STMesh operation could not be completed because it contains an invalid texture format.
    STErrorMeshInvalidTextureFormat                        = 62,
    
    /// The colorize operation could not be completed because `STColorizer` doesn't have keyframes.
    STErrorColorizerNoKeyframes                            = 80,
    
    /// The colorize operation could not be completed because `STColorizer` doesn't have a mesh.
    STErrorColorizerEmptyMesh                              = 81,
};


//------------------------------------------------------------------------------
