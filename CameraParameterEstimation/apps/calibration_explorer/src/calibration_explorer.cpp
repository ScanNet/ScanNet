#define BSC_IMPLEMENTATION
#define BSC_USE_IMGUI
#define BSC_USE_WINDOW
#include "basics.h"

////////////////////////////////////////////////////////////////////////////////
// GLOBAL STATE
////////////////////////////////////////////////////////////////////////////////

struct Options
{
  char * color_filename;
  char * depth_filename;
  char * input_parameter_filename;
  char * output_parameter_filename;
};

struct CameraParams
{
  bsc::mat3 cK, dK;
  bsc::mat4 d2c_T;
  bsc::vec2i c_size;
  bsc::vec2i d_size;
  r32 dc[5], cc[5];
};

enum ViewMode
{
  IMAGE_MODE,
  POINTCLOUD_MODE
};

static ViewMode view_mode;

static Options opts;
static CameraParams cam_params;
static CameraParams init_cam_params;
static r32 alpha = 0.5f;

static bsc::img color_img;
static bsc::img_u16 depth_img;
static bsc::img undistorted_color_img;
static bsc::img_u16 undistorted_depth_img;
static bsc::img_u16 registered_depth_img;

static bsc::gpu_geometry pointcloud_geo;
static bsc::mat4 view;
static bsc::mat4 proj;
static bsc::camera cam;
static bsc::trackball_controls cam_controls;

static bsc::gpu_texture color_tex;
static bsc::gpu_texture depth_tex;

static bsc::shader_prog explorer_shader;
static bsc::shader_prog pointcloud_shader;
static i32 panel_width = 400;
static bool use_bitshift = false;
////////////////////////////////////////////////////////////////////////////////
// GPU CODE
////////////////////////////////////////////////////////////////////////////////

// vertex shader ( full screen quad by Morgan McGuire)
char* vs_image = (char*) SHADER_HEAD STR 
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

char* fs_image = (char*) SHADER_HEAD STR 
(
  uniform sampler2D color_tex;
  uniform sampler2D depth_tex;

  in vec2 v_texcoords;
  uniform float alpha;

  out vec4 frag_color;

  void main()
  {
    vec2 flipped_coord = vec2( v_texcoords.x,
                                1.0 - v_texcoords.y );

    vec3 color = texture( color_tex, flipped_coord ).rgb;
    float depth = texture( depth_tex, flipped_coord ).r * 8;
    vec3 d = vec3(depth);
    frag_color = vec4( alpha * d + (1-alpha) * color, 1.0 );
  }
);

char *vs_pointcloud = (char *)SHADER_HEAD STR(
    layout(location = 0) in vec3 position;
    layout(location = 4) in vec4 color_a;
    uniform mat4 mvp;
    uniform float point_size;
    out vec4 v_color;

    void main() {
      gl_Position = mvp * vec4(position, 1.0);
      gl_PointSize = point_size;
      
      v_color = vec4(color_a.rgb, 1.0f);
    });

char *fs_pointcloud = (char *)SHADER_HEAD STR(
    in vec4 v_color;
    out vec4 frag_color;
    void main() 
    {
      // cute hack of round points
        vec2 coord = gl_PointCoord - vec2(0.5);  //from [0,1] to [-0.5,0.5]
        if(length(coord) > 0.5)                  //outside of circle radius?
          discard;
        frag_color = v_color;
    });

////////////////////////////////////////////////////////////////////////////////
// CPU CODE
////////////////////////////////////////////////////////////////////////////////

void 
render_explorer_image( r32 alpha, char * vs_src, char * fs_src )
{
  static GLuint exploerer_vao = 0;

  if (exploerer_vao == 0)
  {
    glGenVertexArrays(1, &exploerer_vao);
    
    bsc::create_shader_prog_from_source( vs_src, fs_src, explorer_shader );
  }

  bsc::use_program( explorer_shader );
  bsc::set_uniform( explorer_shader, "color_tex", 1 );
  bsc::set_uniform( explorer_shader, "depth_tex", 2 );
  bsc::set_uniform( explorer_shader, "alpha", alpha );
  glBindVertexArray( exploerer_vao );
  glDrawArrays(GL_TRIANGLES, 0, 3);

  glBindVertexArray(0);
}

