/*
    This file is part of the Structure SDK.
    Copyright © 2015 Occipital, Inc. All rights reserved.
    http://structure.io
*/

#pragma once

#import <Structure/Structure.h>
#import <CoreMotion/CoreMotion.h>
#import <GLKit/GLKit.h>

#pragma mark - STMesh

// Dictionary key to specify writeToFile output file format. See STMeshWriteOptionFileFormat.
extern NSString* const kSTMeshWriteOptionFileFormatKey;

/**

Constants indicating a file format to store an STMesh on disk.

See also:

- [STMesh writeToFile:options:error:]
*/
typedef NS_ENUM(NSInteger, STMeshWriteOptionFileFormat)
{
    /** Wavefront OBJ format. If the mesh has a texture, an MTL file will also be generated, and the texture saved to a JPEG file.
    Note: Filenames with spaces are not supported by OBJ and will return an error.
    */
    STMeshWriteOptionFileFormatObjFile     = 0,
    
    /// Wavefront OBJ format, compressed into a ZIP file. The archive will also embed the MTL and JPEG file if the mesh has a texture.
    STMeshWriteOptionFileFormatObjFileZip  = 1
};

/** Reference to face-vertex triangle mesh data.

Stores mesh data as a collection of vertices and faces. STMesh objects are references, and access to the underlying data should be protected by locks in case multiple threads may be accessing it.

Since OpenGL ES only supports 16 bits unsigned short for face indices, meshes larges than 65535 faces have to be split into smaller submeshes. STMesh is therefore a reference to a collection of partial meshes, each of them having less than 65k faces.
*/
@interface STMesh : NSObject

/// Number of partial meshes.
- (int)numberOfMeshes;

/// Number of faces of a given submesh.
/// @param meshIndex Index to the partial mesh.
- (int)numberOfMeshFaces:(int)meshIndex;

/// Number of vertices of a given submesh.
/// @param meshIndex Index to the partial mesh.
- (int)numberOfMeshVertices:(int)meshIndex;

/// Number of lines (edges) of a given submesh.
/// @param meshIndex Index to the partial mesh.
- (int)numberOfMeshLines:(int)meshIndex;

/// Checks whether per-vertex normals are available
- (BOOL)hasPerVertexNormals;

/// Checks whether per-vertex colors are available
- (BOOL)hasPerVertexColors;

/// Checks whether per-vertex UV texture coordinates are available
- (BOOL)hasPerVertexUVTextureCoords;

/// Pointer to a contiguous chunk of `numberOfMeshVertices:meshIndex` `GLKVector3` values representing vertex coordinates (x, y, z).
/// @param meshIndex Index to the partial mesh.
- (GLKVector3 *)meshVertices:(int)meshIndex;

/**
Pointer to a contiguous chunk of `numberOfMeshVertices:meshIndex` `GLKVector3` values representing per-vertex normals (nx, ny, nz).
@note Returns `NULL` is there are no per-vertex normals.
@param meshIndex Index to the partial mesh.
*/
- (GLKVector3 *)meshPerVertexNormals:(int)meshIndex;

/**
Pointer to a contiguous chunk of `numberOfMeshVertices:meshIndex` `GLKVector3` values representing vertices colors (r, g, b).
@note Returns `NULL` is there are no per-vertex colors.
@param meshIndex Index to the partial mesh.
*/
- (GLKVector3 *)meshPerVertexColors:(int)meshIndex;

/**
Pointer to a contiguous chunk of `numberOfMeshVertices:meshIndex` `GLKVector2` values representing normalized texture coordinates (u, v).
@note Returns `NULL` is there are no per-vertex texture coordinates.
@param meshIndex Index to the partial mesh.
*/
- (GLKVector2 *)meshPerVertexUVTextureCoords:(int)meshIndex;

/// Pointer to a contiguous chunk of `(3 * numberOfMeshFaces:meshIndex)` 16 bits `unsigned short` values representing vertex indices. Each face is represented by three vertex indices.
/// @param meshIndex Index to the partial mesh.
- (unsigned short *)meshFaces:(int)meshIndex;

/// Optional texture associated to the mesh. The pixel buffer is encoded using `kCVPixelFormatType_420YpCbCr8BiPlanarFullRange`.
- (CVPixelBufferRef)meshYCbCrTexture;

/** 
Pointer to a contiguous chunk of `(2 * numberOfMeshLines:meshIndex)` 16 bits `unsigned short` values representing vertex indices. Each line is represented by two vertex indices. These lines can be used for wireframe rendering, using GL_LINES.
@param meshIndex Index to the partial mesh.
*/
- (unsigned short *)meshLines:(int)meshIndex;

