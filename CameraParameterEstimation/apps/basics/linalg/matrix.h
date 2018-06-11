#pragma once

//---------------------------------------------------
// Interface
//---------------------------------------------------


namespace bsc
{
  template <typename T> struct matrix2;
  template <typename T> struct matrix3;
  template <typename T> struct matrix4;


  // MATRIX 2
  template <typename T>
  struct matrix2 {

    union
    {
      T data[4];
      vector2<T> cols[2];
    };

    matrix2 ( bool initialize = true );
    matrix2 ( const T constant );
    matrix2 ( const T *other );
    matrix2 ( const matrix2<T> & other );
    matrix2 ( const matrix3<T> & other );
    matrix2 ( const matrix4<T> & other );
    matrix2 ( const vector2<T> & diagonal );
    matrix2 ( const vector2<T> & col0,
              const vector2<T> & col1 );

    matrix2<T> & operator= ( const matrix2<T> & other );

    void operator+= ( const matrix2<T> & other );
    void operator+= ( const T constant );
    

    void operator-=( const matrix2<T> & other );
    void operator-=( const T constant );

    void operator*= ( const matrix2<T> & other );
    void operator*= ( const T constant );
    void pointwiseMultiply ( const matrix2<T> &other );

    void operator/= ( const matrix2<T> & other );
    void operator/= ( const T constant );

    vector2<T>& operator[] ( unsigned int i );
    vector2<T>  operator[] ( unsigned int i ) const;

    bool operator== ( const matrix2<T> & other ) const;
    bool operator!= ( const matrix2<T> & other ) const;

    const constexpr int dimensionality () const  { return 2; } // whats the point?
    const constexpr int size () const  { return 4; }
  };
  // TODO: add forward declaration of other operators!

  // MATRIX 3
  template <typename T>
  struct matrix3 {

    union
    {
      T data[9];
      vector3<T> cols[3];
    };

    matrix3 ( bool initialize = true );
    matrix3 ( const T constant );
    matrix3 ( const T * other );
    matrix3 ( const matrix2<T> & other );
    matrix3 ( const matrix3<T> & other );
    matrix3 ( const matrix4<T> & other );
    matrix3 ( const vector3<T> & diagonal );
    matrix3 ( const vector3<T> & col0,
              const vector3<T> & col1,
              const vector3<T> & col2 );

    matrix3<T> & operator= ( const matrix3<T> & other );

    void operator+= ( const matrix3<T> & other );
    void operator+= ( const T constant );

    void operator-= ( const matrix3<T> & other );
    void operator-= ( const T constant );

    void operator*= ( const matrix3<T> & other );
    void operator*= ( const T constant );
    void pointwiseMultiply( const matrix3<T> &other );

    void operator/= ( const matrix3<T> & other );
    void operator/= ( const T constant );

    vector3<T>& operator[] ( unsigned int i );
    vector3<T>  operator[] ( unsigned int i ) const;

    bool operator== ( const matrix2<T> & other ) const;
    bool operator!= ( const matrix2<T> & other ) const;

    const constexpr int dimensionality () const  { return 3; }
    const constexpr int size () const  { return 9; }
  };


  // MATRIX 4
  template <typename T>
  struct matrix4 {

    union
    {
      T data[16];
      vector4<T> cols[4];
    };

    matrix4 ( bool initialize = true );
    matrix4 ( const T constant );
    matrix4 ( const T * other );
    matrix4 ( const matrix2<T> & other );
    matrix4 ( const matrix3<T> & other );
    matrix4 ( const matrix4<T> & other );
    matrix4 ( const vector4<T> & diagonal );
    matrix4 ( const vector4<T> & col0,
              const vector4<T> & col1,
              const vector4<T> & col2,
              const vector4<T> & col3 );

    matrix4<T> & operator= ( const matrix4<T> & other );

    void operator+= ( const matrix4<T> & other );
    void operator+= ( const T constant );

    void operator-= ( const matrix4<T> & other );
    void operator-= ( const T constant );

    void operator*= ( const matrix4<T> & other );
    void operator*= ( const T constant );
    void pointwiseMultiply ( const matrix4<T> &other );

    void operator/= ( const matrix4<T> & other );
    void operator/= ( const T constant );

    vector4<T>& operator[] ( unsigned int i );
    vector4<T>  operator[] ( unsigned int i ) const;

    bool operator== ( const matrix2<T> & other ) const;
    bool operator!= ( const matrix2<T> & other ) const;

    const constexpr int dimensionality () const  { return 4; }
    const constexpr int size () const  { return 16; }
  };

////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
////////////////////////////////////////////////////////////////////////////////

  template< typename T, template <typename Q> class matrixX >
  T determinant ( const matrixX<T> & matrix );

  template< typename T, template <typename Q> class matrixX >
  T trace ( const matrixX<T> & matrix );

  template< typename T, template <typename Q> class matrixX >
  matrixX<T> inverse ( const matrixX<T> & matrix );

  template< typename T, template <typename Q> class matrixX >
  matrixX<T> transpose ( const matrixX<T> & matrix );

  template< typename T>
  matrix4<T> look_at ( const vector3<T> & eye, 
                       const vector3<T> & center, 
                       const vector3<T> & up );

  template< typename T>
  matrix4<T> perspective ( const T & fovy, 
                           const T & aspect, 
                           const T & z_near, 
                           const T & z_far);

  template< typename T>
  matrix4<T> frustum ( const T & left, const T & right, 
                       const T & bottom, const T & top, 
                       const T & z_near, const T & z_far );

  template< typename T>
  matrix4<T> ortho ( const T & left, const T & right, 
                     const T & bottom, const T & top, 
                     const T & z_near, const T & z_far );

  template< typename T>
  vector3<T> project ( const vector4<T> & obj, 
                       const matrix4<T> & model, 
                       const matrix4<T> & project, 
                       const vector4<T> & viewport );

  template< typename T>
  vector4<T> unproject ( const vector3<T> & win, 
                         const matrix4<T> & model, 
                         const matrix4<T> & project, 
                         const vector4<T> & viewport );

  template< typename T>
  matrix4<T> translate( const matrix4<T> & m, const vector3<T> & t );

  template< typename T>
  matrix4<T> scale( const matrix4<T> & m, const vector3<T> & s );

  template< typename T>
  matrix4<T> rotate( const matrix4<T> & m, 
                     const T & angle, 
                     const vector3<T> & axis );

  matrix4<float>
  pca( const vector3<float> * points, 
       const i32 n_points );

  void 
  eig( const matrix3<float> * mat, 
       matrix3<float> * eigvec, 
       vector3<float> * eigval );

  typedef matrix2<r32> mat2;
  typedef matrix3<r32> mat3;
  typedef matrix4<r32> mat4;

  typedef matrix2<r64> mat2d;
  typedef matrix2<r32> mat2f;
  typedef matrix2<i32> mat2i;

  typedef matrix3<r64> mat3d;
  typedef matrix3<r32> mat3f;
  typedef matrix3<i32> mat3i;

  typedef matrix4<r64> mat4d;
  typedef matrix4<r32> mat4f;
  typedef matrix4<i32> mat4i;
}

//---------------------------------------------------
// Implementation
//---------------------------------------------------
namespace bsc
{
  // MATRIX 2
  template <typename T>
  matrix2<T>::
  matrix2 ( bool intialize )
  {
    if ( intialize )
    {
      this->data[ 0] = (T)1.0;
      this->data[ 1] = (T)0.0;
      this->data[ 2] = (T)0.0;
      this->data[ 3] = (T)1.0;
    }
  }

  template <typename T>
  inline matrix2<T> ::
  matrix2 ( const T c )
  {
    for ( int i = 0 ; i < 4 ; ++i )
    {
      data[i] = c;
    }
  }

  template <typename T>
  inline matrix2<T> ::
  matrix2 ( const T * other )
  {
    for ( int i = 0 ; i < 4 ; ++i )
    {
      data[i] = other[i];
    }
  }


  template <typename T>
  matrix2<T>::
  matrix2 ( const matrix2<T> & other )
  {
    this->data[0] = other.data[0];
    this->data[1] = other.data[1];
    this->data[2] = other.data[2];
    this->data[3] = other.data[3];
  }

  template <typename T>
  matrix2<T>::
  matrix2 ( const matrix3<T> & other )
  {
    this->data[0] = other.data[0];
    this->data[1] = other.data[1];
    this->data[2] = other.data[3];
    this->data[3] = other.data[4];
  }

  template <typename T>
  matrix2<T>::
  matrix2 ( const matrix4<T> & other )
  {
    this->data[0] = other.data[0];
    this->data[1] = other.data[1];
    this->data[2] = other.data[4];
    this->data[3] = other.data[5];
  }


  template <typename T>
  matrix2<T>::
  matrix2 ( const vector2<T> & diagonal )
  {
    for ( int i = 0 ; i < 4 ; ++i )
    {
      data[i] = 0;
    }
    data[0]  = diagonal.x;
    data[3]  = diagonal.y;

  }

  template <typename T>
  matrix2<T>::
  matrix2 ( const vector2<T> & col0, const vector2<T> & col1 )
  {
    cols[0] = col0;
    cols[1] = col1;
  }

  template <typename T>
  inline void matrix2<T>::
  operator+= ( const matrix2<T> & other )
  {
    for ( int i = 0 ; i < 4 ; i += 2 )
    {
      this->data[i]   += other.data[i];
      this->data[i+1] += other.data[i+1];
    }
  }

  template <typename T>
  inline void matrix2<T>::
  operator+= ( const T constant )
  {
    for ( int i = 0 ; i < 4 ; i += 2 )
    {
      this->data[i]   += constant;
      this->data[i+1] += constant;
      this->data[i+2] += constant;
    }
  }

