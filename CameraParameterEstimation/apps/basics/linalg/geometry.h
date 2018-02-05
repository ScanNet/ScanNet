#pragma once
////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////

namespace bsc
{
  
  // TODO : How to draw this stuff by the way?
  //        Also shouldn't I have many by default?

  // GEOMETRIC ENTITIES
  struct ray
  {
    vec3 o;
    vec3 v;
  };

  struct plane
  {
    vec3 n; 
    r32 d;
  };

  struct aa_box
  {
    vec3 min_p;
    vec3 max_p;
  };

  // ANGLES
  r32 angle( const vec3 v1, const vec3 v2 ); 

  // INTERSECTIONS
  vec3 intersect( const ray r, const plane p );
  aa_box intersect( const aa_box *ba, const aa_box *bb );

  // UNIONS
  void merge( aa_box * box, vec3 p );
  
  // DISTANCES
  r32 distance( vec2 a, vec2 b );
  r32 distance( vec3 a, vec3 b );
  r32 distance( vec4 a, vec4 b );
  
  // VOLUMES
  r32 volume( const aa_box *b );

}
////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////
#ifdef BSC_IMPLEMENTATION

namespace bsc
{
  // ANGLES
  r32
  angle( const vec3 v1, const vec3 v2 )
  {
    r32 dot_prod = dot(v1, v2);
    r32 len_1 = norm(v1);
    r32 len_2 = norm( v2 );
    return acos( dot_prod / len_1 * len_2 );
  }

  // INTERSECTIONS
  vec3
  intersect( ray r, plane p )
  {
    r32 t = -( dot( r.o, p.n ) + p.d ) / dot( r.v, p.n );
    return r.o + t * r.v;
  }

  // UNIONS
  void
  merge( aa_box * box, vec3 p )
  {
    // TODO: Implement min on vectors
    box->min_p = vec3( min( box->min_p.x, p.x ),
                       min( box->min_p.y, p.y ),
                       min( box->min_p.z, p.z ) );
    box->max_p = vec3( max( box->max_p.x, p.x ),
                       max( box->max_p.y, p.y ),
                       max( box->max_p.z, p.z ) );
  }

  //DISTANCES
  r32
  distance( vec3 a, vec3 b )
  {
    return( norm( a-b ) );
  }

  r32
  distance( vec4 a, vec4 b )
  {
    return( norm( a-b ) );
  }
}


#endif //BSC_IMPLEMENTATION