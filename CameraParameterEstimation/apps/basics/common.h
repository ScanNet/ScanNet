// basics v0.07
// by Maciej Halber

////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  r32;
typedef double r64;

////////////////////////////////////////////////////////////////////////////////
// MACROS
////////////////////////////////////////////////////////////////////////////////

#define STR(x) #x
#define XSTR(x) STR(x)
#define COUNT_OF(x) (sizeof(x) / sizeof((x)[0]))

#define CCAT(A, B) A ## B
#define XCCAT(A, B) CCAT(A, B)
#define XSTR_CCAT(A,B) XSTR(XCCAT(A,B))

// TODO foreach loop
// TODO iterating over enum
// TODO get max of enum

////////////////////////////////////////////////////////////////////////////////
// SIMPLE COMMON FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
#ifdef BSC_IMPLEMENTATION

namespace bsc
{
  template <typename T>
  inline T
  clamp ( T val, T min = (T)0, T max = (T)1 )
  {
    if ( val < min ) val = min;
    if ( val > max ) val = max;
    return val;
  }

  template <typename T>
  inline T
  lerp ( T a, T b, T t )
  {
    return a + ( b - a ) * t;
  }

  template <typename T>
  inline void
  swap ( T & a, T & b )
  {
    T tmp = a;
    a = b;
    b = tmp;
  }

  template <typename T>
  inline T
  square ( const T & a )
  {
    return a * a;
  }

  template <typename T>
  inline T
  max ( T a, T b )
  {
    return a > b ? a : b;
  }

  template <typename T>
  inline T
  min ( T a, T b )
  {
    return a > b ? b : a;
  }

  template <typename T>
  inline T
  min3 ( T a, T b, T c )
  {
    return min( c, min( a, b ) );
  }

  template <typename T>
  inline T
  max3 ( T a, T b, T c )
  {
    return max( c, max( a, b ) );
  }


  template <typename T>
  inline T
  deg2rad ( T deg )
  {
    return deg * M_PI / 180.0;
  }

  template <typename T>
  inline T
  rad2deg ( T rad )
  {
    return rad * 180.0 / M_PI;
  }

  template< int n_reps = 1, 
            typename TimeUnit = std::chrono::duration<double>, 
            typename F, typename ...Args >
  typename TimeUnit::rep MeasureTime ( F function, Args&&... arguments )
  {
    using namespace std::chrono;
    auto start = system_clock::now();

    for ( int i = 0 ; i < n_reps; ++i )
    {
      function( std::forward<Args>(arguments)... );
    }

    auto duration = duration_cast<TimeUnit>( system_clock::now() - start );

    return duration.count() / (double)n_reps;
  }

  template<typename TimeUnit = std::chrono::duration<double> >
  struct StopWatch
  {
    std::chrono::time_point<std::chrono::system_clock> s; // start
    std::chrono::time_point<std::chrono::system_clock> e; // end

    void start () { s = std::chrono::system_clock::now(); }
    void read ()  { s = std::chrono::system_clock::now(); }
    typename TimeUnit::rep duration() 
    { 
      return std::chrono::duration_cast<TimeUnit>( e - s ).count(); 
    }

    typename TimeUnit::rep elapsed () 
    { 
      return std::chrono::duration_cast<TimeUnit>( 
        std::chrono::system_clock::now() - s ).count(); 
    }
    
  };

  typedef StopWatch< std::chrono::duration<float> > stop_watch;


  // Based on notes by Humus : 
  // http://www.humus.name/index.php?page=Comments&ID=296
  inline int
  string_hash ( const char * str )
  {
    int hash = 0;
    for ( int i = 0; i < strlen( str ) ; ++i )
      hash = hash * 65599 + str[i];
    return hash;
  }


  // Floating point comparisons based on Thomas Funkhouser gaps library
  inline bool
  is_positive( r32 s, r32 epsilon )
  {
      return (s > epsilon);
  }

  inline bool
  is_positive( r64 s, r64 epsilon  )
  {
      return (s > epsilon);
  }

  inline bool
  is_negative ( r32 s, r32 epsilon  )
  {
      return ( s < -epsilon );
  }

  inline bool
  is_negative ( r64 s, r64 epsilon  )
  {
      return ( s < -epsilon );
  }