  template <typename T>
  inline matrix2<T>
  operator+ ( const matrix2<T> & m1, const matrix2<T> & m2 )
  {
    matrix2<T> result( m1[0] + m2[0],
            m1[1] + m2[1],
            m1[2] + m2[2] );
    return result;
  }

  template <typename T>
  inline matrix2<T>
  operator+ ( const matrix2<T> & m, const T constant )
  {
    matrix2<T> result( m[0] + constant,
            m[1] + constant,
            m[2] + constant);
    return result;
  }

  template <typename T>
  inline matrix2<T>
  operator+ ( const T constant, const matrix2<T> & m )
  {
    matrix2<T> result( m[0] + constant,
            m[1] + constant,
            m[2] + constant );
    return result;
  }



  template <typename T>
  inline void matrix2<T>::
  operator-= ( const matrix2<T> & other )
  {
    for ( int i = 0 ; i < 4 ; i += 2 )
    {
      this->data[i]   -= other.data[i];
      this->data[i+1] -= other.data[i+1];
      this->data[i+2] -= other.data[i+2];
    }
  }

  template <typename T>
  inline void matrix2<T>::
  operator-= ( const T constant )
  {
    for ( int i = 0 ; i < 4 ; i += 2 )
    {
      this->data[i]   -= constant;
      this->data[i+1] -= constant;
      this->data[i+2] -= constant;
    }
  }

  template <typename T>
  inline matrix2<T>
  operator- ( const matrix2<T> & m1, const matrix2<T> & m2 )
  {
    matrix2<T> result( m1[0] - m2[0],
            m1[1] - m2[1] );
    return result;
  }

  template <typename T>
  inline matrix2<T>
  operator- ( const matrix2<T> & m, const T constant )
  {
    matrix2<T> result( m[0] - constant,
            m[1] - constant );
    return result;
  }

  template <typename T>
  inline matrix2<T>
  operator- ( const T constant, const matrix2<T> & m )
  {
    matrix2<T> result( constant - m[0],
            constant - m[1] );
    return result;
  }

  template <typename T>
  inline matrix2<T>
  operator- ( const matrix2<T> & m )
  {
    matrix2<T> result( -m[0],
            -m[1] );
    return result;
  }

  template <typename T>
  inline void matrix2<T>::
  operator*= ( const matrix2<T> & other )
  {
    T vals[4];

    vals[0] = other.data[0] * data[0] + other.data[1] * data[2];
    vals[1] = other.data[0] * data[1] + other.data[1] * data[3];

    vals[2] = other.data[2] * data[0] + other.data[3] * data[2];
    vals[3] = other.data[2] * data[1] + other.data[3] * data[3];

    for ( int i = 0 ; i < 4 ; i += 2 )
    {
      this->data[i]   = vals[i]  ;
      this->data[i+1] = vals[i+1];
    }
  }

  template <typename T>
  inline void matrix2<T>::
  operator*= ( const T constant )
  {
    for ( int i = 0 ; i < 4 ; i += 2 )
    {
      this->data[i]   *= constant;
      this->data[i+1] *= constant;
    }
  }

  template <typename T>
  inline matrix2<T>
  operator* ( const matrix2<T> & m1, const matrix2<T> & m2 )
  {
    T vals[4];

    vals[0] = m2.data[0] * m1.data[0] + m2.data[1] * m1.data[2];
    vals[1] = m2.data[0] * m1.data[1] + m2.data[1] * m1.data[3];

    vals[2] = m2.data[2] * m1.data[0] + m2.data[3] * m1.data[2];
    vals[3] = m2.data[2] * m1.data[1] + m2.data[3] * m1.data[3];


    matrix2<T> result( vals );
    return result;
  }

  template <typename T>
  inline matrix2<T>
  operator* ( const matrix2<T> & m, const T constant )
  {
    matrix2<T> result( m[0] * constant,
                       m[1] * constant );
    return result;
  }

  template <typename T>
  inline matrix2<T>
  operator* ( const T constant, const matrix2<T> & m )
  {
    matrix2<T> result( m[0] * constant,
            m[1] * constant );
    return result;
  }

  template <typename T>
  inline vector2<T>
  operator* ( const matrix2<T> & m, const vector2<T> & v )
  {
    vector2<T> output;

    output.x = m.data[0] * v.x + m.data[2] * v.y;
    output.y = m.data[1] * v.x + m.data[3] * v.y;

    return output;
  }

  template <typename T>
  inline void matrix2<T>::
  pointwiseMultiply ( const matrix2<T> & other )
  {
    for ( int i = 0 ; i < 4 ; i += 2 )
    {
      this->data[i]   *= other.data[i];
      this->data[i+1] *= other.data[i+1];
    }
  }


  template <typename T>
  inline void matrix2<T>::
  operator/= ( const matrix2<T> & other )
  {
    matrix2<T> inv_other = Inverse( other );
    (*this) *= inv_other;
  }

  template <typename T>
  inline void matrix2<T>::
  operator/= ( const T constant )
  {
    float inv_constant = (T)1.0 / constant;
    for ( int i = 0 ; i < 4 ; i += 2 )
    {
      this->data[i]   *= inv_constant;
      this->data[i+1] *= inv_constant;
    }
  }

  template <typename T>
  inline matrix2<T>
  operator/ ( const matrix2<T> & m1, const matrix2<T> & m2 )
  {
    matrix2<T> result( m1 );
    result /= m2;
    return result;
  }

  template <typename T>
  inline matrix2<T>
  operator/ ( const matrix2<T> & m, const T constant )
  {
    float inv_constant = (T)1.0 / constant;
    matrix2<T> result( m[0] * inv_constant,
                       m[1] * inv_constant,
                       m[2] * inv_constant );
    return result;
  }

  template <typename T>
  inline matrix2<T>
  operator/ ( const T constant, const matrix2<T> & m )
  {
    matrix2<T> result( constant / m[0],
            constant / m[1],
            constant / m[2] );
    return result;
  }

  template <typename T>
  inline vector2<T> & matrix2<T>::
  operator[] ( unsigned int i )
  {
    return cols[i];
  }

  template <typename T>
  inline vector2<T> matrix2<T>::
  operator[] ( unsigned int i ) const
  {
    return cols[i];
  }


  // MATRIX 3
  template <typename T>
  matrix3<T> ::
  matrix3 ( bool intialize )
  {
    if ( intialize )
    {
      this->data[0] = (T)1.0;
      this->data[1] = (T)0.0;
      this->data[2] = (T)0.0;

      this->data[3] = (T)0.0;
      this->data[4] = (T)1.0;
      this->data[5] = (T)0.0;

      this->data[6] = (T)0.0;
      this->data[7] = (T)0.0;
      this->data[8] = (T)1.0;
    }
  }

  template <typename T>
  matrix3<T> ::
  matrix3 ( const T c )
  {
    for ( int i = 0 ; i < 9 ; ++i )
    {
      data[i] = c;
    }
  }

  template <typename T>
  matrix3<T> ::
  matrix3 ( const T * other )
  {
    for ( int i = 0 ; i < 9 ; ++i )
    {
      data[i] = other[i];
    }
  }

  template <typename T>
  matrix3<T>::
  matrix3 ( const matrix2<T> & other )
  {
    this->data[0] = other.data[0];
    this->data[1] = other.data[1];
    this->data[2] = (T)0.0;
    this->data[3] = other.data[2];
    this->data[4] = other.data[3];
    this->data[5] = (T)0.0;
    this->data[6] = (T)0.0;
    this->data[7] = (T)0.0;
    this->data[8] = (T)1.0;
  }

  template <typename T>
  matrix3<T> ::
  matrix3 ( const matrix3<T> & other )
  {
    this->data[0] = other.data[0];
    this->data[1] = other.data[1];
    this->data[2] = other.data[2];
    this->data[3] = other.data[3];
    this->data[4] = other.data[4];
    this->data[5] = other.data[5];
    this->data[6] = other.data[6];
    this->data[7] = other.data[7];
    this->data[8] = other.data[8];
  }


  template <typename T>
  matrix3<T>::
  matrix3 ( const matrix4<T> & other )
  {
    this->data[ 0] = other.data[ 0];
    this->data[ 1] = other.data[ 1];
    this->data[ 2] = other.data[ 2];
    this->data[ 3] = other.data[ 4];
    this->data[ 4] = other.data[ 5];
    this->data[ 5] = other.data[ 6];
    this->data[ 6] = other.data[ 8];
    this->data[ 7] = other.data[ 9];
    this->data[ 8] = other.data[10];
  }

  template <typename T>
  matrix3<T> ::
  matrix3 ( const vector3<T> & diagonal )
  {
    for ( int i = 0 ; i < 9 ; ++i )
    {
      data[i] = 0;
    }
    data[0] = diagonal.x;
    data[4] = diagonal.y;
    data[8] = diagonal.z;
  }

  template <typename T>
  matrix3<T> ::
  matrix3 ( const vector3<T> & col0, const vector3<T> & col1, const vector3<T> & col2 )
  {
    cols[0] = col0;
    cols[1] = col1;
    cols[2] = col2;
  }

  template <typename T>
  inline matrix3<T> & matrix3<T> ::
  operator= ( const matrix3<T> & other )
  {
    this->data[0] = other.data[0];
    this->data[1] = other.data[1];
    this->data[2] = other.data[2];
    this->data[3] = other.data[3];
    this->data[4] = other.data[4];
    this->data[5] = other.data[5];
    this->data[6] = other.data[6];
    this->data[7] = other.data[7];
    this->data[8] = other.data[8];
    return *this;
  }

