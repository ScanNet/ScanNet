////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////

// TODO this will most likely go through many iterations to arrive at something
// good.
// TODO: Make methods create multiple by default!
// TODO: What happens if data is updated often? Run Experiments
// TODO: Look at ubo's + pipelines
// TODO: Look at instancing
// TODO: Add filtering options to textures
// TODO: Look at samplers
// TODO: Should texture units have specific properties?

// This assumes 3d data. Probably will want this to have dimensionality as well
// We are also mixing describing input data and description of draw command. 
// Need to add instanced drawing and see how the code changes.

enum FilteringSetting
{
  TEST
};



enum Attachements
{
  TEST3
};

enum GeometryPropertiesFlags_
{
  POSITION  = 1 << 0, // layout( location = 0 )
  NORMAL    = 1 << 1, // layout( location = 1 )
  TANGENT   = 1 << 2, // layout( location = 2 )
  TEX_COORD = 1 << 3, // layout( location = 3 )
  COLOR_A   = 1 << 4, // layout( location = 4 )
  COLOR_B   = 1 << 5, // layout( location = 5 )
  COLOR_C   = 1 << 6, // layout( location = 6 )
  COLOR_D   = 1 << 7, // layout( location = 7 )
  STRUCTURED = 1 << 8,

  SIMPLE_MESH   = POSITION | NORMAL | STRUCTURED,
  POINTCLOUD    = POSITION,
};

enum FramebufferType_
{
  DEFAULT,
  RGB,
  RGBA,
};

typedef i32 InternalDataType;
typedef i32 GeometryPropertiesFlags;
typedef i32 FramebufferType;

namespace bsc
{
// GEOMETRY
  struct geometry_data
  {
    r32 * positions = NULL;
    r32 * normals   = NULL;
    r32 * tangents  = NULL;
    r32 * texcoords = NULL;
    u8  * colors_a  = NULL;
    u8  * colors_b  = NULL;
    u8  * colors_c  = NULL;
    u8  * colors_d  = NULL;
    u32 * indices   = NULL;
    i32 n_vertices  = -1;
    i32 n_elements  = -1;
  };

  // TODO : Need to experiment with glMapBufferRange.
  struct gpu_geometry
  {
    u32 vao     = -1;
    u32 vbo     = -1;
    u32 ebo     = -1;
    i32 n_indices  = -1;
    i32 n_elements = -1;
    i32 buffer_size = -1;
    GeometryPropertiesFlags flags;
  };

  i32 update_gpu_geo( const geometry_data * host_data, 
                      const gpu_geometry * geo, 
                      const i32 flags = SIMPLE_MESH );
  i32 init_gpu_geo( const geometry_data * host_data,   
                    gpu_geometry * geo, 
                    const i32 flags = SIMPLE_MESH );
  i32 free_gpu_geo( gpu_geometry * geo );
  
  void draw( gpu_geometry * geo, 
             GLenum draw_mode = GL_TRIANGLES );


// TEXTURES
  // TODO: Should gpu texture store it's size?
  struct gpu_texture
  {
    u32 id;
    
    i32 width;
    i32 height;
    i32 n_comp;

    // THESE ARE OPENGL SPECIFIC.
    GLenum type;
    u32 unit;
  };
  
  void init_gpu_tex( const u8 * data, 
                     const i32 w, 
                     const i32 h, 
                     const i32 n_comp, 
                     gpu_texture * tex, 
                     const u32 unit = 1 );
  void update_gpu_tex( const u8 * data, 
                       gpu_texture * tex );
  void init_gpu_tex( const u16 * data, 
                     const i32 w, 
                     const i32 h, 
                     const i32 n_comp, 
                     gpu_texture * tex, 
                     const u32 unit = 1 );
  void update_gpu_tex( const u16 * data, 
                       gpu_texture * tex );

  void init_gpu_tex( const i32 w,
                     const i32 h,
                     const i32 n_comp,
                     const GLenum type,
                     const GLenum format,
                     const GLint internal_format,
                     gpu_texture * tex,
                     const u32 unit = 1 );