/**
Intersect the mesh with a ray specified by the origin and end points. If TRUE is returned, `intersection` will contain the first (x, y, z) coordinate on the mesh that the ray intersects when traveling from the origin to the end.
@return TRUE if there is an intersection, FALSE otherwise
@param origin The origin of ray.
@param end The end of ray.
@param intersection The intersection point to the mesh if intersection happens.
*/
- (BOOL)intersectWithRayOrigin:(GLKVector3)origin rayEnd:(GLKVector3)end intersection:(GLKVector3 *)intersection;

/** Saves the STMesh to a file

    //Sample usage
    NSError* error;
    [myMesh writeToFile:@"/path/to/mesh.obj" 
                options:@{kSTMeshWriteOptionFileFormatKey: STMeshWriteOptionFileFormatObjFile} 
                  error:&error];

@param filePath Path to output file.
@param options Dictionary of options. The valid keys are:

- `kSTMeshWriteOptionFileFormatKey`: STMeshWriteOptionFileFormat value to specify the output file format. Required. 

@param error will contain detailed information if the provided options are incorrect.

*/
- (BOOL)writeToFile:(NSString *)filePath options:(NSDictionary *)options error:(NSError* __autoreleasing *)error;

/// Create a copy of the current mesh
/// @param mesh The mesh from which to copy.
- (instancetype)initWithMesh:(STMesh *)mesh;

/** This returns an asynchronous task to create a decimated low-poly version of the given mesh with a maximal target number of faces.

    //Sample usage
    __block STMesh* outputMesh;
    STBackgroundTask* task;
    task = [STMesh newDecimateTaskWithMesh:myMesh
                                  numFaces:2000
                         completionHandler:^(STMesh *result, NSError *error) { outputMesh = result; };
    [task start];
    [task waitUntilCompletion];

@note If the target number of faces is larger than the current mesh number of faces, no processing is done.
@param inputMesh Input mesh to decimate.
@param numFaces Target number of faces to decimate.
@param completionHandler Block to execute once the task is finished or cancelled.
*/
+ (STBackgroundTask*)newDecimateTaskWithMesh:(STMesh *)inputMesh numFaces:(unsigned int)numFaces completionHandler:(void(^)(STMesh *result, NSError *error))completionHandler;

/**
Returns an asynchronous task to create a version of the given mesh with holes filled.

@note This will also result in a smoother mesh, with non-manifold faces removed.
@param inputMesh Input mesh to fill holes.
@param completionHandler Block to execute once the task is finished or cancelled.
*/
+ (STBackgroundTask*)newFillHolesTaskWithMesh:(STMesh *)inputMesh completionHandler:(void(^)(STMesh *result, NSError *error))completionHandler;

@end

//------------------------------------------------------------------------------
# pragma mark - STScene

/** Common data shared and updated by the SLAM pipeline.

An STScene object contains information about the sensor and the reconstructed mesh.
Special care should be taken when accessing STScene members if an STTracker or STMapper is still active, as they could be accessing the STScene from background threads.
In particular, STMesh objects should be properly locked.
*/
@interface STScene : NSObject

/** Mandatory initializer for STScene.

@param glContext a valid EAGLContext.
@param textureUnit A GL_TEXTUREX unit which will be used when SLAM objects need to render meshes to an OpenGL texture.
*/
- (instancetype)initWithContext:(EAGLContext *)glContext
              freeGLTextureUnit:(GLenum)textureUnit;

/** Reference to the current scene mesh.

This mesh may be modified by a background thread if an instance of STMapper is running, so proper locking is necessary.
*/
- (STMesh *)lockAndGetSceneMesh;

/// Unlocks the mesh previously locked with `lockAndGetSceneMesh`.
- (void)unlockSceneMesh;

/** Render the scene mesh from the given viewpoint.

A virtual camera with the given projection and pose matrices will be used to render the mesh using OpenGL. This method is generally faster than using sceneMeshRef and manually rendering it, since in most cases STScene can reuse mesh data previously uploaded to the GPU.

@param cameraPose A GLKMatrix4 camera position used for rendering.
@param glProjection projection matrix used during rendering. See also [STDepthFrame glProjectionMatrix] and [STColorFrame glProjectionMatrix].
@param alpha A float value for transparency between 0 (fully transparent) and 1 (fully opaque)
@param highlightOutOfRangeDepth whether the areas of the mesh which are below Structure Sensor minimal depth range should be highlighted in red.
@param wireframe whether or not to render a wireframe of the mesh or filled polygons. If enabled, STMapper needs `liveWireframeMeshEnabled` to be set.
@return TRUE on success, FALSE if there is no STMapper attached to the same STScene with the corresponding (wireframe or triangles) live mode enabled.
*/
- (BOOL)renderMeshFromViewpoint:(GLKMatrix4)cameraPose
             cameraGLProjection:(GLKMatrix4)glProjection
                          alpha:(float)alpha
       highlightOutOfRangeDepth:(BOOL)highlightOutOfRangeDepth
                      wireframe:(BOOL)wireframe;