  template <typename T>
  inline void matrix3<T>::
  operator+= ( const matrix3<T> & other )
  {
    for ( int i = 0 ; i < 9 ; i += 3 )
    {
      this->data[i]   += other.data[i];
      this->data[i+1] += other.data[i+1];
      this->data[i+2] += other.data[i+2];
    }
  }

  template <typename T>
  inline void matrix3<T>::
  operator+= ( const T constant )
  {
    for ( int i = 0 ; i < 9 ; i += 3 )
    {
      this->data[i]   += constant;
      this->data[i+1] += constant;
      this->data[i+2] += constant;
    }
  }

  template <typename T>
  inline matrix3<T>
  operator+ ( const matrix3<T> & m1, const matrix3<T> & m2 )
  {
    matrix3<T> result( m1[0] + m2[0],
            m1[1] + m2[1],
            m1[2] + m2[2] );
    return result;
  }

  template <typename T>
  inline matrix3<T>
  operator+ ( const matrix3<T> & m, const T constant )
  {
    matrix3<T> result( m[0] + constant,
            m[1] + constant,
            m[2] + constant);
    return result;
  }

  template <typename T>
  inline matrix3<T>
  operator+ ( const T constant, const matrix3<T> & m )
  {
    matrix3<T> result( m[0] + constant,
            m[1] + constant,
            m[2] + constant );
    return result;
  }



  template <typename T>
  inline void matrix3<T>::
  operator-= ( const matrix3<T> & other )
  {
    for ( int i = 0 ; i < 9 ; i += 3 )
    {
      this->data[i]   -= other.data[i];
      this->data[i+1] -= other.data[i+1];
      this->data[i+2] -= other.data[i+2];
    }
  }

  template <typename T>
  inline void matrix3<T>::
  operator-= ( const T constant )
  {
    for ( int i = 0 ; i < 9 ; i += 3 )
    {
      this->data[i]   -= constant;
      this->data[i+1] -= constant;
      this->data[i+2] -= constant;
    }
  }

  template <typename T>
  inline matrix3<T>
  operator- ( const matrix3<T> & m1, const matrix3<T> & m2 )
  {
    matrix3<T> result( m1[0] - m2[0],
            m1[1] - m2[1],
            m1[2] - m2[2] );
    return result;
  }

  template <typename T>
  inline matrix3<T>
  operator- ( const matrix3<T> & m, const T constant )
  {
    matrix3<T> result( m[0] - constant,
            m[1] - constant,
            m[2] - constant );
    return result;
  }

  template <typename T>
  inline matrix3<T>
  operator- ( const T constant, const matrix3<T> & m )
  {
    matrix3<T> result( constant - m[0],
            constant - m[1],
            constant - m[2] );
    return result;
  }

  template <typename T>
  inline matrix3<T>
  operator- ( const matrix3<T> & m )
  {
    matrix3<T> result( -m[0],
            -m[1],
            -m[2] );
    return result;
  }

  template <typename T>
  inline void matrix3<T>::
  operator*= ( const matrix3<T> & other )
  {
    T vals[9];


    vals[0] = other.data[0] * data[0] +
              other.data[1] * data[3] +
              other.data[2] * data[6];
    vals[1] = other.data[0] * data[1] +
              other.data[1] * data[4] +
              other.data[2] * data[7];
    vals[2] = other.data[0] * data[2] +
              other.data[1] * data[5] +
              other.data[2] * data[8];

    vals[3] = other.data[3] * data[0] +
              other.data[4] * data[3] +
              other.data[5] * data[6];
    vals[4] = other.data[3] * data[1] +
              other.data[4] * data[4] +
              other.data[5] * data[7];
    vals[5] = other.data[3] * data[2] +
              other.data[4] * data[5] +
              other.data[5] * data[8];

    vals[6] = other.data[6] * data[0] +
              other.data[7] * data[3] +
              other.data[8] * data[6];
    vals[7] = other.data[6] * data[1] +
              other.data[7] * data[4] +
              other.data[8] * data[7];
    vals[8] = other.data[6] * data[2] +
              other.data[7] * data[5] +
              other.data[8] * data[8];


    for ( int i = 0 ; i < 9 ; i += 3 )
    {
      this->data[i]   = vals[i]  ;
      this->data[i+1] = vals[i+1];
      this->data[i+2] = vals[i+2];
    }
  }

  template <typename T>
  inline void matrix3<T>::
  operator*= ( const T constant )
  {
    for ( int i = 0 ; i < 9 ; i += 3 )
    {
      this->data[i]   *= constant;
      this->data[i+1] *= constant;
      this->data[i+2] *= constant;
    }
  }

  template <typename T>
  inline matrix3<T>
  operator* ( const matrix3<T> & m1, const matrix3<T> & m2 )
  {
    T vals[9];

    vals[0] = m2.data[0] * m1.data[0] +
              m2.data[1] * m1.data[3] +
              m2.data[2] * m1.data[6];
    vals[1] = m2.data[0] * m1.data[1] +
              m2.data[1] * m1.data[4] +
              m2.data[2] * m1.data[7];
    vals[2] = m2.data[0] * m1.data[2] +
              m2.data[1] * m1.data[5] +
              m2.data[2] * m1.data[8];

    vals[3] = m2.data[3] * m1.data[0] +
              m2.data[4] * m1.data[3] +
              m2.data[5] * m1.data[6];
    vals[4] = m2.data[3] * m1.data[1] +
              m2.data[4] * m1.data[4] +
              m2.data[5] * m1.data[7];
    vals[5] = m2.data[3] * m1.data[2] +
              m2.data[4] * m1.data[5] +
              m2.data[5] * m1.data[8];

    vals[6] = m2.data[6] * m1.data[0] +
              m2.data[7] * m1.data[3] +
              m2.data[8] * m1.data[6];
    vals[7] = m2.data[6] * m1.data[1] +
              m2.data[7] * m1.data[4] +
              m2.data[8] * m1.data[7];
    vals[8] = m2.data[6] * m1.data[2] +
              m2.data[7] * m1.data[5] +
              m2.data[8] * m1.data[8];

    matrix3<T> result( vals );
    return result;
  }

  template <typename T>
  inline matrix3<T>
  operator* ( const matrix3<T> & m, const T constant )
  {
    matrix3<T> result( m[0] * constant,
            m[1] * constant,
            m[2] * constant );
    return result;
  }

  template <typename T>
  inline matrix3<T>
  operator* ( const T constant, const matrix3<T> & m )
  {
    matrix3<T> result( m[0] * constant,
            m[1] * constant,
            m[2] * constant );
    return result;
  }

  template <typename T>
  inline vector3<T>
  operator* ( const matrix3<T> & m, const vector3<T> & v )
  {
    vector3<T> output;

    output.x = m.data[0] * v.x + m.data[3] * v.y + m.data[6] * v.z;
    output.y = m.data[1] * v.x + m.data[4] * v.y + m.data[7] * v.z;
    output.z = m.data[2] * v.x + m.data[5] * v.y + m.data[8] * v.z;

    return output;
  }

  template <typename T>
  inline void matrix3<T>::
  pointwiseMultiply ( const matrix3<T> & other )
  {
    for ( int i = 0 ; i < 9 ; i += 3 )
    {
      this->data[i]   *= other.data[i];
      this->data[i+1] *= other.data[i+1];
      this->data[i+2] *= other.data[i+2];
    }
  }


  template <typename T>
  inline void matrix3<T>::
  operator/= ( const matrix3<T> & other )
  {
    matrix3<T> inv_other = Inverse( other );
    (*this) *= inv_other;
  }

  template <typename T>
  inline void matrix3<T>::
  operator/= ( const T constant )
  {
    float inv_constant = (T)1.0 / constant;
    for ( int i = 0 ; i < 9 ; i += 3 )
    {
      this->data[i]   *= inv_constant;
      this->data[i+1] *= inv_constant;
      this->data[i+2] *= inv_constant;
    }
  }

  template <typename T>
  inline matrix3<T>
  operator/ ( const matrix3<T> & m1, const matrix3<T> & m2 )
  {
    matrix3<T> result( m1 );
    result /= m2;
    return result;
  }

  template <typename T>
  inline matrix3<T>
  operator/ ( const matrix3<T> & m, const T constant )
  {
    float inv_constant = (T)1.0 / constant;
    matrix3<T> result( m[0] * inv_constant,
            m[1] * inv_constant,
            m[2] * inv_constant );
    return result;
  }

  template <typename T>
  inline matrix3<T>
  operator/ ( const T constant, const matrix3<T> & m )
  {
    matrix3<T> result( constant / m[0],
            constant / m[1],
            constant / m[2] );
    return result;
  }

  template <typename T>
  inline vector3<T> & matrix3<T> ::
  operator[] ( unsigned int i )
  {
    return cols[i];
  }

  template <typename T>
  inline vector3<T> matrix3<T> ::
  operator[] ( unsigned int i ) const
  {
    return cols[i];
  }


  // MATRIX 4

  template <typename T>
  matrix4<T>::
  matrix4 ( bool intialize )
  {
    if ( intialize )
    {
      this->data[ 0] = (T)1.0;
      this->data[ 1] = (T)0.0;
      this->data[ 2] = (T)0.0;
      this->data[ 3] = (T)0.0;

      this->data[ 4] = (T)0.0;
      this->data[ 5] = (T)1.0;
      this->data[ 6] = (T)0.0;
      this->data[ 7] = (T)0.0;

      this->data[ 8] = (T)0.0;
      this->data[ 9] = (T)0.0;
      this->data[10] = (T)1.0;
      this->data[11] = (T)0.0;

      this->data[12] = (T)0.0;
      this->data[13] = (T)0.0;
      this->data[14] = (T)0.0;
      this->data[15] = (T)1.0;
    }
  }

