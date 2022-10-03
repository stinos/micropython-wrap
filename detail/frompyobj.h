#ifndef MICROPYTHON_WRAP_DETAIL_FROMPYOBJ_H
#define MICROPYTHON_WRAP_DETAIL_FROMPYOBJ_H

#include "micropython.h"
#include "topyobj.h"
#include <functional>

namespace upywrap
{
  template< class T >
  struct SelectFromPyObj;

  //Test if the given type is a ClassWrapper< T >'s type
  template< class T >
  bool IsClassWrapperOfType( const mp_obj_type_t& type );

  //Retrieve ClassWrapper from mp_obj_t
  template< class T >
  struct ClassFromPyObj;

  //Extract Arg from mp_obj_t
  template< class Arg >
  struct FromPyObj : std::false_type
  {
  };

  template<>
  struct FromPyObj< void > : std::true_type
  {
    static void Convert( mp_obj_t )
    {
    }
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

  namespace detail
  {
    //uPy has no built-in conversion to unsigned integer.
    //Define it here so we can make use of the full range: we could also use
    //mp_obj_get_int but that only gives half of the range before overflowing
    //If arg is an mp_obj_int_t conversion is done via largeIntConv since the
    //implementation is different for 32bit and 64bit builds.
    template< class LargeIntToT >
    auto mp_obj_get_uint( mp_const_obj_t arg, LargeIntToT largeIntConv ) ->
#if UPYWRAP_HAS_CPP20
      typename std::invoke_result_t< decltype( largeIntConv ), mp_obj_int_t* >
#else
      typename std::result_of< decltype( largeIntConv )( mp_obj_int_t* ) >::type
#endif
    {
      if( arg == mp_const_false )
      {
        return 0u;
      }
      else if( arg == mp_const_true )
      {
        return 1u;
      }
      else if( mp_obj_is_small_int( arg ) )
      {
        using return_t = decltype( largeIntConv( nullptr ) );
        return safe_integer_cast< return_t >( MP_OBJ_SMALL_INT_VALUE( arg ) );
      }
      else if( mp_obj_is_exact_type( arg, &mp_type_int ) )
      {
        return largeIntConv( (mp_obj_int_t*) arg );
      }
      else
      {
        RaiseTypeException( arg, "unsigned integer" );
      }
#if !defined( _MSC_VER ) || defined( _DEBUG )
      return 0u;
#endif
    }

    //mpz -> mp_uint_t for use with mp_obj_get_uint, all builds
    static mp_uint_t mpz_to_uint( const mp_obj_int_t* self )
    {
      mp_uint_t value;
      if( mpz_as_uint_checked( &self->mpz, &value ) )
      {
        return value;
      }
      RaiseOverflowException( "Value too large for integer" );
#if !defined( _MSC_VER ) || defined( _DEBUG )
      return 0u;
#endif
    }
  }

  template<>
  struct FromPyObj< mp_uint_t > : std::true_type
  {
    static mp_uint_t Convert( mp_obj_t arg )
    {
      return detail::mp_obj_get_uint( arg, detail::mpz_to_uint );
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

  template<>
  struct FromPyObj< std::int16_t > : std::true_type
  {
    static std::int16_t Convert( mp_obj_t arg )
    {
      return safe_integer_cast< std::int16_t >( FromPyObj< mp_int_t >::Convert( arg ) );
    }
  };

  template<>
  struct FromPyObj< std::uint16_t > : std::true_type
  {
    static std::uint16_t Convert( mp_obj_t arg )
    {
      return safe_integer_cast< std::uint16_t >( FromPyObj< mp_uint_t >::Convert( arg ) );
    }
  };

#if defined( __LP64__ ) || defined( _WIN64 )
  //64bit build, cast 32bit integers from 64bit native uPy type while checking for overflow

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
    static unsigned Convert( mp_obj_t arg )
    {
      return safe_integer_cast< unsigned >( FromPyObj< mp_uint_t >::Convert( arg ) );
    }
  };
#else
  //32bit build, 64bit integers are handled through mpz

  namespace detail
  {
    //mpz -> 64bit integer for 32bit builds
    inline std::uint64_t mpz_to_64bit_int( const mp_obj_int_t* arg, bool is_signed )
    {
      static_assert( MPZ_DIG_SIZE == 16, "Expected MPZ_DIG_SIZE == 16" );

      //see mpz_as_int_checked
      const std::uint64_t maxCalcThreshold = is_signed ? 140737488355327 : 281474976710655;

      auto i = &arg->mpz;
      if( !is_signed && i->neg )
      {
        RaiseTypeException( "Source integer must be unsigned" );
      }

      auto d = i->dig + i->len;
      std::uint64_t val = 0;

      while( d-- > i->dig )
      {
        if( val > maxCalcThreshold )
        {
          RaiseOverflowException( "Value too large for 64bit integer" );
        }
        val = ( val << MPZ_DIG_SIZE ) | *d;
      }

#ifdef _MSC_VER
  #pragma warning( disable : 4146 )
#endif
      if( i->neg )
      {
        val = -val;
      }
#ifdef _MSC_VER
  #pragma warning( default : 4146 )
#endif

      return val;
    }
  }

