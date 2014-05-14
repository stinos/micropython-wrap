#ifndef MICROPYTHONMODULE_MICROPYTHON_H
#define MICROPYTHONMODULE_MICROPYTHON_H

extern "C"
{
  #include <micropython/py/misc.h>
  #include <micropython/py/mpconfig.h>
  #include <micropython/py/qstr.h>
  #include <micropython/py/nlr.h>
  #include <micropython/py/obj.h>
  #include <micropython/py/objfun.h>
  #include <micropython/py/objmodule.h>
  #include <micropython/py/runtime.h>
}

#include <limits>
#include <cmath>

namespace upywrap
{
  inline mp_obj_module_t* CreateModule( const char* name )
  {
    const qstr qname = qstr_from_str( name );
    mp_obj_module_t* mod = (mp_obj_module_t*) mp_obj_new_module( qname );
    mp_module_register( qname, mod );
    return mod;
  }

  inline void RaiseTypeException( const char* msg )
  {
    nlr_raise( mp_obj_new_exception_msg( &mp_type_TypeError, msg ) );
  }

  //Implement some casts used and check for overflow where trunctaion is needed.
  //Only implemented for signed/unsiged 64bit to corresponding 32bit, rest resolves to static_cast.
  template< class S, class T >
  struct safe_integer_caster
  {
    static T Convert( S src )
    {
      return static_cast< T >( src );
    }
  };

#if defined( __LP64__ ) || defined( _WIN64 )
  template<>
  struct safe_integer_caster< machine_int_t, int >
  {
    static int Convert( machine_int_t src )
    {
      if( std::abs( src ) > std::numeric_limits< int >::max() )
        RaiseTypeException( "Argument does not fit in 32bit integer" );
      return static_cast< int >( src );
    }
  };

  template<>
  struct safe_integer_caster< std::size_t, unsigned >
  {
    static unsigned Convert( std::size_t src )
    {
      if( src > std::numeric_limits< unsigned >::max() )
        RaiseTypeException( "Argument does not fit in 32bit integer" );
      return static_cast< unsigned >( src );
    }
  };
#endif

  template< class T, class S >
  T safe_integer_cast( S src )
  {
    return safe_integer_caster< S, T >::Convert( src );
  }

}

#endif