/// Clear the scene mesh and state.
- (void)clear;

@end

#pragma mark - STTracker option keys

/**

Constants indicating the tracking algorithm used by STTracker.

See also:
 
- [STTracker initWithScene:options:error:]

*/
typedef NS_ENUM(NSInteger, STTrackerType)
{
    /// Will only use the depth information from Structure Sensor. This tracker works best at close/mid-range, in combination with `kSTTrackerTrackAgainstModelKey`.
    STTrackerDepthBased = 0,
    
    /// Will use both the depth information from Structure Sensor and the color information from the iOS device camera. Only `kCVPixelFormatType_420YpCbCr8BiPlanarFullRange` with VGA resolution (640x480) is supported for color buffer format.
    STTrackerDepthAndColorBased,
};

/**

Constants indicating a tracking quality hint to STTracker.

See also:
 
- [STTracker initWithScene:options:error:]
- [STTracker setOptions:error:]
*/
typedef NS_ENUM(NSInteger, STTrackerQuality)
{
    /// Best during scanning, but it will also take more CPU resources.
    STTrackerQualityAccurate = 0,

    /// Designed for very fast tracking, it works best when tracking against a static mesh (e.g. after a scan has been done), or when the available CPU resources are limited.
    STTrackerQualityFast,
};

/**

Constants indicating the status of an STTracker.

See also:
 
- [STTracker status]
*/
typedef NS_ENUM(NSInteger, STTrackerStatus)
{
    /// Information not available, e.g. in case of complete track loss or before initialization.
    STTrackerStatusNotAvailable = -1,
    
    /// Everything went fine.
    STTrackerStatusGood = 0,
    
    /// The tracker is dodgy, with an unknown reason.
    STTrackerStatusDodgyForUnknownReason,
    
    /// The camera is moving fast or the camera is far from the model, making pose estimate less accurate.
    STTrackerStatusFastMotion,
    
    /// Structure Sensor is too close to the scene and maybe parts are thus out of range.
    STTrackerStatusTooClose,

    /// Structure Sensor is too far to the model, making the tracker less stable.
    STTrackerStatusTooFar,
    
    /// The tracker is being successful again after being lost. But it is still not yet totally confident about the estimates.
    STTrackerStatusRecovering,
    
    /// The tracker does not see the model anymore, so it is likely to drift.
    STTrackerStatusModelLost,
};

// Dictionary keys for [STTracker initWithScene: options: error:]
extern NSString* const kSTTrackerTypeKey;
extern NSString* const kSTTrackerQualityKey;
extern NSString* const kSTTrackerTrackAgainstModelKey;
extern NSString* const kSTTrackerAvoidPitchRollDriftKey;
extern NSString* const kSTTrackerAvoidHeightDriftKey;
extern NSString* const kSTTrackerAcceptVaryingColorExposureKey;
extern NSString* const kSTTrackerBackgroundProcessingEnabledKey;

//------------------------------------------------------------------------------
# pragma mark - STTracker

/** Track the 3D position of the Structure Sensor.

STTracker uses sensor information and optionally IMU data to estimate how the camera is being moved over time, in real-time.

See also:

- STTrackerQuality
- STTrackerStatus
- STTrackerType
*/
@interface STTracker : NSObject

/// STScene object storing common SLAM information.
@property (nonatomic, retain) STScene *scene;

/// The initial camera pose. Tracking will use this as the first frame pose.
@property (nonatomic) GLKMatrix4 initialCameraPose;

