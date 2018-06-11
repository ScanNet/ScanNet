#pragma once

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////

namespace bsc
{
  template <typename T> struct vector2;
  template <typename T> struct vector3;
  template <typename T> struct vector4;

  // VECTOR 2
  template <typename T>
  struct vector2
  {
    union
    {
      struct { T x, y; };
      struct { T r, g; };
      struct { T p, q; };
      T data[2];
    };

    vector2 ( void );
    vector2 ( T v );
    vector2 ( T x, T y );
    vector2 ( const vector2<T> &other );
    vector2 ( const vector3<T> &other );
    vector2 ( const vector4<T> &other );

    vector2<T> & operator= ( const vector2<T> & other );

    void operator+= ( const vector2<T> & other );
    void operator+= ( const T constant );

    void operator-= ( const vector2<T> & other );
    void operator-= ( const T constant );

    void operator*= ( const vector2<T> & other );
    void operator*= ( const T constant );

    void operator/= ( const vector2<T> & other );
    void operator/= ( const T constant );

    bool operator== ( const vector2<T> & other ) const;
    bool operator!= ( const vector2<T> & other ) const;

    T operator[] ( u32 i ) const;
    T& operator[] ( u32 i );

    const constexpr int size() const  { return 2; }
  };

  // VECTOR 3
  template <typename T>
  struct vector3
  {
    union
    {
      struct { T x, y, z; };
      struct { T r, g, b; };
      struct { T p, q, s; };
      T data[3];
    };

    vector3<T> ( void );
    vector3<T> ( T v );
    vector3<T> ( T x, T y, T z );
    vector3<T> ( const vector3<T> & other );
    vector3<T> ( const T v, const vector2<T> & other );
    vector3<T> ( const vector2<T> & other, const T v );
    vector3<T> ( const vector4<T> & other );

    vector3<T> & operator= ( const vector3<T> & other );

    void operator+= ( const vector3<T> & other );
    void operator+= ( const T constant );

    void operator-= ( const vector3<T> & other );
    void operator-= ( const T constant );

    void operator*= ( const vector3<T> & other );
    void operator*= ( const T constant );

    void operator/= ( const vector3<T> & other );
    void operator/= ( const T constant );

    bool operator== ( const vector3<T> & other ) const;
    bool operator!= ( const vector3<T> & other ) const;

    T operator[] ( u32 i ) const;
    T& operator[] ( u32 i );

    const constexpr int size() const  { return 3; }
  };

  // VECTOR 4
  template <typename T>
  struct vector4
  {
    union
    {
      struct { T x, y, z, w; };
      struct { T r, g, b, a; };
      struct { T p, q, s, t; };
      T data[4];
    };

    vector4 ( void );
    vector4 ( T v );
    vector4 ( T x, T y, T z, T w );
    vector4 ( const vector4<T> &other );
    vector4 ( const vector2<T> &other, const T v );
    vector4 ( const vector2<T> &other, const T v1, const T v2 );
    vector4 ( const vector3<T> &other, const T v );
    vector4 ( const T v, const vector3<T> &other );

    vector4<T> & operator= ( const vector4<T> & v );

    void operator+= ( const vector4<T> & other );
    void operator+= ( const T constant );

    void operator-= ( const vector4<T> & other );
    void operator-= ( const T constant );

    void operator*= ( const vector4<T> & other );
    void operator*= ( const T constant );

    void operator/= ( const vector4<T> & other );
    void operator/= ( const T constant );

    bool operator== ( const vector4<T> & other ) const;
    bool operator!= ( const vector4<T> & other ) const;

    T operator[] ( u32 i ) const;
    T& operator[] ( u32 i );

    const constexpr int size() const { return 4; }
  };

  template< typename T, template <typename Q> class vecX >
  T dot ( const vecX<T> & a, const vecX<T> & b );

  template< typename T, template <typename Q> class vecX >
  T norm_sq ( const vecX<T> & a );

  template< typename T, template <typename Q> class vecX >
  T norm ( const vecX<T> & a );

  template< typename T, template <typename Q> class vecX >
  T distance ( const vecX<T> & a, const vecX<T> & b );