void
generate_pointcloud_data( bsc::geometry_data & pointcloud_data,
                          const bsc::img * color_image,
                          const bsc::img_u16 *depth_image,
                          const bsc::mat3 K )
{
  using namespace bsc;
  u32 n_pos = depth_image->width * depth_image->height;
  vec3 * positions = (vec3*)malloc( n_pos * sizeof(vec3) );
  vector4<u8> * colors = (vector4<u8>*)malloc( n_pos * sizeof(vector4<u8>) );
 
  r32 scale = (r32)color_image->height / depth_image->height;

  i32 pos_idx = 0, col_idx = 0;
  for ( i32 y = 0 ; y < depth_image->height ; y++ )
  {
    for ( i32 x = 0 ; x < depth_image->width ; x++ )
    {
      u16 depth = *(depth_image->at(x,y));
      const u8 * rgb = color_image->at( scale * x, scale * y );
      if ( depth == 0 ) {continue;}
      if ( rgb[0] == 0 && rgb[1] == 0 && rgb[2] == 0 ) {continue;}
      r32 d = depth * 0.001f;
      
      vec3 pos;
      pos.x = ((x - 2.025 * K[2][0]) * d) / (2.025 * K[0][0]);
      pos.y = ((y - 2.025 * K[2][1]) * d) / (2.025 * K[1][1]);
      pos.z = d;
      positions[ pos_idx++ ] = pos;

      vector4<u8> color;
      color.r = rgb[0];
      color.g = rgb[1];
      color.b = rgb[2];
      color.a = 255;
      colors[ col_idx++ ] = color;
    }
  }

  pointcloud_data.positions = &(positions[0].x);
  pointcloud_data.colors_a = &(colors[0].r);
  pointcloud_data.n_vertices = col_idx;
}

void 
create_3d_pointcloud ( bsc::gpu_geometry *pointcloud_geo, 
                       const bsc::img * color_image,
                       const bsc::img_u16 *depth_image,
                       const bsc::mat3 K,
                       bool first )
{
  using namespace bsc;
  geometry_data pointcloud_data;

  generate_pointcloud_data( pointcloud_data, color_image, depth_image, K );

  if ( first )
  {
    init_gpu_geo( &pointcloud_data, pointcloud_geo, POSITION | COLOR_A );
  }
  else
  {
    update_gpu_geo( &pointcloud_data, pointcloud_geo, POSITION | COLOR_A );
  }
  free( pointcloud_data.colors_a );
  free( pointcloud_data.positions );
}

void 
render_explorer_pointcloud( bsc::gpu_geometry *pointcloud_geo, 
                            const bsc::img *color_image,
                            const bsc::img_u16 *depth_image,
                            const bsc::mat3 K,
                            bool should_update, 
                            char * vs_src, char * fs_src )
{
  if ( pointcloud_geo->vao == -1 )
  {
    create_3d_pointcloud( pointcloud_geo, color_image, depth_image, K, true );
    bsc::create_shader_prog_from_source( vs_src, fs_src, pointcloud_shader );
  }

  glEnable(GL_PROGRAM_POINT_SIZE);

  if ( should_update )
  {
    create_3d_pointcloud( pointcloud_geo, color_image, depth_image, K, false );
  }

  bsc::use_program( pointcloud_shader );
  bsc::set_uniform( pointcloud_shader, "mvp", proj * view );
  bsc::set_uniform( pointcloud_shader, "point_size", 3.0f );
  bsc::draw( pointcloud_geo, GL_POINTS );
}

bool 
is_valid( bsc::vec2 loc, bsc::vec2 size )
{
  return ( loc.x >= 0 && loc.x < size.x && loc.y >= 0 && loc.y < size.y );
}

