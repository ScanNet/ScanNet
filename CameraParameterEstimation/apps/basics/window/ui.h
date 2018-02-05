// ui.h

// TODO: compile time string hashing
// TODO: implement progress bar
// TODO: add game controler support?
// TODO: add modifiers to mouse click queries
// TODO: Work on slider - make it templated, add sizing, add styling, add vertical vs horizontal option
// TODO: change hot to hover?
// TODO: Implement text editing using stb_textedit
#pragma once

namespace bsc
{
  namespace ui
  {
    // keys / modifiers and mouse button identifiers
    enum keys
    {
      key_space         = 32,
      key_apostrophe    = 39,  /* ' */
      key_comma         = 44,  /* , */
      key_minus         = 45,  /* - */
      key_period        = 46,  /* . */
      key_slash         = 47,  /* / */
      key_0             = 48,
      key_1             = 49,
      key_2             = 50,
      key_3             = 51,
      key_4             = 52,
      key_5             = 53,
      key_6             = 54,
      key_7             = 55,
      key_8             = 56,
      key_9             = 57,
      key_semicolon     = 59,  /* ; */
      key_equal         = 61,  /* = */
      key_a             = 65,
      key_b             = 66,
      key_c             = 67,
      key_d             = 68,
      key_e             = 69,
      key_f             = 70,
      key_g             = 71,
      key_h             = 72,
      key_i             = 73,
      key_j             = 74,
      key_k             = 75,
      key_l             = 76,
      key_m             = 77,
      key_n             = 78,
      key_o             = 79,
      key_p             = 80,
      key_q             = 81,
      key_r             = 82,
      key_s             = 83,
      key_t             = 84,
      key_u             = 85,
      key_v             = 86,
      key_w             = 87,
      key_x             = 88,
      key_y             = 89,
      key_z             = 90,
      key_left_bracket  = 91,  /* [ */
      key_backslash     = 92,  /* \ */
      key_right_bracket = 93,  /* ] */
      key_grave_accent  = 96,  /* ` */
      key_world_1       = 161, /* non-us #1 */
      key_world_2       = 162, /* non-us #2 */
      key_escape        = 256,
      key_enter         = 257,
      key_tab           = 258,
      key_backspace     = 259,
      key_insert        = 260,
      key_delete        = 261,
      key_right         = 262,
      key_left          = 263,
      key_down          = 264,
      key_up            = 265,
      key_page_up       = 266,
      key_page_down     = 267,
      key_home          = 268,
      key_end           = 269,
      key_caps_lock     = 280,
      key_scroll_lock   = 281,
      key_num_lock      = 282,
      key_pri32_screen  = 283,
      key_pause         = 284,
      key_f1            = 290,
      key_f2            = 291,
      key_f3            = 292,
      key_f4            = 293,
      key_f5            = 294,
      key_f6            = 295,
      key_f7            = 296,
      key_f8            = 297,
      key_f9            = 298,
      key_f10           = 299,
      key_f11           = 300,
      key_f12           = 301,
      key_f13           = 302,
      key_f14           = 303,
      key_f15           = 304,
      key_f16           = 305,
      key_f17           = 306,
      key_f18           = 307,
      key_f19           = 308,
      key_f20           = 309,
      key_f21           = 310,
      key_f22           = 311,
      key_f23           = 312,
      key_f24           = 313,
      key_f25           = 314,
      key_kp_0          = 320,
      key_kp_1          = 321,
      key_kp_2          = 322,
      key_kp_3          = 323,
      key_kp_4          = 324,
      key_kp_5          = 325,
      key_kp_6          = 326,
      key_kp_7          = 327,
      key_kp_8          = 328,
      key_kp_9          = 329,
      key_kp_decimal    = 330,
      key_kp_divide     = 331,
      key_kp_multiply   = 332,
      key_kp_subtract   = 333,
      key_kp_add        = 334,
      key_kp_enter      = 335,
      key_kp_equal      = 336,
      key_left_shift    = 340,
      key_left_control  = 341,
      key_left_alt      = 342,
      key_left_super    = 343,
      key_right_shift   = 344,
      key_right_control = 345,
      key_right_alt     = 346,
      key_right_super   = 347,
      key_menu          = 348,
      no_of_keys        = key_menu
    };