  template< typename T, template <typename Q> class vecX >
  vecX<T> normalize ( const vecX<T> & a );

  template< typename T, template <typename Q> class vecX >
  vecX<T> abs ( const vecX<T> & a );

  template< typename T, template <typename Q> class vecX >
  vecX<T> clamp ( const vecX<T> & a, const T val_min = 0.0, const T val_max = 1.0 );

  template< typename T >
  vector3<T> cross ( const vector3<T> & a, const vector3<T> & b );

  template< typename T >
  T parallelogramArea ( const vector3<T> & a, const vector3<T> & b );

  template< typename T >
  T parallelogramArea ( const vector2<T> & a, const vector2<T> & b );

  template< typename T>
  T scalarTripleProduct( const vector3<T> & a, const vector3<T> & b, const vector3<T> & c );

  template< typename T>
  T parallelepipedVolume( const vector3<T> & a, const vector3<T> & b, const vector3<T> & c );

  template< typename T >
  int ccw ( const vector2<T> & a, const vector2<T> & b );

  typedef vector2<r32> vec2; // the default
  typedef vector3<r32> vec3; // the default
  typedef vector4<r32> vec4; // the default

  typedef vector2<r64> vec2d;
  typedef vector2<r32> vec2f;
  typedef vector2<i32> vec2i;

  typedef vector3<r64> vec3d;
  typedef vector3<r32> vec3f;
  typedef vector3<i32> vec3i;

  typedef vector4<r64> vec4d;
  typedef vector4<r32> vec4f;
  typedef vector4<i32> vec4i;
}


////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

// VECTOR 2

namespace bsc
{
  template <typename T>
  vector2<T> ::
  vector2 ( void ) : x( (T) 0 ), y( (T) 0 )
  {}


  template <typename T>
  vector2<T> ::
  vector2 ( T v ) : x( v ), y( v )
  {}


  template <typename T>
  vector2<T> ::
  vector2 ( T x, T y ) : x( x ), y( y )
  {}


  template <typename T>
  vector2<T> ::
  vector2 ( const vector2<T> &other ) : x( other.x ), y( other.y )
  {}


  template <typename T>
  vector2<T> ::
  vector2 ( const vector3<T> &other ) : x( other.x ), y( other.y )
  {}


  template <typename T>
  vector2<T> ::
  vector2 ( const vector4<T> &other ) : x( other.x ), y( other.y )
  {}

  template <typename T>
  inline vector2<T> & vector2<T> ::
  operator= ( const vector2<T> & v)
  {
    this->x = v.x;
    this->y = v.y;
    return *this;
  }

  template <typename T>
  inline void vector2<T> ::
  operator+= ( const vector2<T> & other )
  {
    this->x += other.x;
    this->y += other.y;
  }


  template <typename T>
  inline void vector2<T> ::
  operator+= ( const T constant )
  {
    this->x += constant;
    this->y += constant;
  }


  template <typename T>
  inline vector2<T>
  operator+ ( const vector2<T> & v1, const vector2<T> & v2 )
  {
    vector2<T> result( v1.x + v2.x,
                  v1.y + v2.y );
    return result;
  }

  template <typename T>
  inline vector2<T>
  operator+ ( const vector2<T> & v, const T constant )
  {
    vector2<T> result( v.x + constant,
                    v.y + constant );
    return result;
  }

  template <typename T>
  inline vector2<T>
  operator+ ( const T constant, const vector2<T> & v )
  {
    vector2<T> result( v.x + constant,
                    v.y + constant );
    return result;
  }


  template <typename T>
  inline void vector2<T> ::
  operator-= ( const vector2<T> & other )
  {
    this->x -= other.x;
    this->y -= other.y;
  }

  template <typename T>
  inline void vector2<T> ::
  operator-= ( const T constant )
  {
    this->x -= constant;
    this->y -= constant;
  }


  template <typename T>
  inline vector2<T>
  operator- ( const vector2<T> & v1, const vector2<T> & v2 )
  {
    vector2<T> result( v1.x - v2.x,
                    v1.y - v2.y );
    return result;
  }

