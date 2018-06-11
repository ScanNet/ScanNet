#pragma once

// NOTE: This image class assumes image with top-left as the origin. This
// is unlike OpenGL, which uses bottom-right, but is consistent with how image
// formats store this info. Possibly will need to change that if deemed necessary.
// Note that this implies left-handed coordinate system.

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////

#ifndef BSC_IMAGE_H
#define BSC_IMAGE_H

namespace bsc
{
  template <typename T>
  struct image
  {
    i32 width;
    i32 height;
    i32 ncomp;
    i32 rowsize;
    
    T * data;

    image( void );
    image( i32 w, i32 h, i32 ncomp );
    image( i32 w, i32 h, i32 ncomp, T * data );
    image( const image<T> &other );
    image( const char * filename );
    ~image();

    image<T> & operator= ( const image<T> & other);

    const T*  at( i32 i ) const;
    T* at( i32 i );
    const T*  at( i32 x, i32 y ) const;
    T* at( i32 x, i32 y );

    const T*  operator()( i32 i ) const;
    T* operator()( i32 i );
    const T*  operator()( i32 x, i32 y ) const;
    T* operator()( i32 x, i32 y );

    void resize( i32 w, i32 h );

    // TODO: those should be out of this struct
    void vert_flip();
    T maximum();
    T minimum();
    i32 read( const char * filename );
    i32 write( const char * filename );
  };

  typedef image<u8>  img;
  typedef image<u8>  img_u8;
  typedef image<u16> img_u16;
  typedef image<r32> img_r32;
}

#endif // BSC_IMAGE_H

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////
#ifdef BSC_IMPLEMENTATION

template <typename T>
bsc::image<T>::
image ( void ) : width( 0 ), 
                 height( 0 ), 
                 ncomp( 0 ),
                 rowsize( 0 ), 
                 data( nullptr )
{

}

template <typename T>
bsc::image<T>::
image ( i32 w, i32 h, i32 n ) : width( w ), 
                                height( h ),
                                ncomp( n )
{
  this->rowsize = n * w;
  this->data = (T*)calloc( rowsize * h, sizeof( T ) );
}

template <typename T>
bsc::image<T>::
image ( i32 w, i32 h, i32 n, T * d ) : width( w ), 
                                       height( h ), 
                                       ncomp( n ), 
                                       data( nullptr )
{
  this->rowsize = w * n;
  i32 mem_size = rowsize * h * sizeof( T );
  data = (T*)malloc( mem_size );
  memcpy(data, d, mem_size );
}

template <typename T>
bsc::image<T>::
image ( const bsc::image<T> & other ) : width( other.width ), 
                                        height( other.height ), 
                                        ncomp( other.ncomp ),
                                        rowsize( other.rowsize ),
                                        data( nullptr )
{
  i32 mem_size = width * height * ncomp * sizeof( T );
  data = (T*)malloc( mem_size );
  memcpy( data, other.data, mem_size );
} 

template <typename T>
bsc::image<T>::
image ( const char * filename ): width( 0 ), 
                                 height( 0 ), 
                                 ncomp( 0 ), 
                                 data( nullptr )
{
  read( filename );
}

template <typename T>
bsc::image<T>::
~image ()
{
  if ( this->data != nullptr ) free( data );
  this->data    = nullptr;
  this->width   = 0;
  this->height  = 0;
  this->rowsize = 0;
  this->ncomp   = 0;
}

template <typename T>
i32 
read_jpg( const char *filename, bsc::image<T> * img  )
{
  if ( sizeof(T) > 1 )
  {
    printf("Image reading warning -> JPEG do not support high precision!\n" );
  }
  // jpgs will be assumed to be 3 channel u8 images
  img->data = (T*)stbi_load( filename, &(img->width),
                                       &(img->height),
                                       &(img->ncomp),
                                       3 );
  // TODO: we might need to flip!
  // TODO: check what happens if you have single channel
  if ( img->data == NULL )
  {
    printf("Image reading error -> Could not read image %s\n", filename );
    return 0;
  }
  return 1;
}