    enum mod
    {
      shift,
      ctrl,
      alt,
      super
    };

    enum mouse_button
    {
      lmb,
      rmb,
      mmb
    };

    // main state of ui - tells us everything we need to know about the window
    // TODO: Should this be renamed to context? 
    struct state
    {
      // graphical user i32raface state
      i32 hot_item;
      i32 active_item;

      // user input state
      i32 keys_down[ bsc::ui::no_of_keys ];
      r32 keys_down_duration[ bsc::ui::no_of_keys ];
      r32 keys_down_prev_duration[ bsc::ui::no_of_keys ];

      i32 mods_down[ 4 ];
      r32 mods_down_duration[ 4 ];

      i32 mouse_down[ 3 ];
      r32 mouse_down_duration[ 3 ];
      r32 mouse_down_prev_duration[ 3 ];

      vec2d mouse_pos;
      vec2d mouse_prev_pos;
      vec2d mouse_clicked_pos;
      vec2d mouse_released_pos;

      r32 mouse_wheel_offset;
      r32 mouse_wheel_prev_offset;

      i32 mouse_is_hovering_window;

      r32 frame_start_time;
      r32 frame_end_time;
      r32 delta_time;
    };

    void begin_frame();
    void end_frame();
#ifdef BSC_USE_IMGUI
    void init_imgui();
#endif
    // simplistic graphical user interface

    i32 button ( const char * text, const r32 x, const r32 y, 
                 const r32 w = 128, const r32 h=32, const r32 cor_rad=3 );
    i32 label ( const char * text, const r32 x, const r32 y );

    i32 slider ( const char * text, const i32 x, const i32 y, 
                 const i32 max, i32 &value );
    i32 slider ( const char * text, const i32 x, const i32 y, 
                 const r32 max, r32 &value );
    i32 slider ( const char * text, const i32 x, const i32 y, 
                 const r64 max, r64 &value );

    i32 flip_switch ( const char * text, const i32 x, const i32 y, bool & value );
    i32 checkbox( const char * text, const i32 x, const i32 y, bool & value );

    // TODO: implement those
    // i32 ProgressBar();
    // i32 ProgressCircle();
    // i32 InputText( const i32 id, const i32 x, i32 y, const char * text );

    // query based keyboard and mouse input handling
    i32 is_key_pressed( const i32 key_idx, const i8 mod_idx = -1 );
    i32 is_key_down( const i32 key_idx, const i8 mod_idx = -1 );
    i32 is_key_held( const i32 key_idx, const i8 mod_idx = -1 );
    i32 is_key_released( const i32 key_idx, const i8 mod_idx = -1 );

    i32 is_mouse_clicked ( const i32 button_idx, const i8 mod_idx = -1 );
    i32 is_mouse_held ( const i32 button_idx, const i8 mod_idx = -1 );
    i32 is_mouse_released ( const i32 button_idx, const i8 mod_idx = -1 );

    i32 is_mouse_over( const vec4i & rect );

    vec2d get_mouse_drag_delta( const i32 button_idx );
    r32 get_mouse_wheel_offset();

  }

  // TODO : This should return pointer, not reference
  static ui::state g_UI;
  ui::state & get_UI();

}

// TODO: asserts should not be in release code
// TODO: possible readability issues due to overuse of the if/else shortcut. Remodel, unroll loops?
// TODO: keyboard support for slider, use inputs values by hand

////////////////////////////////////////////////////////////////////////////////
// UI implementation
////////////////////////////////////////////////////////////////////////////////

#ifdef BSC_IMPLEMENTATION

bsc::ui::state & bsc::
get_UI()
{
  return g_UI;
}

#ifdef BSC_USE_IMGUI
// Forward declare new frame function for imgui
void ImGui_BscImpl_NewFrame( const bsc::window * window, 
                             const bsc::ui::state * ui_state );
#endif