  template <typename T>
  inline vector2<T>
  operator- ( const vector2<T> & v, const T constant )
  {
    vector2<T> result( v.x - constant,
                    v.y - constant );
    return result;
  }

  template <typename T>
  inline vector2<T>
  operator- ( const T constant, const vector2<T> & v )
  {
    vector2<T> result( constant - v.x,
                    constant - v.y );
    return result;
  }


  template <typename T>
  inline vector2<T>
  operator- ( const vector2<T> & v )
  {
    vector2<T> result( -v.x,
                    -v.y );
    return result;
  }


  template <typename T>
  inline void vector2<T> ::
  operator*= ( const vector2<T> & other )
  {
    this->x *= other.x;
    this->y *= other.y;
  }

  template <typename T>
  inline void vector2<T> ::
  operator*= ( const T constant )
  {
    this->x *= constant;
    this->y *= constant;
  }


  template <typename T>
  inline vector2<T>
  operator* ( const vector2<T> & v1, const vector2<T> & v2 )
  {
    vector2<T> result( v1.x * v2.x,
                    v1.y * v2.y );
    return result;
  }

  template <typename T>
  inline vector2<T>
  operator* ( const vector2<T> & v, const T constant )
  {
    vector2<T> result( v.x * constant,
                    v.y * constant );
    return result;
  }

  template <typename T>
  inline vector2<T>
  operator* ( const T constant, const vector2<T> & v )
  {
    vector2<T> result( v.x * constant,
                    v.y * constant );
    return result;
  }

  template <typename T>
  inline void vector2<T> ::
  operator/= ( const vector2<T> & other )
  {
    this->x /= other.x;
    this->y /= other.y;
  }

  template <typename T>
  inline void vector2<T> ::
  operator/= ( const T constant )
  {
    T inv_constant = 1.0f / constant;
    this->x *= inv_constant;
    this->y *= inv_constant;
  }

  template <typename T>
  inline vector2<T>
  operator/ ( const vector2<T> & v1, const vector2<T> & v2 )
  {
    vector2<T> result( v1.x / v2.x,
                    v1.y / v2.y );
    return result;
  }

  template <typename T>
  inline vector2<T>
  operator/ ( const vector2<T> & v, const T constant )
  {
    vector2<T> result( v.x / constant,
                    v.y / constant );
    return result;
  }

  template <typename T>
  inline vector2<T>
  operator/ ( const T constant, const vector2<T> & v )
  {
    vector2<T> result( constant / v.x,
                    constant / v.y );
    return result;
  }


  template <typename T>
  inline T vector2<T> ::
  operator[] ( u32 i ) const
  {
    return this->data[ i ];
  }


  template <typename T>
  inline T& vector2<T> ::
  operator[] ( u32 i )
  {
      return this->data[ i ];
  }


  template <typename T>
  inline bool vector2<T> ::
  operator== ( const vector2<T>& other ) const
  {
    if ( this->x == other.x &&
      this->y == other.y )
    {
      return true;
    }
    else
    {
      return false;
    }
  }


  template <typename T>
  inline bool vector2<T> ::
  operator!= (const vector2<T>& other) const
  {
    return !(*this == other);
  }


  // VECTOR 3

  template <typename T>
  vector3<T> ::
  vector3 ( void ) : x( (T)0 ), y( (T)0 ), z( (T)0 )
  {}

  template <typename T>
  vector3<T> ::
  vector3 ( T v ) : x( v ), y( v ), z( v )
  {}

  template <typename T>
  vector3<T> ::
  vector3( T x, T y, T z ) : x( x ), y( y ), z( z )
  {
  }

  template <typename T>
  vector3<T> ::
  vector3 ( const vector3<T> &other ) : x( other.x ), y( other.y ), z( other.z )
  {
  }

  template <typename T>
  vector3<T> ::
  vector3 ( const T v, const vector2<T> &other ) : x( v ), y( other.x ), z( other.y )
  {}

  template <typename T>
  vector3<T> ::
  vector3 ( const vector2<T> &other, const T v ) : x( other.x ), y( other.y ), z( v )
  {}

