#pragma once

namespace bsc
{
/*
  gpu_asset create_plane( const u32 div_u = 1, const u32 div_v = 1 );
  gpu_asset create_cube();
  gpu_asset create_sphere( const u32 div_u = 32, const u32 div_v = 32 );
*/

  void create_extrusion( gpu_geometry * geo,
                         const vec3 * line_points, const i32 n_line_points, 
                         const vec2 * shape_points, const i32 n_shape_points,
                         const r32 size, const vec3i * color_map );

};

#ifdef BSC_IMPLEMENTATION

namespace bsc
{

  void
  create_extrusion( gpu_geometry * geo,
                    const vec3 * line_points, const i32 n_line_points, 
                    const vec2 * shape_points, const i32 n_shape_points,
                    const r32 size, const vec3i * color_map )
  {
    vec3 *tmp_pos = (vec3 *)malloc(n_shape_points * n_line_points * sizeof(vec3));
    vec4 *tmp_col = (vec4 *)malloc(n_line_points * sizeof(vec4));

    for (i32 i = 0; i < n_line_points; i++)
    {
      // calculate current and next color
      r32 cur_offset = 8.0f * (r32)i / (r32)n_line_points;
      i32 col_idx = floor(cur_offset);
      r32 b = cur_offset - col_idx;
      r32 a = 1.0 - b;

      vec3 color_a( color_map[col_idx].r,
                    color_map[col_idx].g,
                    color_map[col_idx].b);
      vec3 color_b( color_map[col_idx + 1].r,
                    color_map[col_idx + 1].g,
                    color_map[col_idx + 1].b);

      // figure out the base
      vec3 point_a = line_points[i];
      vec3 point_b, point_c, n1, n2;
      if (i == n_line_points - 1)
      {
        point_c = line_points[i-1];
        n2 = point_a - point_c;
        point_b = point_a - n2;
        n1 = point_b - point_a;
      }
      if (i == 0)
      {
        point_b = line_points[i+1];
        n1 = point_b - point_a;
        point_c = point_a - n1;
        n2 = point_a - point_c;
      }
      else
      {
        point_b = vec3(line_points[i+1]);
        point_c = vec3(line_points[i-1]);
        n1 = point_b - point_a;
        n2 = point_a - point_c;
      }

      vec3 up(0, 1, 0);
      vec3 u1 = normalize(cross(up, n1));
      vec3 u2 = normalize(cross(up, n2));
      vec3 u = normalize(u1 + u2);
      vec3 v = normalize(cross(u, n1));

      // with the base we can push any shape really.
      for ( i32 j = 0 ; j < n_shape_points ; ++j )
      {
        r32 u_mul = shape_points[j].x * size;
        r32 v_mul = shape_points[j].y * size;
        tmp_pos[ n_shape_points * i + j ] = point_a + u_mul * u + v_mul * v; 
      }
      // tmp_pos[4 * i + 0] = point_a + size * v;
      // tmp_pos[4 * i + 1] = point_a + size * u;
      // tmp_pos[4 * i + 2] = point_a - size * v;
      // tmp_pos[4 * i + 3] = point_a - size * u;

      tmp_col[i] = vec4(a * color_a + b * color_b, 255.0f);
    }

    const int n_tri_per_seg = n_shape_points * 2;
    r32 positions[n_tri_per_seg * 3 * 3 * (n_line_points - 1)];
    u8 colors[n_tri_per_seg * 3 * 4 * (n_line_points - 1)];

    i32 pos_idx = 0;
    i32 col_idx = 0;

    // TODO: Use DrawElements?!
    for (int i = 0; i < n_line_points - 1; ++i)
    {
      i32 ba = n_shape_points * i;
      i32 bb = n_shape_points * (i + 1);

      for (int j = 0; j < n_shape_points; ++j)
      {
        vec3 cur_pos[6];
        vec4 cur_col[6];

        cur_pos[0] = tmp_pos[ba + j];
        cur_pos[1] = tmp_pos[ba + (j + 1) % n_shape_points];
        cur_pos[2] = tmp_pos[bb + j];

        cur_pos[3] = tmp_pos[ba + (j + 1) % n_shape_points];
        cur_pos[4] = tmp_pos[bb + j];
        cur_pos[5] = tmp_pos[bb + (j + 1) % n_shape_points];

        cur_col[0] = tmp_col[i];
        cur_col[1] = tmp_col[i];
        cur_col[2] = tmp_col[i + 1];

        cur_col[3] = tmp_col[i];
        cur_col[4] = tmp_col[i + 1];
        cur_col[5] = tmp_col[i + 1];

        r32 *pos_cpy_loc = (r32 *)&(cur_pos[0]);
        r32 *col_cpy_loc = (r32 *)&(cur_col[0]);

        for (i32 k = 0; k < 3 * 6; ++k)
        {
          positions[pos_idx++] = pos_cpy_loc[k];
        }

        for (i32 k = 0; k < 4 * 6; ++k)
        {
          colors[col_idx++] = (u8)col_cpy_loc[k];
        }
      }
    }

    bsc::geometry_data extrusion;
    extrusion.positions = positions;
    extrusion.colors_a = colors;
    extrusion.n_vertices = n_tri_per_seg * 3 * (n_line_points - 1);

    bsc::init_gpu_geo(&extrusion, geo, POSITION | COLOR_A );
    free(tmp_pos);
    free(tmp_col);
  }


/*
  gpu_asset
  create_plane( const u32 div_u, const u32 div_v )
  {
    // storage
    const u32 vert_count = (div_u + 1) * (div_v + 1);
    const u32 tri_count  = 2 * div_u * div_v;
    r32 positions[ 3 * vert_count ];
    r32 texcoords[ 2 * vert_count ];
    r32 normals[ 3 * vert_count ];
    r32 tangents[ 3 * vert_count ];
    u32 indices[ 2 * tri_count ];

    // generate vertex data
    for ( u32 v = 0 ; v < div_v + 1 ; ++v )
    {
      u32 base = v * (div_u + 1);
      for ( u32 u = 0 ; u < div_u + 1 ; ++u )
      {
        u32 idx = base + u;
        texcoords[ 2 * idx + 0 ] = (r32)u / div_u;
        texcoords[ 2 * idx + 1 ] = (r32)v / div_v;

        positions[ 3 * idx + 0 ] = -0.5f + texcoords[ 2 * idx + 0 ];
        positions[ 3 * idx + 1 ] = -0.5f + texcoords[ 2 * idx + 1 ];
        positions[ 3 * idx + 2 ] = 0.0f;
        
        normals[ 3 * idx + 0 ] = 0.0f;
        normals[ 3 * idx + 1 ] = 0.0f;
        normals[ 3 * idx + 2 ] = 1.0f;

        tangents[ 3 * idx + 0 ] = 0.0f;
        tangents[ 3 * idx + 1 ] = 1.0f;
        tangents[ 3 * idx + 2 ] = 0.0f;
      }
    }

    // generate face data
    int idx = 0;
    for ( u32 v = 0 ; v < div_v ; ++v )
    {
      u32 row_1 = v * ( div_u + 1 );
      u32 row_2 = ( v + 1 ) * ( div_u + 1 );
      for ( u32 u = 0 ; u < div_u ; ++u )
      {
        u32 a = row_1 + u;
        u32 b = row_1 + u + 1;
        u32 c = row_2 + u;
        u32 d = row_2 + u + 1; 

        indices[ idx++ ] =  a;
        indices[ idx++ ] =  b;
        indices[ idx++ ] =  c;
        indices[ idx++ ] =  c;
        indices[ idx++ ] =  b;
        indices[ idx++ ] =  d;
      }
    }

    // store in a helper structure
    mesh_data host_plane;
    host_plane.positions = &(positions[ 0 ]);
    host_plane.texcoords = &(texcoords[ 0 ]);
    host_plane.normals   = &(normals[ 0 ]);
    host_plane.tangents  = &(tangents[ 0 ]);
    host_plane.indices   = &(indices[ 0 ]);

    // upload to gpu
    gpu_asset device_plane;
    device_plane.vert_count = vert_count;
    device_plane.tri_count  = tri_count;
    init_gpu_mem( &host_plane, &device_plane  );

    return device_plane;
  }

  gpu_asset
  create_sphere( const u32 div_u, const u32 div_v )
  {
    // storage
    const u32 vert_count = (div_u + 1) * (div_v + 1);
    const u32 tri_count  = 2 * div_u * div_v;
    r32 positions[ 3 * vert_count ];
    r32 texcoords[ 2 * vert_count ];
    r32 normals[ 3 * vert_count ];
    r32 tangents[ 3 * vert_count ];
    u32 indices[ 2 * tri_count ];

    // generate vertex data
    for ( u32 v = 0 ; v < div_v + 1 ; ++v )
    {
      u32 base = v * (div_u + 1);
      for ( u32 u = 0 ; u < div_u + 1 ; ++u )
      {
        u32 idx = base + u;
        texcoords[ 2 * idx + 0 ] = (r32)u / div_u;
        texcoords[ 2 * idx + 1 ] = (r32)v / div_v;
        float phi   = texcoords[ 2 * idx + 0 ] * M_PI;
        float theta = texcoords[ 2 * idx + 1 ] * 2 * M_PI;

        positions[ 3 * idx + 0 ] = cosf(theta) * sinf(phi);
        positions[ 3 * idx + 1 ] = sinf(theta) * sinf(phi);
        positions[ 3 * idx + 2 ] = cosf(phi);

        tangents[ 3 * idx + 0 ] = cosf(theta + M_PI/2.0f) * sinf(phi);
        tangents[ 3 * idx + 1 ] = sinf(theta + M_PI/2.0f) * sinf(phi);
        tangents[ 3 * idx + 2 ] = cosf(phi);
        
        normals[ 3 * idx + 0 ] = positions[ 3 * idx + 0 ];
        normals[ 3 * idx + 1 ] = positions[ 3 * idx + 1 ];
        normals[ 3 * idx + 2 ] = positions[ 3 * idx + 2 ]; 
      }
    }

    // generate face data
    int idx = 0;
    for ( u32 v = 0 ; v < div_v ; ++v )
    {
      u32 row_1 = v * ( div_u + 1 );
      u32 row_2 = ( v + 1 ) * ( div_u + 1 );
      for ( u32 u = 0 ; u < div_u ; ++u )
      {
        u32 a = row_1 + u;
        u32 b = row_1 + u + 1;
        u32 c = row_2 + u;
        u32 d = row_2 + u + 1; 

        indices[ idx++ ] =  a;
        indices[ idx++ ] =  b;
        indices[ idx++ ] =  c;
        indices[ idx++ ] =  b;
        indices[ idx++ ] =  d;
        indices[ idx++ ] =  c;
      }
    }

    // store in a helper structure
    mesh_data host_sphere;
    host_sphere.positions = &(positions[ 0 ]);
    host_sphere.texcoords = &(texcoords[ 0 ]);
    host_sphere.normals   = &(normals[ 0 ]);
    host_sphere.tangents  = &(tangents[ 0 ]);
    host_sphere.indices   = &(indices[ 0 ]);

    // upload to gpu
    gpu_asset device_sphere;
    device_sphere.vert_count = vert_count;
    device_sphere.tri_count  = tri_count;
    init_gpu_mem( &host_sphere, &device_sphere  );

    return device_sphere;
  }


// Possibly it would be better to simply create a function
// that creates simple sphere with shared normals, and then add
// subdivision function, and unwelding function. 
// For now this works
  gpu_asset
  create_cube()
  {
    // // storage
    const u32 vert_count = 24;
    const u32 tri_count  = 12;
    r32 positions[ 3 * vert_count ] =
    {
      // FRONT
      -0.5,  0.5, -0.5, //  0
       0.5,  0.5, -0.5, //  1
      -0.5,  0.5,  0.5, //  2
       0.5,  0.5,  0.5, //  3
      // BACK
      -0.5, -0.5, -0.5, //  4
       0.5, -0.5, -0.5, //  5
      -0.5, -0.5,  0.5, //  6
       0.5, -0.5,  0.5, //  7
      // LEFT
       0.5,  0.5, -0.5, //  8
       0.5, -0.5, -0.5, //  9
       0.5,  0.5,  0.5, // 10
       0.5, -0.5,  0.5, // 11
      // RIGHT
      -0.5,  0.5, -0.5, // 12
      -0.5, -0.5, -0.5, // 13
      -0.5,  0.5,  0.5, // 14
      -0.5, -0.5,  0.5, // 15
      // TOP
      -0.5,  0.5,  0.5, // 16
       0.5,  0.5,  0.5, // 17
       0.5, -0.5,  0.5, // 18
      -0.5, -0.5,  0.5, // 19
      // BOTTOM
      -0.5,  0.5, -0.5, // 20
       0.5,  0.5, -0.5, // 21
       0.5, -0.5, -0.5, // 22
      -0.5, -0.5, -0.5, // 23
    };
    r32 texcoords[ 2 * vert_count ]; // TODO 
    r32 normals[ 3 * vert_count ] = 
        {
      // FRONT
       0.0,  1.0,  0.0, //  0
       0.0,  1.0,  0.0, //  1
       0.0,  1.0,  0.0, //  2
       0.0,  1.0,  0.0, //  3
      // BACK
       0.0, -1.0,  0.0, //  4
       0.0, -1.0,  0.0, //  5
       0.0, -1.0,  0.0, //  6
       0.0, -1.0,  0.0, //  7
      // LEFT
       1.0,  0.0,  0.0, //  8
       1.0,  0.0,  0.0, //  9
       1.0,  0.0,  0.0, // 10
       1.0,  0.0,  0.0, // 11
      // RIGHT
      -1.0,  0.0,  0.0, // 12
      -1.0,  0.0,  0.0, // 13
      -1.0,  0.0,  0.0, // 14
      -1.0,  0.0,  0.0, // 15
      // TOP
       0.0,  0.0,  1.0, // 16
       0.0,  0.0,  1.0, // 17
       0.0,  0.0,  1.0, // 18
       0.0,  0.0,  1.0, // 19
      // BOTTOM
       0.0,  0.0, -1.0, // 20
       0.0,  0.0, -1.0, // 21
       0.0,  0.0, -1.0, // 22
       0.0,  0.0, -1.0, // 23
    };
    r32 tangents[ 3 * vert_count ];  // TODO
    u32 indices[ 3 * tri_count ] = 
    {
       0,  3,  1,  0,  2,  3, // front
       5,  6,  4,  5,  7,  6, // back 
       8, 11,  9,  8, 10, 11, // left 
      12, 13, 14, 13, 15, 14, // right 
      16, 18, 17, 16, 19, 18, // top
      20, 21, 22, 20, 22, 23 // bottom
    };

    // store in a helper structure
    mesh_data host_cube;
    host_cube.positions = &(positions[ 0 ]);
    host_cube.texcoords = &(texcoords[ 0 ]);
    host_cube.normals   = &(normals[ 0 ]);
    host_cube.tangents  = &(tangents[ 0 ]);
    host_cube.indices   = &(indices[ 0 ]);

    // // upload to gpu
    gpu_asset device_cube;
    device_cube.vert_count = vert_count;
    device_cube.tri_count  = tri_count;
    init_gpu_mem( &host_cube, &device_cube  );

    return device_cube;
  }
*/
}

#endif