  template <typename T>
  matrix4<T>::
  matrix4 ( const T c )
  {
    for ( int i = 0 ; i < 16 ; ++i )
    {
      data[i] = c;
    }
  }

  template <typename T>
  matrix4<T>::
  matrix4 ( const T * other )
  {
    for ( int i = 0 ; i < 16 ; ++i )
    {
      data[i] = other[i];
    }
  }

  template <typename T>
  matrix4<T>::
  matrix4 ( const matrix2<T> & other )
  {
    this->data[ 0] = other.data[0];
    this->data[ 1] = other.data[1];
    this->data[ 2] = (T)0.0;
    this->data[ 3] = (T)0.0;
    this->data[ 4] = other.data[2];
    this->data[ 5] = other.data[3];
    this->data[ 6] = (T)0.0;
    this->data[ 7] = (T)0.0;
    this->data[ 8] = (T)0.0;
    this->data[ 9] = (T)0.0;
    this->data[10] = (T)1.0;
    this->data[11] = (T)0.0;
    this->data[12] = (T)0.0;
    this->data[13] = (T)0.0;
    this->data[14] = (T)0.0;
    this->data[15] = (T)1.0;
  }

  template <typename T>
  matrix4<T>::
  matrix4 ( const matrix3<T> & other )
  {
    this->data[ 0] = other.data[0];
    this->data[ 1] = other.data[1];
    this->data[ 2] = other.data[2];
    this->data[ 3] = (T)0.0;
    this->data[ 4] = other.data[3];
    this->data[ 5] = other.data[4];
    this->data[ 6] = other.data[5];
    this->data[ 7] = (T)0.0;
    this->data[ 8] = other.data[6];
    this->data[ 9] = other.data[7];
    this->data[10] = other.data[8];
    this->data[11] = (T)0.0;
    this->data[12] = (T)0.0;
    this->data[13] = (T)0.0;
    this->data[14] = (T)0.0;
    this->data[15] = (T)1.0;
  }

  template <typename T>
  matrix4<T>::
  matrix4 ( const matrix4<T> & other )
  {
    this->data[ 0] = other.data[ 0];
    this->data[ 1] = other.data[ 1];
    this->data[ 2] = other.data[ 2];
    this->data[ 3] = other.data[ 3];
    this->data[ 4] = other.data[ 4];
    this->data[ 5] = other.data[ 5];
    this->data[ 6] = other.data[ 6];
    this->data[ 7] = other.data[ 7];
    this->data[ 8] = other.data[ 8];
    this->data[ 9] = other.data[ 9];
    this->data[10] = other.data[10];
    this->data[11] = other.data[11];
    this->data[12] = other.data[12];
    this->data[13] = other.data[13];
    this->data[14] = other.data[14];
    this->data[15] = other.data[15];
  }

  template <typename T>
  matrix4<T>::
  matrix4 ( const vector4<T> & diagonal )
  {
    for ( int i = 0 ; i < 16 ; ++i )
    {
      data[i] = (T)0.0;
    }
    data[ 0]  = diagonal.x;
    data[ 5]  = diagonal.y;
    data[10] = diagonal.z;
    data[15] = diagonal.w;
  }

  template <typename T>
  matrix4<T>::
  matrix4 ( const vector4<T> & col0, const vector4<T> & col1, const vector4<T> & col2, const vector4<T> & col3 )
  {
    cols[0] = col0;
    cols[1] = col1;
    cols[2] = col2;
    cols[3] = col3;

  }

  template <typename T>
  inline matrix4<T> & matrix4<T>::
  operator= ( const matrix4<T> & other )
  {
    this->data[ 0] = other.data[ 0];
    this->data[ 1] = other.data[ 1];
    this->data[ 2] = other.data[ 2];
    this->data[ 3] = other.data[ 3];
    this->data[ 4] = other.data[ 4];
    this->data[ 5] = other.data[ 5];
    this->data[ 6] = other.data[ 6];
    this->data[ 7] = other.data[ 7];
    this->data[ 8] = other.data[ 8];
    this->data[ 9] = other.data[ 9];
    this->data[10] = other.data[10];
    this->data[11] = other.data[11];
    this->data[12] = other.data[12];
    this->data[13] = other.data[13];
    this->data[14] = other.data[14];
    this->data[15] = other.data[15];
    return *this;
  }

  template <typename T>
  inline void matrix4<T>::
  operator+= ( const matrix4<T> & other )
  {
    for ( int i = 0 ; i < 16 ; i += 4 )
    {
      this->data[i]   += other.data[i];
      this->data[i+1] += other.data[i+1];
      this->data[i+2] += other.data[i+2];
      this->data[i+3] += other.data[i+3];
    }
  }

  template <typename T>
  inline void matrix4<T>::
  operator+= ( const T constant )
  {
    for ( int i = 0 ; i < 16 ; i += 4 )
    {
      this->data[i]   += constant;
      this->data[i+1] += constant;
      this->data[i+2] += constant;
      this->data[i+3] += constant;
    }
  }

  template <typename T>
  inline matrix4<T>
  operator+ ( const matrix4<T> & m1, const matrix4<T> & m2 )
  {
    matrix4<T> result( m1[0] + m2[0],
            m1[1] + m2[1],
            m1[2] + m2[2],
            m1[3] + m2[3] );
    return result;
  }

  template <typename T>
  inline matrix4<T>
  operator+ ( const matrix4<T> & m, const T constant )
  {
    matrix4<T> result( m[0] + constant,
            m[1] + constant,
            m[2] + constant,
            m[3] + constant );
    return result;
  }

  template <typename T>
  inline matrix4<T>
  operator+ ( const T constant, const matrix4<T> & m )
  {
    matrix4<T> result( m[0] + constant,
            m[1] + constant,
            m[2] + constant,
            m[3] + constant );
    return result;
  }



  template <typename T>
  inline void matrix4<T>::
  operator-= ( const matrix4<T> & other )
  {
    for ( int i = 0 ; i < 16 ; i += 4 )
    {
      this->data[i]   -= other.data[i];
      this->data[i+1] -= other.data[i+1];
      this->data[i+2] -= other.data[i+2];
      this->data[i+3] -= other.data[i+3];
    }
  }

  template <typename T>
  inline void matrix4<T>::
  operator-= ( const T constant )
  {
    for ( int i = 0 ; i < 16 ; i += 4 )
    {
      this->data[i]   -= constant;
      this->data[i+1] -= constant;
      this->data[i+2] -= constant;
      this->data[i+3] -= constant;
    }
  }

  template <typename T>
  inline matrix4<T>
  operator- ( const matrix4<T> & m1, const matrix4<T> & m2 )
  {
    matrix4<T> result( m1[0] - m2[0],
            m1[1] - m2[1],
            m1[2] - m2[2],
            m1[3] - m2[3] );
    return result;
  }

  template <typename T>
  inline matrix4<T>
  operator- ( const matrix4<T> & m, const T constant )
  {
    matrix4<T> result( m[0] - constant,
            m[1] - constant,
            m[2] - constant,
            m[3] - constant );
    return result;
  }

  template <typename T>
  inline matrix4<T>
  operator- ( const T constant, const matrix4<T> & m )
  {
    matrix4<T> result( constant - m[0],
            constant - m[1],
            constant - m[2],
            constant - m[3] );
    return result;
  }

  template <typename T>
  inline matrix4<T>
  operator- ( const matrix4<T> & m )
  {
    matrix4<T> result( -m[0],
            -m[1],
            -m[2],
            -m[3] );
    return result;
  }

  template <typename T>
  inline void matrix4<T>::
  operator*= ( const matrix4<T> & other )
  {
    T vals[16];

    vals[ 0] = other.data[ 0] * data[ 0] + other.data[ 1] * data[ 4] +
               other.data[ 2] * data[ 8] + other.data[ 3] * data[12];
    vals[ 1] = other.data[ 0] * data[ 1] + other.data[ 1] * data[ 5] +
               other.data[ 2] * data[ 9] + other.data[ 3] * data[13];
    vals[ 2] = other.data[ 0] * data[ 2] + other.data[ 1] * data[ 6] +
               other.data[ 2] * data[10] + other.data[ 3] * data[14];
    vals[ 3] = other.data[ 0] * data[ 3] + other.data[ 1] * data[ 7] +
               other.data[ 2] * data[11] + other.data[ 3] * data[15];

    vals[ 4] = other.data[ 4] * data [0] + other.data[ 5] * data[ 4] +
               other.data[ 6] * data[ 8] + other.data[ 7] * data[12];
    vals[ 5] = other.data[ 4] * data [1] + other.data[ 5] * data[ 5] +
               other.data[ 6] * data[ 9] + other.data[ 7] * data[13];
    vals[ 6] = other.data[ 4] * data[ 2] + other.data[ 5] * data[ 6] +
               other.data[ 6] * data[10] + other.data[ 7] * data[14];
    vals[ 7] = other.data[ 4] * data[ 3] + other.data[ 5] * data[ 7] +
               other.data[ 6] * data[11] + other.data[ 7] * data[15];

    vals[ 8] = other.data[ 8] * data [0] + other.data[ 9] * data[ 4] +
               other.data[10] * data[ 8] + other.data[11] * data[12];
    vals[ 9] = other.data[ 8] * data[ 1] + other.data[ 9] * data[ 5] +
               other.data[10] * data[ 9] + other.data[11] * data[13];
    vals[10] = other.data[ 8] * data[ 2] + other.data[ 9] * data[ 6] +
               other.data[10] * data[10] + other.data[11] * data[14];
    vals[11] = other.data[ 8] * data[ 3] + other.data[ 9] * data[ 7] +
               other.data[10] * data[11] + other.data[11] * data[15];

    vals[12] = other.data[12] * data[ 0] + other.data[13] * data[ 4] +
               other.data[14] * data[ 8] + other.data[15] * data[12];
    vals[13] = other.data[12] * data[ 1] + other.data[13] * data[ 5] +
               other.data[14] * data[ 9] + other.data[15] * data[13];
    vals[14] = other.data[12] * data[ 2] + other.data[13] * data[ 6] +
               other.data[14] * data[10] + other.data[15] * data[14];
    vals[15] = other.data[12] * data[ 3] + other.data[13] * data[ 7] +
               other.data[14] * data[11] + other.data[15] * data[15];

    for ( int i = 0 ; i < 16 ; i += 4 )
    {
      this->data[i]   = vals[i]  ;
      this->data[i+1] = vals[i+1];
      this->data[i+2] = vals[i+2];
      this->data[i+3] = vals[i+3];
    }
  }