/** Recommended initializer since STTracker cannot be used until an STScene has been provided.

    //Sample usage
    NSDictionary* options = @{
        kSTTrackerTypeKey                       = @(STTrackerDepthBased),
        kSTTrackerQualityKey                    = @(STTrackerQualityAccurate),
        kSTTrackerTrackAgainstModelKey          = @TRUE,
        kSTTrackerAcceptVaryingColorExposureKey = @FALSE,
        kSTTrackerBackgroundProcessingEnabledKey= @TRUE
    };
    NSError* error = nil;
    STTracker* tracker = [[STTracker alloc] initWithScene:myScene options:options error:&error];

@param scene The STScene context.

@param options Dictionary of options. The valid keys are:

- `kSTTrackerTypeKey`: STTrackerType value to specify the tracking algorithm. Required.
- `kSTTrackerQualityKey`: STTrackerQuality value to specify a tracking quality hint. Default is `STTrackerQualityAccurate`.
- `kSTTrackerTrackAgainstModelKey`: boolean value to enable tracking against the model. Default is `@NO`. If enabled, the tracker will attempt to match the current image with the current state of the reconstructed model. This can drastically reduce the pose estimation drift.
__Note:__ this option requires an STMapper to be attached to the scene, with the `liveTriangleMeshEnabled` property set.
- `kSTTrackerAvoidPitchRollDriftKey`: boolean value to enable pitch and roll drift cancellation using IMU. Default is `@NO`. This can eliminate drift along these rotation axis, but can also result in lower short term accuracy. Recommended for unbounded tracking.
- `kSTTrackerAvoidHeightDriftKey`: boolean value to enable height drift cancellation using ground plane detection. Default is `@NO`. This can eliminate vertical drift, but can also result in lower short term accuracy. Recommended for unbounded tracking.
- `kSTTrackerBackgroundProcessingEnabledKey`: boolean value to enable background processing. This can significantly reduce the time spent in the tracker before getting a new pose, but the tracker may keep using CPU/GPU resources between frames.
- `kSTTrackerAcceptVaryingColorExposureKey`: boolean value to accept varying exposures for the iOS camera. Default is `@NO`. To ensure the optimal robustness, it is recommended to lock the iOS color camera exposure during tracking if the `STTrackerDepthAndColorBased` tracker type is being used. By default the tracker will return an error if it detects a change in the exposure settings, but it can be forced to accept it by enabling this option.

@param error will contain detailed information if the provided options are incorrect.
*/
- (instancetype)initWithScene:(STScene *)scene options:(NSDictionary*)options error:(NSError* __autoreleasing *)error;

/// Reset the tracker to its initial state.
- (void)reset;

/** Update the camera pose estimate using the given depth frame.

@return TRUE if success, FALSE otherwise, filling error with the explanation.
@note colorFrame can be nil if the tracker type is `STTrackerDepthBased`. For tracker type `STTrackerDepthAndColorBased`, only `kCVPixelFormatType_420YpCbCr8BiPlanarFullRange` is supported for sampleBuffer format.
@param depthFrame The STDepthFrame depth frame.
@param colorFrame The STColorFrame color buffer.
@param error On return, if it fails, points to an NSError describing the failure.
*/
- (BOOL)updateCameraPoseWithDepthFrame:(STDepthFrame *)depthFrame colorFrame:(STColorFrame*)colorFrame error:(NSError* __autoreleasing *)error;

/// Update the current pose estimates using the provided motion data.
/// @param motionData Provided motion data.
- (void)updateCameraPoseWithMotion:(CMDeviceMotion *)motionData;

/// Returns the most recent camera pose estimate.
- (GLKMatrix4)lastFrameCameraPose;

/// Returns the status after the last pose estimation.
- (STTrackerStatus)status;

/// Returns the best estimate of the camera pose at the given future timestamp.
/// @param timestamp Capture timestamp in seconds since the iOS device boot (same clock as CoreMotion and AVCapture).
- (GLKMatrix4)predictCameraPoseAtTimestamp:(NSTimeInterval)timestamp;

/** Dynamically adjust the tracker options. Currently, only `kSTTrackerQualityKey` and `kSTTrackerBackgroundProcessingEnabledKey` can be changed after initialization. Other options will remain unchanged.
@return TRUE if success, FALSE otherwise, filling error with the explanation.
@param options Dictionary of options. The valid keys are:

- `kSTTrackerQualityKey`: STTrackerQuality value to specify a tracking quality hint. Default is `STTrackerQualityAccurate`.
- `kSTTrackerBackgroundProcessingEnabledKey`: boolean value to enable background processing. This can significantly reduce the time spent in the tracker before getting a new pose, but the tracker may keep using CPU/GPU resources between frames.
@param error will contain detailed information if the provided options are incorrect.
*/
- (BOOL)setOptions:(NSDictionary *)options error:(NSError* __autoreleasing *)error;

@end

# pragma mark - STMapper

// Dictionary keys for [STMapper initWithScene: options:]
extern NSString* const kSTMapperVolumeResolutionKey;

