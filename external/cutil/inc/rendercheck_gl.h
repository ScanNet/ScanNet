/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

#ifndef _RENDERCHECK_GL_H_
#define _RENDERCHECK_GL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <map>
#include <string>

#include <GL/glew.h>

#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
    #include <GL/freeglut.h>
#endif

#include <nvShaderUtils.h>

using std::vector;
using std::map;
using std::string;

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#if _DEBUG
#define CHECK_FBO     checkStatus(__FILE__, __LINE__, true)
#else
#define CHECK_FBO     true
#endif



class CheckRender
{
public:
    CheckRender(unsigned int width, unsigned int height, unsigned int Bpp,
   		bool bQAReadback, bool bUseFBO, bool bUsePBO);

    virtual ~CheckRender();

    virtual void allocateMemory( unsigned int width, unsigned int height, unsigned int Bpp,
	    	                    bool bQAReadback, bool bUseFBO, bool bUsePBO );

    virtual void setExecPath(char *path) {
        strcpy(m_ExecPath, path);
    }
    virtual void EnableQAReadback(bool bStatus) { m_bQAReadback = bStatus; }
    virtual bool IsQAReadback() { return m_bQAReadback; }
    virtual bool IsFBO()        { return m_bUseFBO; }
    virtual bool IsPBO()        { return m_bUsePBO; }
    virtual void * imageData()  { return m_pImageData; }

    // Interface to this class functions
    virtual void setPixelFormat(GLenum format) { m_PixelFormat = format; }
    virtual int  getPixelFormat() { return m_PixelFormat; }
    virtual bool checkStatus(const char *zfile, int line, bool silent) = 0;
    virtual bool readback( GLuint width, GLuint height ) = 0;
    virtual bool readback( GLuint width, GLuint height, GLuint bufObject ) = 0;
    virtual bool readback( GLuint width, GLuint height, unsigned char *membuf ) = 0;

    virtual void bindReadback();
    virtual void unbindReadback();

    virtual void savePGM(  const char *zfilename, bool bInvert, void **ppReadBuf );
    virtual void savePPM(  const char *zfilename, bool bInvert, void **ppReadBuf );

    virtual bool PGMvsPGM( const char *src_file, const char *ref_file, const float epsilon, const float threshold = 0.0f );
    virtual bool PPMvsPPM( const char *src_file, const char *ref_file, const float epsilon, const float threshold = 0.0f );

    void    setThresholdCompare(float value) { m_fThresholdCompare = value; }

    virtual void dumpBin(void *data, unsigned int bytes, const char *filename);
    virtual bool compareBin2BinUint(const char *src_file, const char *ref_file, unsigned int nelements, const float epsilon, const float threshold);
    virtual bool compareBin2BinFloat(const char *src_file, const char *ref_file, unsigned int nelements, const float epsilon, const float threshold);

protected:
    unsigned int  m_Width, m_Height, m_Bpp;
    unsigned char *m_pImageData;  // This is the image data stored in system memory
    bool          m_bQAReadback, m_bUseFBO, m_bUsePBO;
    GLuint        m_pboReadback;
    GLenum        m_PixelFormat;
    float         m_fThresholdCompare;
    char          m_ExecPath[256];
};


class CheckBackBuffer : public CheckRender
{
public:
    CheckBackBuffer(unsigned int width, unsigned int height, unsigned int Bpp, bool bUseOpenGL = true);
    virtual ~CheckBackBuffer();

    virtual bool checkStatus(const char *zfile, int line, bool silent);
    virtual bool readback( GLuint width, GLuint height );
    virtual bool readback( GLuint width, GLuint height, GLuint bufObject );
    virtual bool readback( GLuint width, GLuint height, unsigned char *membuf );

private:
    virtual void bindFragmentProgram() {}; 
    virtual void bindRenderPath() {};
    virtual void unbindRenderPath() {};

    // bind to the BackBuffer to Texture
    virtual void bindTexture() {}; 

    // release this bind
    virtual void unbindTexture() {}; 
};


// structure defining the properties of a single buffer
struct bufferConfig {
    string name;
    GLenum format;
    int bits;
};

// structures defining properties of an FBO
struct fboConfig {
    string name;
    GLenum colorFormat;
    GLenum depthFormat;
    int redbits;
    int depthBits;
    int depthSamples;
    int coverageSamples;
};

struct fboData {
    GLuint colorTex; //color texture
    GLuint depthTex; //depth texture
    GLuint fb;      // render framebuffer
    GLuint resolveFB; //multisample resolve target
    GLuint colorRB; //color render buffer
    GLuint depthRB; // depth render buffer
};


class CFrameBufferObject 
{
public:
    CFrameBufferObject (unsigned int width, unsigned int height, unsigned int Bpp, bool bUseFloat, GLenum eTarget);

    virtual ~CFrameBufferObject();

    GLuint createTexture(GLenum target, int w, int h, GLint internalformat, GLenum format);
    void    attachTexture(  GLenum texTarget, 
                            GLuint texId, 
                            GLenum attachment   = GL_COLOR_ATTACHMENT0_EXT, 
                            int mipLevel        = 0, 
                            int zSlice          = 0);

    bool initialize(unsigned width, unsigned height, fboConfig & rConfigFBO, fboData & rActiveFBO);
    bool create( GLuint width, GLuint height, fboConfig &config, fboData &data );
    bool createMSAA( GLuint width, GLuint height, fboConfig *p_config, fboData *p_data );
    bool createCSAA( GLuint width, GLuint height, fboConfig *p_config, fboData *p_data );

    virtual void freeResources();
    virtual bool checkStatus(const char *zfile, int line, bool silent);

    virtual void renderQuad(int width, int height, GLenum eTarget);

    // bind to the Fragment Program
    void bindFragmentProgram() {
       glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_textureProgram);
       glEnable(GL_FRAGMENT_PROGRAM_ARB);
    }

    // bind to the FrameBuffer Object
    void bindRenderPath() {
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, m_fboData.fb );
    }

    // release current FrameBuffer Object
    void unbindRenderPath() {
        glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
    }

    // bind to the FBO to Texture
    void bindTexture() {
        glBindTexture( m_eGLTarget, m_fboData.colorTex );
    }

    // release this bind
    void unbindTexture() {
        glBindTexture( m_eGLTarget, 0 );
    }

    GLuint getFbo()      { return m_fboData.fb; }
    GLuint getTex()      { return m_fboData.colorTex; }
    GLuint getDepthTex() { return m_fboData.depthTex; }

private:
    GLuint    m_Width, m_Height;
    fboData   m_fboData;
    fboConfig m_fboConfig;

    GLuint    m_textureProgram;
    GLuint    m_overlayProgram;

    bool      m_bUseFloat;
    GLenum    m_eGLTarget;
};


// CheckFBO - render and verify contents of the FBO
class CheckFBO: public CheckRender
{
public:
	CheckFBO(unsigned int width, unsigned int height, unsigned int Bpp);
	CheckFBO(unsigned int width, unsigned int height, unsigned int Bpp, CFrameBufferObject *pFrameBufferObject);

	virtual ~CheckFBO();

	virtual bool checkStatus(const char *zfile, int line, bool silent);
	virtual bool readback( GLuint width, GLuint height );
	virtual bool readback( GLuint width, GLuint height, GLuint bufObject );
	virtual bool readback( GLuint width, GLuint height, unsigned char *membuf );

private:
	CFrameBufferObject *m_pFrameBufferObject;
};

#endif // _RENDERCHECK_GL_H_