template <typename T>
i32 
read_png( const char *filename, bsc::image<T> * img )
{
  u8 * file_contents = 0;
  size_t file_size;
  u32 lode_error;

  // load file
  lode_error = lodepng_load_file( &file_contents, &file_size, filename ); 
  if ( lode_error )
  {
    printf("Image reading error -> %s\n", lodepng_error_text( lode_error ) );
    return 0;
  }

  // inspect header to get the data
  LodePNGState state;
  lodepng_state_init( &state );
  lode_error = lodepng_inspect( (u32*)&(img->width), 
                                (u32*)&(img->height),
                                &state,
                                file_contents,
                                file_size );
  if ( lode_error )
  {
    printf("Image reading error -> %s\n", lodepng_error_text( lode_error ) );
    return 0;
  }

  // extract colortype and bit_depth info
  LodePNGColorType color_type = state.info_png.color.colortype;
  i32 file_bit_depth          = state.info_png.color.bitdepth;

  // compare what is in file and where we want to store
  i32 type_bit_depth= sizeof(T) * 8;
  if ( file_bit_depth > type_bit_depth ) 
  {
    printf( "Image reading warning -> File stores data at higher"
            " precision than provided type\n" );
  }
  
  // decide on number of components
  switch( color_type )
  {
    case LCT_GREY:
      img->ncomp = 1;
      break;
    case LCT_GREY_ALPHA:
      img->ncomp = 2;
      break;
    case LCT_RGB:
      img->ncomp = 3;
      break;
    case LCT_RGBA:
      img->ncomp = 4;
      break;
    default:
      printf("Image reading error -> No support for pallete PNG\n" );
      return 0;
  }

  lode_error = lodepng_decode_memory( (u8**)&(img->data), 
                                      (u32*)&(img->width), 
                                      (u32*)&(img->height), 
                                      file_contents, 
                                      file_size,
                                      color_type, 
                                      type_bit_depth );
  img->rowsize = img->width * img->ncomp;
  if ( lode_error )
  {
    printf("Image reading error -> %s\n", lodepng_error_text( lode_error ) );
    return 0;
  }

  // if file stores high precision data
  if ( file_bit_depth > 8 )
  {
    i32 total_n_values = img->width * img->height * img->ncomp;
    for ( int i = 0 ; i < total_n_values ; ++i )
    {
      u16 val = (u16)img->data[i];
      val = ( ((val & 0x00FF ) << 8 ) | 
              ((val & 0xFF00 ) >> 8 ) );
      img->data[i] = val;
    }
  }
  
  return 1;
}

template <typename T>
i32 bsc::image<T>::
read( const char * filename )
{
  // clear image
  // TODO: Realloc if requested image is different size...
  width  = 0;
  height = 0;
  ncomp  = 0;
  if ( data != nullptr ) { free( data ); data=nullptr; }

  // determine extension
  char * period = strrchr( filename, '.' );
  if ( period == NULL )
  {
    printf( "Image reading error -> No file extension\n" );
    return 0;
  }
  char * extension = period + 1;
  
  // call appropriate function
  if ( !strcmp( extension, "jpg" ) || !strcmp( extension, "jpeg") )
  {
    return read_jpg( filename, this );
  }
  else if ( !strcmp( extension, "png") )
  {
    return read_png ( filename, this );
  }
  else
  {
    printf( "Image reading error -> Unsuported image format\n" );
    return 0;
  }
}

// TODO(maciej): This function should propably not be templated?
template <typename T>
i32 
write_jpg( const char *filename, bsc::image<T> * img  )
{
  if ( sizeof(T) > 1 )
  {
    printf("Image writing warning -> JPG cannot store high precision!"
           "Possible corruption!\n" );
    // TODO(maciej): just copy data temporarily and store as 8bits?
  }
  if ( img->ncomp != 3 && img->ncomp != 4 )
  {
    printf("%d\nImage writing error -> Unsupported number of components\n", img->ncomp );
    return 0;
  }
  // NOTE(maciej): What happens if we try to do single channel jpg?
  int success = tje_encode_to_file_at_quality( filename, 2,
                                               img->width, 
                                               img->height, 
                                               img->ncomp, 
                                               (const u8*)img->data );
  if ( !success )
  {
    printf("Image writing error -> Could not write file %s\n", filename );
  }
  return success;
}

