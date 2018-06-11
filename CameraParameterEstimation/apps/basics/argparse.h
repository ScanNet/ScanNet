// Simple argument parsing
// Author:  Maciej Halber
// Date:    03/24/16
// License: Public Domain


// NOTE: This is a bit crazy part of basics, with a lot of templating .
// Possibly change this to something closer to c.
// This thing needs to go.

#pragma once

#include <stdlib.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace bsc
{
  template <typename T>
  struct argument
  {
    std::string name;
    std::string short_name;
    std::string message;
    T * value = NULL;

    bool required = false;
    int positional = -1;
    int length = -1;

    argument ( std::string name, 
               std::string message, 
               T * value, 
               int length = 0 );
    argument ( std::string short_name, 
               std::string name, 
               std::string message, 
               T * value, 
               int length = 1 );
  };

  class arg_parse
  {
  public:
    std::string name;
    std::string description;


    void add( argument<char*> arg ) 
    { 
      add( arg, cstr_args ); 
    }

    void add( argument<std::string> arg ) 
    { 
      add( arg, str_args ); 
    }
    
    void add( argument<int> arg )
    { 
      add( arg, int_args ); 
    }
    
    void add( argument<float> arg ) 
    { 
      add( arg, float_args ); 
    }
    
    void add( argument<double> arg )
    { 
      add( arg, double_args ); 
    }
    
    void add( argument<bool> arg )
    { 
      add( arg, bool_args ); 
    }

    void parse( int argc, char** argv );

    bool exists_already( std::string name ) const;
    
    argument<std::string> * find_string( std::string key ) 
    { 
      return find( key, str_args ); 
    }

    argument<int> * find_int( std::string key )
    { 
      return find( key, int_args ); 
    }
    
    argument<float> * find_float( std::string key )
    {
      return find( key, float_args ); 
    }
    
    argument<double> * find_double( std::string key )
    { 
      return find( key, double_args ); 
    }
    
    argument<bool>  * find_bool( std::string key )
    { 
      return find( key, bool_args ); 
    }

    void print_help() const;

  private:
    std::unordered_map< std::string, argument< std::string > > str_args;
    std::unordered_map< std::string, argument< char* > >       cstr_args;
    std::unordered_map< std::string, argument< int > >         int_args;
    std::unordered_map< std::string, argument< float > >       float_args;
    std::unordered_map< std::string, argument< double > >      double_args;
    std::unordered_map< std::string, argument< bool > >        bool_args;

    template <typename T>
    void add( argument<T> & arg, 
              std::unordered_map< std::string, 
              argument< T > > & args );

    template <typename T>
    argument<T> * find( std::string key, 
                        std::unordered_map< std::string, 
                        argument< T > > & args );

    template <typename T>
    argument<T> * find_at_position ( int position, 
                                     std::unordered_map< std::string, 
                                     argument< T > > & args );
  };

  static int arg_parse_num_required = 0;
}


////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////
#ifdef BSC_IMPLEMENTATION

// argument constructors. little lengthy due to error checking
template <typename T>
bsc::argument<T>::
argument ( std::string name, std::string message, T * value, int length )
{
  if ( name.empty() )
  {
    printf( "argument name must not be an empty string\n" );
    exit(-1);
  }

  this->name        = name;
  this->value       = value;
  this->message     = message;
  this->length      = length;

  if ( name[0] == '-' && name[1] == '-' )
  {
    this->required = false;
    if ( length < 2 ) this->length = 1;
    if ( name.length() < 3 )
    {
      printf( "argument \"%s\" name must be at least 3"
              " characters long! ( --<name> )\n", name.c_str() );
      exit(-1);
    }
  }
  else if ( name[0] == '-' )
  {
    this->required = false;
    if ( length < 2 ) this->length = 1;
    if ( name.length() != 2 )
    {
      printf( "argument \"%s\" shorthand \"%s\" must be at least 2"
              " characters long ( -<letter> )\n",
              name.c_str(), name.c_str() );
      exit(-1);
    }
  }
  else
  {
    this->required   = true;
    this->positional = ++arg_parse_num_required - 1;
  }
}

