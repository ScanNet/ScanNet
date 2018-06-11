#define BSC_IMPLEMENTATION
#define BSC_USE_WINDOW
#define BSC_USE_IMGUI
#include <basics.h>
#include "grid3d.h"

////////////////////////////////////////////////////////////////////////////////
// GPU CODE
////////////////////////////////////////////////////////////////////////////////
char * debug_vs = (char *)SHADER_HEAD STR
(
  layout (location = 0) in vec3 position;
  layout (location = 4) in vec4 color;
  out vec4 v_color;
  uniform mat4 mvp;
  void main()
  {
    gl_Position = mvp * vec4(position, 1.0);
    v_color = color;
  }
);

char * debug_fs = (char *)SHADER_HEAD STR
(
  in vec4 v_color;
  out vec4 frag_color;
  void main()
  {
    frag_color = v_color;
  }
);

char * pointcloud_vs = (char *)SHADER_HEAD STR
(
  layout(location = 0) in vec3 position;
  layout(location = 1) in vec3 normal;
  layout(location = 4) in vec4 color;
  out vec4 v_color;
  out vec3 v_normal;
  uniform mat4 mvp;
  void main() {
    gl_Position = mvp * vec4(position, 1.0);
    v_normal = normal;
    v_color = color;
  }
);

char * pointcloud_fs = (char *)SHADER_HEAD STR
(
  in vec4 v_color;
  in vec3 v_normal;
  out vec4 frag_color;
  uniform bool show_normals;
  void main() {
    if (!show_normals)
    {
      frag_color = v_color;
    }
    else
    {
      frag_color = vec4(0.5 * (v_normal + 1.0), 1.0);
    }
  }
);

char *plane_vs = (char *)SHADER_HEAD STR
(
  layout(location = 0) in vec3 position;
  uniform mat4 mvp;
  void main() {
    gl_Position = mvp * vec4(position, 1.0);
  }
);

char *plane_fs = (char *)SHADER_HEAD STR
(
  uniform vec4 color;
  out vec4 frag_color;
  void main() {
    frag_color = color;
  }
);

////////////////////////////////////////////////////////////////////////////////
// I/O
////////////////////////////////////////////////////////////////////////////////

struct SequenceData
{
  bsc::mat3 depth_intrinsics;
  bsc::mat4 depth_to_color;
  std::vector<bsc::mat4> plane_poses;
  std::vector<std::string> image_names;
  int n_images;
};

struct Options
{
  bool print_verbose;
  bool save_slice_visualization;
  bool bitshift;
  std::string configuration_filename;
  std::string estimated_lut_filename;
  std::string applied_lut_filename;
  float n_slices;
  float max_depth;
};

static SequenceData seq_data;
static Options opts;

static int 
ReadParametersFile( bsc::mat3 &intrinsics,
                    bsc::mat4 &extrinsics, 
                    const char* filename )
{
  intrinsics = bsc::mat3();
  extrinsics = bsc::mat4();
  FILE * params_file = fopen( filename, "r" );
  if ( params_file )
  {
    // Parse file
    char buffer[1024];
    int line_number = 0;

    while (fgets(buffer, 1024, params_file))
    {
      char cmd[1024];
      line_number++;
      bool success = 1;
      if (sscanf(buffer, "%s =", cmd) != (unsigned int)1)
      {
        continue;
      }
      if (cmd[0] == '#')
        continue;

      if (!strcmp(cmd, "fx_depth"))
      {
        success = (sscanf(buffer, "%s = %f\n", cmd, &intrinsics[0][0] ) != 2);
      }
      else if (!strcmp(cmd, "fy_depth"))
      {
        success = (sscanf(buffer, "%s = %f\n", cmd, &intrinsics[1][1] ) != 2);
      }
      else if (!strcmp(cmd, "mx_depth"))
      {
        success = (sscanf(buffer, "%s = %f\n", cmd, &intrinsics[2][0] ) != 2);
      }
      else if (!strcmp(cmd, "my_depth"))
      {
        success = (sscanf(buffer, "%s = %f\n", cmd, &intrinsics[2][1] ) != 2);
      }
      else if (!strcmp(cmd, "depthToColorExtrinsics"))
      {
        bsc::mat4 m;
        success = (sscanf(buffer, 
                          "%s = %f %f %f %f\n"
                                "%f %f %f %f\n"
                                "%f %f %f %f\n"
                                "%f %f %f %f\n", 
                                cmd, 
                                &m[0][0], &m[1][0], &m[2][0], &m[3][0], 
                                &m[0][1], &m[1][1], &m[2][1], &m[3][1], 
                                &m[0][2], &m[1][2], &m[2][2], &m[3][2], 
                                &m[0][3], &m[1][3], &m[2][3], &m[3][3] ) != 17);
        extrinsics = m;
      }
      else
      {
        continue;
      }
    }
    fclose(params_file);
    return 1;
  }
  
  printf( "Could not open parameters file %s\n", filename );
  
  return 0;
}