/** 
Integrate sensor data to reconstruct a 3D model of a scene.

STMapper will update the scene mesh progressively as new depth frames are integrated.
It works in a background thread, which means that it may update the scene object at any time.

The mapper works over a fixed cuboid defining the volume of interest in the scene.
This volume can be initialized interactively using STCameraPoseInitializer.

The volume is defined by its size in the real world, in meters, and is discretized into cells.
The volume resolution specifies the number of cells. As a consequence, the maximal level of detail which can be obtained by STMapper is roughly determined by volumeSizeInMeters / volumeResolution.
In short, the bigger the volume size, the higher the resolution has to be to keep the same level of detail.
*/
@interface STMapper : NSObject

/// The STScene model which will be updated.
@property (nonatomic, retain) STScene *scene;

/// Whether the STMapper should automatically build a triangle mesh in background when new depth frames are provided. Default is FALSE.
@property (nonatomic) BOOL liveTriangleMeshEnabled;

/// Whether the STMapper should automatically build a wireframe mesh in background when new depth frames are provided. Default is FALSE.
@property (nonatomic) BOOL liveWireframeMeshEnabled;

/// Subsampling factor to apply when building a live triangle mesh. The minimal value is 1, corresponding to the maximal density. A value of N means that the triangle density will be divided by N, making processing faster. Default is 1.
@property (nonatomic) int liveTriangleMeshSubsamplingFactor;

/// Subsampling factor to apply when building a live wireframe mesh. The minimal value is 1, corresponding to the maximal density. A value of N means that the line density will be divided by N, making processing faster. Default is 2.
@property (nonatomic) int liveWireframeMeshSubsamplingFactor;

/// The rectangular cuboid size in meters.
@property (nonatomic) GLKVector3 volumeSizeInMeters;

/**
Number of cells for each dimension.

To keep the level of details isotropic, it is recommended to use a similar same aspect ratio as volumeSizeInMeters.
To keep mapping real-time, the recommended value is 128x128x128.

@note The volume resolution cannot be changed after initialization.
*/
@property (nonatomic, readonly) GLKVector3 volumeResolution;

/** Initialize with a given scene and volume resolution.
@param scene The STScene context.
@param options Dictionary of options. The valid keys are:

- `kSTMapperVolumeResolutionKey`: Specifies the volume resolution as an NSArray, e.g. @[@128, @128, @128].
*/
- (instancetype)initWithScene:(STScene *)scene
                      options:(NSDictionary*)options;

/**
Specify whether the volume cuboid has been initialized on top of a support plane.

If the mapper is aware that the volume is on top of a support plane, it will adapt the pipeline to be more robust.
@param hasIt YES to enable support plane adjustment, NO to disable it.
*/
- (void)setHasSupportPlane:(BOOL)hasIt;

/// Reset the mapper state. This will also stop any background processing.
- (void)reset;

/// Integrate a new depth frame to the model. The processing will be performed in a background queue, so this method is non-blocking.
/// @param depthFrame The depth Frame.
/// @param cameraPose the viewpoint to use for mapping.
- (void)integrateDepthFrame:(STDepthFrame *)depthFrame
                 cameraPose:(GLKMatrix4)cameraPose;

/**
Waits until ongoing processing in the background queue finishes, and build the final triangle mesh with the given density. If the same instance of STMapper is to be used after this call, the `reset` method should be called for both the STMapper and the STScene.

@param subsamplingLevel controls the mesh density. The minimal value is 1, corresponding to the maximal density. A value of N means that the triangle density will be divided by N. See also [STMapper liveTriangleMeshSubsamplingFactor].
 
@note If liveTriangleMeshEnabled was set and liveTriangleMeshSubsamplingFactor is equal to subsamplingLevel, this operation will be very fast.
*/
- (void)finalizeTriangleMeshWithSubsampling:(int)subsamplingLevel;

@end

//------------------------------------------------------------------------------
# pragma mark - STCameraPoseInitializer

/// Required dictionary key to specify the pose initialization strategy.
extern NSString* const kSTCameraPoseInitializerStrategyKey;

