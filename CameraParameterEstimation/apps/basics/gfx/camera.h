#pragma once

// TODO: implement first person controls
// TODO Fix trackball starting position bug
namespace bsc
{
  struct camera
  {
    vec3 position;
    quatf orientation;
  };

  struct first_person_controls
  {
    r32 speed;
    r32 Pitch;
    r32 Yaw;

    bool rotate ( bsc::camera * cam, const bsc::mat3f * orientation );
    bool pan ( bsc::camera * cam, const bsc::mat3f * orientation );
    bool move ( bsc::camera * cam, const bsc::mat3f * orientation );

    mat4 initialize ( bsc::camera * camera, 
                            const bsc::vec3 & cam_pos, 
                            const bsc::vec3 & center, 
                            const bsc::vec3 & up );
    void update ( bsc::camera * cam, bsc::mat4 * view  );
  };

  struct trackball_controls
  {
    // hand tuned defaults
    r32 rotation_speed = 2.0f;
    r32 zoom_speed     = 1.2f;
    r32 pan_speed      = 0.94f;

    // TODO: This helper is a hack that I do not necessarily understand. Need to look more into that.
    vec3 helper;

    bool zoom ( bsc::camera * cam, 
                const bsc::mat4 * view );
    bool pan ( bsc::camera * cam, 
               const bsc::mat4 * view,
               const bsc::vec4i * viewport );
    bool rotate ( bsc::camera * cam, 
                  const bsc::mat4 * view,
                  const bsc::vec4i * viewport );

    bsc::mat4 initialize ( bsc::camera * camera, 
                           const bsc::vec3 & cam_pos, 
                           const bsc::vec3 & center, 
                           const bsc::vec3 & up );
    void update ( bsc::camera * cam, 
                  bsc::mat4 * view, 
                  const bsc::vec4i * viewport );

    void set_front_view( bsc::camera * cam, bsc::mat4 * view );
    void set_back_view( bsc::camera * cam, bsc::mat4 * view );
    void set_right_view( bsc::camera * cam, bsc::mat4 * view );
    void set_left_view( bsc::camera * cam, bsc::mat4 * view );
    void set_top_view( bsc::camera * cam, bsc::mat4 * view );
    void set_bottom_view( bsc::camera * cam, bsc::mat4 * view );
  };
}


#ifdef BSC_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////
// Camera controls - Trackball
////////////////////////////////////////////////////////////////////////////////

static bsc::vec3
click_to_sphere( float x, float y, const bsc::vec4i * viewport )
{
  using namespace bsc;
  r32 w = (*viewport)[2] - (*viewport)[0];
  r32 h = (*viewport)[3] - (*viewport)[1];

  vec3 pos ( 2.0 * x / w - 1.0,
             1.0 - 2.0 * y / h ,
             0.0 );

  r32 length    = norm( pos );
  r32 length_sq = square( length );

  if ( length_sq > 0.5f ) pos.z = 0.5f / length;
  else                    pos.z = sqrtf( 1.0f - length_sq );

  pos = normalize( pos );

  return pos;
}

bsc::mat4 bsc::trackball_controls::
initialize ( bsc::camera * camera, 
             const bsc::vec3 & cam_pos, 
             const bsc::vec3 & center, 
             const bsc::vec3 & up )
{
  bsc::mat4 view     = look_at( cam_pos, center, up );
  camera->position    = cam_pos;
  camera->orientation = to_quat( view );

  helper = vec3( 0, 0, norm(cam_pos) );
  bsc::quatf p( 0, helper );
  p = camera->orientation * p * conjugate( camera->orientation );
  helper = p.im;

  return view;
}

bool bsc::trackball_controls::
zoom ( bsc::camera * cam, 
       const bsc::mat4 * view )
{
  float step = 0.1f;
  float wheel_state = bsc::ui::get_mouse_wheel_offset();

  if ( wheel_state )
  {
    vec3 forward  = -(*view)[ 2 ] ;
    helper += forward * step * zoom_speed * wheel_state;
    return true;
  }
  return false;
}

bool bsc::trackball_controls::
pan( bsc::camera * cam, 
     const bsc::mat4 * view,
     const bsc::vec4i * viewport )
{
  ui::state & UI = get_UI();

  i8 is_mmb = ui::is_mouse_held( ui::mmb );

  if ( is_mmb )
  {
    vec2d mouse_pos  = UI.mouse_pos;
    vec2d mouse_drag = ui::get_mouse_drag_delta ( ui::mmb );
    if ( norm( mouse_drag ) == 0 ) return false;

    r32 f = norm( helper ) * this->pan_speed;
    r32 w = (*viewport)[2] - (*viewport)[0];
    r32 h = (*viewport)[3] - (*viewport)[1];
    vec3 offset = ( f * ( (*view)[0]) * (r32)( mouse_drag.x / w ) ) +
                  ( f * (-(*view)[1]) * (r32)( mouse_drag.y / h ) );

    helper += offset;

    return true;
  }
  return false;
}