  template<>
  struct FromPyObj< std::int64_t > : std::true_type
  {
    static std::int64_t Convert( mp_obj_t arg )
    {
      const auto val = detail::mp_obj_get_uint( arg, [] ( const mp_obj_int_t* s ) { return detail::mpz_to_64bit_int( s, true ); } );
      return static_cast< std::int64_t >( val );
    }
  };

  template<>
  struct FromPyObj< std::uint64_t > : std::true_type
  {
    static std::uint64_t Convert( mp_obj_t arg )
    {
      return detail::mp_obj_get_uint( arg, [] ( const mp_obj_int_t* s ) { return detail::mpz_to_64bit_int( s, false ); } );
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
      return safe_integer_cast< float >( FromPyObj< double >::Convert( arg ) );
    }
  };

  template<>
  struct FromPyObj< std::string > : std::true_type
  {
    static std::string Convert( mp_obj_t arg )
    {
      size_t len;
      auto chars = mp_obj_str_get_data( arg, &len );
      return std::string( chars, len );
    }
  };

#if UPYWRAP_HAS_CPP17
  template<>
  struct FromPyObj< std::string_view > : std::true_type
  {
    static std::string_view Convert( mp_obj_t arg )
    {
      size_t len;
      auto chars = mp_obj_str_get_data( arg, &len );
      return std::string_view( chars, len );
    }
  };

  inline bool HasStringView()
  {
    return true;
  }
#else
  inline bool HasStringView()
  {
    return false;
  }
#endif

  template<>
  struct FromPyObj< const char* > : std::true_type
  {
    static const char* Convert( mp_obj_t arg )
    {
      return mp_obj_str_get_str( arg );
    }
  };

  #if UPYWRAP_HAS_CPP17
  template< class T >
  struct FromPyObj< std::optional< T > > : std::true_type
  {
    static std::optional< T > Convert( mp_obj_t arg )
    {
      if( arg == mp_const_none )
      {
        return std::nullopt;
      }
      return SelectFromPyObj< T >::type::Convert( arg );
    }
  };

  inline bool HasOptional()
  {
    return true;
  }
  #else
  inline bool HasOptional()
  {
    return false;
  }
  #endif

  template< class T >
  struct FromPyObj< std::vector< T > > : std::true_type
  {
    typedef std::vector< T > vec_type;

    static vec_type Convert( mp_obj_t arg )
    {
      size_t len;
      mp_obj_t* items;
      mp_obj_get_array( arg, &len, &items ); //works for list and tuple
      vec_type ret( len );
      std::transform( items, items + len, ret.begin(), SelectFromPyObj< T >::type::Convert );
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
      const auto dict = static_cast< mp_obj_dict_t* >( MP_OBJ_TO_PTR( arg ) );
      const auto map = &dict->map;
      //this is basically dict_iter_next but that isn't exposed as an API method
      for( size_t i = 0 ; i < map->alloc ; ++i )
      {
        if( mp_map_slot_is_filled( map, i ) )
        {
          ret.emplace( SelectFromPyObj< K >::type::Convert( map->table[ i ].key ),
                       SelectFromPyObj< V >::type::Convert( map->table[ i ].value ) );
        }
      }
      return ret;
    }
  };

  template< template < class... > class TupleLike, class... A >
  struct TupleFromPyObj
  {
    using tuple_type = TupleLike< A... >;

    static tuple_type Convert( mp_obj_t arg )
    {
      size_t len;
      mp_obj_t* items;
      mp_obj_get_array( arg, &len, &items );
      if( len != sizeof...( A ) )
      {
        RaiseTypeException( "Not enough tuple elements" );
      }
      return make_it( items, make_index_sequence< sizeof...( A ) >() );
    }

    template< size_t... Indices >
    static tuple_type make_it( const mp_obj_t* args, index_sequence< Indices... > )
    {
      return tuple_type( SelectFromPyObj< A >::type::Convert( args[ Indices ] )... );
    }
  };

  template< class... A >
  struct FromPyObj< std::tuple< A... > > : TupleFromPyObj< std::tuple, A... >, std::true_type
  {
  };

  template< class A, class B >
  struct FromPyObj< std::pair< A, B > > : TupleFromPyObj< std::pair, A, B >, std::true_type
  {
  };