  template <typename T>
  vector3<T> ::
  vector3 ( const vector4<T> &other ) : x( other.x ), y( other.y ), z( other.z )
  {}

  template <typename T>
  inline vector3<T> & vector3<T> ::
  operator=( const vector3<T> & other )
  {
    this->x = other.x;
    this->y = other.y;
    this->z = other.z;
    return *this;
  }

  template <typename T>
  inline void vector3<T> ::
  operator+= ( const vector3<T> & other )
  {
    this->x += other.x;
    this->y += other.y;
    this->z += other.z;
  }


  template <typename T>
  inline void vector3<T> ::
  operator+= ( const T constant )
  {
    this->x += constant;
    this->y += constant;
    this->z += constant;
  }


  template <typename T>
  inline vector3<T>
  operator+ ( const vector3<T> & v1, const vector3<T> & v2 )
  {
    vector3<T> result( v1.x + v2.x,
            v1.y + v2.y,
            v1.z + v2.z );
    return result;
  }

  template <typename T>
  inline vector3<T>
  operator+ ( const vector3<T> & v, const T constant )
  {
    vector3<T> result( v.x + constant,
            v.y + constant,
            v.z + constant );
    return result;
  }

  template <typename T>
  inline vector3<T>
  operator+ ( const T constant, const vector3<T> & v )
  {
    vector3<T> result( v.x + constant,
            v.y + constant,
            v.z + constant );
    return result;
  }


  template <typename T>
  inline void vector3<T> ::
  operator-= ( const vector3<T> & other )
  {
    this->x -= other.x;
    this->y -= other.y;
    this->z -= other.z;
  }

  template <typename T>
  inline void vector3<T> ::
  operator-= ( const T constant )
  {
    this->x -= constant;
    this->y -= constant;
    this->z -= constant;
  }


  template <typename T>
  inline vector3<T>
  operator- ( const vector3<T> & v1, const vector3<T> & v2 )
  {
    vector3<T> result( v1.x - v2.x,
            v1.y - v2.y,
            v1.z - v2.z );
    return result;
  }

  template <typename T>
  inline vector3<T>
  operator- ( const vector3<T> & v, const T constant )
  {
    vector3<T> result( v.x - constant,
            v.y - constant,
            v.z - constant );
    return result;
  }

  template <typename T>
  inline vector3<T>
  operator- ( const T constant, const vector3<T> & v )
  {
    vector3<T> result( constant - v.x,
            constant - v.y,
            constant - v.z );
    return result;
  }


  template <typename T>
  inline vector3<T>
  operator- ( const vector3<T> & v )
  {
    vector3<T> result( -v.x,
            -v.y,
            -v.z );
    return result;
  }


  template <typename T>
  inline void vector3<T> ::
  operator*= ( const vector3<T> & other )
  {
    this->x *= other.x;
    this->y *= other.y;
    this->z *= other.z;
  }

  template <typename T>
  inline void vector3<T> ::
  operator*= ( const T constant )
  {
    this->x *= constant;
    this->y *= constant;
    this->z *= constant;
  }


  template <typename T>
  inline vector3<T>
  operator* ( const vector3<T> & v1, const vector3<T> & v2 )
  {
    vector3<T> result( v1.x * v2.x,
            v1.y * v2.y,
            v1.z * v2.z );
    return result;
  }

  template <typename T>
  inline vector3<T>
  operator* ( const vector3<T> & v, const T constant )
  {
    vector3<T> result( v.x * constant,
            v.y * constant,
            v.z * constant );
    return result;
  }

  template <typename T>
  inline vector3<T>
  operator* ( const T constant, const vector3<T> & v )
  {
    vector3<T> result( v.x * constant,
            v.y * constant,
            v.z * constant );
    return result;
  }

  template <typename T>
  inline void vector3<T> ::
  operator/= ( const vector3<T> & other )
  {
    this->x /= other.x;
    this->y /= other.y;
    this->z /= other.z;
  }

