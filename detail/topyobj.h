#ifndef MICROPYTHON_WRAP_DETAIL_TOPYOBJ_H
#define MICROPYTHON_WRAP_DETAIL_TOPYOBJ_H

#include "micropython.h"
#include "util.h"
#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#if UPYWRAP_HAS_CPP17
#include <optional>
#endif
#if UPYWRAP_THROW_ERROR_CODE
#include <system_error>
#endif

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
      return mp_obj_new_str( reinterpret_cast< const char* >( a.data() ), a.length() );
    }
  };

#if UPYWRAP_HAS_CPP17
  template<>
  struct ToPyObj< std::string_view > : std::true_type
  {
    static mp_obj_t Convert( const std::string_view& a )
    {
      return mp_obj_new_str( a.data(), a.length() );
    }
  };
#endif

  template<>
  struct ToPyObj< const char* > : std::true_type
  {
    static mp_obj_t Convert( const char* a )
    {
      return mp_obj_new_str( a, ::strlen( a ) );
    }
  };

  #if UPYWRAP_HAS_CPP17
  template< class T >
  struct ToPyObj< std::optional< T > > : std::true_type
  {
    static mp_obj_t Convert( const std::optional< T >& arg )
    {
      if( arg )
      {
        return SelectToPyObj< T >::type::Convert( *arg );
      }
      return mp_const_none;
    }
  };
  #endif

  #if UPYWRAP_THROW_ERROR_CODE
  template<>
  struct ToPyObj< std::error_code > : std::true_type
  {
    static mp_obj_t Convert( const std::error_code& ec )
    {
      if( ec )
      {
        throw std::runtime_error( ec.message() );
      }
      return mp_const_none;
    }
  };

  inline bool HasErrorCode()
  {
    return true;
  }
  #else
  inline bool HasErrorCode()
  {
    return false;
  }
  #endif

  //Generic conversion of pair of iterators to uPy list, so external code
  //can use this to build converters for more types than just vector.
  template< class It, class Transform >
  static mp_obj_t ConvertToList( It begin, size_t numItems, Transform transform )
  {
    auto list = reinterpret_cast< mp_obj_list_t* >( MP_OBJ_TO_PTR( mp_obj_new_list( numItems, nullptr ) ) );
    std::transform( begin, begin + numItems, list->items, transform );
    return list;
  }

  template< class T >
  struct ToPyObj< std::vector< T > > : std::true_type
  {
    static mp_obj_t Convert( const std::vector< T >& a )
    {
      return ConvertToList( a.cbegin(), a.size(), SelectToPyObj< T >::type::Convert );
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
      AddConvertedToVec( mp_obj_t* items ) :
        items( items ),
        next( 0 )
      {
      }

      template< class T >
      void operator ()( const T& a )
      {
        items[ next ] = SelectToPyObj< T >::type::Convert( a );
        ++next;
      }

    private:
      mp_obj_t* items;
      size_t next;
    };
  }

  template< class... A >
  struct ToPyObj< std::tuple< A... > > : std::true_type
  {
    typedef std::tuple< A... > tuple_type;

    static mp_obj_t Convert( const tuple_type& a )
    {
      const auto numItems = sizeof...( A );
      auto tuple = reinterpret_cast< mp_obj_tuple_t* >( MP_OBJ_TO_PTR( mp_obj_new_tuple( numItems, nullptr ) ) );
      detail::AddConvertedToVec addtoVec( tuple->items );
      apply( addtoVec, a );
      return tuple;
    }
  };

  template< class A, class B >
  struct ToPyObj< std::pair< A, B > > : std::true_type
  {
    static mp_obj_t Convert( const std::pair< A, B >& p )
    {
      return ToPyObj< std::tuple< A, B > >::Convert( p );
    }
  };

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

#if UPYWRAP_USE_CHARSTRING
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

  template< class T >
  mp_obj_t ToPy( const T& arg )
  {
    return SelectToPyObj< T >::type::Convert( arg );
  }

#if UPYWRAP_USE_CHARSTRING
  inline mp_obj_t ToPy( const char* arg )
  {
    return SelectToPyObj< const char* >::type::Convert( arg );
  }
#endif

  inline mp_obj_t ToPy( mp_obj_t arg )
  {
    return SelectToPyObj< mp_obj_t >::type::Convert( arg );
  }

  template< class T >
  mp_obj_t ToPy( T& arg, typename std::enable_if< !ToPyObj< typename remove_all< T >::type >::value && !is_shared_ptr< T >::value >::type* = nullptr )
  {
    return SelectToPyObj< T& >::type::Convert( arg );
  }

  template< class T >
  mp_obj_t ToPy( std::shared_ptr< T > arg, typename std::enable_if< !ToPyObj< typename remove_all< T >::type >::value && !std::is_reference< T >::value >::type* = nullptr )
  {
    return SelectToPyObj< std::shared_ptr< T > >::type::Convert( arg );
  }
}

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_TOPYOBJ_H
