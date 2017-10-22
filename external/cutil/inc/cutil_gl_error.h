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

#ifndef CUTIL_GL_ERROR
#define CUTIL_GL_ERROR

/* CUda UTility Library */

// includes, system
#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  include <windows.h>
#  include <stdlib.h>
#  undef min
#  undef max
#endif

// includes, graphics
#if defined (__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

////////////////////////////////////////////////////////////////////////////
//! Check for OpenGL error
//! @return CUTTrue if no GL error has been encountered, otherwise 0
//! @param file  __FILE__ macro
//! @param line  __LINE__ macro
//! @note The GL error is listed on stderr
//! @note This function should be used via the CHECK_ERROR_GL() macro
////////////////////////////////////////////////////////////////////////////
CUTBoolean CUTIL_API
cutCheckErrorGL( const char* file, const int line) 
{
	CUTBoolean ret_val = CUTTrue;

	// check for error
	GLenum gl_error = glGetError();
	if (gl_error != GL_NO_ERROR) 
	{
#ifdef _WIN32
		char tmpStr[512];
		// NOTE: "%s(%i) : " allows Visual Studio to directly jump to the file at the right line
		// when the user double clicks on the error line in the Output pane. Like any compile error.
		sprintf_s(tmpStr, 255, "\n%s(%i) : GL Error : %s\n\n", file, line, gluErrorString(gl_error));
		OutputDebugString(tmpStr);
#endif
		fprintf(stderr, "GL Error in file '%s' in line %d :\n", file, line);
		fprintf(stderr, "%s\n", gluErrorString(gl_error));
		ret_val = CUTFalse;
	}
	return ret_val;
}

#ifdef _DEBUG

#define CUT_CHECK_ERROR_GL()                                               \
	if( CUTFalse == cutCheckErrorGL( __FILE__, __LINE__)) {                  \
	exit(EXIT_FAILURE);                                                  \
	}
// Use this one to do : if(CUT_GL_HAS_ERROR)
#define CUT_GL_HAS_ERROR (cutCheckErrorGL( __FILE__, __LINE__) ? CUTFalse : CUTTrue )
#ifdef _WIN32
#define CUT_CHECK_ERROR_GL2()\
    if(CUT_GL_HAS_ERROR)\
	{\
		MessageBox(NULL, "Error in OpenGL. Check VStudio Output...", "Error", MB_OK);\
		exit(EXIT_FAILURE);\
	}
#else // Not _WIN32:
#define CUT_CHECK_ERROR_GL2()\
    if(CUT_GL_HAS_ERROR)\
	{\
		printf("press a key...\n");\
		getc(stdin);\
		exit(EXIT_FAILURE);\
	}
#endif

#else

#define CUT_CHECK_ERROR_GL()
#define CUT_CHECK_ERROR_GL2()
#define CUT_GL_HAS_ERROR CUTFalse

#endif // _DEBUG

#endif // CUTIL_GL_ERROR
