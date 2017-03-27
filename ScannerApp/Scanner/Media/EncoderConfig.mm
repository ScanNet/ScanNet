//
//  EncoderConfig.m
//  Scanner
//
//  Created by Toan Vuong on 11/21/15.
//  Copyright © 2015 Toan Vuong. All rights reserved.
//

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#import <VideoToolbox/VideoToolbox.h>

#include "EncoderConfig.h"

EncoderConfig::EncoderConfig(unsigned width,
                       unsigned height,
                       MediaFormat format,
                       int frameRate,
                       int avgBitrate,
                       unsigned intraFrameRate,
                       int32_t profileLevel,
                       int32_t profileNum)
: m_width(width)
, m_height(height)
, m_format(format)
, m_frameRate(frameRate)
, m_intraFrameRate(intraFrameRate)
, m_profileLevel(profileLevel)
, m_profileNum(profileNum)
, m_avgBitrate(avgBitrate)
, m_dictionary(NULL)
{
    if (format == H264)
    {
        _createH264SessionProperties();
    } else if (format == JPEG)
    {
        _createJpegSessionProperties();
    }
}

EncoderConfig::~EncoderConfig()
{
}

unsigned EncoderConfig::getWidth() const
{
    return m_width;
}

unsigned EncoderConfig::getHeight() const
{
    return m_height;
}

EncoderConfig::MediaFormat EncoderConfig::getFormat() const
{
    return m_format;
}

CFDictionaryRef EncoderConfig::getProperties()
{
    return (__bridge CFDictionaryRef) m_dictionary;
}

void EncoderConfig::_createJpegSessionProperties()
{
    // TODO: Add some values for these
    CFMutableDictionaryRef dict =
        CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFNumberRef tmp = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &m_frameRate);
    
    CFDictionaryAddValue(dict, kVTCompressionPropertyKey_ExpectedFrameRate, tmp);
    
    CFRelease(tmp);
    
    m_dictionary = CFBridgingRelease(dict);
}

void EncoderConfig::_createH264SessionProperties()
{
    CFMutableDictionaryRef dict =
        CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    if (dict)
    {
        CFNumberRef tmp = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &m_intraFrameRate);
        CFDictionaryAddValue(dict, kVTCompressionPropertyKey_MaxKeyFrameInterval, tmp);
        CFRelease(tmp);
        //CFDictionaryAddValue(m_dictionary, kVTCompressionPropertyKey_MaxKeyFrameIntervalDuration, CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &m_frameRate));
        CFDictionaryAddValue(dict, kVTCompressionPropertyKey_AllowTemporalCompression, kCFBooleanTrue);
        CFDictionaryAddValue(dict, kVTCompressionPropertyKey_AllowFrameReordering, kCFBooleanFalse);
        //CFDictionaryAddValue(m_dictionary, kVTCompressionPropertyKey_ProfileLevel, kVTProfileLevel_H264_High_4_2);
        CFDictionaryAddValue(dict, kVTCompressionPropertyKey_ProfileLevel, kVTProfileLevel_H264_High_AutoLevel);
        //CFDictionaryAddValue(m_dictionary, kVTCompressionPropertyKey_H264EntropyMode, kVTH264EntropyMode_CABAC);
        CFDictionaryAddValue(dict, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue);
        tmp = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &m_frameRate);
        CFDictionaryAddValue(dict, kVTCompressionPropertyKey_ExpectedFrameRate, tmp);
        CFRelease(tmp);
        NSLog(@"[EncoderConfig] framerate = %d", m_frameRate);
        
        // -- should be duration of entire session, which we don't know, so don't set
        //const double duration = 1.0 / m_frameRate;
        //CFDictionaryAddValue(m_dictionary, kVTCompressionPropertyKey_ExpectedDuration, CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &duration));
        
        const int mappedBitrate = m_avgBitrate * 1024;
        tmp = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &mappedBitrate);
        CFDictionaryAddValue(dict, kVTCompressionPropertyKey_AverageBitRate, tmp);
        CFRelease(tmp);
        NSLog(@"[EncoderConfig] bitrate = %d", mappedBitrate);
        
        const float quality = 1.0f;//0.95f;
        tmp = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &quality);
        CFDictionaryAddValue(dict, kVTCompressionPropertyKey_Quality, tmp);
        CFRelease(tmp);
        
        //const int32_t maxSliceBytes = 1300;
        //CFDictionaryAddValue(m_dictionary, kVTCompressionPropertyKey_MaxH264SliceBytes, CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &maxSliceBytes));
    
        m_dictionary = CFBridgingRelease(dict);
    }
}