static int
ReadPlanePoses( bsc::mat4 depth_to_color,
                std::vector<bsc::mat4> &plane_poses, 
                const int n_poses,
                const char *filename )
{
  bsc::mat4 flip; // since Tom also has orgin in bottom left we need to flip.
  flip[1][1] = -1;
  flip[2][2] = -1;
  depth_to_color = flip * depth_to_color * flip;

  // We actually want color to depth
  bsc::mat4 color_to_depth = bsc::inverse( depth_to_color );
  
  // Open camera poses file
  FILE * fp = fopen(filename, "r");
  if (!fp)
  {
    return 0;
  }

  // Read file with plane poses( from matlab )
  plane_poses.reserve(n_poses);
  for (int i = 0; i < n_poses; i++)
  {
    // Load in the plane pose
    bsc::mat4 pose;
    fscanf(fp, "%f %f %f %f", &(pose[0][0]), &(pose[1][0]), &(pose[2][0]), &(pose[3][0]));
    fscanf(fp, "%f %f %f %f", &(pose[0][1]), &(pose[1][1]), &(pose[2][1]), &(pose[3][1]));
    fscanf(fp, "%f %f %f %f", &(pose[0][2]), &(pose[1][2]), &(pose[2][2]), &(pose[3][2]));
    fscanf(fp, "%f %f %f %f", &(pose[0][3]), &(pose[1][3]), &(pose[2][3]), &(pose[3][3]));

    plane_poses.push_back( color_to_depth * pose  );
  }

  // Close file
  fclose(fp);

 // Return success
  return 1;
}

int ParseParametersCmd(const char *buffer, char *cmd, SequenceData *data)
{
  char filename[1024];
  if (sscanf(buffer, "%s%s", cmd, filename) != (unsigned int)2)
    return 0;

  bsc::mat3 intrinsics_matrix;
  bsc::mat4 extrinsics_matrix;
  if (!ReadParametersFile(intrinsics_matrix,
                          extrinsics_matrix,
                          filename))
  {
    fprintf(stderr, "\nUnable to read parameters file %s\n", filename);
    exit(-1);
    return 0;
  }

  data->depth_intrinsics = intrinsics_matrix;
  data->depth_to_color = extrinsics_matrix;

  return 1;
}

int ParsePlanePosesCmd(const char *buffer, char *cmd, SequenceData *data)
{
  char filename[1024];
  if (sscanf(buffer, "%s%s", cmd, filename) != (unsigned int)2)
    return 0;

  if (!ReadPlanePoses( data->depth_to_color,
                       data->plane_poses, 
                       data->n_images, 
                       filename ) )
  {
    fprintf(stderr, "\nUnable to read plane_poses file %s\n", filename);
    exit(-1);
    return 0;
  }

  return 1;
}

int ParseNImagesCmd(const char *buffer, char *cmd, SequenceData *data)
{
  int n_images = 0;
  if (sscanf(buffer, "%s%d", cmd, &n_images) != (unsigned int)2)
    return 0;

  data->n_images = n_images;

  return 1;
}

int ParseScanCmd(const char *buffer, char *cmd, SequenceData *data)
{
  char depth_name[1024], rgb_name[1024];
  float dummy[16];
  if (sscanf(buffer, "%s%s%s%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f", cmd,
             depth_name, rgb_name,
             &dummy[0], &dummy[1], &dummy[2], &dummy[3],
             &dummy[4], &dummy[5], &dummy[6], &dummy[7],
             &dummy[8], &dummy[9], &dummy[10], &dummy[11],
             &dummy[12], &dummy[13], &dummy[14], &dummy[15]) != (unsigned int)19)
  {
    return 0;
  }

  data->image_names.push_back(std::string(depth_name));
  return 1;
}

