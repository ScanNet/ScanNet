//
//  EncoderConfig.h
//  Scanner
//
//  Created by Toan Vuong on 11/21/15.
//  Copyright © 2015 Toan Vuong. All rights reserved.
//

#ifndef ENCODER_CONFIG_H_
#define ENCODER_CONFIG_H_

class EncoderConfig
{
public:
    
    enum MediaFormat
    {
        JPEG,
        H264
    };

    EncoderConfig(unsigned width,
                  unsigned height,
                  MediaFormat format = JPEG,
                  int frameRate = 30,
                  int avgBitrate = 5000,
                  unsigned intraFrameRate = 100,
                  int32_t profileLevel = 0, // TODO
                  int32_t profileNum = 0 // TODO
                  );
    
    ~EncoderConfig();
    
    unsigned getWidth() const;
    unsigned getHeight() const;
    MediaFormat getFormat() const;
    int getFPS() const { return m_frameRate; }
    int getBitrate() const { return m_avgBitrate; }
    
    CFDictionaryRef getProperties();
    
private:
    void _createJpegSessionProperties();
    void _createH264SessionProperties();
    
    unsigned m_width;
    unsigned m_height;
    MediaFormat m_format;
    int m_frameRate;
    unsigned m_intraFrameRate;
    int32_t m_profileLevel;
    int32_t m_profileNum;
    int m_avgBitrate;
    
    NSDictionary *m_dictionary;
};

#endif /* ENCODER_CONFIG_H_ */