template <typename T>
void
undistort ( const bsc::image<T> *src, 
            bsc::image<T> *dst, 
            bsc::mat3 K, 
            r32 coeff[5] )
{
  using namespace bsc;

  for ( i32 y = 0 ; y < src->height ; y++ )
  {
    for ( i32 x = 0 ; x < src->width ; x++ )
    {
      vec2 nic_loc;
      vec2 sample_loc;

      //Normalized image coords
      nic_loc.x = ( (x+0.5) - K[2][0]) / K[0][0];
      nic_loc.y = ( (y+0.5) - K[2][1]) / K[1][1];

      float r2 = nic_loc.x * nic_loc.x + nic_loc.y * nic_loc.y;
    
      // Radial distortion
      sample_loc.x = nic_loc.x * (1.0f + r2 * coeff[0] + r2*r2 * coeff[1] + r2*r2*r2 * coeff[4] );
      sample_loc.y = nic_loc.y * (1.0f + r2 * coeff[0] + r2*r2 * coeff[1] + r2*r2*r2 * coeff[4] );

      // Tangential distortion
      sample_loc.x += 2.0f * coeff[2] * nic_loc.x * nic_loc.y + coeff[3] * ( r2 + 2.0f * nic_loc.x * nic_loc.x ); 
      sample_loc.y += coeff[2] * ( r2 + 2.0f * nic_loc.y * nic_loc.y ) + 2.0f * coeff[3] * nic_loc.x * nic_loc.y ;

      // Move back to the image space
      sample_loc.x = sample_loc.x * K[0][0] + K[2][0];
      sample_loc.y = sample_loc.y * K[1][1] + K[2][1];

      if ( is_valid ( sample_loc, bsc::vec2( dst->width, dst->height ) ) )
      {
        const T * src_ptr = src->at( sample_loc.x, sample_loc.y );
        T * dst_ptr = dst->at( x, y );
        for ( i32 n = 0 ; n < src->ncomp ; ++n )
        {
          dst_ptr[n] = src_ptr[n];  
        }
        // *(dst->at( x, y )) = *(src->at( sample_loc.x, sample_loc.y ));
      }
    }
  }
}


void 
splat( bsc::img_u16 * image, bsc::vec2 coord, u16 value )
{
  coord.x = round(coord.x);
  coord.y = round(coord.y);
  
  i32 splat_rad = 1;
  for ( i32 oy = -splat_rad ; oy <= splat_rad ; ++oy )
  {
    for ( i32 ox= -splat_rad ; ox <= splat_rad ; ++ox )
    {
      if ( *(image->at( coord.x + ox, coord.y + oy )) == 0 ) 
        *(image->at( coord.x + ox, coord.y + oy )) = value;
      else if ( *(image->at( coord.x + ox, coord.y + oy ) ) > value )
        *(image->at( coord.x + ox, coord.y + oy ) ) = value;
    }
  }
}