void bsc::ui::
begin_frame()
{
  bsc::window & g_window = bsc::get_window();

  // we will decide which item is hot on per-frame basis
  g_UI.hot_item = 0;

  // update current mouse position within specified window
  g_UI.mouse_prev_pos = g_UI.mouse_pos;
  glfwGetCursorPos( g_window.handle, &g_UI.mouse_pos.x, &g_UI.mouse_pos.y);
  if ( !glfwGetWindowAttrib( g_window.handle, GLFW_FOCUSED) )
  {
    g_UI.mouse_pos = bsc::vec2d( -1.0f, -1.0f );
  }

  // update key durations -> based on a clever trick from ocornut's dear imgui
  for ( i32 key_idx = 0 ; key_idx < bsc::ui::no_of_keys ; ++key_idx )
  {
    g_UI.keys_down_prev_duration[ key_idx ] = g_UI.keys_down_duration[ key_idx ];
    g_UI.keys_down_duration[ key_idx ] =
      g_UI.keys_down[ key_idx ] ? ( g_UI.keys_down_duration[ key_idx ] == -1.0f ? 0.0f
                                : (g_UI.keys_down_duration[ key_idx ] + g_UI.delta_time) )
                                : -1.0f;
  }

  // update modifiers durations
  for ( i32 mod_idx = 0 ; mod_idx < 4 ; ++mod_idx )
  {
    g_UI.mods_down_duration[ mod_idx ] =
      g_UI.mods_down[ mod_idx ] ? ( g_UI.mods_down_duration[ mod_idx ] == -1.0f ? 0.0f
                                : g_UI.mods_down_duration[ mod_idx ] + g_UI.delta_time )
                                : -1.0f;
  }

  // update mouse_buttons durations
  for ( i32 mb_idx = 0 ; mb_idx < 3 ; ++mb_idx )
  {
    g_UI.mouse_down_prev_duration[ mb_idx ] = g_UI.mouse_down_duration[ mb_idx ];
    g_UI.mouse_down_duration[ mb_idx ] =
      g_UI.mouse_down[ mb_idx ] ? ( g_UI.mouse_down_duration[ mb_idx ] == -1.0f ? 0.0f
                                : g_UI.mouse_down_duration[ mb_idx ] + g_UI.delta_time )
                                : -1.0f;
    // HACK -> if any of mouse buttons is down, let's force glfw to refresh
    // if ( g_UI.mouse_down[ mb_idx ] ) glfwPostEmptyEvent();

  }

  // reset mouse state - based on dear imgui glfw example
  g_UI.mouse_wheel_offset      = g_UI.mouse_wheel_prev_offset;
  g_UI.mouse_wheel_prev_offset = 0.0f;

  // HACK -> if there has been a wheel spin, let's force glfw to refresh
  // if ( g_UI.mouse_wheel_offset != 0.0f ) glfwPostEmptyEvent();

  g_UI.frame_start_time = glfwGetTime();

#ifdef BSC_USE_IMGUI
  ImGui_BscImpl_NewFrame( &g_window, &g_UI );
#endif
}

void bsc::ui::
end_frame ()
{
  // if left mouse button is not down on this frame, reset active item
  if ( g_UI.mouse_down[ bsc::ui::lmb ] == 0 )
  {
     g_UI.active_item = 0;
  }

  // update frame time
  g_UI.frame_end_time = glfwGetTime();
  g_UI.delta_time = g_UI.frame_end_time - g_UI.frame_start_time;

}


// mouse and keyboard input
i32 bsc::ui::
is_key_down ( const i32 key_idx, const i8 mod_idx )
{
  assert( key_idx >= 0 && key_idx < bsc::ui::no_of_keys );

  return ( mod_idx < 0 ) ? g_UI.keys_down_duration[ key_idx ] >= 0.0f
                         : ( g_UI.keys_down_duration[ key_idx ] >= 0.0f && 
                             g_UI.mods_down_duration[ mod_idx ] >= 0.0f );
}