  template <typename T>
  inline void vector3<T> ::
  operator/= ( const T constant )
  {
    T inv_constant = 1.0f / constant;
    this->x *= inv_constant;
    this->y *= inv_constant;
    this->z *= inv_constant;
  }

  template <typename T>
  inline vector3<T>
  operator/ ( const vector3<T> & v1, const vector3<T> & v2 )
  {
    vector3<T> result( v1.x / v2.x,
            v1.y / v2.y,
            v1.z / v2.z );
    return result;
  }

  template <typename T>
  inline vector3<T>
  operator/ ( const vector3<T> & v, const T constant )
  {
    vector3<T> result( v.x / constant,
            v.y / constant,
            v.z / constant );
    return result;
  }

  template <typename T>
  inline vector3<T>
  operator/ ( const T constant, const vector3<T> & v )
  {
    vector3<T> result( constant / v.x,
                    constant / v.y,
                    constant / v.z );
    return result;
  }

  template <typename T>
  inline T vector3<T> ::
  operator[] ( u32 i ) const
  {
    return this->data[ i ];
  }


  template <typename T>
  inline T& vector3<T> ::
  operator[] ( u32 i )
  {
    return this->data[ i ];
  }


  template <typename T>
  inline bool vector3<T> ::
  operator== ( const vector3<T>& other ) const
  {
    if ( this->x == other.x &&
         this->y == other.y &&
         this->z == other.z )
    {
      return true;
    }
    else
    {
      return false;
    }
  }


  template <typename T>
  inline bool vector3<T> ::
  operator!= ( const vector3<T>& other ) const
  {
    return !(*this == other);
  }


  // VECTOR 4
  template <typename T>
  vector4<T> ::
  vector4 ( void ) : x( (T)0 ), y( (T)0 ), z( (T)0 ), w( (T)0 )
  {}

  template <typename T>
  vector4<T> ::
  vector4 ( T v ) : x( v ), y( v ), z( v ), w( v )
  {}

  template <typename T>
  vector4<T> ::
  vector4 ( T x, T y, T z, T w ) : x( x ), y( y ), z( z ), w( w )
  {}

  template <typename T>
  vector4<T> ::
  vector4 ( const vector4<T> &other ) : x( other.x ), y( other.y ), z( other.z ), w( other.w )
  {}

  template <typename T>
  vector4<T> ::
  vector4( const vector2<T> &other, const T v ) : x( other.x ), y( other.y ), z( v ), w( v )
  {}

  template <typename T>
  vector4<T> ::
  vector4 ( const vector2<T> &other, const T v1, const T v2 ) : x( other.x ), y( other.y ), z( v1 ), w( v2 )
  {}

  template <typename T>
  vector4<T> ::
  vector4 ( const vector3<T> &other, const T v ) : x( other.x ),y( other.y ), z( other.z ), w( v ) {}

  template <typename T>
  vector4<T> ::
  vector4 ( const T v, const vector3<T> &other ) : x( v ), y( other.x ), z( other.y ), w( other.z )
  {}

  template <typename T>
  inline vector4<T> & vector4<T> ::
  operator= ( const vector4<T> & v)
  {
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    this->w = v.w;
    return *this;
  }

  template <typename T>
  inline void vector4<T> ::
  operator+= ( const vector4<T> & other )
  {
    this->x += other.x;
    this->y += other.y;
    this->z += other.z;
    this->w += other.w;
  }

  template <typename T>
  inline void vector4<T> ::
  operator+= ( const T constant )
  {
    this->x += constant;
    this->y += constant;
    this->z += constant;
    this->w += constant;
  }

  template <typename T>
  inline vector4<T>
  operator+ ( const vector4<T> & v1, const vector4<T> & v2 )
  {
    vector4<T> result( v1.x + v2.x,
            v1.y + v2.y,
            v1.z + v2.z,
            v1.w + v2.w );
    return result;
  }

  template <typename T>
  inline vector4<T>
  operator+ ( const vector4<T> & v, const T constant )
  {
    vector4<T> result( v.x + constant,
            v.y + constant,
            v.z + constant,
            v.w + constant );
    return result;
  }