void
transform_img ( const bsc::img_u16 *src, 
                bsc::img_u16 *dst, 
                CameraParams * cam_params )
{
  using namespace bsc;
  
  // extract useful data
  mat4 T = cam_params->d2c_T;
  // Switches - from the d2d to our
  mat4 C1;
  C1[1][1] = -1;
  C1[2][2] = -1;
  T = C1 * T * C1;
  // image origin at the top
  mat4 C2;
  C2[1][1] = -1;
  T = C2 * T * C2;


  mat3 dK = cam_params->dK;
  mat3 cK = cam_params->cK;
  
  // imitialize positions
  u32 n_pos = src->width * src->height;
  u32 size = n_pos * sizeof(bsc::vec3);
  vec3 * positions = (vec3*)malloc(size);
  memset( positions, 0, size );

  // backproject
  for ( i32 y = 0 ; y < src->height ; y++ )
  {
    for ( i32 x = 0 ; x < src->width ; x++ )
    {
      u16 depth = *(src->at(x,y));
      if ( depth == 0 ) {continue;}
      r32 d = depth * 0.001f;
      
      vec3 pos;
      pos.x = ((x - dK[2][0]) * d) / dK[0][0];
      pos.y = ((y - dK[2][1]) * d) / dK[1][1];
      pos.z = -d;
      positions[ y * src->width + x] = pos;
    }
  }
  
  // transform
  for ( size_t pos_idx = 0 ; pos_idx < n_pos ; ++pos_idx )
  {
    vec3 pos = positions[ pos_idx ];

    if ( pos == vec3( 0.0f, 0.0f, 0.0f ) ) { continue; }
    vec4 homogenous_pos = vec4( pos, 1.0f );
    homogenous_pos = T * homogenous_pos;
    positions[pos_idx] = vec3( homogenous_pos );
  }
  
  // project back
  for ( i32 y = 0 ; y < src->height ; y++ )
  {
    for ( i32 x = 0 ; x < src->width ; x++ )
    {
      vec3 pos = positions[ y * src->width + x];
      if ( pos == vec3( 0.0f, 0.0f, 0.0f ) ) { continue; }

      vec3 projected_pos;// = cK * pos;
      // projected_pos.x /= projected_pos.z; 
      // projected_pos.y /= projected_pos.z; 
      projected_pos.x = cK[2][0] + pos.x * cK[0][0] / -pos.z;
      projected_pos.y = cK[2][1] + pos.y * cK[1][1] / -pos.z;
      projected_pos.z = pos.z;
      // if ( x < 10 && y <10)
      // {
      //   print_vec3( projected_pos );
      // }

      if ( is_valid( projected_pos, bsc::vec2(dst->width, dst->height) ) )
      {
        splat( dst, projected_pos, (u16)( -projected_pos.z * 1000.0f) );
      }
    }
  }

  free(positions);
}
i32 
read_parameters( const char * filename, CameraParams * cam_params )
{
  FILE * f = fopen( filename, "r" );
  if ( f )
  {
    fscanf( f, "colorWidth = %d\n", &(cam_params->c_size.x) );
    fscanf( f, "colorHeight = %d\n", &(cam_params->c_size.y));
    fscanf( f, "depthWidth = %d\n", &(cam_params->d_size.x));
    fscanf( f, "depthHeight = %d\n", &(cam_params->d_size.y));
    fscanf( f, "fx_color = %f\n", &(cam_params->cK[0][0]) );
    fscanf( f, "fy_color = %f\n", &(cam_params->cK[1][1]) );
    fscanf( f, "mx_color = %f\n", &(cam_params->cK[2][0]) );
    fscanf( f, "my_color = %f\n", &(cam_params->cK[2][1]) );
    fscanf( f, "fx_depth = %f\n", &(cam_params->dK[0][0]) );
    fscanf( f, "fy_depth = %f\n", &(cam_params->dK[1][1]) );
    fscanf( f, "mx_depth = %f\n", &(cam_params->dK[2][0]) );
    fscanf( f, "my_depth = %f\n", &(cam_params->dK[2][1]) );
    fscanf( f, "k1_color = %f\n", &cam_params->cc[0] );
    fscanf( f, "k2_color = %f\n", &cam_params->cc[1] );
    fscanf( f, "k3_color = %f\n", &cam_params->cc[2] );
    fscanf( f, "k4_color = %f\n", &cam_params->cc[3] );
    fscanf( f, "k5_color = %f\n", &cam_params->cc[4] );
    fscanf( f, "k1_depth = %f\n", &cam_params->dc[0] );
    fscanf( f, "k2_depth = %f\n", &cam_params->dc[1] );
    fscanf( f, "k3_depth = %f\n", &cam_params->dc[2] );
    fscanf( f, "k4_depth = %f\n", &cam_params->dc[3] );
    fscanf( f, "k5_depth = %f\n", &cam_params->dc[4] );
    bsc::mat4f m;
    fscanf( f, "depthToColorExtrinsics = %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", 
                                          &(m[0][0]), &(m[1][0]), &(m[2][0]), &(m[3][0]), 
                                          &(m[0][1]), &(m[1][1]), &(m[2][1]), &(m[3][1]),
                                          &(m[0][2]), &(m[1][2]), &(m[2][2]), &(m[3][2]),
                                          &(m[0][3]), &(m[1][3]), &(m[2][3]), &(m[3][3]) );
    cam_params->d2c_T = m;
    fclose(f);
    return 1;
  } 
  else
  {
    printf("Unable to open files %s\n", filename );
    return 0;
  }
}

