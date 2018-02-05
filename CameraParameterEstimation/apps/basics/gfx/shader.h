// bascis shader program

// TODO :Modify this to use more modern glProgramPipeline.

#pragma once

#define SHADER_HEAD "#version 330 core\n"

namespace bsc
{
  enum shader_type
  {
    vertex_shader,
    fragment_shader,
    geometry_shader,
    tess_control_shader,
    tess_evaluation_shader
  };

  struct shader
  {
    GLuint id;
    shader_type type;
    GLint compiled;
  };

  struct shader_prog
  {
    GLuint id;
    GLint linked;
  };

  // TODO : Should take pointers!
  i32 create_shader_prog_from_source(const char *vertex_shader_source,
                                     const char *fragment_shader_source,
                                     shader_prog &p);
  i32 create_shader_prog_from_source(const char *vertex_shader_source,
                                     const char *geometry_shader_source,
                                     const char *fragment_shader_source,
                                     shader_prog &p);

  i32 create_shader_prog ( const char * vertex_shader_filename,
                           const char * fragment_shader_filename,
                           shader_prog & p );
  
  i32 create_shader_prog ( const char * vertex_shader_filename,
                           const char * geometry_shader_filename,
                           const char * fragment_shader_filename,
                           shader_prog & p  );

  i32 compile_shader ( const char * source, shader & s );
  i32 link_program ( const shader & vs, const shader & fs, 
                     shader_prog & p );
  i32 link_program ( const shader & vs, const shader & gs, const shader & fs, 
                     shader_prog & p );
  i32 use_program ( const shader_prog & p );

  void set_uniform ( const shader_prog & p, const char *attrib_name, r32 val );
  void set_uniform ( const shader_prog & p, const char *attrib_name, i32 val );
  void set_uniform ( const shader_prog & p, const char *attrib_name, u32 val );
  void set_uniform ( const shader_prog & p, const char *attrib_name, bool val );

  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const r32 * val, 
                     const u32 count = 1 );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const i32 * val, 
                     const u32 count = 1 );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const u32 * val, 
                     const u32 count = 1 );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const bool * val, 
                     const u32 count = 1 );

  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, 
                     float x, float y, float z);
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const vec2f &v );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const vec2f *v, 
                     const u32 count = 1 );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const vec3f &v );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const vec3f *v, 
                     const u32 count = 1 );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const vec4f &v);
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const vec4f *v, 
                     const u32 count = 1 );

  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const mat3f &m );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const mat3f *m,
                     const u32 count = 1 );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const mat4f &m );
  void set_uniform ( const shader_prog & p, 
                     const char *attrib_name, const mat4f *m,
                     const u32 count = 1 );

  // void PrintActiveUniforms( const shader_prog & p );
  // void PrintActiveAttribs( const shader_prog & p );

  // void bindAttribLocation( GLuint location, const char * name);
  // void bindFragDataLocation( GLuint location, const char * name );

}

#ifdef BSC_IMPLEMENTATION

// TODO: Move it to a dedication file loading location
inline i32
text_file_read ( const char * filename, char ** file_contents )
{
  FILE *fp;
  long l_size;

  fp = fopen ( filename , "r" );
  if( !fp ) perror(filename),exit(1);

  fseek( fp , 0L , SEEK_END);
  l_size = ftell( fp );
  rewind( fp );

  // allocate memory for entire content 
  (*file_contents) = (char*)calloc( 1, l_size + 1 );
  if ( !(*file_contents) )
  { 
    fclose( fp ); 
    fputs( "memory alloc fails", stderr ); 
    exit( 1 );
  }

  // copy the file into the buffer 
  if ( fread( (*file_contents), l_size, 1, fp) != 1 )
  {
    fclose(fp); 
    free( (*file_contents) ); 
    fputs( "entire read fails",stderr );
    exit( 1 );
  }

  fclose(fp);
  return 1;
}