  template <typename T>
  inline vector4<T>
  operator+ ( const T constant, const vector4<T> & v )
  {
    vector4<T> result( v.x + constant,
            v.y + constant,
            v.z + constant,
            v.w + constant );
    return result;
  }


  template <typename T>
  inline void vector4<T> ::
  operator-= ( const vector4<T> & other )
  {
    this->x -= other.x;
    this->y -= other.y;
    this->z -= other.z;
    this->w -= other.w;
  }

  template <typename T>
  inline void vector4<T> ::
  operator-= ( const T constant )
  {
    this->x -= constant;
    this->y -= constant;
    this->z -= constant;
    this->w -= constant;
  }


  template <typename T>
  inline vector4<T>
  operator- ( const vector4<T> & v1, const vector4<T> & v2 )
  {
    vector4<T> result( v1.x - v2.x,
            v1.y - v2.y,
            v1.z - v2.z,
            v1.w - v2.w );
    return result;
  }

  template <typename T>
  inline vector4<T>
  operator- ( const vector4<T> & v, const T constant )
  {
    vector4<T> result( v.x - constant,
            v.y - constant,
            v.z - constant,
            v.w - constant );
    return result;
  }

  template <typename T>
  inline vector4<T>
  operator- ( const T constant, const vector4<T> & v )
  {
    vector4<T> result( constant - v.x,
            constant - v.y,
            constant - v.z,
            constant - v.w );
    return result;
  }


  template <typename T>
  inline vector4<T>
  operator- ( const vector4<T> & v )
  {
    vector4<T> result( -v.x,
            -v.y,
            -v.z,
            -v.w );
    return result;
  }


  template <typename T>
  inline void vector4<T> ::
  operator*= ( const vector4<T> & other )
  {
    this->x *= other.x;
    this->y *= other.y;
    this->z *= other.z;
    this->w *= other.w;
  }

  template <typename T>
  inline void vector4<T> ::
  operator*= ( const T constant )
  {
    this->x *= constant;
    this->y *= constant;
    this->z *= constant;
    this->w *= constant;
  }


  template <typename T>
  inline vector4<T>
  operator* ( const vector4<T> & v1, const vector4<T> & v2 )
  {
    vector4<T> result( v1.x * v2.x,
            v1.y * v2.y,
            v1.z * v2.z,
            v1.w * v2.w );
    return result;
  }

  template <typename T>
  inline vector4<T>
  operator* ( const vector4<T> & v, const T constant )
  {
    vector4<T> result( v.x * constant,
            v.y * constant,
            v.z * constant,
            v.w * constant );
    return result;
  }

  template <typename T>
  inline vector4<T>
  operator* ( const T constant, const vector4<T> & v )
  {
    vector4<T> result( v.x * constant,
            v.y * constant,
            v.z * constant,
            v.w * constant );
    return result;
  }

  template <typename T>
  inline void vector4<T> ::
  operator/= ( const vector4<T> & other )
  {
    this->x /= other.x;
    this->y /= other.y;
    this->z /= other.z;
    this->w /= other.w;
  }

  template <typename T>
  inline void vector4<T> ::
  operator/= ( const T constant )
  {
    T inv_constant = 1.0f / constant;
    this->x *= inv_constant;
    this->y *= inv_constant;
    this->z *= inv_constant;
    this->w *= inv_constant;
  }

  template <typename T>
  inline vector4<T>
  operator/ ( const vector4<T> & v1, const vector4<T> & v2 )
  {
    vector4<T> result( v1.x / v2.x,
            v1.y / v2.y,
            v1.z / v2.z,
            v1.w / v2.w );
    return result;
  }

  template <typename T>
  inline vector4<T>
  operator/ ( const vector4<T> & v, const T constant )
  {
    vector4<T> result( v.x / constant,
            v.y / constant,
            v.z / constant,
            v.w / constant );
    return result;
  }

  template <typename T>
  inline vector4<T>
  operator/ ( const T constant, const vector4<T> & v )
  {
    vector4<T> result( constant / v.x,
            constant / v.y,
            constant / v.z,
            constant / v.w );
    return result;
  }

