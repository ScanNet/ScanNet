// basics window

// simple window handler, wrapping around the glfw for window and ogl3.3 context creation, with glew for gl extensions.
#pragma once



namespace bsc
{

  struct window
  {
    GLFWwindow * handle    = NULL;
    struct NVGcontext * vg = NULL; // this really should be part of gui.h
    vec2i size    = vec2i( -1, -1 );
    vec2i fb_size = vec2i( -1, -1 );
    vec4i viewport = vec4i(-1, -1, -1, -1);
    r64 pixel_ratio = -1.0;
    r64 aspect_ratio = -1.0;
    vec3f bckgrd_col_top;
    vec3f bckgrd_col_bot;
    r32 buffer_swap_time;

    // TODO Change these to function pointers? More C style?
    std::function< void() > display;
    std::function< i32() > init;
    std::function< void( i32, const char **) > drag_and_drop;
  };

  static window g_window;
  window & get_window();

  i32 create_window( const char * title, const i32 width, const i32 height, 
                     const vec3 color_top = vec3( 0.45, 0.45, 0.45 ),
                     const vec3 color_bot = vec3( 0.45, 0.45, 0.45 ));
  i32 clear_window( vec4i viewport = vec4i( -1, -1, -1, -1 ) );
  i32 main_loop();

  void take_screenshot( vec4i viewport );
  void refresh_display();

  void set_display_funct( std::function< void() > display_funct );
  void set_init_funct( std::function< i32() > init_funct );
  void set_drag_and_drop_funct ( 
    std::function < void( i32, const char **) > drag_and_drop_funct );

  void install_callbacks( GLFWwindow * handle );

  // graphics context should be owned by io, not window (?)
  struct NVGcontext * get_vector_graphics_context();
}


// bascics window functions

#ifdef BSC_IMPLEMENTATION


////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////
bsc::window & bsc::
get_window()
{
  return g_window;
}

void bsc::
set_display_funct( std::function< void() > display_funct )
{
  g_window.display = display_funct;
}

void bsc::
set_init_funct( std::function< i32() > init_funct )
{
  g_window.init = init_funct;
}

void bsc::
set_drag_and_drop_funct( std::function < void( i32, const char ** ) > drag_and_drop_funct )
{
  g_window.drag_and_drop = drag_and_drop_funct;
}

i32 bsc::
create_window ( const char * title, const i32 width, const i32 height,
                const bsc::vec3 color_top, const bsc::vec3 color_bot )
{

  // initialize and create GLFW window to obtain modern opengl context(not so modern on mac, haha)
  if ( !glfwInit() )
  {
    printf( "Could not init glfw\n" );
    return 0;
  }

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  glfwWindowHint( GLFW_SAMPLES, 4 ); // INVESTIGATE!
  // glfwWindowHint( GLFW_DECORATED, GL_FALSE ); // <- this will remove "fluff"
  glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );

  g_window.handle = glfwCreateWindow( width, height, title, NULL, NULL );

  if ( !g_window.handle )
  {
    printf( "Could not create glfw window\n" );
    glfwTerminate();
    return 0;
  }

  install_callbacks( g_window.handle );

  // Note seting SwapInterval to 0 essentialy disables v_sync!
  glfwMakeContextCurrent( g_window.handle );
  glfwSwapInterval( 0 );

  // lets get gl extension from glew
  // TODO: need to replace this library
  #if defined(_WIN32) || defined(_WIN64)
  #ifndef __EMSCRIPTEN__
    glewExperimental = GL_TRUE;
    if ( glewInit() != GLEW_OK )
    {
      printf( "Could not init glew\n" );
      return 0;
    }
  #endif
  #endif

  // lets create the vector graphics context
  #ifdef __EMSCRIPTEN__
    // g_window.vg = nvgCreateGLES2( NVG_ANTIALIAS | NVG_STENCIL_STROKES );
  #else
    // g_window.vg = nvgCreateGL3( NVG_ANTIALIAS | NVG_STENCIL_STROKES );
  #endif

  // if ( g_window.vg == NULL )
  // {
    // printf( "Could not init nanovg\n" );
    // return 0;
  // }

  // some sane opengl defaults
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glBlendEquation( GL_FUNC_ADD );

  glfwGetWindowSize( g_window.handle, &g_window.size.x, &g_window.size.y );
  glfwGetFramebufferSize( g_window.handle, 
                          &g_window.fb_size.x, &g_window.fb_size.y );
  g_window.pixel_ratio = (r32)g_window.fb_size.x / (r32)g_window.size.x;
  g_window.aspect_ratio = (r32)g_window.size.x / (r32)g_window.size.y;
  g_window.viewport = bsc::vec4i( 0, 0, g_window.fb_size.x, g_window.fb_size.y);
  g_window.bckgrd_col_top = color_top;
  g_window.bckgrd_col_bot = color_bot;

  return 1;
}

