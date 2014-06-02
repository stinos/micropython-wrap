#ifndef MICROPYTHONMODULE_TOPYOBJ_H
#define MICROPYTHONMODULE_TOPYOBJ_H

#include "micropython.h"
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <algorithm>

namespace upywrap
{
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
  struct ToPyObj< machine_int_t > : std::true_type
  {
    static mp_obj_t Convert( machine_int_t a )
    {
      return mp_obj_new_int( a );
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

#if defined( __LP64__ ) || defined( _WIN64 )
  template<>
  struct ToPyObj< int > : std::true_type
  {
    static mp_obj_t Convert( int arg )
    {
      return ToPyObj< machine_int_t >::Convert( safe_integer_cast< machine_int_t >( arg ) );
    }
  };
#endif

  template<>
  struct ToPyObj< mp_float_t > : std::true_type
  {
    static mp_obj_t Convert( mp_float_t a )
    {
      return mp_obj_new_float( a );
    }
  };

  template<>
  struct ToPyObj< std::string > : std::true_type
  {
    static mp_obj_t Convert( const std::string& a )
    {
      return mp_obj_new_str( reinterpret_cast< const char* >( a.data() ), safe_integer_cast< uint >( a.length() ), false );
    }
  };

  template< class T >
  struct ToPyObj< std::vector< T > > : std::true_type
  {
    static mp_obj_t Convert( const std::vector< T >& a )
    {
      const auto numItems = a.size();
      std::vector< mp_obj_t > items( numItems );
      std::transform( a.cbegin(), a.cend(), items.begin(), ToPyObj< T >::Convert );
      return mp_obj_new_list( safe_integer_cast< uint >( numItems ), items.data() );
    }
  };

  template< class K, class V >
  struct ToPyObj< std::map< K, V > > : std::true_type
  {
    static mp_obj_t Convert( const std::map< K, V >& a )
    {
      const auto numItems = a.size();
      auto dict = mp_obj_new_dict( safe_integer_cast< uint >( numItems ) );
      std::for_each( a.cbegin(), a.cend(), [&dict] ( decltype( *a.cbegin() )& p )
      {
        mp_obj_dict_store( dict, ToPyObj< K >::Convert( p.first ), ToPyObj< V >::Convert( p.second ) );
      } );
      return dict;
    }
  };

  template< class... A >
  struct ToPyObj< std::tuple< A... > > : std::true_type
  {
    typedef std::tuple< A... > tuple_type;

    struct AddConvertedToVec
    {
      std::vector< mp_obj_t > items;

      template< class T >
      void operator ()( const T& a )
      {
        items.push_back( ToPyObj< T >::Convert( a ) );
      }
    };

    static mp_obj_t Convert( const tuple_type& a )
    {
      const auto numItems = sizeof...( A );
      AddConvertedToVec addtoVec;
      apply( addtoVec, a );
      return mp_obj_new_tuple( safe_integer_cast< uint >( numItems ), addtoVec.items.data() );
    }
  };


  //Check if a qualifier for one of the supported ToPyObj types is supported -
  //this is the case only if it's returned by value
  template< class T >
  struct IsSupportedToPyObjQualifier : std::is_same< T, typename remove_all< T >::type >
  {
  };

  //Store ClassWrapper in mp_obj_t
  template< class T >
  struct ClassToPyObj;

  //Select bewteen FromPyObj and ClassFromPyObj
  template< class T >
  struct SelectToPyObj
  {
    typedef ToPyObj< typename remove_all< T >::type > builtin_type;
    typedef ClassToPyObj< typename remove_all_const< T >::type > class_type;

    typedef typename std::conditional< builtin_type::value, IsSupportedToPyObjQualifier< T >, std::true_type >::type is_valid_builtinq;
    static_assert( is_valid_builtinq::value, "Unsupported qualifier for builtin uPy types (must be returned by value)" );

    typedef typename std::conditional< builtin_type::value, builtin_type, class_type >::type type;
  };
}

#endif
