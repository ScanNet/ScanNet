#pragma once

//---------------------------------------------------
// Interface
//---------------------------------------------------


namespace bsc
{
  template <typename T>
  struct quaternion
  {
    union
    {
      struct { T x, y, z, w; };
      struct { vector3<T> im; T re; };
    };

    quaternion();
    quaternion( const T re );
    quaternion( const vector3<T> & im );
    quaternion( const T re, const vector3<T> & im );
    quaternion( const T w, const T x, const T y, const T z );
    quaternion( const quaternion<T> & other );

    quaternion<T> & operator= ( const quaternion<T> & other );

    void operator+=( const quaternion<T> & other );
    void operator+=( const T constant );
    quaternion<T> operator+( const quaternion<T> & other ) const;
    quaternion<T> operator+( const T constant ) const;

    void operator-=( const quaternion<T> & other );
    void operator-=( const T constant );
    quaternion<T> operator- ( void );
    quaternion<T> operator-( const quaternion<T> & other ) const;
    quaternion<T> operator-( const T constant ) const;

    void operator*=( const quaternion<T> & other );
    void operator*=( const T constant );
    quaternion<T> operator*( const quaternion<T> & other ) const;
    quaternion<T> operator*( const T constant ) const;

    void operator/=( const quaternion<T> & other );
    void operator/=( const T constant );
    quaternion<T> operator/( const quaternion<T> & other ) const;
    quaternion<T> operator/( const T constant ) const;

    bool operator==( const quaternion<T> & other ) const;
    bool operator!=( const quaternion<T> & other ) const;

    T operator[] ( const u32 i ) const;
    T& operator[] ( const u32 i );
  };
  
  template <typename T>
  T dot ( const quaternion<T> & q1,  const quaternion<T> & q2 );

  template <typename T>
  T norm ( const quaternion<T> & q );

  template <typename T>
  T normSq ( const quaternion<T> & q );

  template <typename T>
  quaternion<T> normalize( const quaternion<T> & q );

  template <typename T>
  quaternion<T> conjugate( const quaternion<T> & q );

  template <typename T>
  quaternion<T> inverse( const quaternion<T> & q );

  template <typename T>
  quaternion<T> slerp( const quaternion<T> & q, 
                       const quaternion<T> & r, 
                       const T t );

  template <typename T>
  matrix3<T> to_mat3 ( const quaternion<T> & q );

  template <typename T>
  matrix4<T> to_mat4 ( const quaternion<T> & q );

  template <typename T>
  quaternion<T> to_quat ( const matrix3<T> & m );

  template <typename T>
  quaternion<T> to_quat ( const matrix4<T> & m ); 
  
  typedef quaternion<r64> quatd;
  typedef quaternion<r32> quatf;
  typedef quaternion<r32> quat;
}

//---------------------------------------------------
// Interface
//---------------------------------------------------

namespace bsc
{

  template <typename T>
  quaternion<T>::
  quaternion ()
  {
    this->re      = 0;
    this->im = vector3<T>();
  }

  template <typename T>
  quaternion<T>::
  quaternion ( const T re )
  {
    this->re = re;
    this->im = vector3<T>();
  }

  template <typename T>
  quaternion<T>::
  quaternion ( const vector3<T> & im )
  {
    this->re = (T)0;
    this->im = im;
  }

  template <typename T>
  quaternion<T>::
  quaternion ( const T re, const vector3<T> & im )
  {
    this->re = re;
    this->im = im;
  }

  template <typename T>
  quaternion<T>::
  quaternion ( const T w, const T x, const T y, const T z )
  {
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
  }

  template <typename T>
  quaternion<T>::
  quaternion ( const quaternion<T> & other )
  {
    this->re = other.re;
    this->im = other.im;
  }

  template <typename T>
  inline T quaternion<T>::
  operator[] ( const u32 i ) const
  {
    return *(&(this->x) + i);
  }

  template <typename T>
  inline T& quaternion<T>::
  operator[] ( const u32 i )
  {
    return *(&(this->x) + i);
  }

  template <typename T>
  inline quaternion<T> & quaternion<T>::
  operator= ( const quaternion<T> & other )
  {
    this->re = other.re;
    this->im = other.im;
    return *this;
  }

