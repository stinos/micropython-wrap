#ifndef MICROPYTHONMODULE_FROMPYOBJ_H
#define MICROPYTHONMODULE_FROMPYOBJ_H

#include "micropython.h"
#include "topyobj.h"
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <limits>

namespace upywrap
{
  //Extract Arg from mp_obj_t
  template< class Arg >
  struct FromPyObj;

  template<>
  struct FromPyObj< machine_int_t >
  {
    static machine_int_t Convert( mp_obj_t arg )
    {
      return mp_obj_get_int( arg );
    }
  };

#if defined( __LP64__ ) || defined( _WIN64 )
  template<>
  struct FromPyObj< int >
  {
    static int Convert( mp_obj_t arg )
    {
      return safe_integer_cast< int >( FromPyObj< machine_int_t >::Convert( arg ) );
    }
  };
#endif

  template<>
  struct FromPyObj< mp_float_t >
  {
    static mp_float_t Convert( mp_obj_t arg )
    {
      return mp_obj_get_float( arg );
    }
  };

  template<>
  struct FromPyObj< std::string >
  {
    static std::string Convert( mp_obj_t arg )
    {
      uint len;
      auto chars = mp_obj_str_get_data( arg, &len );
      return std::string( chars, safe_integer_cast< size_t >( len ) );
    }
  };

  template< class T >
  struct FromPyObj< std::vector< T > >
  {
    static std::vector< T > Convert( mp_obj_t arg )
    {
      uint len;
      mp_obj_t* items;
      mp_obj_get_array( arg, &len, &items ); //works for list and tuple
      std::vector< T > ret( safe_integer_cast< size_t >( len ) );
      std::transform( items, items + len, ret.begin(), FromPyObj< T >::Convert );
      return ret;
    }
  };

  template< class R, class... Args >
  struct FromPyObj< std::function< R( Args... ) > >
  {
    typedef typename std::function< R( Args... ) > std_fun_type;

    static std_fun_type Convert( mp_obj_t arg )
    {
      if( MP_OBJ_IS_TYPE( arg, &mp_type_fun_native ) )
      {
        //TODO if nativeFun actually points to NativeCall::Call or NativeMemberCall::Call, and we can
        //figure that out somehow, we do not have to go through the double conversion native->mp_obj_t->native
        typedef mp_obj_t( *py_fun_type )( typename project2nd< Args, mp_obj_t >::type... );

        const mp_obj_fun_native_t* nativeFun = (mp_obj_fun_native_t*) arg;
        const py_fun_type pyFun = (py_fun_type) nativeFun->fun;
        return std_fun_type(
          [pyFun] ( Args... args ) -> R
          {
            return FromPyObj< R >::Convert( pyFun( ToPyObj< Args >::Convert( args )... ) );
          } );
      }
      else if( MP_OBJ_IS_TYPE( arg, &mp_type_fun_bc ) )
      {
        mp_obj_fun_bc_t* fun = (mp_obj_fun_bc_t*) arg;
        return std_fun_type(
          [fun] ( Args... args ) -> R
          {
            mp_obj_t pyArgs[] = { ToPyObj< Args >::Convert( args )... };
            return FromPyObj< R >::Convert( fun_bc_call( fun, sizeof...( Args ), 0, pyArgs ) );
          } );
      }
      else
      {
        RaiseTypeException( "This function cannot be converted to a native function" );
        return std_fun_type();
      }
    }
  };
}

#endif