bool bsc::trackball_controls::
rotate ( bsc::camera * cam, 
         const bsc::mat4 * view,
         const bsc::vec4i * viewport )
{
  ui::state & UI = get_UI();

  if ( ui::is_mouse_held( ui::lmb ) )
  {
    vec2d mouse_pos  = UI.mouse_pos;
    vec2d mouse_drag = ui::get_mouse_drag_delta ( ui::lmb );
    if ( norm( mouse_drag ) == 0 ) return false;

    vec3 start = click_to_sphere( mouse_pos.x, 
                                  mouse_pos.y,
                                  viewport );
    vec3   end = click_to_sphere( mouse_pos.x + mouse_drag.x, 
                                  mouse_pos.y + mouse_drag.y ,
                                  viewport );

    // get rotation axis in world coordinates
    vec3 axis = normalize( cross( start, end ) );

    r32 angle  = acosf( min( dot( start, end ), 1.0f ) );
    angle *= this->rotation_speed;

    // TODO: add quaternion from angle axis
    bsc::quatf cur_rot( cosf( angle * 0.5f ), axis * sinf( angle * 0.5f ) );
    cam->orientation = cur_rot * cam->orientation ;

    // TODO: and point transform using quaternions
    bsc::quatf tmp_camera_pos( 0, helper );
    tmp_camera_pos = cur_rot * tmp_camera_pos * conjugate( cur_rot );
    helper = tmp_camera_pos.im;

    return true;
  }
  return false;
}

void bsc::trackball_controls::
update( bsc::camera * cam, bsc::mat4 * view, const bsc::vec4i * viewport )
{
  bsc::ui::state & UI = bsc::get_UI();

  bool hover = ( UI.hot_item > 0 || UI.active_item > 0 ) ? 1 : 0;
  bool update = false;

  if ( !hover )
  {
    update |= zoom( cam, view );
    update |= pan( cam, view, viewport );
    update |= rotate( cam, view, viewport );

    if ( update )
    {
      mat4 cur_view = to_mat4( cam->orientation );

      cur_view[3] = vec4f(  dot( helper, vec3( cur_view[0] ) ),
                            dot( helper, vec3( cur_view[1] ) ),
                           -dot( helper, vec3( cur_view[2] ) ), 1.0f );
      (*view)       = cur_view;
      cam->position = vec3( inverse( *view )[3] );
    }
  }
}

void bsc::trackball_controls::
set_front_view( bsc::camera * cam, bsc::mat4 * view )
{
  (*view) = initialize( cam, bsc::vec3( 0.0f, 0.0f, 10.0f ),
                             bsc::vec3( 0.0f, 0.0f, 0.0f),
                             bsc::vec3( 0.0f, 1.0f, 0.0f) );
}

void bsc::trackball_controls::
set_back_view( bsc::camera * cam, bsc::mat4 * view )
{
  (*view) = initialize( cam, bsc::vec3( 0.0f, 0.0f, -10.0f ),
                             bsc::vec3( 0.0f, 0.0f, 0.0f),
                             bsc::vec3( 0.0f, 1.0f, 0.0f) );
}

void bsc::trackball_controls::
set_right_view( bsc::camera * cam, bsc::mat4 * view )
{
  (*view) = initialize( cam, bsc::vec3( 10.0f, 0.0f, 0.0f ),
                             bsc::vec3( 0.0f, 0.0f, 0.0f),
                             bsc::vec3( 0.0f, 1.0f, 0.0f) );
}

void bsc::trackball_controls::
set_left_view( bsc::camera * cam, bsc::mat4 * view )
{
  (*view) = initialize( cam, bsc::vec3( -10.0f, 0.0f, 0.0f ),
                             bsc::vec3( 0.0f, 0.0f, 0.0f),
                             bsc::vec3( 0.0f, 1.0f, 0.0f) );
}

void bsc::trackball_controls::
set_top_view( bsc::camera * cam, bsc::mat4 * view )
{
  // just move camera minimally along z, to prevent the degenerate case
  (*view) = initialize( cam, bsc::vec3( 0.0f, 10.0f, 0.001f ),
                             bsc::vec3( 0.0f, 0.0f, 0.0f),
                             bsc::vec3( 0.0f, 1.0f, 0.0f) );
}


