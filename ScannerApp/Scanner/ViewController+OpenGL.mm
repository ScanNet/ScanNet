/*
 This file is part of the Structure SDK.
 Copyright © 2015 Occipital, Inc. All rights reserved.
 http://structure.io
 */

#import "ViewController.h"
#import "ViewController+OpenGL.h"
#import "EAGLView.h"

#include <cmath>
#include <limits>

@implementation ViewController (OpenGL)

#pragma mark -  OpenGL

- (void)setupGL
{
    // Create an EAGLContext for our EAGLView.
    _display.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if (!_display.context) { NSLog(@"Failed to create ES context"); }
    
    [EAGLContext setCurrentContext:_display.context];
    [(EAGLView*)self.view setContext:_display.context];
    [(EAGLView*)self.view setFramebuffer];
    
    _display.yCbCrTextureShader = [[STGLTextureShaderYCbCr alloc] init];
    _display.rgbaTextureShader = [[STGLTextureShaderRGBA alloc] init];
    
    // Set up texture and textureCache for images output by the color camera.
    CVReturn texError = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, _display.context, NULL, &_display.videoTextureCache);
    if (texError) { NSLog(@"Error at CVOpenGLESTextureCacheCreate %d", texError); }
    
    glGenTextures(1, &_display.depthAsRgbaTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _display.depthAsRgbaTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


- (void)setupGLViewport
{
    const float vgaAspectRatio = 640.0f/480.0f;
    
    // Helper function to handle float precision issues.
    auto nearlyEqual = [] (float a, float b) { return std::abs(a-b) < std::numeric_limits<float>::epsilon(); };
    
    CGSize frameBufferSize = [(EAGLView*)self.view getFramebufferSize];
    
    float imageAspectRatio = 1.0f;
    
    float framebufferAspectRatio = frameBufferSize.width/frameBufferSize.height;
    
    // The iPad's diplay conveniently has a 4:3 aspect ratio just like our video feed.
    // Some iOS devices need to render to only a portion of the screen so that we don't distort
    // our RGB image. Alternatively, you could enlarge the viewport (losing visual information),
    // but fill the whole screen.
    if (!nearlyEqual (framebufferAspectRatio, vgaAspectRatio))
        imageAspectRatio = 480.f/640.0f;
    
    _display.viewport[0] = 0;
    _display.viewport[1] = 0;
    _display.viewport[2] = frameBufferSize.width*imageAspectRatio;
    _display.viewport[3] = frameBufferSize.height;
}

- (void)uploadGLColorTexture:(STColorFrame*)colorFrame
{
    if (!_display.videoTextureCache)
    {
        NSLog(@"Cannot upload color texture: No texture cache is present.");
        return;
    }
    
    // Clear the previous color texture.
    if (_display.lumaTexture)
    {
        CFRelease (_display.lumaTexture);
        _display.lumaTexture = NULL;
    }
    
    // Clear the previous color texture
    if (_display.chromaTexture)
    {
        CFRelease (_display.chromaTexture);
        _display.chromaTexture = NULL;
    }
    
    // Displaying image with width over 1280 is an overkill. Downsample it to save bandwidth.
    while( colorFrame.width > 2560 )
        colorFrame = colorFrame.halfResolutionColorFrame;
    
    CVReturn err;
    
    // Allow the texture cache to do internal cleanup.
    CVOpenGLESTextureCacheFlush(_display.videoTextureCache, 0);
    
    CVImageBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(colorFrame.sampleBuffer);
    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    
    OSType pixelFormat = CVPixelBufferGetPixelFormatType (pixelBuffer);
    NSAssert(pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange, @"YCbCr is expected!");
    
    // Activate the default texture unit.
    glActiveTexture (GL_TEXTURE0);
    
    // Create an new Y texture from the video texture cache.
    err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _display.videoTextureCache,
                                                       pixelBuffer,
                                                       NULL,
                                                       GL_TEXTURE_2D,
                                                       GL_RED_EXT,
                                                       (int)width,
                                                       (int)height,
                                                       GL_RED_EXT,
                                                       GL_UNSIGNED_BYTE,
                                                       0,
                                                       &_display.lumaTexture);
    
    if (err)
    {
        NSLog(@"Error with CVOpenGLESTextureCacheCreateTextureFromImage: %d", err);
        return;
    }
    
    // Set good rendering properties for the new texture.
    glBindTexture(CVOpenGLESTextureGetTarget(_display.lumaTexture), CVOpenGLESTextureGetName(_display.lumaTexture));
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Activate the default texture unit.
    glActiveTexture (GL_TEXTURE1);
    // Create an new CbCr texture from the video texture cache.
    err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                       _display.videoTextureCache,
                                                       pixelBuffer,
                                                       NULL,
                                                       GL_TEXTURE_2D,
                                                       GL_RG_EXT,
                                                       (int)width/2,
                                                       (int)height/2,
                                                       GL_RG_EXT,
                                                       GL_UNSIGNED_BYTE,
                                                       1,
                                                       &_display.chromaTexture);
    
    if (err)
    {
        NSLog(@"Error with CVOpenGLESTextureCacheCreateTextureFromImage: %d", err);
        return;
    }
    
    // Set rendering properties for the new texture.
    glBindTexture(CVOpenGLESTextureGetTarget(_display.chromaTexture), CVOpenGLESTextureGetName(_display.chromaTexture));
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
}