i32 bsc::ui::
is_key_pressed ( const i32 key_idx, const i8 mod_idx )
{
  assert( key_idx >= 0 && key_idx < bsc::ui::no_of_keys );

  return ( mod_idx < 0 ) ? g_UI.keys_down_duration[ key_idx ] == 0.0f
                         : ( g_UI.keys_down_duration[ key_idx ] == 0.0f && 
                             g_UI.mods_down_duration[ mod_idx ] >= 0.0f );
}

i32 bsc::ui::
is_key_held ( const i32 key_idx, const i8 mod_idx )
{
  assert( key_idx >= 0 && key_idx < bsc::ui::no_of_keys );

  return ( mod_idx < 0 ) ? g_UI.keys_down_duration[ key_idx ] > 0.0f
                         : ( g_UI.keys_down_duration[ key_idx ] > 0.0f && 
                             g_UI.mods_down_duration[ mod_idx ] >= 0.0f );
}

i32 bsc::ui::
is_key_released ( const i32 key_idx, const i8 mod_idx )
{
  assert( key_idx >= 0 && key_idx < bsc::ui::no_of_keys );

  bool was_released = ( g_UI.keys_down_duration[ key_idx ] == -1.0f && 
                        g_UI.keys_down_prev_duration[ key_idx ] > 0.0f );
  bool was_released_mod = was_released && 
                          g_UI.mods_down_duration[ mod_idx ] >= 0.0f;

  return ( mod_idx < 0 ) ? was_released
                     : was_released_mod;
}

i32 bsc::ui::
is_mouse_clicked( const i32 button_idx, const i8 mod_idx )
{
  assert( button_idx >= 0 && button_idx < 3 );

  bool was_clicked = (g_UI.mouse_down_duration[ button_idx ] == 0.0f);

  if ( mod_idx < 0 )
  {
    if ( was_clicked ) g_UI.mouse_clicked_pos = g_UI.mouse_pos;
    return was_clicked;
  }
  else
  {
    bool was_clicked_mod = was_clicked && 
                         g_UI.mods_down_duration[ mod_idx ] >= 0.0f;
    if ( was_clicked_mod ) g_UI.mouse_clicked_pos = g_UI.mouse_pos;
    return was_clicked_mod;
  }
}

i32 bsc::ui::
is_mouse_held( const i32 button_idx, const i8 mod_idx )
{
  assert ( button_idx >= 0 && button_idx < 3 );

  bool mod_is_on = (g_UI.mods_down_duration[ shift ] >= 0.0) ||
                   (g_UI.mods_down_duration[ ctrl ]  >= 0.0) ||
                   (g_UI.mods_down_duration[ alt ]   >= 0.0) ||
                   (g_UI.mods_down_duration[ super ] >= 0.0);
  
  if ( mod_is_on )
  {

    if ( mod_idx < 0 )
    {
      // we're holding modifier, but function doesn't specify one -> no held
      return false;
    }
    else
    {
      return ( g_UI.mouse_down_duration[ button_idx ] > 0.0f &&
               g_UI.mods_down_duration[ mod_idx ] >= 0.0f ) ? 1
                                                            : 0;
    }
  }
  else
  {
    if ( mod_idx >= 0 ) 
    {
      // we are not holding modifier, but function specifies one -> no held
      return false;
    }
    else
    {
      return ( g_UI.mouse_down_duration[ button_idx ] > 0.0f ) ? 1
                                                               : 0;  
    }
  }
  return false;
}

i32 bsc::ui::
is_mouse_over( const vec4i & rect )
{
  return ( g_UI.mouse_pos.x > rect.x && 
           g_UI.mouse_pos.y > rect.y &&
           g_UI.mouse_pos.x <= (rect.x + rect.z) && 
           g_UI.mouse_pos.y <= (rect.y + rect.w) ) ? 1 : 0;
}

i32 bsc::ui::
is_mouse_released  ( const i32 button_idx, const i8 mod_idx )
{
  assert( button_idx >= 0 && button_idx < 3 );

  bool was_released = ( g_UI.mouse_down_duration[ button_idx ] == -1.0f ) && 
                      ( g_UI.mouse_down_prev_duration[ button_idx ] > 0.0f );

  if ( was_released ) g_UI.mouse_released_pos = g_UI.mouse_pos;

  return was_released;
}

