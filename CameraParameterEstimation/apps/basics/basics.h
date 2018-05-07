// basics v0.07
// by Maciej Halber

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// This is a 'work-in-progress', header-only library, aimed at simplifying
// writing graphics api. Currently *all* of the features are in progress and 
// might change.

// Semi stable features:
//  - window creation and display loops ( powered by GLFW3 )
//  - linear algebra ( vec2/3/4, mat2/3/4, quaternions -> modelled after glm )
//  - opengl shader creation

// BIG TODO's
//  - 2d graphics api
//  - 3d graphics api 
//  - data containers ( hashmaps, grids, kdtrees )

// This library is needs C++11 compiler, however use of C++ features is aimed to
// be kept low. C++ features that this library does use:
// - operator overloading ( for math )
// - function overridng
// - templates ( very sparsely, for math )
// - threads

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//TODO(maciej): Insert define flags to improve modularity of basics

////////////////////////////////////////////////////////////////////////////////
// EMSCRIPTEN
////////////////////////////////////////////////////////////////////////////////

#ifndef __BSC__
#define __BSC__

#ifdef __EMSCRIPTEN__
  #include "emscripten.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// C / C++ HEADERS
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

// TODO(maciej): try to limit usage of cpp headers?
#include <limits>
#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include <cmath>

////////////////////////////////////////////////////////////////////////////////
// THIRD PARTY LIBRARIES
////////////////////////////////////////////////////////////////////////////////

#ifdef BSC_USE_WINDOW // Graphics through window
  
  // glew for opengl extension
  // TODO(maciej): To be removed and replaced with some inline loader
  #ifndef __EMSCRIPTEN__
    #if defined(_WIN32) || defined(_WIN64)
      #include "GL/glew.h"
    #endif
  #else
    #define GLFW_INCLUDE_ES2
  #endif

  // glfw configuration
  // TODO(maciej): Unlikely in near feature, but we might strip this.
  #define GLFW_INCLUDE_GLCOREARB  /* don't drag in legacy GL headers. */
  #define GLFW_NO_GLU 
  #include "GLFW/glfw3.h"

#else  // offscreen rendering

  #ifdef __APPLE__
    #include <OpenGL/gl3.h>
    #include <OpenGL/OpenGL.h>
    #include <OpenGL/CGLTypes.h>
    #include <OpenGL/CGLCurrent.h>
  #endif

  #ifdef __unix__
    #include <GL/glew.h>
    #include <GL/gl.h>
    #include <GL/glext.h>
    #include <GL/osmesa.h>
  #endif
  
  //TODO: Windows

#endif


// tiny_dir for easy directory stepping and file name/extension tokenizing
#include "extern/tiny_dir.h"


// IMAGES
// Basics supports reading/writing of 2 image types: PNG, JPEG
// This is accomplished by a handful of nice single header libraries

// stb_image.h by Sean T. Barret -> JPEG(read)
// tiny_jpeg.h by Sergio Gonzalez -> JPEG(write)
// lodepng.h by Lode Vandevenne -> PNG(read/write)

// TODO(maciej): Add support for BMP and TGA

// stb_image configuration
#define STBI_ONLY_JPEG
#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include "extern/stb_image.h"

// stb_image_write configuration
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "extern/stb_image_write.h"

// lodepng configuration
// TODO(maciej): Can we use zlib implementation from stb?
#define LODEPNG_IMPLEMENTATION
#include "extern/lodepng.h"

// tinyjpeg configuration
#define TJE_IMPLEMENTATION
#include "extern/tiny_jpeg.h"

// 3D formats
// TODO(maciej): This needs to be added

// Optionally a support is provided for imgui. Ideally bsc::ui will allow user
// to create immediate mode interfaces, but for the time being, user can enable
// using imgui.
// Note that you only need to add imgui calls in your rendering code. Standard
// per frame calls and initialization is managed by bsc::ui
#ifdef BSC_USE_IMGUI
#include "extern/imgui/imgui.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// BASICS MODULES
////////////////////////////////////////////////////////////////////////////////

// NOTE: (maciej) These files are quite big, and thye only contain a font...
// need to come up with better idea
#include "ubuntu_mono.h"

#include "common.h"

#include "argparse.h"

#include "linalg/vector.h"
#include "linalg/matrix.h"
#include "linalg/quaternion.h"
#include "linalg/geometry.h"
#include "linalg/debug.h"

#include "gfx/image.h"
#include "gfx/shader.h"

#ifdef BSC_USE_WINDOW
  #include "window/window.h"
  #include "gfx/camera.h" // camera controls only make sense with mouse
  #include "gfx/draw2d.h" // TODO: Fix that!
#endif

#include "gfx/gpu_assets.h"
#include "gfx/param_shapes.h"


#endif /*__BSC__*/
