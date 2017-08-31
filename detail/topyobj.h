#ifndef MICROPYTHON_WRAP_DETAIL_TOPYOBJ_H
#define MICROPYTHON_WRAP_DETAIL_TOPYOBJ_H

#include "util.h"
#include "micropython.h"
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>
#include <cstring>

namespace upywrap
{
  template< class T >
  struct SelectToPyObj;

  //Create mp_obj_t from Ret
  template< class Ret >
  struct ToPyObj : std::false_type
  {
  };

  template<>
  struct ToPyObj< void > : std::true_type
  {
    static mp_obj_t Convert()
    {
      return mp_const_none;
    }
  };

  template<>
  struct ToPyObj< mp_obj_t > : std::true_type
  {
    static mp_obj_t Convert( mp_obj_t arg )
    {
      return arg;
    }
  };

  template<>
  struct ToPyObj< mp_int_t > : std::true_type
  {
    static mp_obj_t Convert( mp_int_t a )
    {
      return mp_obj_new_int( a );
    }
  };

  template<>
  struct ToPyObj< mp_uint_t > : std::true_type
  {
    static mp_obj_t Convert( mp_uint_t a )
    {
      return mp_obj_new_int_from_uint( a );
    }
  };

  template<>
  struct ToPyObj< bool > : std::true_type
  {
    static mp_obj_t Convert( bool a )
    {
      return a ? mp_const_true : mp_const_false;
    }
  };

  template<>
  struct ToPyObj< std::int16_t > : std::true_type
  {
    static mp_obj_t Convert( std::int16_t arg )
    {
      return ToPyObj< mp_int_t >::Convert( static_cast< std::int16_t >( arg ) );
    }
  };

  template<>
  struct ToPyObj< std::uint16_t > : std::true_type
  {
    static mp_obj_t Convert( std::uint16_t arg )
    {
      return ToPyObj< mp_uint_t >::Convert( static_cast< std::uint16_t >( arg ) );
    }
  };

#if defined( __LP64__ ) || defined( _WIN64 )
  //64bit build, we can safely cast 32bit integers to 64bit 'native' uPy integer
  static_assert( std::is_same< mp_int_t, std::int64_t >::value, "unsupported integer type" );
  static_assert( std::is_same< mp_uint_t, std::uint64_t >::value, "unsupported integer type" );

  template<>
  struct ToPyObj< std::int32_t > : std::true_type
  {
    static mp_obj_t Convert( std::int32_t arg )
    {
      return ToPyObj< mp_int_t >::Convert( static_cast< mp_int_t >( arg ) );
    }
  };

  template<>
  struct ToPyObj< std::uint32_t > : std::true_type
  {
    static mp_obj_t Convert( std::uint32_t arg )
    {
      return ToPyObj< mp_uint_t >::Convert( static_cast< mp_uint_t >( arg ) );
    }
  };
#else
  //32bit build, 64bit integers are handled through mpz
  static_assert( std::is_same< mp_int_t, std::int32_t >::value, "Expected 32bit uPy integer type" );
  static_assert( std::is_same< mp_uint_t, std::uint32_t >::value, "Expected 32bit uPy integer type" );

  template<>
  struct ToPyObj< std::int64_t > : std::true_type
  {
    static mp_obj_t Convert( std::int64_t arg )
    {
      static_assert( std::is_same< long long, std::int64_t >::value, "Expected 64bit long long" );
      return mp_obj_new_int_from_ll( arg );
    }
  };

  template<>
  struct ToPyObj< std::uint64_t > : std::true_type
  {
    static mp_obj_t Convert( std::uint64_t arg )
    {
      static_assert( std::is_same< unsigned long long, std::uint64_t >::value, "Expected 64bit long long" );
      return mp_obj_new_int_from_ull( arg );
    }
  };
#endif

  template<>
  struct ToPyObj< double > : std::true_type
  {
    static mp_obj_t Convert( double a )
    {
      return mp_obj_new_float( a );
    }
  };

  template<>
  struct ToPyObj< float > : std::true_type
  {
    static mp_obj_t Convert( float a )
    {
      return mp_obj_new_float( static_cast< double >( a ) );
    }
  };

  template<>
  struct ToPyObj< std::string > : std::true_type
  {
    static mp_obj_t Convert( const std::string& a )
    {
      return mp_obj_new_str( reinterpret_cast< const char* >( a.data() ), a.length(), false );
    }
  };