// This is a set of shaders for rendering the gradient backgroud
void render_background( bsc::vec3 col_top, bsc::vec3 col_bot )
{
  static GLuint background_vao = 0;
  static bsc::shader_prog background_shader;

  if (background_vao == 0)
  {
    glGenVertexArrays(1, &background_vao);
  
    // vertex shader ( full screen quad by Morgan McGuire)
    char* vs_src = (char*) SHADER_HEAD STR 
    (
      out vec2 v_texcoords;
      void main()
      {
        uint idx = uint( gl_VertexID % 3 );
        gl_Position = vec4(
            (float(   idx        & 1U ) ) * 4.0 - 1.0,
            (float( ( idx >> 1U )& 1U ) ) * 4.0 - 1.0,
            0.0, 1.0);
        v_texcoords = gl_Position.xy * 0.5 + 0.5;
      }
    );

    char* fs_src = (char*) SHADER_HEAD STR 
    (
      in vec2 v_texcoords;
      uniform vec3 col_top;
      uniform vec3 col_bot;
      out vec4 frag_color;
      void main()
      {
        frag_color = vec4( v_texcoords.y * col_top + 
                  ( 1.0 - v_texcoords.y ) * col_bot , 1.0 );
      }
    );
  
    bsc::create_shader_prog_from_source( vs_src, fs_src, background_shader );
  }

  bsc::use_program( background_shader );
  bsc::set_uniform( background_shader, "col_top", col_top );
  bsc::set_uniform( background_shader, "col_bot", col_bot );
  glBindVertexArray( background_vao );
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
}

static i32 screenshot_count = 0;

void bsc::
take_screenshot( bsc::vec4i viewport )
{
  i32 w = g_window.pixel_ratio*(viewport.z - viewport.x);
  i32 h = g_window.pixel_ratio*(viewport.w - viewport.y);
  bsc::img screenshot( w, h, 3 );

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glReadPixels( g_window.pixel_ratio * viewport.x, 
                g_window.pixel_ratio * viewport.y, 
                w, h, 
                GL_RGB, GL_UNSIGNED_BYTE, screenshot.data );

  screenshot.vert_flip();
  char name[512];
  sprintf( name, "screenshot_%d.jpg", screenshot_count++ );
  screenshot.write( name );

}

i32 bsc::
clear_window( bsc::vec4i viewport )
{
  // TODO: Basics will most likely be deferred renderer. We need to remove this. 
  // Also this is strictly renderer dependent
  if ( viewport != bsc::vec4i( -1, -1, -1, -1 ) )
  {
    g_window.viewport = viewport;
  }
  glViewport( g_window.pixel_ratio * viewport[0], 
              g_window.pixel_ratio * viewport[1], 
              g_window.pixel_ratio * viewport[2], 
              g_window.pixel_ratio * viewport[3] );
  g_window.aspect_ratio = (r32)viewport[2] / (r32)viewport[3];
  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
  
  glDisable( GL_DEPTH_TEST );
  render_background( g_window.bckgrd_col_top, 
                     g_window.bckgrd_col_bot );
  glEnable( GL_DEPTH_TEST );

  return 1;
}

#ifdef BSC_USE_IMGUI
  bool ImGui_BscImpl_Init( GLFWwindow* window );
#endif

static void
display_loop()
{
  using namespace bsc;
  r32 start = glfwGetTime();

  // TODO: add state variable to control whether we are waiting or polling for events
  // TODO: add state variable to control vsync?
  glfwPollEvents();

  g_window.display();

  glfwSwapBuffers( g_window.handle );

  r32 end = glfwGetTime();

  g_window.buffer_swap_time = end - start;
}

// TODO: Error types?
i32 bsc::
main_loop ()
{
  if ( !g_window.handle )
  {
    printf( "Window not created, terminating!\n" );
    return 0;
  }

  if ( !g_window.init() )
  {
    printf( "Initialization function failed!\n" );
    return 0;
  }

#ifdef BSC_USE_IMGUI
  ImGui_BscImpl_Init( g_window.handle );
#endif


#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop( display_loop, 0, 0);
#else
  while ( !glfwWindowShouldClose( g_window.handle ) )
  {
    display_loop();
  }
#endif

  return 1;
}

void bsc::
refresh_display()
{
    glfwSwapBuffers( g_window.handle );
}

////////////////////////////////////////////////////////////////////////////////
// Callbacks Forward Declarations
////////////////////////////////////////////////////////////////////////////////

void keyboard_callback ( GLFWwindow*, i32 key, i32, i32 action, i32 );
void char_callback( GLFWwindow*, unsigned int c );
void mouse_button_callback( GLFWwindow* window, i32 button, i32 action, i32 );
void scroll_callback( GLFWwindow* window, double /**/, double yoffset );
void cursor_enter_callback( GLFWwindow* window, i32 entered );
void window_size_callback( GLFWwindow* window, i32 width, i32 height );
void framebuffer_size_callback( GLFWwindow* window, i32 width, i32 height );
void window_refresh_callback( GLFWwindow* window );
void drop_callback( GLFWwindow* window, i32 count, const char ** paths );

void bsc::
install_callbacks( GLFWwindow * window_handle )
{
  glfwSetKeyCallback( window_handle, keyboard_callback );
  glfwSetCharCallback( window_handle, char_callback );
  glfwSetMouseButtonCallback( window_handle, mouse_button_callback );
  glfwSetCursorEnterCallback( window_handle, cursor_enter_callback );
  glfwSetWindowSizeCallback ( window_handle, window_size_callback );
  glfwSetFramebufferSizeCallback ( window_handle, framebuffer_size_callback );
  glfwSetWindowRefreshCallback( window_handle, window_refresh_callback );
  glfwSetScrollCallback ( window_handle, scroll_callback );
#ifndef __EMSCRIPTEN__
  glfwSetDropCallback ( window_handle, drop_callback );
#endif
}
#endif