  namespace detail
  {
    /**
      * Wrap a uPy function call in an std::function.
      * If the function call is a bound method we need to make sure the uPy object is protected
      * from being GC'd as long as the corresponding std::function instance is alive: the uPy object
      * gets captured in the lambda and as such is stored in the std::function. However that is out
      * of reach for the GC mark phase so it might get sweeped leading to nasty crashes when the
      * function is called afterwards. Fix this by storing the uPy object in a PinPyObj: it will
      * have the same lifetime as the function then, no matter how many times it is copied.
      * For mp_obj_fun_builtin_fixed_t/mp_obj_fun_builtin_var_t calls these measures aren't
      * (shouldn't be?) needed since those:
      * - are either one of uPy's functions which are statically defined using MP_DEFINE_CONST_FUN_XXX
      *   so not even considered for GC since they are not on the heap
      * - or else point to a function created by ClassWrapper or FunctionWrapper and those
          are explcicitly added to a uPy dict already to make sure they are never collected
      */
    template< class R, class... Args >
    struct MakeStdFun
    {
      typedef typename std::function< R( Args... ) > std_fun_type;
      typedef mp_obj_t( *py_fun_type )( typename project2nd< Args, mp_obj_t >::type... );

      static std_fun_type Native( const mp_obj_fun_builtin_fixed_t* nativeFun )
      {
        const auto nativeFunPtr = reinterpret_cast< py_fun_type >( nativeFun->fun._0 );
        return std_fun_type(
          [nativeFunPtr] ( Args... args ) -> R
          {
            return SelectFromPyObj< R >::type::Convert( nativeFunPtr( ToPy< Args >( args )... ) );
          } );
      }

      static std_fun_type Native( const mp_obj_fun_builtin_var_t* nativeFun )
      {
        const auto nativeFunPtr = nativeFun->fun.var;
        return std_fun_type(
          [nativeFunPtr] ( Args... args ) -> R
          {
            //+1 to avoid zero-sized array which is illegal for msvc
            mp_obj_t objs[ sizeof...( Args ) + 1 ] = { ToPy< Args >( args )... };
            return SelectFromPyObj< R >::type::Convert( nativeFunPtr( sizeof...( Args ), objs ) );
          } );
      }

      static std_fun_type PythonFun( mp_obj_t fun )
      {
        const PinPyObj pin( fun );
        return std_fun_type(
          [pin] ( Args... args ) -> R
          {
            mp_obj_t objs[ sizeof...( Args ) + 1 ] = { ToPy< Args >( args )... };
            return SelectFromPyObj< R >::type::Convert( mp_call_function_n_kw( pin.Get(), sizeof...( Args ), 0, objs ) );
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
      if( arg == mp_const_none )
      {
        return std_fun_type();
      }
      const auto obj = reinterpret_cast< mp_obj_base_t* >( MP_OBJ_TO_PTR( arg ) );
      const auto type = obj->type;
      if( type == &mp_type_fun_builtin_0 ||
          type == &mp_type_fun_builtin_1 ||
          type == &mp_type_fun_builtin_2 ||
          type == &mp_type_fun_builtin_3 )
      {
        const auto nativeFun = reinterpret_cast< mp_obj_fun_builtin_fixed_t* >( obj );
        return make_fun::Native( nativeFun );
      }
      else if( type == &mp_type_fun_builtin_var )
      {
        const auto nativeFun = reinterpret_cast< mp_obj_fun_builtin_var_t* >( obj );
        return make_fun::Native( nativeFun );
      }
      else if( IsClassWrapperOfType< std_fun_type >( *type ) )
      {
        //If the argument is a ClassWrapper< std_fun_type > just get the function object
        //without any conversion. This is simpler and more performant than wrapping it as
        //a callable with PythonFun. But there's also another important aspect: it enables
        //passing std::function around between native methods without any uPy API calls
        //in between. Which is essential when the std::function is going to be used by
        //another thread: PythonFun creates an std::function using mp_call_function_n_kw.
        //That means it might allocate from the uPy heap and/or pystack and that is
        //undefined behavior when done from a different thread than the one running the
        //interpreter. Also the actual call of the std::function is done by CallReturn so
        //C++ exceptions get translated into nlr_jump, but that just crashes because there
        //is no corresponding nlr_push call first.
        return ClassFromPyObj< std_fun_type >::Convert( obj );
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

#if UPYWRAP_USE_CHARSTRING
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

  template< class T >
  auto FromPy( const mp_obj_t arg ) -> decltype( SelectFromPyObj< T >::type::Convert( arg ) )
  {
    return SelectFromPyObj< T >::type::Convert( arg );
  }

  //Helper for getting an argument in a function with variable number of arguments.
  template< class T >
  T FromPy( mp_uint_t numArgs, const mp_obj_t* args, mp_uint_t argIndex, const T& def )
  {
    return numArgs > argIndex ? FromPy< T >( args[ argIndex ] ) : def;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_FROMPYOBJ_H
