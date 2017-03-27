//
//  Encoder.mm
//  Scanner
//
//  Created by Toan Vuong on 11/21/15.
//  Copyright © 2015 Toan Vuong. All rights reserved.
//
// Sampled from:
// https://github.com/manishganvir/iOS-h264Hw-Toolbox/blob/master/VTToolbox/VTToolbox/H264HwEncoderImpl.m
//

#include <stdexcept>
#include <sstream>

#include "Encoder.h"


Encoder::Encoder(const EncoderConfig &encoderConfig)
: m_compressionSessionRef(NULL)
, m_encoderConfig(encoderConfig)
, m_cb(NULL)
, m_userData(NULL)
, m_frameCount(0)
{
    NSDictionary *sourceImageAttributes =
    @{
      (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_420YpCbCr8BiPlanarFullRange),
      (id)kCVPixelBufferWidthKey: @(m_encoderConfig.getWidth()),
      (id)kCVPixelBufferHeightKey: @(m_encoderConfig.getHeight())
    };
    
    CMVideoCodecType format = m_encoderConfig.getFormat() == EncoderConfig::H264
                            ? kCMVideoCodecType_H264
                            : kCMVideoCodecType_JPEG;
        
    OSStatus result = VTCompressionSessionCreate(kCFAllocatorDefault,
                                                 m_encoderConfig.getWidth(),
                                                 m_encoderConfig.getHeight(),
                                                 format,
                                                 NULL,
                                                 (__bridge CFDictionaryRef)sourceImageAttributes,
                                                 NULL,
                                                 EncodedFrameAvailable,
                                                 this,
                                                 &m_compressionSessionRef);
    
    CFDictionaryRef sessionProperties = m_encoderConfig.getProperties();
    
    result = VTSessionSetProperties(m_compressionSessionRef, sessionProperties);
    
    if (result != noErr)
    {
        throw std::runtime_error("Failed to set VTSession properties");
    }
    
    VTCompressionSessionPrepareToEncodeFrames(m_compressionSessionRef);
    
}

Encoder::~Encoder()
{
    if (m_compressionSessionRef)
    {
        VTCompressionSessionInvalidate(m_compressionSessionRef);
        CFRelease(m_compressionSessionRef);
    }
}

void Encoder::registerEncodedFrameCallback(EncodedFrameCallback cb, void *userData)
{
    m_cb = cb;
    m_userData = userData;
}

void Encoder::encodeFrame(STColorFrame *frame)
{
    CVImageBufferRef imageBuffer = (CVImageBufferRef)CMSampleBufferGetImageBuffer(frame.sampleBuffer);
    
    CMTime pts = CMSampleBufferGetPresentationTimeStamp(frame.sampleBuffer);
        
    m_frameCount++;
    
    VTEncodeInfoFlags encodeInfoFlags = 0;

    OSStatus status = VTCompressionSessionEncodeFrame(m_compressionSessionRef,
                                                      imageBuffer,
                                                      pts,
                                                      kCMTimeInvalid,
                                                      NULL,
                                                      NULL, // Do we want to pass a frame ref? Probably un-needed
                                                      &encodeInfoFlags);
    if (status != 0)
    {
        std::stringstream ss;
        ss << "Error! " << status;
        throw std::runtime_error(ss.str());
    }
    
}

void Encoder::_encodedFrame(CMSampleBufferRef sampleBuffer)
{
    if (!m_cb)
    {
        throw std::runtime_error("Must register a callback before encoding");
    }
    m_cb(sampleBuffer, m_userData);
}

void Encoder::EncodedFrameAvailable(void *outputCallbackRefCon,
                                        void *sourceFrameRefCon,
                                        OSStatus status,
                                        VTEncodeInfoFlags infoFlags,
                                        CMSampleBufferRef sampleBuffer)
{
    if (status != noErr)
    {
        NSLog(@"Encode error: %d", (int) status);
    }
    Encoder *encoder = static_cast<Encoder *>(outputCallbackRefCon);
    if (encoder)
    {
        encoder->_encodedFrame(sampleBuffer);
    }
}