int ReadConfigurationFile(SequenceData *data, const Options *opts)
{
  bsc::stop_watch timer;
  timer.start();
  const char *configuration_filename = opts->configuration_filename.c_str();
  int n_read_scans = 0;

  // Open configuration file
  FILE *configuration_fp = fopen(configuration_filename, "r");
  if (!configuration_fp)
  {
    fprintf(stderr, "Unable to open configuration file %s\n",
            configuration_filename);
    exit(-1);
    return 0;
  }

  if (opts->print_verbose)
  {
    printf("Reading configuration file...\n");
  }

  // Parse file
  char buffer[1024];
  int line_number = 0;

  while (fgets(buffer, 1024, configuration_fp))
  {
    char cmd[1024];
    line_number++;
    bool success = 1;
    if (sscanf(buffer, "%s", cmd) != (unsigned int)1)
      continue;

    if (cmd[0] == '#')
      continue;

    if (!strcmp(cmd, "parameters"))
    {
      success = ParseParametersCmd(buffer, cmd, data);
    }
    else if (!strcmp(cmd, "plane_poses"))
    {
      success = ParsePlanePosesCmd(buffer, cmd, data);
    }
    else if (!strcmp(cmd, "n_images"))
    {
      success = ParseNImagesCmd(buffer, cmd, data);
    }
    else if (!strcmp(cmd, "scan"))
    {
      success &= ParseScanCmd(buffer, cmd, data);
      n_read_scans++;
    }
  }
  fclose(configuration_fp);

  // Print statistics
  if (opts->print_verbose)
  {
    printf("  # Images = %lu (%d)\n", data->image_names.size(), data->n_images);
    printf("Done in %f sec\n\n", timer.elapsed() );
  }

  // Return success
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Drawing
////////////////////////////////////////////////////////////////////////////////

void update_pointcloud(const bsc::img_u16  *depth_image,
                       const bsc::img_u8 *color_image,
                       const bsc::mat3 &K,
                       bsc::geometry_data *p,
                       bool show_colors,
                       bsc::vec3 col,
                       bool bitshift )
{
  i32 w = depth_image->width;
  i32 h = depth_image->height;

  r32 scale = 1.0f;
  if ( color_image != NULL )
  {
    i32 ch = color_image->height;
    scale = (r32) ch / h;
  }
  for (i32 j = 0; j < h; j++)
  {
    for (i32 i = 0; i < w; i++)
    {
      int idx = j * w + i;
      uint16_t d = *depth_image->at(i, j);
      if (bitshift) d = ((d >> 3) & 0x1FFF) | ((d & 0x7) << 13);
      r32 depth = d * 0.001f;

      p->positions[3 * idx + 0] = ((i)-K[2][0]) * depth / K[0][0];
      p->positions[3 * idx + 1] = ((h-j)-K[2][1]) * depth / K[1][1];
      p->positions[3 * idx + 2] = -depth;

      if ( show_colors && color_image != NULL )
      {
        const u8* cur_color = color_image->at(scale*i, scale*j);
        p->colors_a[4 * idx + 0] = cur_color[0];
        p->colors_a[4 * idx + 1] = cur_color[1];
        p->colors_a[4 * idx + 2] = cur_color[2];
        p->colors_a[4 * idx + 3] = 255;
      }
      else
      {
        p->colors_a[4 * idx + 0] = col.r * 255;
        p->colors_a[4 * idx + 1] = col.g * 255;
        p->colors_a[4 * idx + 2] = col.b * 255;
        p->colors_a[4 * idx + 3] = 255;
      }
    }
  }
}

void init_pointcloud(const bsc::img_u16 *depth_image,
                     const bsc::img_u8 *image,
                     const bsc::mat3 &K,
                     bsc::geometry_data *p,
                     bool bitshift )
{
  i32 w = depth_image->width;
  i32 h = depth_image->height;

  p->positions = (r32 *)calloc(w * h * 3, sizeof(r32));
  p->colors_a = (u8 *)calloc(w * h * 4, sizeof(u8));
  p->normals = (r32 *)calloc(w * h * 3, sizeof(r32));
  p->n_vertices = w * h;

  update_pointcloud(depth_image, 
                    image, 
                    K, 
                    p, 
                    true, bsc::vec3(1.0, 1.0, 1.0), 
                    bitshift );
}

void init_axes(bsc::gpu_geometry *cmd)
{
  bsc::geometry_data axes;
  float positions[18] = {0, 0, 0,
                         1, 0, 0,
                         0, 0, 0,
                         0, 1, 0,
                         0, 0, 0,
                         0, 0, 1};
  u8 colors[24] = {255, 0, 0, 255, 255, 0, 0, 255,
                   0, 255, 0, 255, 0, 255, 0, 255,
                   0, 0, 255, 255, 0, 0, 255, 255};
  axes.positions = positions;
  axes.colors_a = colors;
  axes.n_vertices = 6;

  bsc::init_gpu_geo(&axes, cmd, POSITION | COLOR_A);
}

void init_planes(bsc::gpu_geometry *cmds)
{
  float depths[6] = {0.5, 1, 2, 3, 4, 5};
  for (int i = 0; i < 6; ++i)
  {
    bsc::geometry_data plane;
    plane.n_vertices = 4;
    float positions[12] = {-0.64f * depths[i], -0.48f * depths[i], -depths[i],
                            0.64f * depths[i], -0.48f * depths[i], -depths[i],
                            0.64f * depths[i], 0.48f * depths[i], -depths[i],
                           -0.64f * depths[i], 0.48f * depths[i], -depths[i]};
    plane.positions = positions;

    bsc::init_gpu_geo(&plane, &(cmds[i]), POSITION);
  }
}

void init_grid(bsc::gpu_geometry *cmd)
{
  bsc::geometry_data grid;
  grid.n_vertices = 34;
  r32 positions[34 * 3];
  r32 width = 0.054f;
  i32 idx = 0;
  for (int row = 0; row < 10; ++row)
  {
    positions[idx + 0] = row * width;
    positions[idx + 1] = 0;
    positions[idx + 2] = 0;

    positions[idx + 3] = row * width;
    positions[idx + 4] = -6 * width;
    positions[idx + 5] = 0;
    idx += 6;
  }

  for (int col = 0; col < 7; ++col)
  {
    positions[idx + 0] = 0;
    positions[idx + 1] = -col * width;
    positions[idx + 2] = 0;

    positions[idx + 3] = 9 * width;
    positions[idx + 4] = -col * width;
    positions[idx + 5] = 0;
    idx += 6;
  }

  grid.positions = positions;
  bsc::init_gpu_geo(&grid, cmd, POSITION);
}

////////////////////////////////////////////////////////////////////////////////
// Frame Conversion
////////////////////////////////////////////////////////////////////////////////

static float angle_threshold = 25.0f;

void 
ReportBinCounts( const SequenceData * seq_data,
                 const Options * opts  )
{
  i32 max_idx = opts->n_slices;
  int z_bin = opts->n_slices / opts->max_depth;
  i32 bin_counts[ 128 ] = {0};
  i32 total_count = 0;
  std::vector<std::string> bin_names[128];

  r32 max_pairwise_dist = 0.0f;

  for ( size_t i = 0 ; 
        i < seq_data->plane_poses.size() ; 
        ++i )
  {
    r32 angle = bsc::rad2deg(
                bsc::angle( bsc::vec3( 0.0f, 0.0f, 1.0f ),
                            bsc::vec3( seq_data->plane_poses[i][2] ) ) );
    if ( angle < angle_threshold )
    {
      r32 distance =  -seq_data->plane_poses[i][3][2];
      if ( i > 0 )
      {
        r32 pairwise_dist = distance + seq_data->plane_poses[i-1][3][2];
        if ( pairwise_dist > max_pairwise_dist )
        {
          max_pairwise_dist = pairwise_dist;
        }
      }
      i32 idx = distance * z_bin;
      bin_counts[idx] += 1;
      bin_names[idx].push_back( seq_data->image_names[i] );
      total_count++;
    }
  }
  printf("Depth Bins: \n");
  for ( i32 i = 0 ;
        i < max_idx;
        i++ )
  {
    r32 min_dist = i * (1.0f / z_bin);
    r32 max_dist = (i + 1) * (1.0f / z_bin);
    printf( "\tBin %3d (%5.3f - %5.3f) : %3d\n", i, min_dist, max_dist, bin_counts[i] );
  }
  printf("Will use %d/%d images for calibration\n", total_count, 
                                                     seq_data->n_images );
}

void
ApplyUndistortion( const SequenceData * seq_data, const Options * opts )
{
  bsc::stop_watch timer;
  if ( opts->print_verbose )
  {
    printf("Applying undistortion volume\n");
    timer.read();
  }

  // extract relevant info
  i32 w = 640;
  i32 h = 480;
  bsc::img_u16 raw_depth_im;
  bsc::img_u16 depth_im( w, h, 1 );
  Grid3D undistort_table;
  undistort_table.ReadFile( opts->applied_lut_filename.c_str() );
  i32 n_slices = undistort_table.ZRes();

  r32 x_bin = w / undistort_table.XRes();
  r32 y_bin = h / undistort_table.YRes();
  r32 z_bin = undistort_table.ZRes() / undistort_table.MaxDist();

  // check if depth folder exists

  #if defined(_WIN32)
  _mkdir((const char*)"depth");
  #else 
  mkdir((const char*)"depth", 0775); // notice that 777 is different than 0777
  #endif

  // save out images 
  for (i32 im_idx = 0;
       im_idx < seq_data->n_images;
       im_idx++)
  {

    if ( opts->print_verbose )
    {
      float percentage = (float)(im_idx + 1) / seq_data->n_images * 100.0f;
      printf("Progress %5.2f%% (%d/%d)\r", percentage, 
                                         im_idx + 1, 
                                         seq_data->n_images );
      fflush(stdout);
    }
    std::string im_name = seq_data->image_names[im_idx];
    int name_length = im_name.length();
    const char *img_name = im_name.substr(0, name_length - 4).c_str();
    char depth_name[256], depth_raw_name[256];
    sprintf( depth_raw_name, "depth_raw/%s.png", img_name );
    sprintf( depth_name, "depth/%s.png", img_name );
    raw_depth_im.read( depth_raw_name );

    for ( i32 j = 0; j < h; ++j )
    {
      for ( i32 i = 0; i < w; ++i )
      {
        uint16_t d = *raw_depth_im(i, j);
        if (opts->bitshift) d = ((d >> 3) & 0x1FFF) | ((d & 0x7) << 13);
        r32 depth = d * 0.001f;

        r32 z_idx = std::min( (r32)depth * z_bin, (r32)n_slices - 1.0f );

        r32 multiplier2 = 1.0f / undistort_table.GetValue( (float) i / x_bin,
                                                             (float) j / y_bin,
                                                                z_idx );
        r32 new_depth = depth * multiplier2;
        uint16_t new_d = 1000.0f * new_depth;
        if (opts->bitshift)  new_d = ((new_d & 0xE000) >> 13) | ((new_d << 3) & 0xFFF8);
        *depth_im(i, j) = new_d;
      }
    }

    depth_im.write( depth_name ) ;
  }
  if ( opts->print_verbose )
  {
    printf("\nDone in %f sec.\n", timer.elapsed() );
  }
}


void EstimateDistortion(const SequenceData *seq_data, const Options *opts)
{
  
  // Storage + useful variables
  bsc::img_u16 raw_depth_im;
  bsc::vec2 *train_data = (bsc::vec2 *)calloc( 640 * 480 * seq_data->n_images, 
                                               2 * sizeof(r32));
  std::vector<int> skipped;
  skipped.reserve(100);

  bsc::stop_watch timer;


  // Gather training data step
  if ( opts->print_verbose )
  {
    printf("\nGathering training data\n");
    timer.read();
  }
  for (int im_idx = 0;
       im_idx < seq_data->n_images;
       ++im_idx)
  {

    // Read in images
    char depth_raw_name[256];
    const char *img_name = seq_data->image_names[im_idx].c_str();
    sprintf( depth_raw_name, "depth_raw/%s", img_name );
    raw_depth_im.read(depth_raw_name);

    // gather important info
    i32 w = raw_depth_im.width;
    i32 h = raw_depth_im.height;
    bsc::mat4 T = seq_data->plane_poses[im_idx];
    bsc::mat3 K = seq_data->depth_intrinsics;

    // get plane equation
    bsc::vec3 n(T[2]);
    bsc::vec3 p(T[3]);
    bsc::plane plane;
    plane.n = n;
    plane.d = -bsc::dot(n, p);

    // if image deviates from angle, lets skip it
    r32 angle = bsc::rad2deg( bsc::angle( bsc::vec3( 0.0f, 0.0f, 1.0f ), n ) );
    r32 percentage = (float)(im_idx+1) / seq_data->n_images * 100.0f;
    if ( angle > angle_threshold )
    {
      skipped.push_back( im_idx );
      continue;
    }
    else
    {
      if ( opts->print_verbose )
      { 
        printf( "Progress : %5.2f%% (%d/%d)\r", percentage,
                                                im_idx+1, 
                                                seq_data->n_images );
        fflush(stdout);
      }
    }

    // generate pointcloud
    bsc::geometry_data input_pointcloud;
    init_pointcloud(&raw_depth_im, NULL,
                    K, &input_pointcloud, 
                    opts->bitshift);

    // gather the important training data
    for (i32 j = 0; j < h; j++)
    {
      for (i32 i = 0; i < w; i++)
      {
        int idx = j * w + i;
        bsc::vec3 pos;
        bsc::vec3 normal;

        pos.x = input_pointcloud.positions[3 * idx + 0];
        pos.y = input_pointcloud.positions[3 * idx + 1];
        pos.z = input_pointcloud.positions[3 * idx + 2];

        i8 is_valid = (-pos.z > 0.01);

        if (is_valid)
        {
          bsc::vec3 origin(0.0f, 0.0f, 0.0f);
          bsc::vec3 v = pos - origin;
          v = bsc::normalize(v);

          bsc::ray ray;
          ray.o = origin;
          ray.v = v;

          bsc::vec3 new_pos = bsc::intersect(ray, plane);
          train_data[(w * h) * im_idx + idx].x = -pos.z;
          train_data[(w * h) * im_idx + idx].y = -new_pos.z;
        }
        else
        {
          train_data[(w * h) * im_idx + idx].x = -pos.z;
          train_data[(w * h) * im_idx + idx].y = -pos.z;
        }
      }
    }
  }

  // print info regarding finished process.
  if ( opts->print_verbose )
  {
    printf("\nDone in %f sec.\n", timer.elapsed() );
    if ( !skipped.empty() )
    {
      printf("Skipped images: \n");
      for ( size_t idx = 0 ; idx < skipped.size() ; ++idx )
      {
        const char *img_name = seq_data->image_names[ skipped[idx] ].c_str();
        printf("\t %s\n", img_name );
      }
    }
    printf("\nComputing undistortion LUT\n");
    timer.read();
  }

  // Training to volume conversion
  i32 w = 640, h = 480; 
  r32 x_bin = 2;
  r32 y_bin = 2;
  r32 z_bin = opts->n_slices / opts->max_depth;
  Grid3D accumulation( w / x_bin, h / y_bin, opts->n_slices, opts->max_depth );
  Grid3D divisor( w / x_bin, h / y_bin, opts->n_slices, opts->max_depth );

  for (i32 im_idx = 0;
       im_idx < seq_data->n_images;
       im_idx++)
  {
    // we limit the borders as the normals there are bad
    for (i32 j = 0; j < h; ++j)
    {
      for (i32 i = 0; i < w; ++i)
      {
        bsc::vec2 pair = train_data[(w * h) * im_idx + j * w + i];

        i32 x = floor((r32)i / x_bin);
        i32 y = floor((r32)j / y_bin);
        i32 z = floor( pair.x * z_bin );

        if ( z > opts->n_slices - 1 || z == 0.0 )
        {
          continue;
        }
        else
        {
          r32 observed_depth = pair.x;
          r32 real_depth = pair.y;       
          accumulation.Add(x, y, z, observed_depth * real_depth );
          divisor.Add(x, y, z, real_depth * real_depth );
        }
      }
    }
  }

  // Compute the multiplier values
  Grid3D multipliers( accumulation );
  multipliers.Divide( divisor );
  
  // deal with side strip
  int stripe_width = 8 / x_bin;
  for (int z = 0; z < multipliers.ZRes(); ++z)
  {
    for (int y = 0; y < multipliers.YRes(); ++y)
    {
      float val = multipliers.GetValue( multipliers.XRes() - stripe_width - 1, y, z );
      for (int x = multipliers.XRes()-stripe_width; x < multipliers.XRes(); ++x )
      {
        multipliers.SetValue( x, y, z, val );
      }
    }
  }
  
  // Replace unknowns -> This should be like a cross bilateral filter
  for ( int i = 0 ; i < multipliers.NElements() ; ++i )
  {
    if ( multipliers.GetValue(i) == UNKNOWN_GRID_VALUE )
    {
      multipliers.SetValue( i, 1.0 );
    }
  }
  
  multipliers.WriteFile( opts->estimated_lut_filename.c_str() );
  free(train_data);

  // Save slices for debugging
  if ( opts->save_slice_visualization )
  {
    if ( opts->print_verbose )
    { 
      printf( "Saving volume visualization!\n" );
    }
    r32 min = multipliers.Min();
    r32 max = multipliers.Max();
    for (int i = 0; i < multipliers.ZRes(); ++i)
    {
      bsc::image<float> slice( multipliers.XRes(), multipliers.YRes(), 1 );
      int n_entries = slice.width * slice.height;
      for ( int j = 0 ; j < n_entries ; ++j)
      {
        *(slice( j )) = multipliers.GetValue( i * n_entries + j );
      }
      bsc::img_u8 slice_img( slice.width, slice.height, 3 );
      if ( opts->print_verbose )
      {
        printf( "\tSlice %3d -> Min: %5.4f Max %5.4f\n", 
                 i, slice.minimum(), slice.maximum() );
      }
      for ( int j = 0 ; j < n_entries ; ++j )
      {
        r32 val = *slice( j );
        if ( val != UNKNOWN_GRID_VALUE )
          val = (val - min) / (max - min);
        bsc::vec3 color( 1.0, 0.5, 0.0 );
        
        if ( val != UNKNOWN_GRID_VALUE )
        {
          color =bsc::vec3( 0.0, 0.0, 0.0 );
          if ( val < 0.5 )
          {
            color[0] = 1.0 - 2.0 * val;
            color[1] = 2.0 * val;
          }
          else
          {
            color[1] = 1 - 2 * (val - 0.5);
            color[2] = 2 * (val - 0.5); 
          }
        };
        u8 *pixel = slice_img( j % slice.width,
                               j / slice.width );
        pixel[0] = (u8)(color.r * 255.0f);
        pixel[1] = (u8)(color.g * 255.0f);
        pixel[2] = (u8)(color.b * 255.0f);
      }
      char name[256];
      sprintf( name, "slice_%03d.png", i );
      slice_img.write(name);
    }
  }
  printf("Done in %f sec.\n", timer.elapsed() );
}

////////////////////////////////////////////////////////////////////////////////
// Main functionality
////////////////////////////////////////////////////////////////////////////////

static bsc::camera cam;
static bsc::mat4 view;
static bsc::trackball_controls cam_controls;
static bsc::shader_prog debug_prog;
static bsc::shader_prog pointcloud_prog;
static bsc::shader_prog plane_prog;

static bsc::gpu_geometry raw_pointcloud_cmd;
static bsc::gpu_geometry fixed_pointcloud_cmd;
static bsc::gpu_geometry axes_cmd;
static bsc::gpu_geometry grid_cmd;
static bsc::gpu_geometry planes_cmds[6];

static bsc::img_u16 raw_image;
static bsc::img_u16 fixed_image;
static bsc::img_u8  color_image;
static bsc::geometry_data raw_pointcloud;
static bsc::geometry_data fixed_pointcloud;

static bool use_raw      = 1;
static int  cur_idx       = 0;
static bool show_planes  = 0;
static bool show_normals = 0;
static bool show_colors  = 1;

int Init()
{
  using namespace bsc;

  view = cam_controls.initialize( &cam, vec3f( 0.0f, 0.0f, 7.0f ),
                                        vec3f( 0.0f, 0.0f, 0.0f ),
                                        vec3f( 0.0f, 1.0f, 0.0f ) );

  bsc::create_shader_prog_from_source(debug_vs,
                                      debug_fs,
                                      debug_prog);
  bsc::create_shader_prog_from_source(plane_vs,
                                      plane_fs,
                                      plane_prog);
  bsc::create_shader_prog_from_source(pointcloud_vs,
                                      pointcloud_fs,
                                      pointcloud_prog);

  // prepare the pointcloud data to draw on cpu
  int name_length = seq_data.image_names[0].length();
  std::string base_name = seq_data.image_names[0].substr(0, name_length - 4);

  raw_image.read(("depth_raw/" + base_name + ".png").c_str());
  fixed_image.read(("depth/" + base_name + ".png").c_str());
  color_image.read(("color/" + base_name + ".jpg").c_str());

  init_pointcloud(&raw_image, &color_image,
                  seq_data.depth_intrinsics,
                  &raw_pointcloud, opts.bitshift );
  init_pointcloud(&fixed_image, &color_image,
                  seq_data.depth_intrinsics,
                  &fixed_pointcloud, opts.bitshift );

  // send it to gpu
  bsc::init_gpu_geo(&raw_pointcloud, &raw_pointcloud_cmd,
                   POSITION | COLOR_A | NORMAL);
  bsc::init_gpu_geo(&fixed_pointcloud, &fixed_pointcloud_cmd,
                   POSITION | COLOR_A | NORMAL);

  // TODO FIx point cloud initialization and the rest.. we dont need local geometry_data for axes or planes
  init_axes(&axes_cmd);
  init_grid(&grid_cmd);
  init_planes(&planes_cmds[0]);

  return 1;
}

int Display()
{
  using namespace bsc;

  bsc::clear_window();
  bsc::ui::begin_frame();

  std::string raw_image_name = "depth_raw/" + seq_data.image_names[cur_idx];
  std::string fixed_image_name = "depth/" + seq_data.image_names[cur_idx];
  std::string color_image_name = "color/" + seq_data.image_names[cur_idx].substr(0, seq_data.image_names[cur_idx].size() - 3) + "jpg";

  // User input
  if (ui::is_key_pressed(ui::keys::key_s))
  {
    use_raw = !use_raw;
  }

  if (ui::is_key_pressed(ui::keys::key_n))
  {
    show_normals = !show_normals;
  }

  bool need_update = false;
  if (ui::is_key_pressed(ui::keys::key_c))
  {
    need_update = true;
    show_colors = !show_colors;
  }

  if (ui::is_key_pressed(ui::keys::key_a))
  {
    cur_idx = std::max( 0, cur_idx - 1 );
    need_update = true;
  }

  if (ui::is_key_pressed(ui::keys::key_d))
  {
    cur_idx = std::min( cur_idx + 1, seq_data.n_images - 1 );
    need_update = true;
  }

  if (ui::is_key_pressed(ui::keys::key_p))
  {
    show_planes = !show_planes;
  }

  if ( 1 )
  {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("TEST", NULL, ImVec2(380, -1), -1.0f,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("%s", color_image_name.c_str() );
    int selected_idx = cur_idx;
    ImGui::SliderInt("Image Idx", &selected_idx, 0, seq_data.n_images - 1 );
    ImGui::PushButtonRepeat(true);
    if ( ImGui::Button("<<", ImVec2(178, 18) ) )
    {
      selected_idx = std::max( 0, cur_idx - 1 );
    }
    ImGui::SameLine();
    if ( ImGui::Button(">>", ImVec2(178, 18) ) )
    {
      selected_idx = std::min( cur_idx + 1, seq_data.n_images - 1 );
    }
    ImGui::PopButtonRepeat();
    ImGui::Checkbox("Use Raw Data", &use_raw );
    ImGui::End();
    
    if ( selected_idx != cur_idx )
    {
      cur_idx = selected_idx;
      need_update = true;
    }
  }

  if (need_update)
  {
    raw_image_name   = "depth_raw/" + seq_data.image_names[cur_idx];
    fixed_image_name = "depth/" + seq_data.image_names[cur_idx];
    color_image_name = "color/" + seq_data.image_names[cur_idx].substr(0, seq_data.image_names[cur_idx].size() - 3) + "jpg";
    raw_image.read(raw_image_name.c_str());
    fixed_image.read(fixed_image_name.c_str());
    color_image.read(color_image_name.c_str());
    update_pointcloud(&raw_image, &color_image,
                      seq_data.depth_intrinsics, 
                      &raw_pointcloud, show_colors, bsc::vec3( 214.0f / 255.0f, 
                                                               39.0f  / 255.0f, 
                                                               40.0f  / 255.0f),
                      opts.bitshift );
    update_pointcloud( &fixed_image, &color_image,
                       seq_data.depth_intrinsics, 
                       &fixed_pointcloud, show_colors, bsc::vec3(  44.0f / 255.0f, 
                                                                  160.0f / 255.0f, 
                                                                   44.0f / 255.0f),
                       opts.bitshift );

    bsc::update_gpu_geo(&raw_pointcloud, &raw_pointcloud_cmd, POSITION | COLOR_A | NORMAL);
    bsc::update_gpu_geo(&fixed_pointcloud, &fixed_pointcloud_cmd, POSITION | COLOR_A | NORMAL);
  }

  glEnable(GL_LINE_SMOOTH);
  
  // Update camera ( view matrix )
  cam_controls.update(&cam, &view, &bsc::g_window.viewport );

  // viewport + projective transformation setup
  glViewport(0, 0, g_window.fb_size.x, g_window.fb_size.y);
  static r32 near = 0.1f;
  static r32 far = 100.0f;
  static r32 fovy = deg2rad(56.25f);
  r32 aspect_ratio = (r32)g_window.size.x / (r32)g_window.size.y;

  mat4f projection = perspective(fovy,
                                 aspect_ratio,
                                 near,
                                 far);

  // Setup some OpenGL options
  glEnable(GL_DEPTH_TEST);

  mat4 vp = projection * view;
  bsc::use_program(debug_prog);
  bsc::set_uniform(debug_prog, "mvp", vp);

  bsc::draw(&axes_cmd, GL_LINES); 

  if ( !seq_data.plane_poses.empty() )
  {
    bsc::mat4 xform = seq_data.plane_poses[cur_idx];
    bsc::set_uniform(debug_prog, "mvp", vp * xform);
    bsc::draw(&axes_cmd, GL_LINES);
    bsc::draw(&grid_cmd, GL_LINES);
  }

  bsc::use_program(pointcloud_prog);
  bsc::set_uniform(pointcloud_prog, "mvp", vp);
  bsc::set_uniform(pointcloud_prog, "show_normals", show_normals);
  if (use_raw)
  {
    bsc::draw(&raw_pointcloud_cmd, GL_POINTS);
  }
  else
  {
    bsc::draw(&fixed_pointcloud_cmd, GL_POINTS);
  }

  if (show_planes)
  {
    bsc::use_program(plane_prog);
    bsc::set_uniform(plane_prog, "mvp", vp);

    bsc::vec4 colors[6] = {bsc::vec4(0.12, 0.46, 0.7, 0.5),
                           bsc::vec4(0.12, 0.46, 0.7, 0.5),
                           bsc::vec4(0.12, 0.46, 0.7, 0.5),
                           bsc::vec4(0.12, 0.46, 0.7, 0.5),
                           bsc::vec4(0.12, 0.46, 0.7, 0.5),
                           bsc::vec4(0.12, 0.46, 0.7, 0.5)};
    for (int i = 6; i >= 0; --i)
    {
      bsc::set_uniform(plane_prog, "color", colors[i]);
      bsc::draw(&(planes_cmds[i]), GL_TRIANGLE_FAN);
    }
  }

  if ( 1 )
  {
    ImGui::Render();
  }

  bsc::ui::end_frame();
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Argument Parsing
////////////////////////////////////////////////////////////////////////////////
static void
ParseArgs(int argc, char **argv)
{
  // Defaults
  opts.max_depth = 5;
  opts.n_slices = 15;
  opts.bitshift = true;


  bsc::arg_parse args;
  args.name = "calibrate_depth";
  args.description = "Program used to estimate and apply "
                       "undistortion look-up-table.";

  args.add( bsc::argument<std::string>("configuration_filename",
                                       "Name of the configuration file",
                                       &opts.configuration_filename ) );

  args.add( bsc::argument<std::string>( "-e", "--estimate_undistortion",
                                  "Perform undistortion LUT estimation,"
                                  " and save it to a file",
                                  &opts.estimated_lut_filename) );
  args.add( bsc::argument<std::string>( "-a", "--apply_undistortion",
                                  "Apply undistortion LUT to fix depth images",
                                  &opts.applied_lut_filename) );

  args.add( bsc::argument<float>( "-d", "--max_depth",
                                 "Maximum depth distance we are rectifying."
                                 "Effectively maximum depth value stored in"
                                 " undistortion LUT. In meters. (Default: 5m)",
                                  &opts.max_depth ) );

  // NOTE: Expose different resolution?
  args.add( bsc::argument<float>( "-s", "--n_slices",
                                 "Number of slices along z-dimension of LUT."
                                 "Effectively resolution of LUT in z",
                                  &opts.n_slices ) );

  args.add( bsc::argument<bool>( "-b", "--bitshift",
                                 "Decide wheter a SUN3D circular-bitshift"
                                 "should be performed",
                                  &opts.bitshift, 1 ) );

  args.add( bsc::argument<bool>( "-v", "--verbose",
                                 "Output additional information",
                                  &opts.print_verbose, 0 ) );

  args.add( bsc::argument<bool>( "-l", "--slice_visualization",
                                 "Save visualization of slices of LUT as images",
                                  &opts.save_slice_visualization, 0 ) );

  args.parse( argc, argv );
  
}

int main(int argc, char **argv)
{
  ParseArgs( argc, argv );

  ReadConfigurationFile( &seq_data, &opts );

  if ( !opts.estimated_lut_filename.empty() )
  {
    if ( opts.print_verbose ) 
    {
      ReportBinCounts( &seq_data, &opts );
    }
    EstimateDistortion( &seq_data, &opts );
    if ( opts.applied_lut_filename.empty() ) return 1;
  }
 
  if ( !opts.applied_lut_filename.empty() )
  {
    ApplyUndistortion( &seq_data, &opts );
    return 1;
  }
 
  bsc::create_window("Calibration Results", 1024, 768,
                      bsc::vec3( 0.67, 0.67, 0.67), 
                      bsc::vec3( 0.67, 0.97, 0.97));
  
  bsc::set_init_funct(Init);
  bsc::set_display_funct(Display);

  bsc::main_loop();

  return 1;
}