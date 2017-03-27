//
//  Encoder.h
//  Scanner
//
//  Created by Toan Vuong on 11/21/15.
//  Copyright © 2015 Toan Vuong. All rights reserved.
//

#ifndef ENCODER_H_
#define ENCODER_H_

#include <functional>

#import <Foundation/Foundation.h>
#import <VideoToolbox/VideoToolbox.h>
#import <Structure/Structure.h>

#include "EncoderConfig.h"


class Encoder
{
public:
    typedef std::function<void (CMSampleBufferRef, void *)> EncodedFrameCallback;
    
    /// Callback from VideoToolbox when an encoded frame is available
    static void EncodedFrameAvailable(void *outputCallbackRefCon,
                                      void *sourceFrameRefCon,
                                      OSStatus status,
                                      VTEncodeInfoFlags infoFlags,
                                      CMSampleBufferRef sampleBuffer);
    
    /// Ctor
    Encoder(const EncoderConfig &encoderConfig);
    
    /// Dtor
    ~Encoder();
    
    /// Registers a callback for notification when an encoded frame is available
    ///
    /// @param cb The callback function to register
    /// @param userData The user data to be passed into the callback function
    void registerEncodedFrameCallback(EncodedFrameCallback cb, void *userData);
    
    /// Encodes a Yuv420P frame
    ///
    /// @param frameYuv420 The Yuv420P frame to be encoded
    void encodeFrame(STColorFrame *frame);
private:
    void _encodedFrame(CMSampleBufferRef sampleBuffer);

    VTCompressionSessionRef m_compressionSessionRef;
    EncoderConfig m_encoderConfig;
    EncodedFrameCallback m_cb;
    void *m_userData;
    unsigned m_frameCount;
};

#endif