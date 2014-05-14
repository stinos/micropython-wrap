#ifndef MICROPYTHONMODULE_TOPYOBJ_H
#define MICROPYTHONMODULE_TOPYOBJ_H

#include "micropython.h"
#include <string>
#include <vector>
#include <algorithm>

namespace upywrap
{
  //Create mp_obj_t from Ret
  template< class Ret >
  struct ToPyObj;

  template<>
  struct ToPyObj< machine_int_t >
  {
    static mp_obj_t Convert( machine_int_t a )
    {
      return mp_obj_new_int( a );
    }
  };

#if defined( __LP64__ ) || defined( _WIN64 )
  template<>
  struct ToPyObj< int >
  {
    static mp_obj_t Convert( int arg )
    {
      return ToPyObj< machine_int_t >::Convert( safe_integer_cast< machine_int_t >( arg ) );
    }
  };
#endif

  template<>
  struct ToPyObj< mp_float_t >
  {
    static mp_obj_t Convert( mp_float_t a )
    {
      return mp_obj_new_float( a );
    }
  };

  template<>
  struct ToPyObj< std::string >
  {
    static mp_obj_t Convert( const std::string& a )
    {
      return mp_obj_new_str( reinterpret_cast< const byte* >( a.data() ), safe_integer_cast< uint >( a.length() ), false );
    }
  };

  template< class T >
  struct ToPyObj< std::vector< T > >
  {
    static mp_obj_t Convert( const std::vector< T >& a )
    {
      const auto numItems = a.size();
      std::vector< mp_obj_t > items( numItems );
      std::transform( a.cbegin(), a.cend(), items.begin(), ToPyObj< T >::Convert );
      return mp_obj_new_list( safe_integer_cast< uint >( numItems ), items.data() );
    }
  };
}

#endif