  void use_texture( const gpu_texture * tex );

  void free_gpu_tex( gpu_texture * tex );

// FRAMEBUFFERS
// TODO: Should framebuffer store it's size?
  // struct gpu_framebuffer
  // {
  //   u32 id;
  //   i32 width;
  //   i32 height;
  //   gpu_texture tex;
  // };

  // TODO: Add things like format and 
  // void init_framebuffer( const i32 w, 
  //                        const i32 h, 
  //                        const InternalDataType
  //                        gpu_framebuffer * fb, 
  //                        const i23 flags );
  // void use_framebuffer( gpu_framebuffer * fb );
  // void free_framebuffer( gpu_framebuffer * fb );

// RENDERBUFFERS -> TODO : Should we disguise renderbuffer as framebuffer on a specific type / attachment?
// They can be bound to FBO... And it is their only use...
  // struct gpu_renderbuffer
  // {
  //   u32 id;
  //   i32 width;   
  //   i32 height;
  // };
  // void init_renderbuffer( const i32 w, 
  //                         const i32 h, 
  //                         gpu_renderbuffer * fb );
  // void use_renderbuffer( gpu_renderbuffer * fb );
  // void free_renderbuffer( gpu_renderbuffer * fb );
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////
#ifdef BSC_IMPLEMENTATION

// GEOMETRY
// calulates offset of requested flag into buffer stored in geo,
// depending on what has been requested
u64 _get_offset( const bsc::gpu_geometry * geo, const i32 flag )
{
  u64 offset = 0;
  if ( geo->flags & POSITION )
  {
    if ( flag & POSITION ) return offset * geo->n_indices;  
    offset += 3 * sizeof(r32); 
  }
  if ( geo->flags & NORMAL )
  {
    if ( flag & NORMAL ) return offset * geo->n_indices;
    offset += 3 * sizeof(r32); 
  }
  if ( geo->flags & TANGENT )
  {
    if ( flag & TANGENT ) return offset * geo->n_indices;
    offset += 3 * sizeof(r32); 
  }
  if ( geo->flags & TEX_COORD )
  {
    if ( flag & TEX_COORD ) return offset * geo->n_indices;
    offset += 2 * sizeof(r32); 
  }
  if ( geo->flags & COLOR_A )
  {
    if ( flag & COLOR_A ) return offset * geo->n_indices;
    offset += 4 * sizeof(u8); 
  }
  if ( geo->flags & COLOR_B )
  {
    if ( flag & COLOR_B ) return offset * geo->n_indices;
    offset += 4 * sizeof(u8); 
  }
  if ( geo->flags & COLOR_C )
  {
    if ( flag & COLOR_C ) return offset * geo->n_indices;
    offset += 4 * sizeof(u8); 
  }
  if ( geo->flags & COLOR_D )
  {
    if ( flag & COLOR_D ) return offset * geo->n_indices;
    offset += 4 * sizeof(u8); 
  }
  return offset * geo->n_indices;
}

i32 bsc::
update_gpu_geo( const geometry_data * host_data, 
                const gpu_geometry * geo,
                const i32 flags )
{
  
  if( !(flags & geo->flags) )
  {
    error( "Flag mismatch", __LINE__, "Requested flag update was not"
    "part of initialization" );
    return 0;
  }

  glBindVertexArray( geo->vao );
  glBindBuffer( GL_ARRAY_BUFFER, geo->vbo );
 
  if ( flags & POSITION )
  {
    u64 offset = _get_offset( geo, flags & POSITION );
    u64 current_size = 3 * sizeof(r32) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->positions[0]) );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 
                          0, (void*) offset );
  }

  if ( flags & NORMAL )
  {
    u64 offset = _get_offset( geo, flags & NORMAL);
    u64 current_size = 3 * sizeof(r32) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->normals[0]) );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 
                           0, (void*) offset );
  }

  if ( flags & TANGENT )
  {
    u64 offset = _get_offset( geo, flags & TANGENT);
    u64 current_size = 3 * sizeof(r32) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->tangents[0]) );
    glEnableVertexAttribArray( 2 );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 
                           0, (void*) offset );
  }

  if ( flags & TEX_COORD )
  {
    u64 offset = _get_offset( geo, flags & TEX_COORD);
    u64 current_size = 2 * sizeof(r32) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->texcoords[0]) );
    glEnableVertexAttribArray( 3 );
    glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, 
                           0, (void*) offset );
  }

  if ( flags & COLOR_A )
  {
    u64 offset = _get_offset( geo, flags & COLOR_A);
    u64 current_size = 4 * sizeof(u8) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->colors_a[0]) );
    glEnableVertexAttribArray( 4 );
    glVertexAttribPointer( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                            0, (void*) offset );
  }

  if ( flags & COLOR_B )
  {
    u64 offset = _get_offset( geo, flags & COLOR_B);
    u64 current_size = 4 * sizeof(u8) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->colors_b[0]) );
    glEnableVertexAttribArray( 5 );
    glVertexAttribPointer( 5, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                            0, (void*) offset );
  }

  if ( flags & COLOR_C )
  {
    u64 offset = _get_offset( geo, flags & COLOR_C);
    u64 current_size = 4 * sizeof(u8) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->colors_c[0]) );
    glEnableVertexAttribArray( 6 );
    glVertexAttribPointer( 6, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                            0, (void*) offset );
  }

  if ( flags & COLOR_D )
  {
    u64 offset = _get_offset( geo, flags & COLOR_D);
    u64 current_size = 4 * sizeof(u8) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->colors_d)[0] );
    glEnableVertexAttribArray( 7 );
    glVertexAttribPointer( 7, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                            0, (void*) offset );
  }

  glBindVertexArray(0);
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  return 1;
}