i32 
write_parameters( const char * filename, const CameraParams * cam_params )
{
  FILE * f = fopen( filename, "w" );
  if ( f )
  {
    fprintf( f, "colorWidth = %d\n", cam_params->c_size.x);
    fprintf( f, "colorHeight = %d\n", cam_params->c_size.y);
    fprintf( f, "depthWidth = %d\n", cam_params->d_size.x);
    fprintf( f, "depthHeight = %d\n", cam_params->d_size.y);
    fprintf( f, "fx_color = %f\n", cam_params->cK[0][0] );
    fprintf( f, "fy_color = %f\n", cam_params->cK[1][1] );
    fprintf( f, "mx_color = %f\n", cam_params->cK[2][0] );
    fprintf( f, "my_color = %f\n", cam_params->cK[2][1] );
    fprintf( f, "fx_depth = %f\n", cam_params->dK[0][0] );
    fprintf( f, "fy_depth = %f\n", cam_params->dK[1][1] );
    fprintf( f, "mx_depth = %f\n", cam_params->dK[2][0] );
    fprintf( f, "my_depth = %f\n", cam_params->dK[2][1] );
    fprintf( f, "k1_color = %f\n", cam_params->cc[0] );
    fprintf( f, "k2_color = %f\n", cam_params->cc[1] );
    fprintf( f, "k3_color = %f\n", cam_params->cc[2] );
    fprintf( f, "k4_color = %f\n", cam_params->cc[3] );
    fprintf( f, "k5_color = %f\n", cam_params->cc[4] );
    fprintf( f, "k1_depth = %f\n", cam_params->dc[0] );
    fprintf( f, "k2_depth = %f\n", cam_params->dc[1] );
    fprintf( f, "k3_depth = %f\n", cam_params->dc[2] );
    fprintf( f, "k4_depth = %f\n", cam_params->dc[3] );
    fprintf( f, "k5_depth = %f\n", cam_params->dc[4] );
    bsc::mat4f m = cam_params->d2c_T;
    fprintf( f, "depthToColorExtrinsics = %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n", 
                                          m[0][0], m[1][0], m[2][0], m[3][0], 
                                          m[0][1], m[1][1], m[2][1], m[3][1],
                                          m[0][2], m[1][2], m[2][2], m[3][2],
                                          m[0][3], m[1][3], m[2][3], m[3][3] );
    fclose(f);
    return 1;
  }
  return 0;
}

i32
write_ply( const char * filename, const bsc::geometry_data * data )
{
  FILE * f = fopen( filename, "w" );
  if ( f )
  {
    int n_points = data->n_vertices;

    fprintf( f, "ply\n" );
    fprintf( f, "format ascii 1.0\n" );
    fprintf( f, "element vertex %d\n", n_points );
    fprintf( f, "property float x\n" );
    fprintf( f, "property float y\n" );
    fprintf( f, "property float z\n" );
    fprintf( f, "property uchar red\n" );
    fprintf( f, "property uchar green\n" );
    fprintf( f, "property uchar blue\n" );
    fprintf( f, "end_header\n" );
    for ( i32 i = 0 ; i < n_points ; i++ )
    {
      fprintf( f, "%f %f %f %u %u %u\n", data->positions[ 3 * i + 0 ]
                                       , data->positions[ 3 * i + 1 ]
                                       , data->positions[ 3 * i + 2 ]
                                       , data->colors_a[ 4 * i + 0 ]
                                       , data->colors_a[ 4 * i + 1 ]
                                       , data->colors_a[ 4 * i + 2 ] );
    }
    fclose( f );
    return 1;
  }
  printf( "Could not write to file %s\n", filename );
  return 0;
}