i32 bsc::
compile_shader ( const char * source, shader & s )
{
  switch ( s.type ) {
      case bsc::vertex_shader:
          s.id = glCreateShader( GL_VERTEX_SHADER );
          break;
      case bsc::fragment_shader:
          s.id = glCreateShader( GL_FRAGMENT_SHADER );
          break;

    #ifndef __EMSCRIPTEN__
      case bsc::geometry_shader:
          s.id = glCreateShader( GL_GEOMETRY_SHADER );
          break;
      case bsc::tess_control_shader:
          s.id = glCreateShader( GL_TESS_CONTROL_SHADER );
          break;
      case bsc::tess_evaluation_shader:
          s.id = glCreateShader( GL_TESS_EVALUATION_SHADER );
          break;
    #endif
      default:
          return false;
  }

  glShaderSource( s.id, 1, &source, NULL );

  glCompileShader( s.id );
  glGetShaderiv( s.id, GL_COMPILE_STATUS, &s.compiled );

  if ( !s.compiled )
  {
    GLint log_len;
    glGetShaderiv( s.id, GL_INFO_LOG_LENGTH, &log_len );
    if ( log_len > 0 )
    {
      char* log_str = (char*) malloc( log_len );
      GLsizei written;
      glGetShaderInfoLog( s.id, log_len, &written, log_str );
      char * shader_type_str;
      switch ( s.type )
      {
        case bsc::vertex_shader:
          shader_type_str = (char*)"Vertex Shader";
          break;
        case bsc::fragment_shader:
          shader_type_str = (char*)"Fragment Shader";
          break;
        case bsc::geometry_shader:
          shader_type_str = (char*)"Geometry Shader";
          break;
        case bsc::tess_control_shader:
          shader_type_str = (char*)"Tess Control Shader";
          break;
        case bsc::tess_evaluation_shader:
          shader_type_str = (char*)"Tess Evaluation Shader";
          break;
      }
      printf( "%s Compilation Failure :\n%s\n", shader_type_str, log_str );
      free( log_str );
    }
    return 0;
  }

  return s.compiled;
}

i32 bsc::
link_program ( const shader & vs, const shader & fs, shader_prog & p )
{
  // change to asserts?
  if ( !vs.compiled || !fs.compiled )
  {
    printf( "Compile shaders before linking program!\n" );
    return 1;
  }

  p.id = glCreateProgram();
  if ( p.id == 0 )
  {
    printf ( " Failed to create program!\n" );
    return 0;
  }

  glAttachShader( p.id, vs.id );
  glAttachShader( p.id, fs.id );

  glLinkProgram( p.id );
  glGetProgramiv( p.id, GL_LINK_STATUS, &p.linked );

  if ( !p.linked )
  {
    GLint log_len;
    glGetProgramiv( p.id, GL_INFO_LOG_LENGTH, &log_len );
    if( log_len > 0 )
    {
      char* log_str = (char*) malloc( log_len );
      GLsizei written;
      glGetProgramInfoLog( p.id, log_len, &written, log_str );
      printf( "Program Linking Failure : %s\n", log_str );
      free( log_str );
    }
  }

  glDetachShader( p.id, fs.id );
  glDetachShader( p.id, vs.id );

  glDeleteShader( fs.id );
  glDeleteShader( vs.id );

  return p.linked;
}

i32 bsc::
link_program ( const shader & vs, 
               const shader & gs, 
               const shader & fs, 
               shader_prog & p )
{
  // change to asserts?
  if ( !vs.compiled || !fs.compiled )
  {
    printf( "Compile shaders before linking program!\n" );
    return 1;
  }

  p.id = glCreateProgram();
  if ( p.id == 0 )
  {
    printf ( " Failed to create program!\n" );
    return 0;
  }

  glAttachShader( p.id, vs.id );
  glAttachShader( p.id, gs.id );
  glAttachShader( p.id, fs.id );

  glLinkProgram( p.id );
  glGetProgramiv( p.id, GL_LINK_STATUS, &p.linked );

  if ( !p.linked )
  {
    GLint log_len;
    glGetProgramiv( p.id, GL_INFO_LOG_LENGTH, &log_len );
    if( log_len > 0 )
    {
      char* log_str = (char*) malloc( log_len );
      GLsizei written;
      glGetProgramInfoLog( p.id, log_len, &written, log_str );
      printf( "Program Linking Failure : %s\n", log_str );
      free( log_str );
    }
  }

  glDetachShader( p.id, fs.id );
  glDetachShader( p.id, gs.id );
  glDetachShader( p.id, vs.id );

  glDeleteShader( fs.id );
  glDeleteShader( gs.id );
  glDeleteShader( vs.id );

  return p.linked;
}

// TODO: Create differentiation for those
i32 bsc::
create_shader_prog_from_source ( const char * vs_src,
                                 const char * fs_src,
                                 shader_prog & p  )
{

  shader vs, fs;
  vs.type = bsc::vertex_shader;
  fs.type = bsc::fragment_shader;

  if ( !compile_shader( vs_src, vs ) ) return 0;
  if ( !compile_shader( fs_src, fs ) ) return 0;
  if ( !link_program( vs, fs, p ) )    return 0;

  return 1;
}

i32 bsc::
create_shader_prog_from_source ( const char * vs_src,
                                 const char * gs_src,
                                 const char * fs_src,
                                 shader_prog & p  )
{

  shader vs, fs, gs;
  vs.type = bsc::vertex_shader;
  fs.type = bsc::fragment_shader;
  gs.type = bsc::geometry_shader;

  if ( !compile_shader( vs_src, vs ) ) return 0;
  if ( !compile_shader( gs_src, gs ) ) return 0;
  if ( !compile_shader( fs_src, fs ) ) return 0;
  if ( !link_program( vs, gs, fs, p ) )    return 0;

  return 1;
}