// TODO: Errors if creating data fails!!
i32 bsc::
init_gpu_geo ( const geometry_data * host_data, 
               gpu_geometry * geo, 
               const i32 flags )
{
  // Always use position
  assert( flags & POSITION );

  glGenVertexArrays( 1, &(geo->vao)  );
  glGenBuffers( 1, &(geo->vbo) );
  if ( flags & STRUCTURED )
  {
    glGenBuffers( 1, &(geo->ebo) );
  }

  // Initialize empty buffer
  glBindVertexArray( geo->vao );
  glBindBuffer( GL_ARRAY_BUFFER, geo->vbo );

  u64 buf_size = 0;
  if ( flags & POSITION )  buf_size += 3 * sizeof(r32); 
  if ( flags & NORMAL )    buf_size += 3 * sizeof(r32);
  if ( flags & TANGENT )   buf_size += 3 * sizeof(r32);
  if ( flags & TEX_COORD ) buf_size += 2 * sizeof(r32);
  if ( flags & COLOR_A )   buf_size += 4 * sizeof(u8);
  if ( flags & COLOR_B )   buf_size += 4 * sizeof(u8);
  if ( flags & COLOR_C )   buf_size += 4 * sizeof(u8);
  if ( flags & COLOR_D )   buf_size += 4 * sizeof(u8);
  buf_size *= host_data->n_vertices;
  
  glBufferData( GL_ARRAY_BUFFER, buf_size, NULL, GL_STATIC_DRAW);

  geo->flags       = flags;
  geo->n_indices   = host_data->n_vertices;
  geo->n_elements  = host_data->n_elements;
  geo->buffer_size = buf_size;

  if ( !update_gpu_geo( host_data, geo, flags ) )
  {
    return 0;
  }

  if ( flags & STRUCTURED )
  {  
    glBindVertexArray( geo->vao );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, geo->ebo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, 
                  geo->n_elements * sizeof( u32 ), 
                  &(host_data->indices[0]), 
                  GL_STATIC_DRAW ); 
  }

  glBindVertexArray(0);
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); 
  return 1;
}