template <typename T>
bsc::argument<T>::
argument ( std::string short_name, 
           std::string name, 
           std::string message, 
           T * value, int length )
{
  if ( name.length() < 3 )
  {
    printf( "argument \"%s\" name must be at least 3"
            " characters long! ( --<name> )\n", name.c_str() );
    exit(-1);
  }

  if ( short_name.length() > 4 )
  {
    printf( "argument \"%s\" shorthand \"%s\" must be at"
            " less then 4 characters long ( -<letter> )\n",
            name.c_str(), short_name.c_str() );
    exit(-1);
  }

  this->name        = name;
  this->short_name  = short_name;
  this->value       = value;
  this->message     = message;
  this->length      = length;
  this->required    = false;
}


bool bsc::arg_parse ::
exists_already ( std::string name ) const
{
  if ( str_args.find( name )    != str_args.end() )     return true;
  if ( int_args.find( name )    != int_args.end() )     return true;
  if ( float_args.find( name )  != float_args.end() )   return true;
  if ( double_args.find( name ) != double_args.end() )  return true;
  if ( bool_args.find( name )   != bool_args.end() )    return true;
  return false;
}


template <typename T>
void bsc::arg_parse ::
add ( argument<T> & arg, 
      std::unordered_map< std::string, argument< T > > & args )
{
  bool check_name = !exists_already( arg.name );
  bool check_short_name = arg.short_name.empty() 
                          ? 1 
                          : !exists_already( arg.short_name );
  if ( check_name && check_short_name )
  {
    args.insert( std::make_pair( arg.name, arg )  );
    if ( !arg.required ) 
    {
      args.insert( std::make_pair( arg.short_name, arg ) );
    }
  }
  else
  {
    printf("argument with name \"%s\" is in conflict!", arg.name.c_str() );
    print_help();
  }
}

template <typename T>
bsc::argument<T> * bsc::arg_parse ::
find ( std::string key, 
       std::unordered_map< std::string, 
       argument< T > > & args )
{
  auto arg = args.find( key );
  if ( arg != args.end() )
  { 
    return &( arg->second );
  }
  else 
  {
    return nullptr;
  }
}

template <typename T>
bsc::argument<T> * bsc::arg_parse ::
find_at_position ( int position, 
                   std::unordered_map< std::string, 
                   argument< T > > & args )
{
  argument<T> * retval = nullptr;
  for ( auto & arg : args )
  {
    if ( arg.second.positional == position )
    {
      retval = &( arg.second );
      break;
    }
  }
  return retval;
}
////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////

// check if arg passed during parsing is correct, when parsing multiple values
template <typename T>
void check_validity( int idx, 
                     int n_args, 
                     std::string to_parse, 
                     bsc::argument<T> *arg )
{
  if ( idx >= n_args || to_parse[0] == '-' )
  {
    printf ("No more values to parse for argument %s (%s),"
            " correct value is %d argument(s)\n",
            arg->name.c_str(), 
            arg->short_name.c_str(), arg->length );
    exit( -1 );
  }
};

template <typename T>
void parse_arg ( std::vector<std::string> & args_to_parse,
                 int & idx,
                 bsc::argument<T> * arg, 
                 std::string to_parse, 
                 std::function<T(std::string)> acceptance_function, 
                 std::function<T(std::string)> conversion_funct )
{
  if ( arg->length == 0 )
  { 
    *(arg->value) = acceptance_function( to_parse );
  }

  for ( int i = 0 ; i < arg->length ; ++i )
  {
    to_parse = args_to_parse[ ++idx ];
    check_validity( idx, args_to_parse.size(), to_parse, arg );
    *(arg->value + i) = conversion_funct( to_parse );
  }
};