bsc::vec2d bsc::ui::
get_mouse_drag_delta ( const i32 button_idx )
{
  assert ( button_idx >= 0 && button_idx < 3 );

  bool is_duration_positive = ( g_UI.mouse_down_duration[ button_idx ] > 0.0f );
  return is_duration_positive ? ( g_UI.mouse_pos - g_UI.mouse_prev_pos )
                              : bsc::vec2d( 0.0, 0.0 );
}

r32 bsc::ui::
get_mouse_wheel_offset ()
{
    return g_UI.mouse_wheel_offset;
}

i32 bsc::ui::
button ( const char * title, const r32 x, const r32 y, 
         const r32 w, const r32 h, const r32 corner_rad )
{
  i32 retval = false;
//Basic interface is currently disabled
#if 0 
  // manage ui state
  int id = string_hash( title );
  vec4f rect ( x, y, w, h );
  if ( ui::is_mouse_over( rect ) )
  {
    g_UI.hot_item = id;
  }

  if ( ui::is_mouse_over( rect ) && ui::is_mouse_clicked( ui::lmb ) )
  {
    g_UI.active_item = id;
  }

  if ( g_UI.active_item == id && ui::is_mouse_released ( ui::lmb ) )
  {
    g_UI.active_item = 0;
    retval = true;
  }
#endif
  return retval;
}

i32 bsc::ui::
slider ( const char * title, const i32 x, const i32 y, const i32 max, 
         i32 &value )
{
  int retval = 0;
//Basic interface is currently disabled
#if 0 
  int id = string_hash( title );

  // Check for hotness
  vec4f rect ( x, y, 255, 8 );
  if ( ui::is_mouse_over( rect ) )
  {
    g_UI.hot_item = id;
    if (g_UI.active_item == 0 && g_UI.mouse_down[ lmb ] )
      g_UI.active_item = id;
  }

  // Update widget value
  if ( g_UI.active_item == id)
  {
    int mousepos = g_UI.mouse_pos.x - x;
    if (mousepos < 0) mousepos = 0;
    if (mousepos > 255) mousepos = 255;
    int v = (mousepos * max) / 255;
    if (v != value)
    {
      value = v;
      retval = 1;
    }
  }
#endif
  return retval;
}

i32 bsc::ui::
flip_switch ( const char * title, const i32 x, const i32 y, bool & value )
{
   i32 retval = false;
  int id = string_hash( title );

  // manage ui state
  vec4i rect ( x, y, 56, 24 );
  if ( ui::is_mouse_over( rect ) )
  {
    g_UI.hot_item = id;
  }

  if ( ui::is_mouse_over( rect ) && ui::is_mouse_clicked( ui::lmb ) )
  {
    g_UI.active_item = id;
  }

  if ( g_UI.active_item == id && ui::is_mouse_released ( ui::lmb ) )
  {
    g_UI.active_item = 0;
    value = !value;
    retval = true;
  }

  return retval;
}


i32 bsc::ui::
checkbox ( const char * text, const i32 x, const i32 y, bool & value )
{
  i32 retval = false;
  int id = string_hash( text );

  // manage ui state
  // TODO: checked rectange width should be given by the text bounds
  vec4i rect ( x, y, 128, 24 );

  if ( ui::is_mouse_over( rect ) )
  {
    g_UI.hot_item = id;
  }

  if ( ui::is_mouse_over( rect ) && ui::is_mouse_clicked( ui::lmb ) )
  {
    g_UI.active_item = id;
  }

  if ( g_UI.active_item == id && ui::is_mouse_released ( ui::lmb ) )
  {
    g_UI.active_item = 0;
    value = !value;
    retval = true;
  }

  
  return retval;
}


