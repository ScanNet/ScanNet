#pragma once

namespace bsc
{
    template<typename T>
    inline void
    print_mat4( const T& mat, FILE * stream = stdout )
    {
        fprintf ( stream, "%10.5f %10.5f %10.5f %10.5f\n", mat[0][0], mat[1][0], mat[2][0], mat[3][0] );
        fprintf ( stream, "%10.5f %10.5f %10.5f %10.5f\n", mat[0][1], mat[1][1], mat[2][1], mat[3][1] );
        fprintf ( stream, "%10.5f %10.5f %10.5f %10.5f\n", mat[0][2], mat[1][2], mat[2][2], mat[3][2] );
        fprintf ( stream, "%10.5f %10.5f %10.5f %10.5f\n", mat[0][3], mat[1][3], mat[2][3], mat[3][3] );
        fprintf ( stream, "\n" );
    }

    template<typename T>
    inline void
    print_mat3( const T& mat, FILE * stream = stdout )
    {
        fprintf ( stream, "%10.5f %10.5f %10.5f\n", mat[0][0], mat[1][0], mat[2][0] );
        fprintf ( stream, "%10.5f %10.5f %10.5f\n", mat[0][1], mat[1][1], mat[2][1] );
        fprintf ( stream, "%10.5f %10.5f %10.5f\n", mat[0][2], mat[1][2], mat[2][2] );
        fprintf ( stream, "\n" );
    }

    template<typename T>
    inline void
    print_mat2( const T& mat, FILE * stream = stdout )
    {
        fprintf ( stream, "%10.5f %10.5f\n", mat[0][0], mat[1][0] );
        fprintf ( stream, "%10.5f %10.5f\n", mat[0][1], mat[1][1] );
        fprintf ( stream, "\n" );
    }

    template<typename T>
    inline void
    print_vec4( const T& vec, FILE * stream = stdout )
    {
        fprintf ( stream, "%10.5f %10.5f %10.5f %10.5f\n", vec.x, vec.y, vec.z, vec.w );
    }

    template<typename T>
    inline void
    print_vec3( const T& vec, FILE * stream = stdout )
    {
        fprintf ( stream, "%10.5f %10.5f %10.5f\n", vec.x, vec.y, vec.z );
    }

    template<typename T>
    inline void
    print_vec2( const T& vec, FILE * stream = stdout )
    {
        fprintf ( stream, "%10.5f %10.5f\n", vec.x, vec.y );
    }
}