  template <typename T>
  inline void matrix4<T>::
  operator*= ( const T constant )
  {
    for ( int i = 0 ; i < 16 ; i += 4 )
    {
      this->data[i]   *= constant;
      this->data[i+1] *= constant;
      this->data[i+2] *= constant;
      this->data[i+3] *= constant;
    }
  }

  template <typename T>
  inline matrix4<T>
  operator* ( const matrix4<T> & m1, const matrix4<T> & m2 )
  {
    T vals[16];

    vals[ 0] = m2.data[ 0] * m1.data[ 0] + m2.data[ 1] * m1.data[ 4] +
               m2.data[ 2] * m1.data[ 8] + m2.data[ 3] * m1.data[12];
    vals[ 1] = m2.data[ 0] * m1.data[ 1] + m2.data[ 1] * m1.data[ 5] +
               m2.data[ 2] * m1.data[ 9] + m2.data[ 3] * m1.data[13];
    vals[ 2] = m2.data[ 0] * m1.data[ 2] + m2.data[ 1] * m1.data[ 6] +
               m2.data[ 2] * m1.data[10] + m2.data[ 3] * m1.data[14];
    vals[ 3] = m2.data[ 0] * m1.data[ 3] + m2.data[ 1] * m1.data[ 7] +
               m2.data[ 2] * m1.data[11] + m2.data[ 3] * m1.data[15];

    vals[ 4] = m2.data[ 4] * m1.data[ 0] + m2.data[ 5] * m1.data[ 4] +
               m2.data[ 6] * m1.data[ 8] + m2.data[ 7] * m1.data[12];
    vals[ 5] = m2.data[ 4] * m1.data[ 1] + m2.data[ 5] * m1.data[ 5] +
               m2.data[ 6] * m1.data[ 9] + m2.data[ 7] * m1.data[13];
    vals[ 6] = m2.data[ 4] * m1.data[ 2] + m2.data[ 5] * m1.data[ 6] +
               m2.data[ 6] * m1.data[10] + m2.data[ 7] * m1.data[14];
    vals[ 7] = m2.data[ 4] * m1.data[ 3] + m2.data[ 5] * m1.data[ 7] +
               m2.data[ 6] * m1.data[11] + m2.data[ 7] * m1.data[15];

    vals[ 8] = m2.data[ 8] * m1.data[ 0] + m2.data[ 9] * m1.data[ 4] +
               m2.data[10] * m1.data[ 8] + m2.data[11] * m1.data[12];
    vals[ 9] = m2.data[ 8] * m1.data[ 1] + m2.data[ 9] * m1.data[ 5] +
               m2.data[10] * m1.data[ 9] + m2.data[11] * m1.data[13];
    vals[10] = m2.data[ 8] * m1.data[ 2] + m2.data[ 9] * m1.data[ 6] +
               m2.data[10] * m1.data[10] + m2.data[11] * m1.data[14];
    vals[11] = m2.data[ 8] * m1.data[ 3] + m2.data[ 9] * m1.data[ 7] +
               m2.data[10] * m1.data[11] + m2.data[11] * m1.data[15];

    vals[12] = m2.data[12] * m1.data[ 0] + m2.data[13] * m1.data[ 4] +
               m2.data[14] * m1.data[ 8] + m2.data[15] * m1.data[12];
    vals[13] = m2.data[12] * m1.data[ 1] + m2.data[13] * m1.data[ 5] +
               m2.data[14] * m1.data[ 9] + m2.data[15] * m1.data[13];
    vals[14] = m2.data[12] * m1.data[ 2] + m2.data[13] * m1.data[ 6] +
               m2.data[14] * m1.data[10] + m2.data[15] * m1.data[14];
    vals[15] = m2.data[12] * m1.data[ 3] + m2.data[13] * m1.data[ 7] +
               m2.data[14] * m1.data[11] + m2.data[15] * m1.data[15];

    matrix4<T> result( vals );
    return result;
  }

  template <typename T>
  inline matrix4<T>
  operator* ( const matrix4<T> & m, const T constant )
  {
    matrix4<T> result( m[0] * constant,
            m[1] * constant,
            m[2] * constant,
            m[3] * constant );
    return result;
  }

  template <typename T>
  inline matrix4<T>
  operator* ( const T constant, const matrix4<T> & m )
  {
    matrix4<T> result( m[0] * constant,
            m[1] * constant,
            m[2] * constant,
            m[3] * constant );
    return result;
  }
// TODO: this might be wrong - need to test it with rotations etc.
  template <typename T>
  inline vector4<T>
  operator* ( const matrix4<T> & m, const vector4<T> & v )
  {
    vector4<T> output;

    output.x = m.data[0] * v.x + m.data[4] * v.y + m.data[ 8] * v.z + m.data[12] * v.w;
    output.y = m.data[1] * v.x + m.data[5] * v.y + m.data[ 9] * v.z + m.data[13] * v.w;
    output.z = m.data[2] * v.x + m.data[6] * v.y + m.data[10] * v.z + m.data[14] * v.w;
    output.w = m.data[3] * v.x + m.data[7] * v.y + m.data[11] * v.z + m.data[15] * v.w;

    return output;
  }

  template <typename T>
  inline void matrix4<T>::
  pointwiseMultiply ( const matrix4<T> & other )
  {
    for ( int i = 0 ; i < 16 ; i += 4 )
    {
      this->data[i]   *= other.data[i];
      this->data[i+1] *= other.data[i+1];
      this->data[i+2] *= other.data[i+2];
      this->data[i+3] *= other.data[i+3];
    }
  }


  template <typename T>
  inline void matrix4<T>::
  operator/= ( const matrix4<T> & other )
  {
    matrix4<T> inv_other = Inverse( other );
    (*this) *= inv_other;
  }

  template <typename T>
  inline void matrix4<T>::
  operator/= ( const T constant )
  {
    float inv_constant = (T)1.0 / constant;
    for ( int i = 0 ; i < 16 ; i += 4 )
    {
      this->data[i]   *= inv_constant;
      this->data[i+1] *= inv_constant;
      this->data[i+2] *= inv_constant;
      this->data[i+3] *= inv_constant;
    }
  }

  template <typename T>
  inline matrix4<T>
  operator/ ( const matrix4<T> & m1, const matrix4<T> & m2 )
  {
    matrix4<T> result( m1 );
    result /= m2;
    return result;
  }

  template <typename T>
  inline matrix4<T>
  operator/ ( const matrix4<T> & m, const T constant )
  {
    float inv_constant = (T)1.0 / constant;
    matrix4<T> result( m[0] * inv_constant,
            m[1] * inv_constant,
            m[2] * inv_constant,
            m[3] * inv_constant );
    return result;
  }

  template <typename T>
  inline matrix4<T>
  operator/ ( const T constant, const matrix4<T> & m )
  {
    matrix4<T> result( constant / m[0],
            constant / m[1],
            constant / m[2],
            constant / m[3] );
    return result;
  }

  template <typename T>
  inline vector4<T> & matrix4<T>::
  operator[] ( unsigned int i )
  {
    return cols[i];
  }

  template <typename T>
  inline vector4<T> matrix4<T>::
  operator[] ( unsigned int i ) const
  {
    return cols[i];
  }

  // GENERAL FUNCTIONS

  //---------------------------------------------------
  // Implementation
  //---------------------------------------------------


  template <typename T>
  inline T
  compute_determinant ( const matrix2<T> & matrix )
  {
    return ( matrix.data[0] * matrix.data[3] - matrix.data[2] * matrix.data[1] );
  }

  template <typename T>
  inline T
  compute_determinant ( const matrix3<T> & matrix )
  {
    // code folows same intuition as inverse calculation for 3d matrices,
    // however we only compute informatrixion needed
    // for determinant calculation.

    float C[3];

    // get required cofactors
    C[0] = matrix.data[4] * matrix.data[8] - matrix.data[5] * matrix.data[7];
    C[1] = matrix.data[5] * matrix.data[6] - matrix.data[3] * matrix.data[8]; // negated
    C[2] = matrix.data[3] * matrix.data[7] - matrix.data[4] * matrix.data[6];

    return ( matrix.data[0] * C[0] + matrix.data[1] * C[1] + matrix.data[2] * C[2] );
  }