template <typename T>
i32 
write_png( const char *filename, bsc::image<T> * img  )
{
  i32 type_bit_depth = sizeof(T) * 8;
  if ( type_bit_depth > 16 )
  {
    printf("Image writing warning -> PNG cannot store precision higher than"
           " 16 bits. Possible loss of precision\n" );
    // TODO(maciej): Auto conversion?
    type_bit_depth = 16;
  }
  LodePNGColorType color_type = LCT_RGB;
  switch ( img->ncomp )
  {
    case 1: 
      color_type = LCT_GREY;
      break;
    case 2:
      color_type = LCT_GREY_ALPHA;
      break;
    case 3:
      color_type = LCT_RGB;
      break;
    case 4:
      color_type = LCT_RGBA;
      break;
    default:
      printf( "Image writing error -> Image number of components is" 
              " unsupported by PNG format\n" );
      return 0;
  }

  if ( type_bit_depth > 8 )
  {
    i32 total_n_values = img->width * img->height * img->ncomp;
    for ( int i = 0 ; i < total_n_values ; ++i )
    {
      u16 val = (u16)img->data[i];
      val = ( ((val & 0x00FF ) << 8 ) | 
              ((val & 0xFF00 ) >> 8 ) );
      img->data[i] = val;
    }
  }

  u32 lode_error = lodepng_encode_file( filename,
                                        (u8*)img->data,
                                        (u32)(img->width), 
                                        (u32)(img->height),  
                                        color_type, 
                                        type_bit_depth );
  if ( lode_error )
  {
    printf("Image writing error -> %s\n", lodepng_error_text( lode_error ) );
    return 0;
  }
  
  return 1;
}

template <typename T>
i32 bsc::image<T>::
write( const char * filename )
{
  char * period = strrchr( filename, '.' );
  if ( period == NULL )
  {
    error( "Image writing error", __LINE__, "No file extension" );
    return 0;
  }

  char * extension = period + 1;
  if ( !strcmp( extension, "jpg" ) || !strcmp( extension, "jpeg") )
  {
    return write_jpg( filename, this );
  }
  else if ( !strcmp( extension, "png") )
  {
    return write_png( filename, this );
  }
  else
  {
    error( "Image writing error", __LINE__, "Unsupproted image format" );
    return 0;
  }
}

template <typename T>
void bsc::image<T>::
vert_flip()
{
  for ( int j = 0 ; j < height / 2 ; ++j )
  {
    for ( int i = 0 ; i < width ; ++i )
    {
      T * val_A = (*this)( i, j );
      T * val_B = (*this)( i, height - j - 1 );
      for ( int k = 0 ; k < ncomp; ++k )
      {
        swap( *(val_A+k), *(val_B+k) );
      }
    }
  }
}


template <typename T>
T bsc::image<T>::
maximum()
{
  T max = -1e9;
  for ( int i = 0 ; i < height * rowsize ; ++i )
  {
    if ( data[i] > max )
    {
      max = data[i];
    }
  }
  return max;
}

template <typename T>
T bsc::image<T>::
minimum()
{
  T min= 1e9;
  for ( int i = 0 ; i < height * rowsize ; ++i )
  {
    if ( data[i] < min )
    {
      min = data[i];
    }
  }
  return min;
}

template <typename T>
inline bsc::image<T> & bsc::image<T>::
operator= ( const bsc::image<T> & other )
{
  if ( this->data ) free( this->data );
  u64 size = other.width * other.height * other.ncomp * sizeof(T);
  this->width  = other.width;
  this->height = other.height;
  this->ncomp  = other.ncomp;
  this->rowsize = other.rowsize;
  this->data   = (T*)malloc( size );
  memcpy( this->data, other.data, size );
  return *this;
}

template <typename T>
inline const T* bsc::image<T>::
operator() ( i32 i ) const
{
  return &(this->data[ i ]);
}

template <typename T>
inline T* bsc::image<T>::
operator() ( i32 i )
{
  return &(this->data[ i ]);
}

template <typename T>
inline const T* bsc::image<T>::
operator() ( i32 x, i32 y ) const
{
  return &(this->data[ y * this->rowsize + x * this->ncomp ]);
}

template <typename T>
inline T* bsc::image<T>::
operator() ( i32 x, i32 y )
{
    return &(this->data[ y * this->rowsize + x * this->ncomp ]);
}

template <typename T>
inline const T* bsc::image<T>::
at ( i32 i ) const
{
    return &(this->data[ i ]);
}

template <typename T>
inline T* bsc::image<T>::
at ( i32 i )
{
    return &(this->data[ i ]);
}

template <typename T>
inline const T* bsc::image<T>::
at ( i32 x, i32 y ) const
{
    return &(this->data[ this->ncomp * (y * this->width + x) ]);
}

template <typename T>
inline T* bsc::image<T>::
at ( i32 x, i32 y )
{
    return &( this->data[ this->ncomp * (y * this->width + x )] );
}

#endif //BSC_IMAGE_H