i32 bsc::
create_shader_prog ( const char * vs_filename,
                     const char * fs_filename,
                     shader_prog & p  )
{
  char * vs_source, * fs_source;
  text_file_read( vs_filename, &vs_source );
  text_file_read( fs_filename, &fs_source );

  shader vs, fs;
  vs.type = bsc::vertex_shader;
  fs.type = bsc::fragment_shader;

  if ( !compile_shader( vs_source, vs ) ) return 0;
  if ( !compile_shader( fs_source, fs ) ) return 0;
  if ( !link_program( vs, fs, p ) )       return 0;

  free( vs_source );
  free( fs_source );

  return 1;
}


i32 bsc::
create_shader_prog ( const char * vs_filename,
                   const char * gs_filename,
                   const char * fs_filename,
                   shader_prog & p  )
{
  char * vs_source, * gs_source, * fs_source;
  text_file_read( vs_filename, &vs_source );
  text_file_read( gs_filename, &gs_source );
  text_file_read( fs_filename, &fs_source );

  shader vs, gs, fs;
  vs.type = bsc::vertex_shader;
  gs.type = bsc::geometry_shader;
  fs.type = bsc::fragment_shader;

  if ( !compile_shader( vs_source, vs ) ) return 0;
  if ( !compile_shader( gs_source, gs ) ) return 0;
  if ( !compile_shader( fs_source, fs ) ) return 0;
  if ( !link_program( vs, gs, fs, p ) )   return 0;

  free( vs_source );
  free( gs_source );
  free( fs_source );

  return 1;
}

i32 bsc::
use_program( const shader_prog & p )
{
  if ( p.linked ) {
    glUseProgram( p.id );
    return 1;
  }
  return 0;
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const vec2f & v)
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform2fv( location, 1, &(v.data[0]) );
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const vec2f * v, const u32 count )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform2fv( location, count, &( v->data[0] ) );
}


void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, float x, float y, float z )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform3f( location, x, y, z );
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const vec3f & v)
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform3fv( location, 1, &(v.data[0]) );
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const vec3f * v, const u32 count )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform3fv( location, count, &( v->data[0] ) );
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const vec4f &v )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform4fv( location, 1, &(v.data[0]) );
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const vec4f *v, const u32 count )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform4fv( location, count, &(v->data[0]) );
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const mat3f &m )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniformMatrix3fv( location, 1, GL_FALSE, &(m.data[0]) );
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const mat3f *m, const u32 count )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniformMatrix3fv( location, count, GL_FALSE, &(m->data[0]) );
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const mat4f &m )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniformMatrix4fv( location, 1, GL_FALSE, &(m.data[0]) );
}

void bsc::
set_uniform ( const shader_prog & p, 
              const char *attrib_name, const mat4f *m, const u32 count  )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniformMatrix4fv( location, 1, GL_FALSE, &(m->data[0]) );
}

void bsc::
set_uniform  ( const shader_prog & p, 
               const char *attrib_name, bool val )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform1i( location, (GLint)val );
}

void bsc::
set_uniform  ( const shader_prog & p, 
               const char *attrib_name, r32 val )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform1f( location, (GLfloat)val );
}

void bsc::
set_uniform  ( const shader_prog & p, 
               const char *attrib_name, i32 val )
{
    GLuint location = glGetUniformLocation ( p.id, attrib_name );
    glUniform1i( location, (GLint)val );
}

void bsc::
set_uniform  ( const shader_prog & p, 
               const char *attrib_name, u32 val )
{
    GLuint location = glGetUniformLocation ( p.id, attrib_name );
    glUniform1i( location, (GLint)val );
}



void bsc::
set_uniform  ( const shader_prog & p, 
               const char *attrib_name, const bool * val, const u32 count )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform1iv( location, count, (GLint*)val );
}

void bsc::
set_uniform  ( const shader_prog & p, 
               const char *attrib_name, const r32 * val, const u32 count )
{
    GLuint location = glGetUniformLocation( p.id, attrib_name );
    glUniform1fv( location, count, (GLfloat*)val );
}

void bsc::
set_uniform  ( const shader_prog & p, 
               const char *attrib_name, const i32 * val, const u32 count )
{
    GLuint location = glGetUniformLocation ( p.id, attrib_name );
    glUniform1iv( location, count, (GLint*)val );
}

void bsc::
set_uniform  ( const shader_prog & p, 
               const char *attrib_name, const u32 * val, const u32 count )
{
    GLuint location = glGetUniformLocation ( p.id, attrib_name );
    glUniform1uiv( location, count, (GLuint*)val );
}
#endif