i32 
init()
{
  // Just load in images
  color_img.read( opts.color_filename );
  depth_img.read( opts.depth_filename );

  undistorted_color_img = bsc::img( color_img );
  undistorted_depth_img = bsc::img_u16( depth_img );
  registered_depth_img = bsc::img_u16( color_img.width, color_img.height, 1 );

  for ( i32 i = 0 ; i < depth_img.width * depth_img.height ; ++i )
  {
    u16 d_raw = depth_img.data[i];
    if ( use_bitshift )
    {
    depth_img.data[i] = ((d_raw >> 3) & 0x1FFF) | ((d_raw & 0x7) << 13);
    }
    undistorted_depth_img.data[i] = 0;
  }

  for ( i32 i = 0; i < color_img.width * depth_img.height ; ++i )
  {
    undistorted_color_img.data[ 3 * i + 0 ] = 0;
    undistorted_color_img.data[ 3 * i + 1 ] = 0;
    undistorted_color_img.data[ 3 * i + 2 ] = 0;
    
    registered_depth_img.data[i]     = 0;
  }

  init_gpu_tex( color_img.data, 
                color_img.width, 
                color_img.height, 
                color_img.ncomp, 
                &color_tex,
                1 );

  init_gpu_tex( registered_depth_img.data, 
                registered_depth_img.width, // we need to move depth to color!
                registered_depth_img.height, 
                registered_depth_img.ncomp, 
                &depth_tex,
                2 );

  // Initialize camera parameters
  cam_params.dK[0][0]    = 570.5;        init_cam_params.dK[0][0]    = 570.5;
  cam_params.dK[1][1]    = 570.5;        init_cam_params.dK[1][1]    = 570.5;
  cam_params.dK[2][0]    = 317.0;        init_cam_params.dK[2][0]    = 320.0;
  cam_params.dK[2][1]    = 246.0;        init_cam_params.dK[2][1]    = 240.0;
  cam_params.cK[0][0]    = 578.0;        init_cam_params.cK[0][0]    = 578.0;
  cam_params.cK[1][1]    = 578.0;        init_cam_params.cK[1][1]    = 578.0;
  cam_params.cK[2][0]    = 320.0;        init_cam_params.cK[2][0]    = 320.0;
  cam_params.cK[2][1]    = 244.0;        init_cam_params.cK[2][1]    = 240.0;
  cam_params.dc[0]       = -0.024f;      init_cam_params.dc[0]       = 0.0f;
  cam_params.dc[1]       = 0.0f;         init_cam_params.dc[1]       = 0.0f;
  cam_params.dc[2]       = 0.0f;         init_cam_params.dc[2]       = 0.0f;
  cam_params.dc[3]       = 0.0f;         init_cam_params.dc[3]       = 0.0f;
  cam_params.dc[4]       = 0.0f;         init_cam_params.dc[4]       = 0.0f;
  cam_params.cc[0]       = 0.126f;       init_cam_params.cc[0]       = 0.0f;
  cam_params.cc[1]       = -0.236f;      init_cam_params.cc[1]       = 0.0f;
  cam_params.cc[2]       = 0.0f;         init_cam_params.cc[2]       = 0.0f;
  cam_params.cc[3]       = 0.0f;         init_cam_params.cc[3]       = 0.0f;
  cam_params.cc[4]       = 0.0f;         init_cam_params.cc[4]       = 0.0f;
  cam_params.d2c_T       = bsc::mat4();  init_cam_params.d2c_T       = bsc::mat4();
  cam_params.d2c_T[1][0] = -0.004f;
  cam_params.d2c_T[2][0] = 0.009f;
  cam_params.d2c_T[2][1] = 0.01f;
  cam_params.d2c_T[0][1] = 0.004f;
  cam_params.d2c_T[0][2] = -0.009f;
  cam_params.d2c_T[1][2] = -0.01f;
  cam_params.d2c_T[3][0] = 0.038f;
  cam_params.d2c_T[3][1] = 0.003f;
  cam_params.d2c_T[3][2] = 0.022f;

  cam_params.c_size = bsc::vec2i( color_img.width, color_img.height );
  cam_params.d_size = bsc::vec2i( depth_img.width, depth_img.height );

  read_parameters( opts.input_parameter_filename, &cam_params );

  // 3d camera controls
  view = cam_controls.initialize(&cam,
                                 bsc::vec3(5.0f, 5.0f, 5.0f),
                                 bsc::vec3(0.0f, 0.0f, 0.0f),
                                 bsc::vec3(0.0f, 1.0f, 0.0f));

  return 1;
}