i32 bsc::
free_gpu_geo( gpu_geometry * geo )
{
  glDeleteBuffers(1, &(geo->vbo) );
  if ( geo->flags & STRUCTURED )
  {
    glDeleteBuffers(1, &(geo->ebo) );
  }
  glDeleteVertexArrays( 1, &(geo->vao) );
  geo->vao = -1;
  geo->vbo = -1;
  geo->ebo = -1;
  geo->n_indices  = -1;
  geo->n_elements = -1;
  return 1;
}

void bsc::
draw( gpu_geometry * geo, 
      GLenum draw_mode ) 
{
  GeometryPropertiesFlags flags = geo->flags;

  glBindVertexArray( geo->vao );
  
  if ( flags & STRUCTURED )
  {
    glDrawElements( draw_mode, geo->n_elements, GL_UNSIGNED_INT, 0 );
  }
  else
  {
    glDrawArrays( draw_mode, 0, geo->n_indices );
  }

  glBindVertexArray( 0 );
}

// TEXTURES
void bsc::
update_gpu_tex( const u8 * data, 
                gpu_texture * tex )
{
  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );

  GLint internal_format;
  GLenum format = 0;
  switch( tex->n_comp )
  {
    case 1:
      internal_format = GL_R8;
      format = GL_RED;
      break;
    case 2:
      internal_format = GL_RG;
      break;
    case 3:
      internal_format = GL_RGB;
      format = GL_RGB;
      break;
    case 4:
      internal_format = GL_RGBA;
      format = GL_RGBA;
      break;
    default:
      internal_format = GL_RGB;
      format = GL_RGB;
  }

  glTexImage2D( GL_TEXTURE_2D, 0, internal_format, 
                tex->width, tex->height, 0, format, GL_UNSIGNED_BYTE, data );
  glBindTexture( GL_TEXTURE_2D, 0 );
}

void bsc::
init_gpu_tex( const u8 * data, 
              const i32 w, 
              const i32 h, 
              const i32 n_comp, 
              gpu_texture * tex, 
              const u32 unit )
{
  tex->width  = w;
  tex->height = h;
  tex->n_comp = n_comp; 
  tex->type   = GL_UNSIGNED_BYTE;
  tex->unit   = unit;
  glGenTextures( 1, &tex->id );
  
  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  update_gpu_tex( data, tex );

  glBindTexture( GL_TEXTURE_2D, 0 );
}

void bsc::
update_gpu_tex( const u16 * data, 
                gpu_texture * tex )
{
  assert( tex->n_comp > 0 && tex->n_comp <= 1 ); // TODO: Custom asserts?

  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );

  GLint internal_format;
  GLenum format;
  switch( tex->n_comp )
  {
    case 1:
      internal_format = GL_R16;
      format = GL_RED;
      break;
    case 2:
      internal_format = GL_RG16;
      format = GL_RG;
      break;
    case 3:
      internal_format = GL_RGB16;
      format = GL_RGB;
      break;
    case 4:
      internal_format = GL_RGBA16;
      format = GL_RGBA;
      break;
    default:
      internal_format = GL_RGB;
      format = GL_RGB;
  }

  glTexImage2D( GL_TEXTURE_2D, 0, internal_format, 
                tex->width, tex->height, 0, format, GL_UNSIGNED_SHORT, data );
  glBindTexture( GL_TEXTURE_2D, 0 );
}

void bsc::
init_gpu_tex( const u16 * data, 
              const i32 w, 
              const i32 h, 
              const i32 n_comp, 
              gpu_texture * tex, 
              const u32 unit )
{
  tex->width  = w;
  tex->height = h;
  tex->n_comp = n_comp; 
  tex->type   = GL_UNSIGNED_SHORT;
  tex->unit   = unit;
  glGenTextures( 1, &tex->id );
  
  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  update_gpu_tex( data, tex );

  glBindTexture( GL_TEXTURE_2D, 0 );
}