/** Camera Pose Initializer Strategy
 
Constants indicating a cube pose initialization strategy.

See also:
 
- [STCameraPoseInitializer initWithVolumeSizeInMeters:options:error:]
 
*/
typedef NS_ENUM(NSInteger, STCameraPoseInitializerStrategy)
{
    /** Tries to detect a ground plane and set the camera pose such that the cuboid scanning volume lies on top of it.
    If no ground plane is found or if the device is not looking downward, it will place the scanning volume at the distance given by the central depth pixels.
    In both cases, the cuboid orientation will be aligned with the provided gravity vector.
    __Note:__ This strategy requires depth information from the Structure Sensor.
    */
    STCameraPoseInitializerStrategyTableTopCube = 0,
    
    /** Aligns the camera orientation using the gravity vector, leaving the translation component to (0,0,0).
    __Note:__ This strategy does not require depth information.
    */
    STCameraPoseInitializerStrategyGravityAlignedAtOrigin,
    
    /** Aligns the camera orientation using the gravity vector, and places the camera center at the center of the scanning volume.
    __Note:__ This strategy does not require depth information.
    */
    STCameraPoseInitializerStrategyGravityAlignedAtVolumeCenter,
    
};

/// Determine an initial camera pose to make the best use of the cuboid scanning volume.
@interface STCameraPoseInitializer : NSObject

/// Width, height and depth of the volume cuboid.
@property (nonatomic) GLKVector3 volumeSizeInMeters;

/// Most recent estimated camera pose, taking Structure Sensor as a reference.
@property (nonatomic, readonly) GLKMatrix4 cameraPose;

/// Whether the pose initializer could find a good pose.
@property (nonatomic, readonly) BOOL hasValidPose;

/// Whether the last cube placement was made with a supporting plane. Useful for STMapper.
@property (nonatomic, readonly) BOOL hasSupportPlane;

/**
Initialize with all the required fields.
@param volumeSize The current volume size in meters. Not used by the `STCameraPoseInitializerStrategyGravityAlignedAtOrigin` strategy.
@param options The option dictionary. The valid keys are:

- `kSTCameraPoseInitializerStrategyKey`: STCameraPoseInitializerStrategy value to specify the estimation strategy. Required.

@param error will contain detailed information if the provided options are incorrect.
*/
- (instancetype)initWithVolumeSizeInMeters:(GLKVector3)volumeSize
                                   options:(NSDictionary *)options
                                     error:(NSError* __autoreleasing *)error;

/**
Update the current pose estimate from a depth frame and a CoreMotion gravity vector, using the strategy specified at init.

@param gravity a gravity vector in IMU coordinates, e.g. as provided by CMDeviceMotion.gravity.
@param depthFrame the current processed depth from Structure Sensor. Can be `nil` if using the strategies `STCameraPoseInitializerStrategyGravityAlignedAtOrigin` or `STCameraPoseInitializerStrategyGravityAlignedAtVolumeCenter`.
@param error will contain detailed information if the estimation failed.
@return TRUE on success, FALSE on failure.
*/
- (BOOL)updateCameraPoseWithGravity:(GLKVector3)gravity depthFrame:(STDepthFrame *)depthFrame error:(NSError* __autoreleasing *)error;

/**
Determine which pixels of a depth frame are inside the current scanning volume.
 
@param depthFrame the current processed depth from Structure Sensor.
@param outputMask should point to an allocated buffer of (`depthFrame.width * depthFrame.height`) pixels. A mask pixel will be 255 if the corresponding 3D point fits inside the volume, 0 if outside or if there is no depth.
*/
- (void)detectInnerPixelsWithDepthFrame:(STDepthFrame*)depthFrame mask:(uint8_t*)outputMask;

@end

//------------------------------------------------------------------------------
# pragma mark - STCubeRenderer

/** Helper class to render a cuboid.

STCubeRenderer can render a wireframe outline of a cube, and also highlight the part of scene which fits in the given cube.
This can be used to better visualize where the current cuboid scanning volume is located.
*/

@interface STCubeRenderer : NSObject

/// Initialize with required properties.
/// @param glContext The EAGLContext.
- (instancetype)initWithContext:(EAGLContext *)glContext;

/// A depth frame is required before using renderHighlightedDepth.
/// @param depthFrame The depth frame.
- (void)setDepthFrame:(STDepthFrame *)depthFrame;

/// Whether the cube has a support plane. Rendering will be adjusted in that case.
/// @param hasSupportPlane The boolean to enable adjustment of support plane in rendering.
- (void)setCubeHasSupportPlane:(BOOL)hasSupportPlane;

/// Specify the cube size and the volume resolution in cells.
/// @param sizeInMeters The current volume size in meters.
/// @param resolution The current volume resolution.
- (void)adjustCubeSize:(const GLKVector3)sizeInMeters
      volumeResolution:(const GLKVector3)resolution;