  template <typename T>
  inline void quaternion<T>::
  operator+=( const quaternion<T> & other )
  {
    this->re += other.re;
    this->im += other.im;
  }

  template <typename T>
  inline void quaternion<T>::
  operator+=( const T constant )
  {
    this->re += constant;
  }

  template <typename T>
  inline quaternion<T> quaternion<T>::
  operator+( const quaternion<T> & other ) const
  {
    return quaternion<T>( this->re      + other.re,
                   this->im + other.im );
  }

  template <typename T>
  inline quaternion<T> quaternion<T>::
  operator+( const T constant ) const
  {
    return quaternion<T>( this->re + constant,
                   this->im );
  }

  template <typename T>
  inline void quaternion<T>::
  operator-=( const quaternion<T> & other )
  {
    this->re -= other.re;
    this->im -= other.im;
  }

  template <typename T>
  inline void quaternion<T>::
  operator-=( const T constant )
  {
    this->re      -= constant;
  }

  template <typename T>
  inline quaternion<T> quaternion<T>::
  operator- ( void )
  {
    this->re = -this->re;
    this->im = -this->im ;
    return *this;
  }

  template <typename T>
  inline quaternion<T> quaternion<T>::
  operator- ( const quaternion<T> & other ) const
  {
    return quaternion<T>( this->re - other.re,
                   this->im - other.im );
  }

  template <typename T>
  inline quaternion<T> quaternion<T>::
  operator- ( const T constant ) const
  {
    return quaternion<T>( this->re - constant,
                   this->im );
  }

  template <typename T>
  inline void quaternion<T>::
  operator*= ( const quaternion<T> & other )
  {
    T tmp_re = this->re * other.re - bsc::dot( this->im, other.im );
    vector3<T> tmp_im = other.im * this->re  +
    this->im * other.re +
    cross( this->im, other.im );

    this->re = tmp_re;
    this->im = tmp_im;
  }

  template <typename T>
  inline void quaternion<T>::
  operator*= ( const T constant )
  {
    this->re *= constant;
    this->im *= constant;
  }

  template <typename T>
  inline quaternion<T> quaternion<T>::
  operator* ( const quaternion<T> & other ) const
  {
    quaternion<T> Result( *this );
    Result *= other;
    return Result;
  }

  template <typename T>
  inline quaternion<T> quaternion<T>::
  operator* ( const T constant ) const
  {
    return quaternion<T>( this->re * constant,
                   this->im * constant );
  }


  template <typename T>
  inline void quaternion<T>::
  operator/= ( const quaternion<T> & other )
  {
    quaternion<T> otherTmp ( other );
    otherTmp.inverse();

    this *= otherTmp;
  }

  template <typename T>
  inline void quaternion<T>::
  operator/= ( const T constant )
  {
    this->re      /= constant;
    this->im /= constant;
  }

  template <typename T>
  inline quaternion<T> quaternion<T> ::
  operator/ ( const quaternion<T> & other ) const
  {
    quaternion<T> Result( *this );
    Result /= other;
    return Result;
  }

  template <typename T>
  inline quaternion<T> quaternion<T> ::
  operator/ ( const T constant ) const
  {
    return quaternion<T>( this->re / constant,
                   this->im / constant );
  }
  
  // GENERAL FUNCTIONS
  template <typename T>
  inline T
  dot ( const quaternion<T> & q1, const quaternion<T> & q2 )
  {
    return q1.re * q2.re + bsc::dot( q1.im, q2.im );
  }

  template <typename T>
  inline T
  norm ( const quaternion<T> & q )
  {
    return sqrt( bsc::dot( q, q ) );
  }

  template <typename T>
  inline T
  normSq ( const quaternion<T> & q )
  {
    return bsc::dot( q, q );
  }


  template <typename T>
  inline bsc::quaternion<T>
  inverse ( const quaternion<T> & q )
  {
    quaternion<T> p( q );
    T Normalization = bsc::normSq( p );
    p.re /= Normalization;
    p.im = -p.im;
    p.im /= Normalization;
    return p;
  }