void bsc::trackball_controls::
set_bottom_view( bsc::camera * cam, bsc::mat4 * view )
{
  // just move camera minimally along z, to prevent the degenerate case
  (*view) = initialize( cam, bsc::vec3( 0.0f, -10.0f, -0.001f ),
                             bsc::vec3( 0.0f, 0.0f, 0.0f),
                             bsc::vec3( 0.0f, 1.0f, 0.0f) );
}
////////////////////////////////////////////////////////////////////////////////
// Camera controls - Firstperson
////////////////////////////////////////////////////////////////////////////////

// TODO: tighten first person camera controls. 
// Maybe go back to pitch, roll, yaw setup?

/*
bool bsc::first_person_controls::
rotate ( bsc::camera * cam, const bsc::mat3f * orientation )
{
  ui::state & UI = get_UI();

  if ( ui::is_mouse_held( ui::lmb ) )
  {
    vec2d mouse_pos  = UI.mouse_pos;
    vec2d mouse_drag = ui::get_mouse_drag_delta ( ui::lmb );
    if ( norm( mouse_drag ) == 0 ) return false;

    vec3 start = click_to_sphere( mouse_pos.x, mouse_pos.y );
    vec3   end = click_to_sphere( mouse_pos.x + mouse_drag.x, 
                                mouse_pos.y + mouse_drag.y );

    // get rotation axis in world coordinates
    vec3 axis = normalize( cross( start, end ) );
    // axis = (*orientation) * axis;

    r32 angle  = acosf( min( dot( start, end ), 1.0f ) );
    angle *= 2.4f;

    // TODO: add quaternion from angle axis
    quatf cur_rot( cosf( angle * 0.5f ), axis * sinf( angle * 0.5f ) );

    cam->orientation = cam->orientation * cur_rot ;

    return true;
  }
  return false;
}


bool bsc::first_person_controls::
pan( bsc::camera * camera, const bsc::mat3f * orientation )
{
  if ( ui::is_mouse_held( ui::rmb ) )
  {
    vec2d mouse_drag = ui::get_mouse_drag_delta ( ui::rmb );
    if ( norm( mouse_drag ) == 0 ) return false;

    camera->position -= 0.03f * (r32)mouse_drag.x * 
                        this->speed * (*orientation)[ 0 ];
    camera->position += 0.03f * (r32)mouse_drag.y * 
                        this->speed * (*orientation)[ 1 ];

    return true;
  }

  return false;
}

bool bsc::first_person_controls::
move ( bsc::camera * camera, const bsc::mat3f * orientation )
{

  bool moved = false;

  if ( bsc::ui::is_key_down( bsc::ui::key_w ) )
  {
    camera->position -= this->speed * (*orientation)[ 2 ];
    moved = true;
  }

  if ( bsc::ui::is_key_down( bsc::ui::key_s ) )
  {
    camera->position += this->speed * (*orientation)[ 2 ];
    moved = true;
  }

  if ( bsc::ui::is_key_down( bsc::ui::key_a ) )
  {
    camera->position -= this->speed * (*orientation)[ 0 ];
    moved = true;
  }

  if ( bsc::ui::is_key_down( bsc::ui::key_d ) )
  {
    camera->position += this->speed * (*orientation)[ 0 ];
    moved = true;
  }

  if ( bsc::ui::is_key_down( bsc::ui::key_q ) )
  {
    camera->position += this->speed * (*orientation)[ 1 ];
    moved = true;
  }

  if ( bsc::ui::is_key_down( bsc::ui::key_e ) )
  {
    camera->position -= this->speed * (*orientation)[ 1 ];
    moved = true;
  }

  return moved;
}

bsc::mat4 bsc::first_person_controls::
initialize( bsc::camera * camera, 
            const bsc::vec3 & cam_pos, 
            const bsc::vec3 & center, 
            const bsc::vec3 & up )
{
  this->speed = 0.2f; // change speed to velocity?

  mat4 view          = look_at( cam_pos, center, up );
  camera->position    = cam_pos;
  camera->orientation = to_quat( view );

  return view;
}

void bsc::first_person_controls::
update ( bsc::camera * cam, bsc::mat4 * view )
{
  bsc::ui::state & UI = bsc::get_UI();

  bool hover = ( UI.hot_item > 0 || UI.active_item > 0 ) ? 1 : 0;
  bool update = false;

  mat3f orientation = to_mat3( cam->orientation );

  if ( !hover )
  {
    update |= rotate( cam, &orientation );
    update |= pan( cam, &orientation );
  }

  update |= move( cam, &orientation );

  if ( update )
  {
    mat4 cur_view = to_mat4( cam->orientation );
    cur_view[3]    = vec4f( cam->position, 1.0 );
    (*view)        = inverse( cur_view );
  }
}
*/
#endif