void
display()
{
  bsc::vec4i viewport( 0, 0, bsc::g_window.size.x - panel_width, bsc::g_window.size.y );
  bsc::clear_window( viewport );
  
  bsc::ui::begin_frame();

  ImGui::SetNextWindowPos(ImVec2(bsc::g_window.size.x - panel_width, 0));
  ImGui::SetNextWindowSize(ImVec2(panel_width, bsc::g_window.size.y));
  ImGui::Begin("Camera Parameters Widget", NULL, ImVec2(-1, -1), -1.0f,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
               ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

  static bool start = true;
  bool should_update = false;
  if ( start ) { should_update  = true; start = false; };
    // should_update |= ImGui::SliderFloat("d_f1", &(cam_params.dK[0][0]), 500.0f, 650.0f );
    // should_update |= ImGui::SliderFloat("d_f2", &(cam_params.dK[1][1]), 500.0f, 650.0f );
  if ( ImGui::CollapsingHeader( "Depth Intrinsics" ) )
  {
    should_update |= ImGui::SliderFloat("d_f1", &(cam_params.dK[0][0]), 500.0f, 650.0f );
    should_update |= ImGui::SliderFloat("d_f2", &(cam_params.dK[1][1]), 500.0f, 650.0f );
    should_update |= ImGui::SliderFloat("d_c1", &(cam_params.dK[2][0]), 280.0f, 360.0f );
    should_update |= ImGui::SliderFloat("d_c2", &(cam_params.dK[2][1]), 200.0f, 280.0f );
  }

  if ( ImGui::CollapsingHeader ("Depth Distortion Coeffs." ) )
  {
    should_update |= ImGui::SliderFloat( "d_k1", &(cam_params.dc[0]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat( "d_k2", &(cam_params.dc[1]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat( "d_p1", &(cam_params.dc[2]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat( "d_p2", &(cam_params.dc[3]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat( "d_k3", &(cam_params.dc[4]), -1.0f, 1.0f );
  }
  
  if ( ImGui::CollapsingHeader( "Color Intrinsics" ) )
  {
    should_update |= ImGui::SliderFloat("c_f1", &(cam_params.cK[0][0]), 1000.0f, 1250.0f );
    should_update |= ImGui::SliderFloat("c_f2", &(cam_params.cK[1][1]), 1000.0f, 1250.0f );
    should_update |= ImGui::SliderFloat("c_c1", &(cam_params.cK[2][0]), 560.0f, 800.0f );
    should_update |= ImGui::SliderFloat("c_c2", &(cam_params.cK[2][1]), 400.0f, 560.0f );
  }

  if ( ImGui::CollapsingHeader ("Color Distortion Coeffs." ) )
  {
    should_update |= ImGui::SliderFloat( "c_k1", &(cam_params.cc[0]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat( "c_k2", &(cam_params.cc[1]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat( "c_p1", &(cam_params.cc[2]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat( "c_p2", &(cam_params.cc[3]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat( "c_k3", &(cam_params.cc[4]), -1.0f, 1.0f );
  }

  if ( ImGui::CollapsingHeader( "Extrinsics" ) )
  { 
    should_update |= ImGui::SliderFloat4("X", (float*)&(cam_params.d2c_T[0]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat4("Y", (float*)&(cam_params.d2c_T[1]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat4("Z", (float*)&(cam_params.d2c_T[2]), -1.0f, 1.0f );
    should_update |= ImGui::SliderFloat4("POS", (float*)&(cam_params.d2c_T[3]), -0.1f, 0.1f );
  }
  ImGui::Separator();
  ImGui::SliderFloat("Alpha", &alpha, 0.0f, 1.0f);

  CameraParams * cur_params = &cam_params;
  static bool show_initial = false;
  if ( bsc::ui::is_key_pressed( bsc::ui::key_m ) )
  {
    view_mode = (view_mode == IMAGE_MODE ) ? POINTCLOUD_MODE : IMAGE_MODE;
  }

  if ( bsc::ui::is_key_pressed( bsc::ui::key_a ) )
  {
    alpha = (r32)(!(bool)alpha);
  }

  if ( show_initial )
  {
    ImGui::Separator();
    ImGui::Text( "Showing Initial Parameters!" );
  }

  if ( bsc::ui::is_key_pressed( bsc::ui::key_s ) )
  {
    cur_params    = (show_initial) ? &cam_params : &init_cam_params;
    show_initial  = !show_initial;
    should_update = true;
  }


  if ( should_update )
  {
    bsc::stop_watch timer;
    timer.start();
    u32 depth_size = depth_img.width * depth_img.height * sizeof(u16);
    u32 color_size = color_img.width * color_img.height * 3 * sizeof(u8);
    memset( undistorted_depth_img.data, 0, depth_size );
    memset( registered_depth_img.data, 0, color_img.width * color_img.height * sizeof(u16) );
    memset( undistorted_color_img.data, 0, color_size );

    undistort( &depth_img, &undistorted_depth_img, cur_params->dK, cur_params->dc );
    transform_img( &undistorted_depth_img, &registered_depth_img, cur_params );
    undistort( &color_img, &undistorted_color_img, cur_params->cK, cur_params->cc );

    update_gpu_tex( registered_depth_img.data, 
                    &depth_tex );
    update_gpu_tex( undistorted_color_img.data, 
                    &color_tex );

    ImGui::Separator();
    ImGui::Text("Last Update time: %f\n", timer.elapsed() );
  }

  static char filename[512] = "Test.ply";
  ImGui::Separator();
  ImGui::InputText("Ply Name", filename, 512);
  if ( ImGui::Button("Save Ply", ImVec2( 336, 24  ) ) )
  {
    bsc::geometry_data pc_data;
    generate_pointcloud_data( pc_data, &color_img, &registered_depth_img, cur_params->dK );
    write_ply( filename, &pc_data );
  }

  if ( opts.output_parameter_filename && ImGui::Button("Save Parameters", ImVec2( 336, 24  ) ) )
  {
    write_parameters( opts.output_parameter_filename, &cam_params );
  }
  if ( view_mode == IMAGE_MODE )
  {
    bsc::use_texture( &color_tex );
    bsc::use_texture( &depth_tex );
    render_explorer_image( alpha, vs_image, fs_image );
  }
  if ( view_mode == POINTCLOUD_MODE )
  {
    if (bsc::ui::is_mouse_over(viewport) && !ImGui::IsMouseHoveringAnyWindow())
    {
      cam_controls.update( &cam, &view, &viewport );
    }

    static r32 near = 0.1f;
    static r32 far = 100.0f;
    r32 fovy = bsc::deg2rad(45.0f);
    r32 aspect_ratio = bsc::get_window().aspect_ratio;
    proj = bsc::perspective(fovy, aspect_ratio, near, far);

    render_explorer_pointcloud( &pointcloud_geo, 
                                &undistorted_color_img, 
                                &registered_depth_img, 
                                cur_params->dK,
                                should_update,
                                vs_pointcloud,
                                fs_pointcloud );

  }
  if ( ImGui::Button( "Take Screenshot", ImVec2( 336, 24 ) ) )
  {
    bsc::take_screenshot( viewport );
  }

  ImGui::End();
  ImGui::Render();

  bsc::ui::end_frame();
}

void
parse_arguments( int argc, char **argv )
{
  opts.color_filename             = NULL;
  opts.depth_filename             = NULL;
  opts.input_parameter_filename   = NULL;
  opts.output_parameter_filename = NULL;

  bsc::arg_parse args;
  args.add(bsc::argument<char*>("parameter_file",
                                "File containing camera parameters",
                                &(opts.input_parameter_filename)));

  args.add(bsc::argument<char*>("color_image",
                                "Name of color image",
                                &(opts.color_filename)));

  args.add(bsc::argument<char*>("depth_image",
                                "Name of depth image encoded as 16bit png",
                                &(opts.depth_filename)));
  
  args.add(bsc::argument<char*>("-o", "--output_parameters",
                                "Output modified parameters",
                                &(opts.output_parameter_filename), 1 ) );

  args.add(bsc::argument<bool>("-b", "--use_bitshift",
                               "Should modify depth values?",
                               &(use_bitshift), 0 ) );

  args.parse( argc, argv );
}

int main( int argc, char **argv )
{
  parse_arguments( argc, argv );
  bsc::create_window( "Calibration Explorer", 1296 + panel_width, 968 );
  bsc::set_init_funct( init );
  bsc::set_display_funct( display );
  bsc::main_loop();

  return 0;
}