// TODO: Pass type, based on what exactly we want
// Need to build over opengl here.
// void bsc::
// init_gpu_tex( const i32 w,
//               const i32 h,
//               const i32 n_comp,
//               const InternalDataType type,
//               gpu_texture * tex, 
//               const u32 unit )
// {
//   tex->width  = w;
//   tex->height = h;
//   tex->n_comp = n_comp; 
//   tex->type   = type;
//   tex->unit   = unit;
//   glGenTextures( 1, &tex->id );
  
//   glActiveTexture ( GL_TEXTURE0 + tex->unit );
//   glBindTexture( GL_TEXTURE_2D, tex->id );
//   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
//   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

//   GLint internal_format;
//   GLenum format;

//   switch( tex->n_comp )
//   {
//     case 1:
//       format = GL_RED;
//       break;
//     case 2:
//       format = GL_RG;
//       break;
//     case 3:
//       format = GL_RGB;
//       break;
//     case 4:
//       format = GL_RGBA;
//       break;
//     default:
//       format = GL_RGB;
//   }

//   // based on format decide internal format
//   // switch ( tex->type )
//   // {
//   //   case U8:
//   //     PPCAT( )
//   //     break;
//   //   default: 
//   //     internal_format = GL_RGB
//   // }
//   glTexImage2D( GL_TEXTURE_2D, 
//                 0, 
//                 internal_format, // TODO: should depend on format + n_comp
//                 tex->width, 
//                 tex->height, 
//                 0, 
//                 format,  // TODO: Same
//                 tex->data_type, // THIS SHOULD BE FORMAT
//                 NULL );

//   glBindTexture( GL_TEXTURE_2D, 0 );
// }


void bsc::
use_texture( const gpu_texture * tex)
{
    glActiveTexture( GL_TEXTURE0 + tex->unit );
    glBindTexture( GL_TEXTURE_2D, tex->id );
}

void bsc::
free_gpu_tex( gpu_texture * tex )
{
    glDeleteTextures( 1, &tex->id );
    tex->id = 0;
}

//FrameBufferss

// void bsc::
// init_framebuffer( const i32 width, const i32 height, gpu_framebuffer * fb )
// {
//   glGenFramebuffers( 1, &(fb->id) );
//   fb->width = width;
//   fb->height = height;
//   init_gpu_tex( width, height, &(fb->tex), 1 );
//   glFramebufferTexture2D( GL_FRAMEBUFFER, 
//                           GL_COLOR_ATTACHMENT0, 
//                           GL_TEXTURE_2D, 
//                           fb->tex.id, 
//                           0 );
//   glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb->tex.id, 0 );

//   // NOTE: If needed the rbo should be created here!!
//   bool auto_depth = false;
//   if ( auto_depth )
//   {
//     // CREATE RBO
//   }
// }

// // TODO: In OpenGL framebuffer can be GL_READ_FRAMEBUFFER or GL_DRAW_FRAMEBUFFER
// void bsc::
// use_framebuffer( gpu_framebuffer * fb )
// {
//   glBindFramebuffer( GL_FRAMEBUFFER, fb->id );
// }

// void bsc::
// free_framebuffer( gpu_framebuffer * fb )
// {
//   glDeleteFramebuffers( 1, &(fb->id) );
// }

// // TODO: Format?
// void bsc::
// init_renderbuffer( const i32 width, const i32 height, gpu_renderbuffer * rb )
// {
//   glGenRenderbuffers(1, &(rb->id) );
//   rb->width = width;
//   rb->height = height;
//   glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
//   // This will use currently bound framebuffer!!
//   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb->id );
// }

// void bsc::
// use_renderbuffer( gpu_renderbuffer * rb )
// {
//   glBindRenderbuffer(GL_RENDERBUFFER, rb->id );
// }

// void bsc::
// free_renderbuffer( gpu_renderbuffer * rb )
// {
//   glDeleteRenderbuffers( 1, &(rb->id) );
// }

#endif //BSC_IMPLEMENTATION