  template <typename T>
  T compute_determinant ( const matrix4<T> & matrix )
  {
    // code folows same intuition as inverse calculation for 4d matrices,
    // however we only compute informatrixion needed
    // for determinant calculation.
    float C[4];
    float coeffs[6];

    // coeffs are determinants of 2x2 matrices
    coeffs[0] = matrix.data[10] * matrix.data[15] - matrix.data[14] * matrix.data[11];
    coeffs[1] = matrix.data[ 6] * matrix.data[11] - matrix.data[10] * matrix.data[ 7];
    coeffs[2] = matrix.data[ 2] * matrix.data[ 7] - matrix.data[ 6] * matrix.data[ 3];
    coeffs[3] = matrix.data[ 6] * matrix.data[15] - matrix.data[14] * matrix.data[ 7];
    coeffs[4] = matrix.data[ 2] * matrix.data[11] - matrix.data[10] * matrix.data[ 3];
    coeffs[5] = matrix.data[ 2] * matrix.data[15] - matrix.data[14] * matrix.data[ 3];

    // Cofactor matrix
    /*00*/ C[0] = matrix.data[ 5] * coeffs[0] -
                  matrix.data[ 9] * coeffs[3] +
                  matrix.data[13] * coeffs[1];
    /*01*/ C[1] = matrix.data[ 9] * coeffs[5] -
                  matrix.data[ 1] * coeffs[0] -
                  matrix.data[13] * coeffs[4]; // negated
    /*02*/ C[2] = matrix.data[ 1] * coeffs[3] -
                  matrix.data[ 5] * coeffs[5] +
                  matrix.data[13] * coeffs[2];
    /*03*/ C[3] = matrix.data[ 5] * coeffs[4] -
                  matrix.data[ 9] * coeffs[2] -
                  matrix.data[ 1] * coeffs[1]; // negated

    // determinant
    T det = matrix.data[0] * C[0] + matrix.data[4] * C[1] + matrix.data[8] * C[2] + matrix.data[12] * C[3];

    return det;
  }

  template< typename T, template <typename Q> class matrixX >
  inline T
  determinant ( const matrixX<T> & matrix )
  {
    return compute_determinant ( matrix );
  }




  template< typename T, template <typename Q> class matrixX >
  inline T
  trace ( const matrixX<T> & matrix )
  {
    T trace = (T)0;
    for ( int i = 0 ; i < matrix.Dimensionality() ; ++i )
    {
      trace += matrix.cols[i][i];
    }
    return trace;
  }


  template< typename T >
  inline matrix2<T>
  compute_transpose ( const matrix2<T> & src )
  {
    matrix2<T> trg ( false );
    trg[0][0] = src[0][0];
    trg[0][1] = src[1][0];
    trg[1][0] = src[0][1];
    trg[1][1] = src[1][1];
    return trg;
  }

  template< typename T >
  inline matrix3<T>
  compute_transpose ( const matrix3<T> & src )
  {
    matrix3<T> trg ( false );
    trg[0][0] = src[0][0];
    trg[0][1] = src[1][0];
    trg[0][2] = src[2][0];

    trg[1][0] = src[0][1];
    trg[1][1] = src[1][1];
    trg[1][2] = src[2][1];

    trg[2][0] = src[0][2];
    trg[2][1] = src[1][2];
    trg[2][2] = src[2][2];

    return trg;
  }

  template< typename T >
  inline matrix4<T>
  compute_transpose ( const matrix4<T> & src )
  {
    matrix4<T> trg ( false );
    trg[0][0] = src[0][0];
    trg[0][1] = src[1][0];
    trg[0][2] = src[2][0];
    trg[0][3] = src[3][0];

    trg[1][0] = src[0][1];
    trg[1][1] = src[1][1];
    trg[1][2] = src[2][1];
    trg[1][3] = src[3][1];

    trg[2][0] = src[0][2];
    trg[2][1] = src[1][2];
    trg[2][2] = src[2][2];
    trg[2][3] = src[3][2];

    trg[3][0] = src[0][3];
    trg[3][1] = src[1][3];
    trg[3][2] = src[2][3];
    trg[3][3] = src[3][3];

    return trg;
  }

  template< typename T, template <typename Q> class matrixX >
  inline matrixX<T>
  transpose ( const matrixX<T> & matrix )
  {
    return compute_transpose( matrix );
  }


  template <typename T>
  void
  compute_inverse ( matrix2<T> & matrix )
  {
    T one_over_det = ((T)1.0) / Determinant( matrix );

    T vals[4];
    vals[0] =  matrix.data[3];
    vals[1] = -matrix.data[1];
    vals[2] = -matrix.data[2];
    vals[3] =  matrix.data[0];

    matrix.data[0] = one_over_det * vals[0];
    matrix.data[1] = one_over_det * vals[1];
    matrix.data[2] = one_over_det * vals[2];
    matrix.data[3] = one_over_det * vals[3];
  }

  template <typename T>
  void
  compute_inverse ( matrix3<T> & matrix )
  {
    // To calculate inverse :
    //     1. Transpose M
    //     2. Calculate cofactor matrix C
    //     3. Caluclate determinant of M
    //     4. Inverse is given as (1/det) * C

    // Access cheat sheat for transpose matrix:
    //     original indices
    //      0  1  2
    //      3  4  5
    //      6  7  8

    //     transposed indices
    //      0  3  6
    //      1  4  7
    //      2  5  8

    // Calulate cofactor matrix
    float C[9];

    C[0] = matrix.data[4] * matrix.data[8] - matrix.data[7] * matrix.data[5];
    C[1] = matrix.data[7] * matrix.data[2] - matrix.data[1] * matrix.data[8]; // negated
    C[2] = matrix.data[1] * matrix.data[5] - matrix.data[4] * matrix.data[2];

    C[3] = matrix.data[6] * matrix.data[5] - matrix.data[3] * matrix.data[8]; // negated
    C[4] = matrix.data[0] * matrix.data[8] - matrix.data[6] * matrix.data[2];
    C[5] = matrix.data[3] * matrix.data[2] - matrix.data[0] * matrix.data[5]; // negated

    C[6] = matrix.data[3] * matrix.data[7] - matrix.data[6] * matrix.data[4];
    C[7] = matrix.data[6] * matrix.data[1] - matrix.data[0] * matrix.data[7]; // negated
    C[8] = matrix.data[0] * matrix.data[4] - matrix.data[3] * matrix.data[1];

    // determinant
    T det = matrix.data[0] * C[0] + matrix.data[3] * C[1] + matrix.data[6] * C[2];
    T one_over_det = (T)1.0 / det;

    // store result
    matrix.data[ 0] = one_over_det * C[ 0];
    matrix.data[ 1] = one_over_det * C[ 1];
    matrix.data[ 2] = one_over_det * C[ 2];
    matrix.data[ 3] = one_over_det * C[ 3];
    matrix.data[ 4] = one_over_det * C[ 4];
    matrix.data[ 5] = one_over_det * C[ 5];
    matrix.data[ 6] = one_over_det * C[ 6];
    matrix.data[ 7] = one_over_det * C[ 7];
    matrix.data[ 8] = one_over_det * C[ 8];
  }