  // NOTE: Can we simplify this? Inigo's avoiding trigonometry notes.
  template <typename T>
  inline bsc::quaternion<T>
  slerp ( const quaternion<T> & q, const quaternion<T> & r, const T t )
  {
    r32 a = acos( dot(q, r) );
    quaternion<T> p;
    if ( fabs( a ) > 1e-6 )
    {
      p = q * (sin(a * (1.0-t)) / sin(a)) + r * (sin(a * t) / sin(a));
    }
    else
    {
      p = q * (1.0 - t) + r * t;
    }
    return p;
  }

  template <typename T>
  inline bsc::quaternion<T>
  conjugate ( const quaternion<T> & q )
  {
    quaternion<T> p( q );
    p.im = -p.im;
    return p;
  }

  template <typename T>
  inline bsc::quaternion<T>
  normalize ( const quaternion<T> & q )
  {
    quaternion<T> p( q );
    p.im /= bsc::norm( p );
    return p;
  }


  // derivation http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
  template <typename T>
  inline matrix3<T>
  to_mat3 ( const quaternion<T> & q )
  {
    matrix3<T> m(false);
    T xx( q.x * q.x );
    T xy( q.x * q.y );
    T xz( q.x * q.z );
    T xw( q.x * q.w );

    T yy( q.y * q.y );
    T yz( q.y * q.z );
    T yw( q.y * q.w );

    T zz( q.z * q.z );
    T zw( q.z * q.w );

    m[0][0] = 1 - 2 * (yy +  zz);
    m[0][1] = 2 * (xy + zw);
    m[0][2] = 2 * (xz - yw);

    m[1][0] = 2 * (xy - zw);
    m[1][1] = 1 - 2 * (xx +  zz);
    m[1][2] = 2 * (yz + xw);

    m[2][0] = 2 * (xz + yw);
    m[2][1] = 2 * (yz - xw);
    m[2][2] = 1 - 2 * (xx +  yy);
    return m;
  }

  template <typename T>
  inline matrix4<T>
  to_mat4 ( const quaternion<T> & q )
  {
    return matrix4<T>( to_mat3( q ) );
  }

  // derivation  http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
  template <typename T>
  inline bsc::quaternion<T>
  to_quat ( const matrix3<T> & m )
  {
    r32 tr = m[0][0] + m[1][1] + m[2][2];
    quaternion<T> q;
    if (tr > 0)
    {
      r32 S = sqrtf( tr + 1.0f ) * 2;
      q.w = 0.25f * S;
      q.x = ( m[1][2] - m[2][1] ) / S;
      q.y = ( m[2][0] - m[0][2] ) / S;
      q.z = ( m[0][1] - m[1][0] ) / S;
    }
    else if ( ( m[0][0] > m[1][1]) && (m[0][0] > m[2][2] ) )
    {
      r32 S = sqrtf( 1.0f + m[0][0] - m[1][1] - m[2][2]) * 2.0f;
      q.w = ( m[1][2] - m[2][1] ) / S;
      q.x = 0.25f * S;
      q.y = ( m[1][0] + m[0][1] ) / S;
      q.z = ( m[2][0] + m[0][2] ) / S;
    }
    else if ( m[1][1] > m[2][2] )
    {
      r32 S = sqrtf( 1.0f + m[1][1] - m[0][0] - m[2][2]) * 2.0f;
      q.w = ( m[2][0] - m[0][2] ) / S;
      q.x = ( m[1][0] + m[0][1] ) / S;
      q.y = 0.25f * S;
      q.z = ( m[2][1] + m[1][2] ) / S;
    } else
    {
      r32 S = sqrtf( 1.0f + m[2][2] - m[0][0] - m[1][1] ) * 2.0f;
      q.w = ( m[0][1] - m[1][0] ) / S;
      q.x = ( m[2][0] + m[0][2] ) / S;
      q.y = ( m[2][1] + m[1][2] ) / S;
      q.z = 0.25f * S;
    }
    return q;
  }

  template <typename T>
  inline bsc::quaternion<T>
  to_quat ( const matrix4<T> & m )
  {
    return to_quat( matrix3<T>(m) );
  }

}