-(int)indexFromLoc:(int) x y:(int) y width:(int) width
{
    return 4 * (width * y + x);
}

- (void)uploadGLColorTextureFromDepth:(STDepthFrame*)depthFrame
{    
    //NSLog(@"Labeling Features");
    [_depthAsRgbaVisualizer convertDepthFrameToRgba:depthFrame];

    const static bool negative = false;
    const static int boxsize   = 5;
    
    int width = _depthAsRgbaVisualizer.width;
    int height = _depthAsRgbaVisualizer.height;
    
//#ifdef DEBUG_MODE
    for(int i = 0; i < [corners count] / 2; i++)
    {
        int j = 2 * i;
        if(j + 1 >= [corners count])
            break;
        
        float x1 = [[corners objectAtIndex:(j)] floatValue];
        float y1 = [[corners objectAtIndex:(j + 1)] floatValue];
        
        if(x1 >= 0 && x1 <=1 && y1 >= 0 && y1 <= 1)
        {
            int x = (int) (x1 * width);
            int y = (int) (y1 * height);
            
            for(int r = y - boxsize / 2; r <= y + boxsize / 2; r++)
            {
                for(int c = x - boxsize / 2; c <= x + boxsize / 2; c++)
                {
                    if(r >= 0 && r < height && c >= 0 && c < width)
                    {
                        int index = [self indexFromLoc:c y:r width:width];
                        
                        _depthAsRgbaVisualizer.rgbaBuffer[index] = 255;
                        _depthAsRgbaVisualizer.rgbaBuffer[index + 1] = 0;
                        _depthAsRgbaVisualizer.rgbaBuffer[index + 2] = 0;
                        
                    }
                }
            }
        }
    }
    
    //NSLog(@"Finished Labeling");
    
    [corners removeAllObjects];
    
    //NSLog(@"Finished clearing corners");
    /*
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, _depthAsRgbaVisualizer.rgbaBuffer, width * height * 4, NULL);
    
    
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    CGImageRef imageRef = CGImageCreate(width, height, 8, 32, 4 * width, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    UIImage *newImage = [UIImage imageWithCGImage:imageRef];
    UIImageWriteToSavedPhotosAlbum(newImage, nil, nil, nil);
     */
//#endif
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _display.depthAsRgbaTexture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _depthAsRgbaVisualizer.rgbaBuffer);
}

- (void)renderSceneForDepthFrame:(STDepthFrame*)depthFrame colorFrameOrNil:(STColorFrame*)colorFrame
{
    if (!self.avCaptureSession) return;
    
    // Activate our view framebuffer.
    [(EAGLView *)self.view setFramebuffer];
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    glViewport (_display.viewport[0], _display.viewport[1], _display.viewport[2], _display.viewport[3]);
    

    //NSLog(@"#Corners: %lu", [corners count]);

    switch (_slamState.scannerState)
    {
        case ScannerStateSceneLabelling:
        case ScannerStateCubePlacement:
        case ScannerStateScanning:
        {
            //NSLog(@"render camera image");
            [self renderCameraImage];
            break;
        }
            
        default: {}
    };
    
    // Check for OpenGL errors
    GLenum err = glGetError ();
    if (err != GL_NO_ERROR)
        NSLog(@"glError = %x", err);
    
    // Display the rendered framebuffer.
    [(EAGLView *)self.view presentFramebuffer];
}

- (void)renderCameraImage
{
    //render color
    if (_useColorCamera)
    {
        if (!_display.lumaTexture || !_display.chromaTexture)
            return;
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(CVOpenGLESTextureGetTarget(_display.lumaTexture),
                      CVOpenGLESTextureGetName(_display.lumaTexture));
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(CVOpenGLESTextureGetTarget(_display.chromaTexture),
                      CVOpenGLESTextureGetName(_display.chromaTexture));
        
        //glDisable(GL_BLEND);
        [_display.yCbCrTextureShader useShaderProgram];
        [_display.yCbCrTextureShader renderWithLumaTexture:GL_TEXTURE0 chromaTexture:GL_TEXTURE1];
    }
    //render depth
    if(_renderDepthOverlay && _display.depthAsRgbaTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _display.depthAsRgbaTexture);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        [_display.rgbaTextureShader useShaderProgram];
        [_display.rgbaTextureShader renderTexture:GL_TEXTURE0];
    }
    glUseProgram (0);
    
}


@end