/// Highlight the depth frame area which fits inside the cube.
/// @param cameraPose the viewpoint to use for rendering.
/// @param alpha transparency factor between 0 (fully transparent) and 1 (fully opaque)
- (void)renderHighlightedDepthWithCameraPose:(GLKMatrix4)cameraPose alpha:(float)alpha;

/**
Render the cube wireframe outline at the given pose.
 
@param cameraPose the viewpoint to use for rendering.
@param depthTestEnabled whether the lines should be drawn with `GL_DEPTH_TEST` enabled. This should typically be disabled if used in combination with renderHighlightedDepthWithCameraPose: to avoid having the lines occluded, but enabled if a mesh is also being rendered in the scene.
@param occlusionTestEnabled whether to use the current depth frame to do occlusion testing. You can turn this off for better performance.
*/
- (void)renderCubeOutlineWithCameraPose:(GLKMatrix4)cameraPose
                       depthTestEnabled:(BOOL)depthTestEnabled
                   occlusionTestEnabled:(BOOL)occlusionTestEnabled;

@end

//------------------------------------------------------------------------------
# pragma mark - STNormalFrame

/** Processed normal frame with normal vector in each pixel.
 
Output class from STNormalEstimator.
*/
@interface STNormalFrame : NSObject

/// Image width.
@property (readonly, nonatomic) int width;

/// Image height.
@property (readonly, nonatomic) int height;

/// Pointer to the beginning of a contiguous chunk of (`width` * `height`) normal pixel values.
@property (readonly, nonatomic) const GLKVector3 *normals;

@end

//------------------------------------------------------------------------------
# pragma mark - STNormalEstimator

/** Helper class to estimate surface normals.
 
STNormalEstimator calculates a unit vector representing the surface normals for each depth pixel.
*/

@interface STNormalEstimator : NSObject

/// Calculates normals with a depth frame. Pixels without depth will have NaN values.
/// @return A STNormalFrame normal frame object.
/// @param floatDepthFrame The depth frame.
- (STNormalFrame *)calculateNormalsWithDepthFrame:(STDepthFrame *)floatDepthFrame;

@end

//------------------------------------------------------------------------------
# pragma mark - STKeyFrame

/**
Store the depth, color and camera pose information of a single frame.
*/
@interface STKeyFrame : NSObject

/// Initialize with required frame data and pose. `depthFrame` can be `nil` if the depth information is not needed.
/// @note Only `kCVPixelFormatType_420YpCbCr8BiPlanarFullRange` is supported for sampleBuffer format.
/// @param colorCameraPose The GLKMatrix4 camera pose for this keyFrame.
/// @param colorFrame The STColorFrame color buffer.
/// @param depthFrame The STDepthFrame depth frame image.
/// @param error An NSError will contain detailed information if the provided options are incorrect.
- (instancetype)initWithColorCameraPose:(GLKMatrix4)colorCameraPose colorFrame:(STColorFrame *)colorFrame depthFrame:(STDepthFrame *)depthFrame error:(NSError* __autoreleasing *)error;
@end

//------------------------------------------------------------------------------
# pragma mark - STKeyFrameManager

/// Dictionary keys for STKeyFrameManager options.
extern NSString* const kSTKeyFrameManagerMaxSizeKey;
extern NSString* const kSTKeyFrameManagerMaxDeltaRotationKey;
extern NSString* const kSTKeyFrameManagerMaxDeltaTranslationKey;

/// Automatically selects and manages keyframes.
@interface STKeyFrameManager : NSObject

/** Initialize with the provided options.
@param options The option dictionary. The valid keys are:

- `kSTKeyFrameManagerMaxSizeKey`: integer value to set the maximal number of keyframes. Default is 48.
- `kSTKeyFrameManagerMaxDeltaRotationKey`: float value to set the maximal rotation in radians tolerated between a frame and the previous keyframes before considering that it is a new keyframe. Default is 30.0*M_PI/180.0 (30 degrees).
- `kSTKeyFrameManagerMaxDeltaTranslationKey`: float value to set the maximal translation in meters tolerated between a frame and the previous keyframes before considering that it is a new keyframe. Default is 0.3 meters.

@param error will contain detailed information if the provided options are incorrect.
*/
- (instancetype)initWithOptions:(NSDictionary *)options error:(NSError* __autoreleasing *)error;

/** Check if the given pose would be a new keyframe.
@return TRUE if the new pose exceeds the `kSTKeyFrameManagerMaxDeltaRotationKey` or `kSTKeyFrameManagerMaxDeltaTranslationKey` criteria.
@param colorCameraPose The camera pose for this keyFrame.
*/
- (BOOL)wouldBeNewKeyframeWithColorCameraPose:(GLKMatrix4)colorCameraPose;