  template <typename T>
  void
  compute_inverse ( matrix4<T> & matrix )
  {
    // TODO(maciej) : find intel document that was describing that
    // Inverse using cramers rule
    //     1. Transpose  M
    //     2. Calculate cofactor matrix C
    //     3. Caluclate determinant of M
    //     4. Inverse is given as (1/det) * C

    // Access cheat sheat:
    //     original indices
    //      0  1  2  3
    //      4  5  6  7
    //      8  9 10 11
    //     12 13 14 15

    //     transposed indices
    //      0  4  8 12
    //      1  5  9 13
    //      2  6 10 14
    //      3  7 11 15


    // Calulate cofactor matrix
    float C[16];
    float coeffs[6];

    // First 8
    // coeffs are determinants of 2x2 matrices
    coeffs[0] = matrix.data[10] * matrix.data[15] - matrix.data[14] * matrix.data[11];
    coeffs[1] = matrix.data[ 6] * matrix.data[11] - matrix.data[10] * matrix.data[ 7];
    coeffs[2] = matrix.data[ 2] * matrix.data[ 7] - matrix.data[ 6] * matrix.data[ 3];
    coeffs[3] = matrix.data[ 6] * matrix.data[15] - matrix.data[14] * matrix.data[ 7];
    coeffs[4] = matrix.data[ 2] * matrix.data[11] - matrix.data[10] * matrix.data[ 3];
    coeffs[5] = matrix.data[ 2] * matrix.data[15] - matrix.data[14] * matrix.data[ 3];

    // Cofactor matrix
    /*00*/ C[0] = matrix.data[ 5] * coeffs[0] -
                  matrix.data[ 9] * coeffs[3] +
                  matrix.data[13] * coeffs[1];
    /*01*/ C[1] = matrix.data[ 9] * coeffs[5] -
                  matrix.data[ 1] * coeffs[0] -
                  matrix.data[13] * coeffs[4]; // negated
    /*02*/ C[2] = matrix.data[ 1] * coeffs[3] -
                  matrix.data[ 5] * coeffs[5] +
                  matrix.data[13] * coeffs[2];
    /*03*/ C[3] = matrix.data[ 5] * coeffs[4] -
                  matrix.data[ 9] * coeffs[2] -
                  matrix.data[ 1] * coeffs[1]; // negated

    /*10*/ C[4] = matrix.data[ 8] * coeffs[3] -
                  matrix.data[ 4] * coeffs[0] -
                  matrix.data[12] * coeffs[1]; // negated
    /*11*/ C[5] = matrix.data[ 0] * coeffs[0] -
                  matrix.data[ 8] * coeffs[5] +
                  matrix.data[12] * coeffs[4];
    /*12*/ C[6] = matrix.data[ 4] * coeffs[5] -
                  matrix.data[ 0] * coeffs[3] -
                  matrix.data[12] * coeffs[2]; // negated
    /*13*/ C[7] = matrix.data[ 0] * coeffs[1] -
                  matrix.data[ 4] * coeffs[4] +
                  matrix.data[ 8] * coeffs[2];

    //Second 8

    // coeffs are Determinants of 2x2 matrices
    coeffs[0] = matrix.data[ 8] * matrix.data[13] - matrix.data[12] * matrix.data[ 9];
    coeffs[1] = matrix.data[ 4] * matrix.data[ 9] - matrix.data[ 8] * matrix.data[ 5];
    coeffs[2] = matrix.data[ 0] * matrix.data[ 5] - matrix.data[ 4] * matrix.data[ 1];
    coeffs[3] = matrix.data[ 4] * matrix.data[13] - matrix.data[12] * matrix.data[ 5];
    coeffs[4] = matrix.data[ 0] * matrix.data[ 9] - matrix.data[ 8] * matrix.data[ 1];
    coeffs[5] = matrix.data[ 0] * matrix.data[13] - matrix.data[12] * matrix.data[ 1];

    // actual coefficient matrix
    /*20*/ C[ 8] = matrix.data[ 7] * coeffs[0] -
                  matrix.data[11] * coeffs[3] +
                  matrix.data[15] * coeffs[1];
    /*21*/ C[ 9] = matrix.data[11] * coeffs[5] -
                  matrix.data[ 3] * coeffs[0] -
                  matrix.data[15] * coeffs[4]; // negated
    /*22*/ C[10] = matrix.data[ 3] * coeffs[3] -
                  matrix.data[ 7] * coeffs[5] +
                  matrix.data[15] * coeffs[2];
    /*23*/ C[11] = matrix.data[ 7] * coeffs[4] -
                  matrix.data[ 3] * coeffs[1] -
                  matrix.data[11] * coeffs[2]; // negated

    /*30*/ C[12] = matrix.data[10] * coeffs[3] -
                  matrix.data[ 6] * coeffs[0] -
                  matrix.data[14] * coeffs[1]; // negated
    /*31*/ C[13] = matrix.data[ 2] * coeffs[0] -
                  matrix.data[10] * coeffs[5] +
                  matrix.data[14] * coeffs[4];
    /*32*/ C[14] = matrix.data[ 6] * coeffs[5] -
                  matrix.data[ 2] * coeffs[3] -
                  matrix.data[14] * coeffs[2]; // negated
    /*33*/ C[15] = matrix.data[ 2] * coeffs[1] -
                  matrix.data[ 6] * coeffs[4] +
                  matrix.data[10] * coeffs[2];

    // determinant
    T det = matrix.data[0] * C[0] + matrix.data[ 4] * C[1] +
            matrix.data[8] * C[2] + matrix.data[12] * C[3];
    T one_over_det = ((T)1) / det;

    // store result
    matrix.data[ 0] = one_over_det * C[ 0];
    matrix.data[ 1] = one_over_det * C[ 1];
    matrix.data[ 2] = one_over_det * C[ 2];
    matrix.data[ 3] = one_over_det * C[ 3];
    matrix.data[ 4] = one_over_det * C[ 4];
    matrix.data[ 5] = one_over_det * C[ 5];
    matrix.data[ 6] = one_over_det * C[ 6];
    matrix.data[ 7] = one_over_det * C[ 7];
    matrix.data[ 8] = one_over_det * C[ 8];
    matrix.data[ 9] = one_over_det * C[ 9];
    matrix.data[10] = one_over_det * C[10];
    matrix.data[11] = one_over_det * C[11];
    matrix.data[12] = one_over_det * C[12];
    matrix.data[13] = one_over_det * C[13];
    matrix.data[14] = one_over_det * C[14];
    matrix.data[15] = one_over_det * C[15];

  }

  template< typename T, template <typename Q> class matrixX >
  inline matrixX<T>
  inverse ( const matrixX<T> & matrix )
  {
    static_assert( std::numeric_limits<T>::is_iec559,
                  "'Inverse' only accepts real numbers!" );
    matrixX<T> matrix_copy ( matrix );
    compute_inverse( matrix_copy );
    return matrix_copy;
  }

  template< typename T>
  inline matrix4<T>
  frustum ( const T & left, const T & right, const T & bottom, const T & top, const T & z_near, const T & z_far )
  {
    T x_diff = right - left;
    T y_diff = top - bottom;
    T z_diff = z_far - z_near;
    T a = (right + left) / x_diff;
    T b = (top + bottom) / y_diff;
    T c = -(z_far + z_near ) / z_diff;
    T d = -(2 * z_far * z_near ) / z_diff;

    T tmp[16] = { 
      ((T)2 * z_near) / x_diff,                      0.0, 0.0,  0.0,
                           0.0, ((T)2 * z_near) / y_diff, 0.0,  0.0,
                             a,                        b,   c, -1.0,
                           0.0,                      0.0,   d,  0.0 };
    return  matrix4<T>( tmp );
  }

  template< typename T>
  inline matrix4<T>
  perspective ( const T & fovy, const T & aspect, const T & z_near, const T & z_far)
  {
    T xmin, xmax, ymin, ymax;

    ymax = z_near * tanf( fovy * T(0.5) );
    ymin = -ymax;

    xmin = ymin * aspect;
    xmax = ymax * aspect;

    return  frustum( xmin, xmax, ymin, ymax, z_near, z_far );
  }


  template< typename T>
  inline matrix4<T>
  ortho ( const T & left, const T & right, const T & bottom, const T & top, const T & z_near, const T & z_far )
  {
    T x_diff = right - left;
    T y_diff = top - bottom;
    T z_diff = z_far - z_near;
    T tx = - ( right + left ) / x_diff;
    T ty = - ( top + bottom ) / y_diff;
    T tz = - ( z_near + z_far ) / z_diff;

    T tmp[16] =  { (T)2 / x_diff,           0.0,            0.0, 0.0,
                            0.0, (T)2 / y_diff,            0.0, 0.0,
                            0.0,           0.0, -(T)2 / z_diff, 0.0,
                              tx,            ty,             tz, 1.0 } ;
    return  matrix4<T>( tmp );
  }


  template< typename T>
  inline matrix4<T>
  look_at ( const vector3<T> & eye, const vector3<T> & center, const vector3<T> & up )
  {
    vector3<T> z = normalize(  eye - center );
    vector3<T> x = normalize( cross( up, z ) );
    vector3<T> y = normalize( cross( z, x ) );

    T tmp[16] = {         x[0],          y[0],        z[0], 0.0,
                          x[1],          y[1],        z[1], 0.0,
                          x[2],          y[2],        z[2], 0.0,
                  -dot(eye,x),   -dot(eye,y),  -dot(eye,z), 1.0 };


    return matrix4<T>( tmp );
  }


  template< typename T>
  inline vector3<T>
  project ( const vector4<T> & obj, const matrix4<T> & modelview,
            const matrix4<T> & project, const vector4<T> & viewport )
  {
    vector4<T> tmp = project * modelview * obj;
    tmp /= tmp.w;

    vector3<T> win;
    win.x = viewport[0] + ( viewport[2] * (tmp.x + 1.0) ) / 2.0;
    win.y = viewport[1] + ( viewport[3] * (tmp.y + 1.0) ) / 2.0;
    win.z = (tmp.z + 1.0) / 2.0;

    return win;
  }

  template< typename T>
  inline vector4<T>
  unproject ( const vector3<T> &win, const matrix4<T> & modelview,
              const matrix4<T> & project, const vector4<T> & viewport )
  {
    matrix4<T> inv_pm = inverse( project * modelview );
    vector4<T> tmp;
    tmp.x = ( 2.0 * ( win.x - viewport[0] ) ) / viewport[2] - 1.0;
    tmp.y = ( 2.0 * ( win.y - viewport[1] ) ) / viewport[3] - 1.0;
    tmp.z = 2.0 * win.z - 1.0;
    tmp.w = 1.0;

    vector4<T> obj = inv_pm * tmp;
    obj /= obj.w;
    return obj;
  }

  template< typename T>
  inline matrix4<T>
  translate( const matrix4<T> & m, const vector3<T> & t )
  {
    matrix4<T> result( m );
    result[3] = m[0] * t.x + m[1] * t.y + m[2] * t.z + m[3];
    return result;
  }

  template< typename T>
  inline matrix4<T>
  scale( const matrix4<T> & m, const vector3<T> & s )
  {
    matrix4<T> result( m );
    result[0] *= s.x;
    result[1] *= s.y;
    result[2] *= s.z;
    return result;
  }