  template<>
  struct ToPyObj< const char* > : std::true_type
  {
    static mp_obj_t Convert( const char* a )
    {
      return mp_obj_new_str( a, ::strlen( a ), false );
    }
  };

  template< class T >
  struct ToPyObj< std::vector< T > > : std::true_type
  {
    static mp_obj_t Convert( const std::vector< T >& a )
    {
      const auto numItems = a.size();
      std::vector< mp_obj_t > items( numItems );
      std::transform( a.cbegin(), a.cend(), items.begin(), SelectToPyObj< T >::type::Convert );
      return mp_obj_new_list( numItems, items.data() );
    }
  };

  template< class K, class V >
  struct ToPyObj< std::map< K, V > > : std::true_type
  {
    static mp_obj_t Convert( const std::map< K, V >& a )
    {
      const auto numItems = a.size();
      auto dict = mp_obj_new_dict( numItems );
      std::for_each( a.cbegin(), a.cend(), [&dict] ( decltype( *a.cbegin() )& p )
      {
        mp_obj_dict_store( dict, SelectToPyObj< K >::type::Convert( p.first ), SelectToPyObj< V >::type::Convert( p.second ) );
      } );
      return dict;
    }
  };

  namespace detail
  {
    struct AddConvertedToVec
    {
      std::vector< mp_obj_t > items;

      template< class T >
      void operator ()( const T& a )
      {
        items.push_back( SelectToPyObj< T >::type::Convert( a ) );
      }
    };
  }

  template< class... A >
  struct ToPyObj< std::tuple< A... > > : std::true_type
  {
    typedef std::tuple< A... > tuple_type;

    static mp_obj_t Convert( const tuple_type& a )
    {
      const auto numItems = sizeof...( A );
      detail::AddConvertedToVec addtoVec;
      apply( addtoVec, a );
      return mp_obj_new_tuple( numItems, addtoVec.items.data() );
    }
  };

  template< class A, class B >
  struct ToPyObj< std::pair< A, B > > : std::true_type
  {
    static mp_obj_t Convert( const std::pair< A, B >& p )
    {
      detail::AddConvertedToVec addtoVec;
      addtoVec( p.first );
      addtoVec( p.second );
      return mp_obj_new_tuple( 2, addtoVec.items.data() );
    }
  };


  //Check if a qualifier for one of the supported ToPyObj types is supported -
  //this is the normally the case only if it's returned/passed by value since that
  //properly mathces the copy semantics: everytime we convert a supported type from
  //it's native value into a uPy object we effectively make a distinct copy.
  //However for performance reasons we allow const references as well when UPYWRAP_PASSCONSTREF is 1:
  //this allows not having to make the extra unused copy of the native argument which is made
  //when passing by value.
  //Note this does kinda breaks semantics: the uPy object does not reference the native object
  //in any way so users have to take care not to use it as such
#ifndef UPYWRAP_PASSCONSTREF
  #define UPYWRAP_PASSCONSTREF 1
#endif

#if UPYWRAP_PASSCONSTREF
  template< class T >
  struct IsSupportedToPyObjQualifier : std::integral_constant
    <
      bool,
      std::is_same< T, typename remove_all< T >::type >::value || 
      std::is_same< T, const T& >::value
    >
  {
  };
#else
  template< class T >
  struct IsSupportedToPyObjQualifier : std::is_same< T, typename remove_all< T >::type >
  {
  };
#endif

  //Store ClassWrapper in mp_obj_t
  template< class T >
  struct ClassToPyObj;

  //Select bewteen ToPyObj and ClassToPyObj
  template< class T >
  struct SelectToPyObj
  {
    typedef ToPyObj< typename remove_all< T >::type > builtin_type;
    typedef ClassToPyObj< T > class_type;

    typedef typename std::conditional< builtin_type::value, IsSupportedToPyObjQualifier< T >, std::true_type >::type is_valid_builtinq;
    static_assert( is_valid_builtinq::value, "Unsupported qualifier for builtin uPy types (must be returned by value)" );

    typedef typename std::conditional< builtin_type::value, builtin_type, class_type >::type type;
  };

#ifndef UPYWRAP_NOCHARSTRING
  template<>
  struct SelectToPyObj< const char* >
  {
    typedef ToPyObj< const char* > type;
  };
#endif

  template<>
  struct SelectToPyObj< mp_obj_t >
  {
    typedef ToPyObj< mp_obj_t > type;
  };
}

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_TOPYOBJ_H
