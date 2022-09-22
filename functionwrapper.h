#ifndef MICROPYTHON_WRAP_FUNCTIONWRAPPER
#define MICROPYTHON_WRAP_FUNCTIONWRAPPER

#include "classwrapper.h"

namespace upywrap
{
  //Main logic for registering functions.
  //Usage:
  //
  //int Foo( int );
  //double Bar( const std::string& );
  //
  //struct Funcs
  //{
  //  func_name_def( Foo )
  //  func_name_def( Bar )
  //};
  //
  //FunctionWrapper wrap( dict );
  //wrap.Def< Funcs::Foo >( Foo );
  //wrap.Def< Funcs::Bar >( Bar, Kwargs( "c", "str" );
  //
  //This will register given functions in dict,
  //so if dict is the global dict of a module "mod"
  //the functions can be used in uPy like this:
  //
  //import mod;
  //mod.Foo();
  //mod.Bar(c="bar");
  //
  //For supported arguments and return values see FromPyObj and ToPyObj classes.
  class FunctionWrapper
  {
  public:
    FunctionWrapper( mp_obj_module_t* mod ) :
      FunctionWrapper( mod->globals )
    {
    }

    FunctionWrapper( mp_obj_dict_t* globals ) :
      globals( globals )
    {
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret( *f ) ( A... ), Arguments arguments, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      typedef NativeCall< name, Ret, A... > call_type;

      auto callerObject = call_type::CreateCaller( f );
      callerObject->convert_retval = conv;
      callerObject->arguments = std::move( arguments );
      functionPointers[ (void*) name ] = callerObject;
      mp_obj_dict_store( globals, new_qstr( name() ), call_type::CreateUPyFunction( *callerObject ) );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret ( *f )( A... ), typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      Def< name, Ret, A... >( f, Arguments(), conv );
    }

  private:
    //wrap native call in function with uPy compatible mp_obj_t( mp_obj_t.... ) signature
    template< index_type index, class Ret, class... A >
    struct NativeCall
    {
      typedef FunctionCall< Ret, A... > call_type;
      typedef typename call_type::func_type func_type;

      static call_type* CreateCaller( func_type f )
      {
        return new call_type( f );
      }

      static mp_obj_t CreateUPyFunction( const call_type& caller )
      {
        if( caller.arguments.HasArguments() )
        {
          if( caller.arguments.NumberOfArguments() != sizeof...( A ) )
          {
            RaiseTypeException( ( std::string( "Wrong number of arguments in definition of " ) + index() ).data()  );
          }
          return MakeFunction( caller.arguments.MimimumNumberOfArguments(), CallKw );
        }
        return CreateFunction< A... >::Create( Call, CallN );
      }

      static mp_obj_t Call( typename project2nd< A, mp_obj_t >::type... args )
      {
        auto f = (call_type*) FunctionWrapper::functionPointers[ (void*) index ];
        return CallReturn< Ret, A... >::Call( f, args... );
      }

      static mp_obj_t CallN( mp_uint_t nargs, const mp_obj_t* args )
      {
        if( nargs != sizeof...( A ) )
        {
          RaiseTypeException( "Wrong number of arguments" );
        }
        auto f = (call_type*) FunctionWrapper::functionPointers[ (void*) index ];
        return CallVar( f, args, make_index_sequence< sizeof...( A ) >() );
      }

      static mp_obj_t CallKw( size_t n_args, const mp_obj_t* pos_args, mp_map_t* kw_args )
      {
        auto f = (call_type*) FunctionWrapper::functionPointers[ (void*) index ];
        Arguments::parsed_obj_t parsedArgs{};
        f->arguments.Parse( n_args, pos_args, kw_args, parsedArgs );
        return CallVar( f, parsedArgs.data(), make_index_sequence< sizeof...( A ) >() );
      }

      template< size_t... Indices >
      static mp_obj_t CallVar( call_type* f, const mp_obj_t* args, index_sequence< Indices... > )
      {
        (void) args;
        return CallReturn< Ret, A... >::Call( f, args[ Indices ]... );
      }
    };

    mp_obj_dict_t* globals;
    static function_ptrs functionPointers;
  };

  //we want a header only library but that yields multiply defined symbols when including this file more then once
  //so as a simple hack: define MMICROPYTHON_WRAP_FUNCTIONWRAPPER_DEFINED in all but one include location
#ifndef MICROPYTHON_WRAP_FUNCTIONWRAPPER_DEFINED
  function_ptrs FunctionWrapper::functionPointers;
#endif

}

#endif //#ifndef MICROPYTHON_WRAP_FUNCTIONWRAPPER