  // derivation : http://www.euclideanspace.com/matrixhs/geometry/rotations/conversions/angleToMatrix/
  template< typename T>
  matrix4<T>
  rotate( const matrix4<T> & m, const T & angle, const vector3<T> & v )
  {
    T c = cos ( angle );
    T s = sin ( angle );
    T t = 1.0 - c;

    vector3<T> axis = normalize( v );

    matrix4<T> rotate( false );
    rotate[0][0] = c + axis.x * axis.x * t;
    rotate[1][1] = c + axis.y * axis.y * t;
    rotate[2][2] = c + axis.z * axis.z * t;

    T tmp_1 = axis.x * axis.y * t;
    T tmp_2 = axis.z * s;

    rotate[0][1] = tmp_1 + tmp_2;
    rotate[1][0] = tmp_1 - tmp_2;

    tmp_1 = axis.x * axis.z * t;
    tmp_2 = axis.y * s;

    rotate[0][2] = tmp_1 - tmp_2;
    rotate[2][0] = tmp_1 + tmp_2;

    tmp_1 = axis.y * axis.z * t;
    tmp_2 = axis.x * s;

    rotate[1][2] = tmp_1 + tmp_2;
    rotate[2][1] = tmp_1 - tmp_2;

    matrix4<T> result( m );
    result[0] = m[0] * rotate[0][0] + m[1] * rotate[0][1] + m[2] * rotate[0][2];
    result[1] = m[0] * rotate[1][0] + m[1] * rotate[1][1] + m[2] * rotate[1][2];
    result[2] = m[0] * rotate[2][0] + m[1] * rotate[2][1] + m[2] * rotate[2][2];
    result[3] = m[3];
    return result;
  }

////////////////////////////////////////////////////////////////////////////////
// Eigendecomposition
////////////////////////////////////////////////////////////////////////////////
  // NOTE: Following code is taken from files distributed by Conelly Barnes

//   namespace eig_decomposition
//   {
//     static double 
//     hypot2( double x, double y ) {
//       return sqrt(x*x+y*y);
//     }

//     // Symmetric Householder reduction to tridiagonal form.

//     static void 
//     tred2( double V[3][3], double d[3], double e[3] ) 
//     {

//     //  This is derived from the Algol procedures tred2 by
//     //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
//     //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
//     //  Fortran subroutine in EISPACK.

//       int n = 3;

//       for (int j = 0; j < n; j++) {
//         d[j] = V[n-1][j];
//       }

//       // Householder reduction to tridiagonal form.

//       for (int i = n-1; i > 0; i--) {

//         // Scale to avoid under/overflow.

//         double scale = 0.0;
//         double h = 0.0;
//         for (int k = 0; k < i; k++) {
//           scale = scale + fabs(d[k]);
//         }
//         if (scale == 0.0) {
//           e[i] = d[i-1];
//           for (int j = 0; j < i; j++) {
//             d[j] = V[i-1][j];
//             V[i][j] = 0.0;
//             V[j][i] = 0.0;
//           }
//         } else {

//           // Generate Householder vector.

//           for (int k = 0; k < i; k++) {
//             d[k] /= scale;
//             h += d[k] * d[k];
//           }
//           double f = d[i-1];
//           double g = sqrt(h);
//           if (f > 0) {
//             g = -g;
//           }
//           e[i] = scale * g;
//           h = h - f * g;
//           d[i-1] = f - g;
//           for (int j = 0; j < i; j++) {
//             e[j] = 0.0;
//           }

//           // Apply similarity transformation to remaining columns.

//           for (int j = 0; j < i; j++) {
//             f = d[j];
//             V[j][i] = f;
//             g = e[j] + V[j][j] * f;
//             for (int k = j+1; k <= i-1; k++) {
//               g += V[k][j] * d[k];
//               e[k] += V[k][j] * f;
//             }
//             e[j] = g;
//           }
//           f = 0.0;
//           for (int j = 0; j < i; j++) {
//             e[j] /= h;
//             f += e[j] * d[j];
//           }
//           double hh = f / (h + h);
//           for (int j = 0; j < i; j++) {
//             e[j] -= hh * d[j];
//           }
//           for (int j = 0; j < i; j++) {
//             f = d[j];
//             g = e[j];
//             for (int k = j; k <= i-1; k++) {
//               V[k][j] -= (f * e[k] + g * d[k]);
//             }
//             d[j] = V[i-1][j];
//             V[i][j] = 0.0;
//           }
//         }
//         d[i] = h;
//       }

//       // Accumulate transformations.

//       for (int i = 0; i < n-1; i++) 
//       {
//         V[n-1][i] = V[i][i];
//         V[i][i] = 1.0;
//         double h = d[i+1];
//         if (h != 0.0) {
//           for (int k = 0; k <= i; k++) {
//             d[k] = V[k][i+1] / h;
//           }
//           for (int j = 0; j <= i; j++) {
//             double g = 0.0;
//             for (int k = 0; k <= i; k++) {
//               g += V[k][i+1] * V[k][j];
//             }
//             for (int k = 0; k <= i; k++) {
//               V[k][j] -= g * d[k];
//             }
//           }
//         }
//         for (int k = 0; k <= i; k++) {
//           V[k][i+1] = 0.0;
//         }
//       }
//       for (int j = 0; j < n; j++) {
//         d[j] = V[n-1][j];
//         V[n-1][j] = 0.0;
//       }
//       V[n-1][n-1] = 1.0;
//       e[0] = 0.0;
//     } 

//     // Symmetric tridiagonal QL algorithm.

//     static void 
//     tql2(double V[3][3], double d[3], double e[3]) {

//     //  This is derived from the Algol procedures tql2, by
//     //  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
//     //  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
//     //  Fortran subroutine in EISPACK.

//       int n = 3;

//       for (int i = 1; i < n; i++) {
//         e[i-1] = e[i];
//       }
//       e[n-1] = 0.0;

//       double f = 0.0;
//       double tst1 = 0.0;
//       double eps = pow(2.0,-52.0);
//       for (int l = 0; l < n; l++) {

//         // Find small subdiagonal element

//         tst1 = max(tst1,fabs(d[l]) + fabs(e[l]));
//         int m = l;
//         while (m < n) {
//           if (fabs(e[m]) <= eps*tst1) {
//             break;
//           }
//           m++;
//         }

//         // If m == l, d[l] is an eigenvalue,
//         // otherwise, iterate.

//         if (m > l) {
//           int iter = 0;
//           do {
//             iter = iter + 1;  // (Could check iteration count here.)

//             // Compute implicit shift

//             double g = d[l];
//             double p = (d[l+1] - g) / (2.0 * e[l]);
//             double r = hypot2(p,1.0);
//             if (p < 0) {
//               r = -r;
//             }
//             d[l] = e[l] / (p + r);
//             d[l+1] = e[l] * (p + r);
//             double dl1 = d[l+1];
//             double h = g - d[l];
//             for (int i = l+2; i < n; i++) {
//               d[i] -= h;
//             }
//             f = f + h;

//             // Implicit QL transformation.

//             p = d[m];
//             double c = 1.0;
//             double c2 = c;
//             double c3 = c;
//             double el1 = e[l+1];
//             double s = 0.0;
//             double s2 = 0.0;
//             for (int i = m-1; i >= l; i--) {
//               c3 = c2;
//               c2 = c;
//               s2 = s;
//               g = c * e[i];
//               h = c * p;
//               r = hypot2(p,e[i]);
//               e[i+1] = s * r;
//               s = e[i] / r;
//               c = p / r;
//               p = c * d[i] - s * g;
//               d[i+1] = h + s * (c * g + s * d[i]);

//               // Accumulate transformation.

//               for (int k = 0; k < n; k++) {
//                 h = V[k][i+1];
//                 V[k][i+1] = s * V[k][i] + c * h;
//                 V[k][i] = c * V[k][i] - s * h;
//               }
//             }
//             p = -s * s2 * c3 * el1 * e[l] / dl1;
//             e[l] = s * p;
//             d[l] = c * p;

//             // Check for convergence.

//           } while (fabs(e[l]) > eps*tst1);
//         }
//         d[l] = d[l] + f;
//         e[l] = 0.0;
//       }
      
//       // Sort eigenvalues and corresponding vectors.

//       for (int i = 0; i < n-1; i++) {
//         int k = i;
//         double p = d[i];
//         for (int j = i+1; j < n; j++) {
//           if (d[j] < p) {
//             k = j;
//             p = d[j];
//           }
//         }
//         if (k != i) {
//           d[k] = d[i];
//           d[i] = p;
//           for (int j = 0; j < n; j++) {
//             p = V[j][i];
//             V[j][i] = V[j][k];
//             V[j][k] = p;
//           }
//         }
//       }
//     }
//   }
  

//   void 
//   eig( const mat3f * mat, 
//        mat3f * eigvec,
//        vec3f * eigval )
//   {
//     bsc::stop_watch timer;
//     timer.start();
//     // copy data into their format
//     double V[3][3];
//     double d[3];
//     double e[3];
//     for ( int i = 0; i < 3 ; ++i )
//     {
//       for ( int j = 0; j < 3 ; ++j )
//       {
//         V[i][j] = (*mat)[i][j];
//       }
//     }
//     // perform the calculation
//     eig_decomposition::tred2( V, d, e );
//     eig_decomposition::tql2( V, d, e );

//     // copy data_back
//     for ( int i = 0; i < 3 ; ++i )
//     {
//       for ( int j = 0; j < 3 ; ++j )
//       {
//         (*eigvec)[j][i] = V[i][j];
//       }
//       (*eigval)[i] = d[i];
//     }
//     printf("Eigen decomposition took : %f\n", timer.elapsed());
//   }
// // need to experiment more. Does not work that great :<
//   mat4f
//   pca( const vec3f * points, const i32 n_points )
//   {
//     // centroid
//     bsc::vec3f centroid;
//     for ( i32 p_idx = 0 ; p_idx < n_points ; ++p_idx )
//     {
//       centroid += points[p_idx];
//     }
//     centroid /= (r32)n_points;

//     // covariance matrix
//     bsc::mat3f cov;
//     for (i32 p_idx = 0; p_idx < n_points; ++p_idx) {
//       bsc::vec3f v = points[p_idx] - centroid;

//       cov[0][0] += v.x*v.x;
//       cov[0][1] += v.x*v.y;
//       cov[0][2] += v.x*v.z;
    
//       cov[1][0] += v.y*v.x;
//       cov[1][1] += v.y*v.y;
//       cov[2][2] += v.y*v.z;
    
//       cov[2][0] += v.z*v.x;
//       cov[2][1] += v.z*v.y;
//       cov[2][2] += v.z*v.z;
//     }
//     cov /= (r32)n_points;

//     // eigen decomposition
//     bsc::mat3f evec;
//     bsc::vec3f eval;
//     bsc::eig( &cov, &evec, &eval );

//     // build matrix
//     bsc::mat4 ret;
//     ret[0] = bsc::vec4( evec[2], 0.0f );
//     ret[1] = bsc::vec4( evec[0], 0.0f );
//     ret[2] = bsc::vec4( evec[1], 0.0f );
//     ret[3] = bsc::vec4f( centroid, 1.0);

//     return ret;
//   }
}