void bsc::arg_parse ::
parse ( int argc, char** argv )
{
  // skip name of program
  name = std::string( *argv );
  argc--; argv++;

  // parse into helper data structure
  std::vector< std::string > args_to_parse;
  args_to_parse.reserve( argc );
  while ( argc > 0 )
  {
    args_to_parse.push_back( std::string( *argv ) );
    argc--; argv++;
  }

  // helper variables
  int idx = 0;
  int n_args = args_to_parse.size();

  // parse value to correct argument
  auto parse_argument = [ &args_to_parse, &idx, n_args ]
                        ( std::string to_parse, 
                          argument<std::string>* str_arg, 
                          argument<char*>* cstr_arg, 
                          argument<int>* int_arg,
                          argument<float>* float_arg, 
                          argument<double>* double_arg, 
                          argument<bool>* bool_arg )
  {
    // define some conversion function wrappers
    std::function<std::string(std::string)> identity  = [] ( std::string arg ) { return arg; };
    std::function<char*(std::string)> to_cstr   = [] ( std::string arg ) { return strdup( arg.c_str() ); };
    std::function<int(std::string)> to_int    = [] ( std::string arg ) { return std::stoi( arg ); };
    std::function<float(std::string)> to_float  = [] ( std::string arg ) { return std::stof( arg ); };
    std::function<double(std::string)> to_double = [] ( std::string arg ) { return std::stod( arg ); };
    std::function<bool(std::string)> to_bool   = [] ( std::string arg ) { return (bool)std::stoi( arg ); };
    std::function<int(std::string)> accept_int    = [] ( std::string arg ) { return 1; };
    std::function<float(std::string)> accept_float    = [] ( std::string arg ) { return 1; };
    std::function<double(std::string)> accept_double    = [] ( std::string arg ) { return 1; };
    std::function<bool(std::string)> accept_bool    = [] ( std::string arg ) { return 1; };

    // actual parsing happens here. We either accept the value 
    // ( if length is 0 ), or convert to correct type using conversion function
    
    // NOTE:This lambda needs to be templated

    if ( str_arg != nullptr )
    {
      parse_arg( args_to_parse, idx, str_arg, to_parse, identity, identity );
    }
    if ( cstr_arg != nullptr )
    {
      parse_arg( args_to_parse, idx, cstr_arg,  to_parse, to_cstr, to_cstr );
    }
    else if ( int_arg != nullptr )
    {
      parse_arg( args_to_parse, idx, int_arg,  to_parse, accept_int, to_int );
    }
    else if ( float_arg != nullptr )
    {
      parse_arg( args_to_parse, idx, float_arg,  to_parse, accept_float, to_float );
    }
    else if ( double_arg != nullptr )
    {
      parse_arg( args_to_parse, idx, double_arg,  to_parse, accept_double, to_double );
    }
    else if ( bool_arg != nullptr ) 
    {
      parse_arg( args_to_parse, idx, bool_arg, to_parse, accept_bool, to_bool );
    }
  };

  // check if help is there
  for ( int str_idx = 0 ; str_idx < n_args ; ++str_idx )
  {
    if ( args_to_parse[str_idx] == "--help" ||
         args_to_parse[str_idx] == "-h" )
    {
      print_help();
    }
  }

  // Some corner cases
  if ( n_args < arg_parse_num_required ) print_help();
  if ( args_to_parse.empty() ) return;

  // Get required positional arguments
  for ( int position = 0 ; position < arg_parse_num_required ; ++position )
  {
    argument<std::string> * str_arg = find_at_position( position, str_args );
    argument<char*> * cstr_arg      = find_at_position( position, cstr_args );
    argument<int> * int_arg         = find_at_position( position, int_args );
    argument<float> * float_arg     = find_at_position( position, float_args );
    argument<double> * double_arg   = find_at_position( position, double_args );
    argument<bool> * bool_arg       = find_at_position( position, bool_args );

    std::string cur_arg = args_to_parse[ idx ];
    if ( cur_arg[0] == '-' )
    {
      printf( "Invalid argument \"%s\" at position %d!\n", 
              cur_arg.c_str(), position );
      print_help();
    }

    parse_argument( cur_arg, str_arg, cstr_arg,
                    int_arg, float_arg, double_arg, bool_arg );
    idx++;
  }

  // Get the rest of arguments
  while ( idx < n_args )
  {
    // check which one it is
    std::string cur_arg = args_to_parse[ idx ];

    // try to find it
    bool found = true;
    argument<std::string> * str_arg = find( cur_arg, str_args );
    argument<char*> * cstr_arg      = find( cur_arg, cstr_args );
    argument<int> * int_arg         = find( cur_arg, int_args );
    argument<float> * float_arg     = find( cur_arg, float_args );
    argument<double> * double_arg   = find( cur_arg, double_args );
    argument<bool> * bool_arg       = find( cur_arg, bool_args );

    if ( str_arg == nullptr && int_arg == nullptr && cstr_arg == nullptr && 
         float_arg == nullptr && double_arg == nullptr &&
         bool_arg == nullptr )
    {
      found = false;
    }

    if ( found )
    {
      parse_argument( cur_arg, str_arg, cstr_arg,
                      int_arg, float_arg, double_arg, bool_arg );
    }
    else
    {
      printf( "Unrecognized argument \"%s\"\n", cur_arg.c_str() );
      print_help();
    }

    idx++;
  }
}