i32 bsc::ui::
label ( const char * text, const r32 x, const r32 y )
{
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
// Callbacks
////////////////////////////////////////////////////////////////////////////////

void
keyboard_callback ( GLFWwindow*, i32 key, i32, i32 action, i32 mods )
{
  using namespace bsc;

  bsc::ui::state & ui = bsc::get_UI();

  // record key presses
  if ( action == GLFW_PRESS )
    ui.keys_down[ key ] = true;
  if ( action == GLFW_RELEASE )
    ui.keys_down[ key ] = false;

  // record modifiers separately for convenience
  ui.mods_down[ ui::shift ] = ui.keys_down[ ui::key_left_shift ]   || 
                              ui.keys_down[ ui::key_right_shift ];
  ui.mods_down[ ui::alt ]   = ui.keys_down[ ui::key_left_alt ]     || 
                              ui.keys_down[ ui::key_right_alt ];
  ui.mods_down[ ui::ctrl ]  = ui.keys_down[ ui::key_left_control ] || 
                              ui.keys_down[ ui::key_right_control ];
  ui.mods_down[ ui::super ] = ui.keys_down[ ui::key_left_super ]   || 
                              ui.keys_down[ ui::key_right_super ];
#ifdef BSC_USE_IMGUI
  ImGuiIO& io = ImGui::GetIO();
  if (action == GLFW_PRESS)
    io.KeysDown[key] = true;
  if (action == GLFW_RELEASE)
    io.KeysDown[key] = false;

  (void)mods; // Modifiers are not reliable across systems
  io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
  io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
#endif
}

void 
char_callback( GLFWwindow*, unsigned int c )
{
#ifdef BSC_USE_IMGUI
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
#endif
}

void
mouse_button_callback( GLFWwindow* window, i32 button, i32 action, i32 )
{
  bsc::ui::state & ui = bsc::get_UI();

  if ( action == GLFW_PRESS )   ui.mouse_down[ button ] = true;
  if ( action == GLFW_RELEASE ) ui.mouse_down[ button ] = false;
#ifdef BSC_USE_IMGUI

#endif
}

void
scroll_callback( GLFWwindow* window, double /**/, double yoffset )
{
  bsc::ui::state & ui = bsc::get_UI();
  ui.mouse_wheel_prev_offset += yoffset;

}

void
cursor_enter_callback( GLFWwindow* window, i32 entered )
{
  bsc::ui::state & ui = bsc::get_UI();

  if ( entered ) ui.mouse_is_hovering_window = true;
  else           ui.mouse_is_hovering_window = false;
}

void
window_size_callback( GLFWwindow* window, i32 width, i32 height )
{
  bsc::window & g_window = bsc::get_window();
  g_window.size.x = width;
  g_window.size.y = height;
  g_window.pixel_ratio = (r32)g_window.fb_size.x / (r32)g_window.size.x;
  g_window.aspect_ratio = (r32)g_window.size.x / (r32)g_window.size.y;
}

void
framebuffer_size_callback( GLFWwindow* window, i32 width, i32 height )
{
  bsc::window & g_window = bsc::get_window();
  g_window.fb_size.x = width;
  g_window.fb_size.y = height;
  g_window.pixel_ratio = (r32)g_window.fb_size.x / (r32)g_window.size.x;
  g_window.aspect_ratio = (r32)g_window.size.x / (r32)g_window.size.y;
}

// this is called when window is resized!
void
window_refresh_callback( GLFWwindow* window )
{
    bsc::window & g_window = bsc::get_window();
    
    g_window.display();
    
    glfwSwapBuffers( window );
}

void
drop_callback( GLFWwindow* window, i32 count, const char ** paths )
{
    bsc::window & g_window = bsc::get_window();
    g_window.drag_and_drop( count, paths );
}

#ifdef BSC_USE_IMGUI

// TODO all the good biz with the render drawlist and initialization
static double       g_Time = 0.0f;
static GLuint       g_FontTexture = 0;
static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_VaoHandle = 0, g_ElementsHandle = 0;


void
ImGui_BscImpl_SetupStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text]                  = ImVec4( 0.9f, 0.9f, 0.9f, 1.00f );
    style.Colors[ImGuiCol_WindowBg]              = ImVec4( 0.2f, 0.2f, 0.2f, 1.00f );

    // Background of sliders / checkboxes etc.
    style.Colors[ImGuiCol_FrameBg]               = ImVec4( 0.37f, 0.39f, 0.43f, 1.00f );
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4( 0.47f, 0.49f, 0.53f, 1.00f );
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4( 0.57f, 0.59f, 0.63f, 1.00f );

    // Window header
    style.Colors[ImGuiCol_TitleBg]               = ImVec4( 0.29f, 0.3f, 0.31f, 1.00f );
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4( 0.19f, 0.2f, 0.21f, 1.00f );
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4( 0.19f, 0.2f, 0.21f, 1.00f );

    // Checkbox
    style.Colors[ImGuiCol_CheckMark]             = ImVec4( 0.29f, 0.30f, 0.31f, 1.00f );

    // Slider grab
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4( 0.29f, 0.30f, 0.31f, 1.00f );
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f );

    // Button
    style.Colors[ImGuiCol_Button]                = ImVec4( 0.31f, 0.32f, 0.36f, 1.00f );
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4( 0.33f, 0.34f, 0.38f, 1.00f );
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4( 0.15f, 0.16f, 0.19f, 1.00f );

    // Collapsing lists
    style.Colors[ImGuiCol_Header]                = ImVec4( 0.15f, 0.16f, 0.16f, 1.00f );
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4( 0.43f, 0.44f, 0.48f, 1.00f );
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4( 0.15f, 0.16f, 0.19f, 1.00f );

    // Scrollbag
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4( 0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4( 0.29f, 0.30f, 0.31f, 1.00f  );
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4( 0.29f, 0.30f, 0.31f, 1.00f  );
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4( 0.29f, 0.30f, 0.31f, 1.00f  );


    style.Alpha                  = 1.0f;
    style.WindowPadding          = ImVec2( 10.0f, 10.0f );
    style.WindowMinSize          = ImVec2( 32.0f, 32.0f );
    style.WindowRounding         = 0.0f;
    style.ChildWindowRounding    = 2.0f;
    style.FramePadding           = ImVec2( 3.0f, 3.0f );;
    style.FrameRounding          = 7.5f;
    style.ItemSpacing            = ImVec2( 6.0f, 4.0f );
    style.ItemInnerSpacing       = ImVec2( 6.0f, 4.0f );
    style.IndentSpacing          = 16.0f;
    style.ColumnsMinSpacing      = 1.0f;
    style.ScrollbarSize          = 20.0f;
    style.ScrollbarRounding      = 2.0f;
    style.GrabMinSize            = 6.0f;
    style.GrabRounding           = 7.5f;
}

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void 
ImGui_BscImpl_RenderDrawLists(ImDrawData* draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state
    GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, &last_active_texture);
    GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
    GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
    GLint last_blend_src; glGetIntegerv(GL_BLEND_SRC, &last_blend_src);
    GLint last_blend_dst; glGetIntegerv(GL_BLEND_DST, &last_blend_dst);
    GLint last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, &last_blend_equation_rgb);
    GLint last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &last_blend_equation_alpha);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box); 
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    const float ortho_projection[4][4] =
    {
        { 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
        { 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
        { 0.0f,                  0.0f,                  -1.0f, 0.0f },
        {-1.0f,                  1.0f,                   0.0f, 1.0f },
    };
    glUseProgram(g_ShaderHandle);
    glUniform1i(g_AttribLocationTex, 0);
    glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
    glBindVertexArray(g_VaoHandle);

    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    // Restore modified GL state
    glUseProgram(last_program);
    glActiveTexture(last_active_texture);
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindVertexArray(last_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFunc(last_blend_src, last_blend_dst);
    if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

static const char* ImGui_BscImpl_GetClipboardText()
{
    return glfwGetClipboardString( bsc::g_window.handle );
}

static void ImGui_BscImpl_SetClipboardText(const char* text)
{
    glfwSetClipboardString( bsc::g_window.handle, text );
}

bool ImGui_BscImpl_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);

    return true;
}

// TODO: Do it with basics?
bool 
ImGui_BscImpl_CreateDeviceObjects()
{
    // Backup GL state
    GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    const GLchar *vertex_shader =
        "#version 330\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "	Frag_UV = UV;\n"
        "	Frag_Color = Color;\n"
        "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar* fragment_shader =
        "#version 330\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
        "}\n";

    g_ShaderHandle = glCreateProgram();
    g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
    g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
    glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
    glCompileShader(g_VertHandle);
    glCompileShader(g_FragHandle);
    glAttachShader(g_ShaderHandle, g_VertHandle);
    glAttachShader(g_ShaderHandle, g_FragHandle);
    glLinkProgram(g_ShaderHandle);

    g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
    g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
    g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
    g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

    glGenBuffers(1, &g_VboHandle);
    glGenBuffers(1, &g_ElementsHandle);

    glGenVertexArrays(1, &g_VaoHandle);
    glBindVertexArray(g_VaoHandle);
    glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    glEnableVertexAttribArray(g_AttribLocationPosition);
    glEnableVertexAttribArray(g_AttribLocationUV);
    glEnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

    ImGui_BscImpl_CreateFontsTexture();

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindVertexArray(last_vertex_array);

    return true;
}

void    
ImGui_BscImpl_InvalidateDeviceObjects()
{
    if ( g_VaoHandle ) glDeleteVertexArrays(1, &g_VaoHandle);
    if ( g_VboHandle ) glDeleteBuffers( 1, &g_VboHandle);
    if ( g_ElementsHandle ) glDeleteBuffers( 1, &g_ElementsHandle );
    g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

    glDetachShader(g_ShaderHandle, g_VertHandle);
    glDeleteShader(g_VertHandle);
    g_VertHandle = 0;

    glDetachShader(g_ShaderHandle, g_FragHandle);
    glDeleteShader(g_FragHandle);
    g_FragHandle = 0;

    glDeleteProgram(g_ShaderHandle);
    g_ShaderHandle = 0;

    if (g_FontTexture)
    {
        glDeleteTextures(1, &g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture = 0;
    }
}

bool    
ImGui_BscImpl_Init( GLFWwindow* window )
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                         // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.RenderDrawListsFn = ImGui_BscImpl_RenderDrawLists;       // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.SetClipboardTextFn = ImGui_BscImpl_SetClipboardText;
    io.GetClipboardTextFn = ImGui_BscImpl_GetClipboardText;
#ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(g_Window);
#endif
    ImGui_BscImpl_SetupStyle();

    // I guess imgui is trying to delete this?
    void * font_data = malloc( ubuntu_mono_ttf_len );
    memcpy( font_data, &(ubuntu_mono_ttf[0]), ubuntu_mono_ttf_len );
    ImFont * font = io.Fonts->AddFontFromMemoryTTF( font_data, 
                                                    ubuntu_mono_ttf_len, 
                                                    14.0);
    font->DisplayOffset.y -= 1;                               
    return true;
}

void ImGui_BscImpl_Shutdown()
{
    ImGui_BscImpl_InvalidateDeviceObjects();
    ImGui::Shutdown();
}

void ImGui_BscImpl_NewFrame( const bsc::window * window, 
                             const bsc::ui::state * ui_state )
{
  
    if ( !g_FontTexture )
        ImGui_BscImpl_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2( window->size.x, window->size.y );
    io.DisplayFramebufferScale = ImVec2( window->fb_size.x / window->size.x,
                                         window->fb_size.y / window->size.y );

    // Setup time step
    double current_time =  glfwGetTime();
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f/60.0f);
    g_Time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (glfwGetWindowAttrib( window->handle, GLFW_FOCUSED))
    {
      io.MousePos = ImVec2( ui_state->mouse_pos.x, 
                            ui_state->mouse_pos.y);   // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
    }
    else
    {
      io.MousePos = ImVec2(-1,-1);
    }

    for (int i = 0; i < 3; i++)
    {
      // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
      io.MouseDown[i] = ui_state->mouse_down[i] || 
                        glfwGetMouseButton( window->handle, i ) != 0;    
    }

    io.MouseWheel = ui_state->mouse_wheel_offset;

    // Hide OS mouse cursor if ImGui is drawing it
    glfwSetInputMode( window->handle, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    // Start the frame
    ImGui::NewFrame();
}

#endif

#endif
