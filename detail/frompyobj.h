#ifndef MICROPYTHONMODULE_FROMPYOBJ_H
#define MICROPYTHONMODULE_FROMPYOBJ_H

#include "micropython.h"
#include "topyobj.h"
#include <functional>

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

  template<>
  struct FromPyObj< bool >
  {
    static bool Convert( mp_obj_t arg )
    {
      return mp_obj_is_true( arg ) ? true : false;
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
    typedef std::vector< T > vec_type;

    static vec_type Convert( mp_obj_t arg )
    {
      uint len;
      mp_obj_t* items;
      mp_obj_get_array( arg, &len, &items ); //works for list and tuple
      vec_type ret( safe_integer_cast< size_t >( len ) );
      std::transform( items, items + len, ret.begin(), FromPyObj< T >::Convert );
      return ret;
    }
  };

  template< class K, class V >
  struct FromPyObj< std::map< K, V > >
  {
    typedef std::map< K, V > map_type;

    static map_type Convert( mp_obj_t arg )
    {
      map_type ret;
      auto dict_iter = mp_obj_new_dict_iterator( (mp_obj_dict_t*) arg, 0 );
      mp_map_elem_t* next = nullptr;
      while( ( next = dict_it_iternext_elem( dict_iter ) ) != MP_OBJ_STOP_ITERATION )
      {
        ret.insert( typename map_type::value_type( FromPyObj< K >::Convert( next->key ),
                                                   FromPyObj< V >::Convert( next->value ) ) );
      }
      return ret;
    }
  };

  template< class... A >
  struct FromPyObj< std::tuple< A... > >
  {
    typedef std::tuple< A... > tuple_type;

    static tuple_type Convert( mp_obj_t arg )
    {
      uint len;
      mp_obj_t* items;
      mp_obj_get_array( arg, &len, &items );
      if( len != safe_integer_cast< uint >( sizeof...( A ) ) )
        RaiseTypeException( "Not enough tuple elements" );
      return make_it( items, make_index_sequence< sizeof...( A ) >() );
    }

    template< size_t... Indices >
    static tuple_type make_it( const mp_obj_t* args, index_sequence< Indices... > )
    {
      return std::make_tuple( FromPyObj< A >::Convert( args[ Indices ] )... );
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