// helper storage struct
struct argument_info
{
  std::string name;
  std::string short_name;
  std::string message;
  std::string type;
  int count;
};


// copy from actual argument storage to local sturctures 
  // that do not vary on type
template <typename T>
void group_arguments ( std::vector<argument_info> & required_args, 
                       std::unordered_map<std::string, argument_info> & optional_args_map,
                       const std::unordered_map< std::string, bsc::argument< T > >& args,
                       const std::string type )
{
  for ( const auto & arg : args )
  {
    argument_info info = { arg.second.name, arg.second.short_name, 
                           arg.second.message, type, arg.second.length };
    if ( arg.second.required ) 
    {
      required_args[ arg.second.positional ] = info;
    }
    else
    {
      optional_args_map.insert( std::make_pair( info.name, info ) );
    }
  }
};

// formatted printing
void print_args( std::vector<argument_info> &args )
{
  for ( const auto & arg : args )
  {
    std::string full_name = arg.name;
    if ( !arg.short_name.empty() ) 
    {
      full_name =  arg.short_name + ", " + full_name;
    }

    if ( arg.count <= 1 )
    {
      printf("  %-24s - %s <%s>\n", full_name.c_str(),
                                    arg.message.c_str(),
                                    arg.type.c_str() );
    }
    else
    {
      printf("  %-24s - %s <%d %ss>\n", full_name.c_str(),
                                        arg.message.c_str(),
                                        arg.count,
                                        arg.type.c_str() );
    }
  }
};

void bsc::arg_parse ::
print_help() const
{

  // printing help is here by default
  argument_info help_info = { "--help", "-h", 
                              "Show this help message and exit", "", 0 };

  // storage
  std::vector< argument_info > required_args;
  std::vector< argument_info > optional_args;
  required_args.resize( arg_parse_num_required );
  std::unordered_map< std::string, argument_info > optional_args_map;

  // Gather info
  group_arguments( required_args, optional_args_map, str_args,    "string");
  group_arguments( required_args, optional_args_map, cstr_args,   "cstring" );
  group_arguments( required_args, optional_args_map, int_args,    "int" );
  group_arguments( required_args, optional_args_map, float_args,  "float" );
  group_arguments( required_args, optional_args_map, double_args, "doule" );
  group_arguments( required_args, optional_args_map, bool_args,   "bool" );

  // Copy form a map to vector ( done to avoid duplicates )
  for ( const auto & elem : optional_args_map )
  {
    optional_args.push_back( elem.second );
  }
  optional_args.push_back( help_info );

  // Sort optionals
  std::sort( optional_args.begin(), optional_args.end(),
            []( const argument_info & a, const argument_info & b )
              {
                int idx_a = a.name.find_last_of("-") + 1;
                int idx_b = b.name.find_last_of("-") + 1;
                std::string a_name = a.name.substr( idx_a, -1 );
                std::string b_name = b.name.substr( idx_b, -1 );
                return a_name < b_name;
              } );

  // Actual printing of help message
  printf( "\nUsage : %s ", name.c_str() );
  for ( const auto & arg : required_args ) printf( "%s ", arg.name.c_str() );
  for ( const auto & arg : optional_args ) printf( "%s ", arg.name.c_str() );
  printf("\n\n");

  if ( !description.empty() )
  {
    printf("Description: %s\n\n", description.c_str() );
  }

  if ( !required_args.empty() )
  {
    printf("Required arguments:\n");
    print_args( required_args );
    printf("\n");
  }

  printf("Optional arguments:\n");
  print_args( optional_args );
  printf("\n");

  exit(-1);
}

#endif //BSC_IMPLEMENTATION