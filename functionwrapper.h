#ifndef MICROPYTHONMODULE_FUNCTIONWRAPPER
#define MICROPYTHONMODULE_FUNCTIONWRAPPER

#include "detail/index.h"
#include "detail/util.h"
#include "detail/functioncall.h"
#include "detail/callreturn.h"

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
  //wrap.Def< Funcs::Bar >( Bar );
  //
  //This will register given functions in dict,
  //so if dict is the global dict of a module "mod"
  //the functions can be used in uPy like this:
  //
  //import mod;
  //mod.Foo();
  //mod.Bar();
  //
  //For supported arguments and return values see FromPyObj and ToPyObj classes.
  class FunctionWrapper
  {
  public:
    FunctionWrapper( mp_obj_dict_t* globals ) :
      globals( globals )
    {
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret( *f ) ( A... ) )
    {
      typedef NativeCall< name, Ret, A... > call_type;

      functionPointers[ (void*) name ] = call_type::CreateCaller( f );
      mp_obj_dict_store( globals, MP_OBJ_NEW_QSTR( qstr_from_str( name() ) ), mp_make_function_n( sizeof...( A ), (void*) call_type::Call ) );
    }

    static function_ptrs functionPointers;

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

      static mp_obj_t Call( typename project2nd< A, mp_obj_t >::type... args )
      {
        auto f = (call_type*) FunctionWrapper::functionPointers[ (void*) index ];
        return CallReturn< Ret, A... >::Call( f, args... );
      }
    };

    mp_obj_dict_t* globals;
  };

  function_ptrs FunctionWrapper::functionPointers;

}

#endif
