#ifndef MICROPYTHONMODULE_FROMPYOBJ_H
#define MICROPYTHONMODULE_FROMPYOBJ_H

#include "micropython.h"
#include "topyobj.h"
#include <functional>

namespace upywrap
{
  //Extract Arg from mp_obj_t
  template< class Arg >
  struct FromPyObj : std::false_type
  {
  };

  template<>
  struct FromPyObj< mp_obj_t > : std::true_type
  {
    static mp_obj_t Convert( mp_obj_t arg )
    {
      return arg;
    }
  };

  template<>
  struct FromPyObj< mp_int_t > : std::true_type
  {
    static mp_int_t Convert( mp_obj_t arg )
    {
      return mp_obj_get_int( arg );
    }
  };

  template<>
  struct FromPyObj< mp_uint_t > : std::true_type
  {
    static mp_uint_t Convert( mp_obj_t arg )
    {
      return safe_integer_cast< mp_uint_t >( mp_obj_get_int( arg ) );
    }
  };

  template<>
  struct FromPyObj< bool > : std::true_type
  {
    static bool Convert( mp_obj_t arg )
    {
      return mp_obj_is_true( arg ) ? true : false;
    }
  };

#if defined( __LP64__ ) || defined( _WIN64 )
  template<>
  struct FromPyObj< int > : std::true_type
  {
    static int Convert( mp_obj_t arg )
    {
      return safe_integer_cast< int >( FromPyObj< mp_int_t >::Convert( arg ) );
    }
  };

  template<>
  struct FromPyObj< unsigned > : std::true_type
  {
    static int Convert( mp_obj_t arg )
    {
      return safe_integer_cast< unsigned >( FromPyObj< mp_uint_t >::Convert( arg ) );
    }
  };
#endif

  template<>
  struct FromPyObj< double > : std::true_type
  {
    static double Convert( mp_obj_t arg )
    {
      return mp_obj_get_float( arg );
    }
  };

  template<>
  struct FromPyObj< float > : std::true_type
  {
    static float Convert( mp_obj_t arg )
    {
      return static_cast< float >( mp_obj_get_float( arg ) );
    }
  };

  template<>
  struct FromPyObj< std::string > : std::true_type
  {
    static std::string Convert( mp_obj_t arg )
    {
      mp_uint_t len;
      auto chars = mp_obj_str_get_data( arg, &len );
      return std::string( chars, safe_integer_cast< size_t >( len ) );
    }
  };

  template<>
  struct FromPyObj< const char* > : std::true_type
  {
    static const char* Convert( mp_obj_t arg )
    {
      return mp_obj_str_get_str( arg );
    }
  };

  template< class T >
  struct FromPyObj< std::vector< T > > : std::true_type
  {
    typedef std::vector< T > vec_type;

    static vec_type Convert( mp_obj_t arg )
    {
      mp_uint_t len;
      mp_obj_t* items;
      mp_obj_get_array( arg, &len, &items ); //works for list and tuple
      vec_type ret( safe_integer_cast< size_t >( len ) );
      std::transform( items, items + len, ret.begin(), FromPyObj< T >::Convert );
      return ret;
    }
  };

  template< class K, class V >
  struct FromPyObj< std::map< K, V > > : std::true_type
  {
    typedef std::map< K, V > map_type;

    static map_type Convert( mp_obj_t arg )
    {
      map_type ret;
      mp_map_elem_t* next = nullptr;
      mp_uint_t cur = 0;
      while( ( next = dict_iter_next( (mp_obj_dict_t*) arg, &cur ) ) != nullptr )
      {
        ret.insert( typename map_type::value_type( FromPyObj< K >::Convert( next->key ),
                                                   FromPyObj< V >::Convert( next->value ) ) );
      }
      return ret;
    }
  };

  template< class... A >
  struct FromPyObj< std::tuple< A... > > : std::true_type
  {
    typedef std::tuple< A... > tuple_type;

    static tuple_type Convert( mp_obj_t arg )
    {
      mp_uint_t len;
      mp_obj_t* items;
      mp_obj_get_array( arg, &len, &items );
      if( len != safe_integer_cast< mp_uint_t >( sizeof...( A ) ) )
        RaiseTypeException( "Not enough tuple elements" );
      return make_it( items, make_index_sequence< sizeof...( A ) >() );
    }

    template< size_t... Indices >
    static tuple_type make_it( const mp_obj_t* args, index_sequence< Indices... > )
    {
      return std::make_tuple( FromPyObj< A >::Convert( args[ Indices ] )... );
    }
  };

  namespace detail
  {
    template< class R, class... Args >
    struct MakeStdFun
    {
      typedef typename std::function< R( Args... ) > std_fun_type;
      typedef mp_obj_t( *py_fun_type )( typename project2nd< Args, mp_obj_t >::type... );

