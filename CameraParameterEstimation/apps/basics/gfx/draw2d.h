#pragma once

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////

// TODO: change this in favour to dynamic memory allocation?
#define MAX_CMDS 8192

namespace bsc
{
  // NOTE : Decide whether we want to have draw_2d and draw_3d or merge them.
  
  namespace draw2d
  {
    enum cmd_t
    {
      LINE_START,
      LINE_TO,
      LINE_END
    };

    struct cmd
    {
      cmd_t type;
      bsc::vec3f pos;
      r32 radius;
    };

    struct context
    {
      cmd cmds[ MAX_CMDS ];
      i32 cmd_idx;
    };
    
    void start_frame();

    void start_line( bsc::vec2 point, r32 width = 1.0 );
    void line_to( bsc::vec2 point, r32 width = 1.0 );
    void end_line( i8 merge = false );

    // NOTE: This should be renderer agnostic
    void initialize();
    void render(); 
  }


  // NOTE : Should context be private ?
  static draw2d::context g_ctx2d;
  draw2d::context * get_2d_context();

}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

#ifdef BSC_IMPLEMENTATION


bsc::draw2d::context * bsc::
get_2d_context()
{
  return &g_ctx2d;
}

// we will be constructing command buffer each frame.
void bsc::draw2d::
start_frame() // TODO: Take context?
{
  g_ctx2d.cmd_idx = 0;
}

// TODO: maybe commands should be just enums and this should be closer to simplistic
// malloc? We can interpret the size and type by enum...
// NOTE: Lines are hard. I should start with circles or rectangles.

void bsc::draw2d::
start_line( bsc::vec2 point, r32 width )
{
  g_ctx2d.cmds[ g_ctx2d.cmd_idx++ ] = { LINE_START, vec3( point, 0.0 ), width };
}

void bsc::draw2d::
line_to( bsc::vec2 point, r32 width )
{
  g_ctx2d.cmds[ g_ctx2d.cmd_idx++ ] = { LINE_TO, vec3( point, 0.0 ), width };
}

void bsc::draw2d::
end_line( i8 merge )
{
  g_ctx2d.cmds[ g_ctx2d.cmd_idx++ ] = { LINE_END, vec3(), (r32)merge };
}

static GLuint tmp_vao = 0;
static GLuint tmp_vbo = 0;
static bool initialized = false;

char* tmp_vs = (char*) SHADER_HEAD STR 
(
  layout (location = 0) in vec3 position;
  uniform mat4 mvp;
  void main()
  {
    gl_Position = mvp * vec4( position, 1.0 );
  }
);

char* tmp_fs = (char*) SHADER_HEAD STR
(
  out vec4 frag_color;
  void main()
  {
    frag_color = vec4( 1.0, 0.0, 0.0, 1.0 );
  }
);

static bsc::shader_prog tmp_shader;
static bsc::mat4 tmp_proj;
static bsc::mat4 tmp_look_at;

void bsc::draw2d::
render()
{
  // TODO: initialization should do some shader initialization etc.

  if ( !initialized )
  {
    glGenVertexArrays( 1, &tmp_vao );
    glGenBuffers( 1, &tmp_vbo );

    glBindVertexArray( tmp_vao );
    glBindBuffer( GL_ARRAY_BUFFER, tmp_vbo );
    // TODO : We will have to have a size for this?
    // NOTE : What does nanovg do to initialize these buffers, and how does it transfer commands.
    // NOTE : Shouldn't we map the buffers?
    glBufferData( GL_ARRAY_BUFFER, MAX_CMDS * sizeof( bsc::vec3 ), NULL, GL_STREAM_DRAW );
    glBindVertexArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    create_shader_prog_from_source( tmp_vs, tmp_fs, tmp_shader );
  
    initialized = true;
  }

  bsc::vec3 positions[ MAX_CMDS ];
  bsc::vec3 first_pos;
  bsc::vec3 prev_pos;
  i32 draw_count = 0;

  for ( int i = 0 ; i < g_ctx2d.cmd_idx ; ++i )
  {
    cmd * cur_cmd = &( g_ctx2d.cmds[i] );
  
    switch ( cur_cmd->type )
    {
      case LINE_START:
        first_pos = cur_cmd->pos;
        prev_pos = cur_cmd->pos;
        break;
      case LINE_TO:
        positions[ draw_count + 0 ] = prev_pos;
        positions[ draw_count + 1 ] = cur_cmd->pos;
        prev_pos = cur_cmd->pos;
        draw_count += 2;
        break;
      case LINE_END:
        if ( cur_cmd->radius > 0 )
        {
          positions[ draw_count + 0 ] = prev_pos;
          positions[ draw_count + 1 ] = first_pos;
          draw_count += 2;
        }
        break;
      default:
        bsc::error( __FILE__, __LINE__, "Unknown error type!");
        break;
    }
  }


  if ( draw_count > 0 )
  {
    i64 size = draw_count * 3 * sizeof(r32);

    glBindVertexArray( tmp_vao );
    glBindBuffer( GL_ARRAY_BUFFER, tmp_vbo );

    glBufferSubData( GL_ARRAY_BUFFER, 0, size, &( positions[0] ) );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    
    glBindVertexArray(0);
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
  
    tmp_look_at = bsc::look_at( bsc::vec3( 0.0f, 0.0f, 0.0f ),
                                bsc::vec3( 0.0f, 0.0f, 1.0f ),
                                bsc::vec3( 0.0f, 1.0f, 0.0f )  );

    // TODO: Change that to projective transform so that we can have floating 
    // huds

    bsc::vec2i res = bsc::get_window().fb_size;
    r32 pixel_ratio = bsc::get_window().pixel_ratio;

    tmp_proj = bsc::ortho(  0.0f, -1.0f/pixel_ratio * res.x,
                            1.0f/pixel_ratio * res.y,  0.0f,
                            0.0f, 10.0f );
    bsc::use_program( tmp_shader );
    bsc::set_uniform( tmp_shader, "mvp", tmp_proj * tmp_look_at );
    
    glBindVertexArray( tmp_vao );
    glDrawArrays( GL_LINES, 0, draw_count );

    glBindVertexArray( 0 );
  }

}

#endif //BSC_IMPLEMENTATION