/**
Potentially add the candidate frame as a keyframe if it meets the new keyframe criteria.
@note Only `kCVPixelFormatType_420YpCbCr8BiPlanarFullRange` is supported for sampleBuffer format.
@return TRUE if the frame was considered a new keyframe, FALSE otherwise.
@param colorCameraPose The GLKMatrix4 camera pose for this keyFrame.
@param colorFrame The STColorFrame color buffer.
@param depthFrame The STDepthFrame depth frame.
*/
- (BOOL)processKeyFrameCandidateWithColorCameraPose:(GLKMatrix4)colorCameraPose
                                         colorFrame:(STColorFrame *)colorFrame
                                         depthFrame:(STDepthFrame *)depthFrame;

/// Manual addition of a keyFrame. This will bypass the criteria of the manager.
/// @param keyFrame The STKeyFrame to add into STKeyFrameManager.
- (void)addKeyFrame:(STKeyFrame *)keyFrame;

/// Get the array of STKeyFrame accumulated so far.
- (NSArray*)getKeyFrames;

/// Clear the current array of keyframes
- (void)clear;

@end

//------------------------------------------------------------------------------
# pragma mark - STColorizer

/// Dictionary key to set the colorization strategy.
extern NSString* const kSTColorizerTypeKey;

/// Dictionary key to optimize for scanning a face in a colorizer.
extern NSString* const kSTColorizerPrioritizeFirstFrameColorKey;

/// Dictionary key to specify a target output number of mesh faces in a colorizer.
extern NSString* const kSTColorizerTargetNumberOfFacesKey;

/// Dictionary key to specify the texture quality of the colorizer.
extern NSString* const kSTColorizerQualityKey;

/**

Constants indicating a colorization strategy to STColorizer.

See also:

- [STColorizer newColorizeTaskWithMesh:scene:keyframes:completionHandler:options:error:]

*/
typedef NS_ENUM(NSInteger, STColorizerType)
{
    /// Generates a color for each vertex of the mesh. Best for small objects.
    STColorizerPerVertex = 0,

    /** Generates a global texture map, and UV coordinates for each vertex of the mesh. Optimized for large rooms.
        __Note:__ Only 640x480 color images are supported by this colorizer.
    */
    STColorizerTextureMapForRoom,

    /// Generates a global texture map, and UV coordinates for each vertex of the mesh. Optimized for objects and people.
    STColorizerTextureMapForObject,
};

/**
 
Constants indicating the quality of STColorizer.
 
*/
typedef NS_ENUM(NSInteger, STColorizerQuality)
{
    /// Use this when speed is not a concern
    STColorizerUltraHighQuality = 0,
    
    /// Use this to balance between quality and speed
    STColorizerHighQuality,
    
    /// Use this option when speed is a concern
    STColorizerNormalQuality,
};

/// Uses the color images from a vector of STKeyFrame to colorize an STMesh.
@interface STColorizer : NSObject

/** Returns a background task to colorize the given mesh using the provided keyframes.

@param mesh The STMesh to colorize.
@param scene The STScene context.
@param keyframes Array of keyframes, e.g. as provided by STKeyFrameManager.
@param completionHandler Block to execute once the task is finished or cancelled.
@param options The option dictionary. The valid keys are:

- `kSTColorizerTypeKey`: STColorizerType value specifying the colorizing algorithm.
- `kSTColorizerPrioritizeFirstFrameColorKey`: boolean value to specify whether the colorizer should give the first frame the highest priority. This is particularly useful when we want to emphasize the appearance in the first frame, e.g., scanning a face. Only supported by `STColorizerTextureMapForObject`.
- `kSTColorizerTargetNumberOfFacesKey`: integer value to specify a target number of faces for the final mesh. Only supported by `STColorizerTextureMapForObject`.
- `kSTColorizerQualityKey`: STColorizerQuality value specifying the desired speed/quality trade-off. Note: only `STColorizerTextureMapForObject` will honor this option. Default is `STColorizerHighQuality`.

@param error will contain detailed information if the provided options are incorrect.
@note All the keyframes must have the same image size.
*/
+ (STBackgroundTask*)newColorizeTaskWithMesh:(STMesh *)mesh
                                       scene:(STScene*)scene
                                   keyframes:(NSArray *)keyframes
                           completionHandler:(void(^)(NSError *error))completionHandler
                                     options:(NSDictionary *)options
                                       error:(NSError* __autoreleasing *)error;
@end