      static std_fun_type Native( const mp_obj_fun_builtin_t* nativeFun )
      {
        const auto pyFun = (py_fun_type) nativeFun->fun;
        return std_fun_type(
          [pyFun] ( Args... args ) -> R
          {
            return FromPyObj< R >::Convert( pyFun( SelectToPyObj< Args >::type::Convert( args )... ) );
          } );
      }

      static std_fun_type PythonFun( mp_obj_t fun )
      {
        return std_fun_type(
          [fun] ( Args... args ) -> R
          {
            //+1 to avoid zero-sized array which is illegal for msvc
            mp_obj_t objs[ sizeof...( Args ) + 1 ] = { SelectToPyObj< Args >::type::Convert( args )... };
            return FromPyObj< R >::Convert( mp_call_function_n_kw( fun, sizeof...( Args ), 0, objs ) );
          } );
      }
    };

    template< class... Args >
    struct MakeStdFun< void, Args... >
    {
      typedef typename std::function< void( Args... ) > std_fun_type;
      typedef mp_obj_t( *py_fun_type )( typename project2nd< Args, mp_obj_t >::type... );

      static std_fun_type Native( const mp_obj_fun_builtin_t* nativeFun )
      {
        const auto pyFun = (py_fun_type) nativeFun->fun;
        return std_fun_type(
          [pyFun] ( Args... args )
          {
            pyFun( SelectToPyObj< Args >::type::Convert( args )... );
          } );
      }

      static std_fun_type PythonFun( mp_obj_t fun )
      {
        return std_fun_type(
          [fun] ( Args... args )
          {
            mp_obj_t objs[ sizeof...( Args ) + 1 ] = { SelectToPyObj< Args >::type::Convert( args )... };
            mp_call_function_n_kw( fun, sizeof...( Args ), 0, objs );
          } );
      }
    };
  }

  template< class R, class... Args >
  struct FromPyObj< std::function< R( Args... ) > > : std::true_type
  {
    typedef typename detail::MakeStdFun< R, Args... > make_fun;
    typedef typename make_fun::std_fun_type std_fun_type;

    static std_fun_type Convert( mp_obj_t arg )
    {
      if( MP_OBJ_IS_TYPE( arg, &mp_type_fun_builtin ) )
      {
        //TODO if nativeFun actually points to NativeCall::Call or NativeMemberCall::Call, and we can
        //figure that out somehow, we do not have to go through the double conversion native->mp_obj_t->native
        const auto nativeFun = (mp_obj_fun_builtin_t*) arg;
        return make_fun::Native( nativeFun );
      }
      else
      {
        return make_fun::PythonFun( arg );
      }
    }
  };


  //Check if a qualifier for one of the supported FromPyObj types is supported -
  //this is the case if it's passed without being modified, i.e. by value, by const value of by const reference.
  //Note this only check qualifiers, whether actual type is supported is told by FromPyObj< T >::value.
  template< class T >
  struct IsSupportedFromPyObjQualifier : std::is_same< T, typename remove_all< T >::type >
  {
  };

  template< class T >
  struct IsSupportedFromPyObjQualifier< const T > : std::true_type
  {
  };

  template< class T >
  struct IsSupportedFromPyObjQualifier< const T& > : std::true_type
  {
  };

  //Retrieve ClassWrapper from mp_obj_t
  template< class T >
  struct ClassFromPyObj;

  //Select bewteen FromPyObj and ClassFromPyObj
  template< class T >
  struct SelectFromPyObj
  {
    typedef FromPyObj< typename remove_all< T >::type > builtin_type;
    typedef ClassFromPyObj< typename remove_all_const< T >::type > class_type;

    typedef typename std::conditional< builtin_type::value, IsSupportedFromPyObjQualifier< T >, std::true_type >::type is_valid_builtinq;
    static_assert( is_valid_builtinq::value, "Unsupported qualifier for builtin uPy types (must be passed by value or const reference)" );

    typedef typename std::conditional< builtin_type::value, builtin_type, class_type >::type type;
  };

#ifndef UPYWRAP_NOCHARSTRING
  //we don't know yet where uPy is going with unicode support etc, so make this an option
  template<>
  struct SelectFromPyObj< const char* >
  {
    typedef FromPyObj< const char* > type;
  };

  inline bool HasCharString()
  {
    return true;
  }
#else
  inline bool HasCharString()
  {
    return false;
  }
#endif

  template<>
  struct SelectFromPyObj< mp_obj_t >
  {
    typedef FromPyObj< mp_obj_t > type;
  };
}

#endif