  template <typename T>
  inline T vector4<T> ::
  operator[] ( u32 i ) const
  {
    return this->data[ i ];
  }

  template <typename T>
  inline T& vector4<T> ::
  operator[] ( u32 i )
  {
    return this->data[ i ];
  }

  template <typename T>
  inline bool vector4<T> ::
  operator== ( const vector4<T>& other ) const
  {
    if ( this->x == other.x &&
       this->y == other.y &&
       this->z == other.z &&
       this->w == other.z )
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  template <typename T>
  inline bool vector4<T> ::
  operator!= ( const vector4<T>& other ) const
  {
    return !(*this == other);
  }


  // Common Functions

  template< typename T>
  inline T
  compute_dot( const vector2<T> & a, const vector2<T> & b )
  {
      return (a[0] * b[0]) + (a[1] * b[1]);
  }


  template< typename T>
  inline T
  compute_dot( const vector3<T> & a, const vector3<T> & b )
  {
      return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
  }

  template< typename T>
  inline T
  compute_dot( const vector4<T> & a, const vector4<T> & b )
  {
      return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]) + (a[3] * b[3]);
  }

  template< typename T, template <typename Q> class vecX >
  inline T
  dot ( const vecX<T> & a, const vecX<T> & b )
  {
      return compute_dot( a, b );
  }

  template< typename T, template <typename Q> class vecX >
  inline T
  norm_sq ( const vecX<T> & a )
  {
      return compute_dot( a, a );
  }

  template< typename T, template <typename Q> class vecX >
  inline T
  norm ( const vecX<T> & a )
  {
      return std::sqrt( norm_sq( a ) );
  }

  template< typename T, template <typename Q> class vecX >
  inline T
  distance ( const vecX<T> & a, const vecX<T> & b )
  {
      return norm( a - b );
  }

  template< typename T, template <typename Q> class vecX >
  inline vecX<T>
  normalize ( const vecX<T> & a )
  {
      return a * (T)( 1.0 / std::sqrt( compute_dot( a, a ) ) );
  }


  template< typename T, template <typename Q> class vecX >
  inline vecX<T>
  abs ( const vecX<T> & a )
  {
      vecX<T> b;
      for ( int i = 0 ; i < a.size() ; ++i )
      {
          b[i] = std::abs( a[i] );
      }
      return b;
  }


  template< typename T, template <typename Q> class vecX >
  inline vecX<T>
  clamp ( const vecX<T> & a, const T val_min, const T val_max )
  {
      vecX<T> b;
      for ( int i = 0 ; i < a.size() ; ++i )
      {
          if ( a[i] < val_min ) b[i]  = val_min;
          else if ( a[i] > val_max ) b[i]  = val_max;
          else b[i] = a[i];
      }
      return b;
  }

  template< typename T >
  inline vector3<T>
  cross ( const vector3<T> & a, const vector3<T> & b )
  {
      return vector3<T>( ( a.y * b.z - a.z * b.y ),
                      ( a.z * b.x - a.x * b.z ),
                      ( a.x * b.y - a.y * b.x ) );
  }
  template< typename T >
  inline T
  parallelogramArea ( const vector3<T> & a, const vector3<T> & b )
  {
      return norm( cross( a, b ) );
  }

  template< typename T >
  inline T
  parallelogramArea ( const vector2<T> & a,
                      const vector2<T> & b )
  {
      return a.x * b.y - a.y * b.x;
  }

  template< typename T>
  inline T
  scalarTripleProduct( const vector3<T> & a,
                       const vector3<T> & b,
                       const vector3<T> & c )
  {
      return dot( a, cross( b, c ) );
  }

  template< typename T>
  inline T
  parallelepipedVolume( const vector3<T> & a,
                        const vector3<T> & b,
                        const vector3<T> & c )
  {
      return norm( scalarTripleProduct( a, b ) );
  }

  template< typename T >
  inline int
  ccw ( const vector2<T> & a, const vector2<T> & b )
  {
      T area = parallelogramArea( a, b );
      if ( area > 0 ) return 1;
      else if ( area < 0 ) return 0;
      else return 2;
  }
}
