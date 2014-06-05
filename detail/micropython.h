#ifndef MICROPYTHONMODULE_MICROPYTHON_H
#define MICROPYTHONMODULE_MICROPYTHON_H

extern "C"
{
  #include <py/mpconfig.h>
  #include <py/misc.h>
  #include <py/qstr.h>
  #include <py/nlr.h>
  #include <py/obj.h>
#ifdef _MSC_VER
#pragma warning ( disable : 4200 )
#endif
  #include <py/objfun.h>
#ifdef _MSC_VER
#pragma warning ( default : 4200 )
#endif
  #include <py/objmodule.h>
  #include <py/runtime.h>
}

#include <limits>
#include <cmath>

namespace upywrap
{
  inline mp_obj_module_t* CreateModule( const char* name, bool doRegister = false )
  {
    const qstr qname = qstr_from_str( name );
    mp_obj_module_t* mod = (mp_obj_module_t*) mp_obj_new_module( qname );
    if( doRegister )
      mp_module_register( qname, mod );
    return mod;
  }

  inline void RaiseTypeException( const char* msg )
  {
    nlr_raise( mp_obj_new_exception_msg( &mp_type_TypeError, msg ) );
  }

  inline mp_obj_t RaiseRuntimeException( const char* msg )
  {
    nlr_raise( mp_obj_new_exception_msg( &mp_type_RuntimeError, msg ) );
  }

#ifdef UPYWRAP_NOEXCEPTIONS
  #define UPYWRAP_TRY
  #define UPYWRAP_CATCH
  bool HasExceptions()
  {
    return false;
  }
#else
  #define UPYWRAP_TRY try {
  #define UPYWRAP_CATCH } catch( const std::exception& e ) { return RaiseRuntimeException( e.what() ); }
  bool HasExceptions()
  {
    return true;
  }
#endif

#if MICROPY_ENABLE_GC && MICROPY_ENABLE_FINALISER
  bool HasFinaliser()
  {
    return true;
  }
#else
  #define UPYWRAP_NOFINALISER
  bool HasFinaliser()
  {
    return false;
  }
#endif

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