  inline bool
  is_positive_or_zero ( r32 s, r32 epsilon )
  {
      return ( s >= -epsilon );
  }

  inline bool
  is_positive_or_zero ( r64 s, r64 epsilon )
  {
      return ( s >= -epsilon );
  }

  inline bool
  is_negative_or_zero ( r32 s, r32 epsilon )
  {
      return ( s <= epsilon );
  }

  inline bool
  is_negative_or_zero ( r64 s, r64 epsilon )
  {
      return ( s <= epsilon );
  }

  inline bool
  is_zero ( r32 s, r32 epsilon )
  {
      return ( is_positive_or_zero( s, epsilon ) && 
               is_negative_or_zero( s, epsilon ) );
  }

  inline bool
  is_zero ( r64 s, r64 epsilon )
  {
      return ( is_positive_or_zero( s, epsilon ) && 
               is_negative_or_zero( s, epsilon ) );
  }

  inline bool
  is_not_zero ( r32 s, r32 epsilon )
  {
      return ( is_positive( s, epsilon ) || 
               is_negative( s, epsilon ) );
  }

  inline bool
  is_not_zero ( r64 s, r64 epsilon )
  {
      return ( is_positive( s, epsilon ) || 
               is_negative( s, epsilon ) );
  }

  inline bool
  equal ( r32 s1, r32 s2, r32 epsilon )
  {
      return is_zero( s1 - s2, epsilon );
  }

  inline bool
  equal ( r64 s1, r64 s2, r64 epsilon )
  {
      return is_zero( s1 - s2, epsilon );
  }

  inline bool
  notEqual ( r32 s1, r32 s2, r32 epsilon )
  {
      return is_not_zero( s1 - s2, epsilon );
  }

  inline bool
  notEqual ( r64 s1, r64 s2, r64 epsilon )
  {
      return is_not_zero( s1 - s2, epsilon );
  }

  inline bool
  greater ( r32 s1, r32 s2, r32 epsilon )
  {
      return is_positive( s1 - s2, epsilon );
  }

  inline bool
  greater ( r64 s1, r64 s2, r64 epsilon )
  {
      return is_positive( s1 - s2, epsilon );
  }

  inline bool
  less ( r32 s1, r32 s2, r32 epsilon )
  {
      return is_negative( s1 - s2, epsilon );
  }

  inline bool
  less ( r64 s1, r64 s2, r64 epsilon )
  {
      return is_negative( s1 - s2, epsilon );
  }

  inline bool
  greater_or_equal ( r32 s1, r32 s2, r32 epsilon )
  {
      return is_positive_or_zero( s1 - s2, epsilon );
  }

  inline bool
  greater_or_equal ( r64 s1, r64 s2, r64 epsilon )
  {
      return is_positive_or_zero( s1 - s2, epsilon );
  }

  inline bool
  less_or_equal ( r32 s1, r32 s2, r32 epsilon )
  {
      return is_negative_or_zero( s1 - s2, epsilon );
  }

  inline bool
  less_or_equal ( r64 s1, r64 s2, r64 epsilon )
  {
      return is_negative_or_zero( s1 - s2, epsilon );
  }

  // error reporting
  inline void 
  error ( const char * title, const int line,
          const char * description )
  {
    fprintf( stderr, "%s at line %d -> %s\n", title, line, description );
  }

// Array stuff
// TODO: Testing!

  inline r32 inner_product( const r32 *vals, const i32 n_vals )
  {
    r32 value = 0.0f;
    for ( int i = 0 ; i < n_vals ; ++i )
    {
      value += vals[i] * vals[i];
    }
    return value;
  }

  inline r32 compute_sum( const r32 *vals, const i32 n_vals )
  {
    // TODO: Assertions and errors
    r32 sum = 0;
    for ( int i = 0 ; i < n_vals ; ++i )
    {
      sum += vals[i];
    }
    return sum;
  }

  inline r32 compute_mean( const r32 *vals, const i32 n_vals )
  {
    r32 sum = compute_sum( vals, n_vals );
    return sum / (r32) n_vals;
  }

  inline r32 compute_stddev( r32 mean, r32 *vals, i32 n_vals )
  {
    r32 sq_sum = inner_product( vals, n_vals );
    return sqrtf( sq_sum / (r32)n_vals - mean * mean );
  }
}
#endif //BSC_IMPLEMENTATION
