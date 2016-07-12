#ifndef MICROPYTHON_WRAP_UTIL_H
#define MICROPYTHON_WRAP_UTIL_H

#include "detail/micropython.h"

namespace upywrap
{
  inline mp_print_t PrintToString( std::string& dest )
  {
    return mp_print_t{ &dest, [] ( void* data, const char* str, mp_uint_t len ) { ( (std::string*) data )->append( str, len ); } };
  }

  inline std::string ExceptionToString( mp_obj_t ex )
  {
    std::string exMessage;
    const auto mp_my_print( PrintToString( exMessage ) );
    mp_obj_print_exception( &mp_my_print, ex );
    return exMessage;
  }

  inline std::string VariableValueToString( mp_obj_t obj, mp_print_kind_t kind = PRINT_REPR )
  {
    std::string var;
    const auto mp_my_print( PrintToString( var ) );
    mp_obj_print_helper( &mp_my_print, obj, kind );
    return var;
  }

  /**
    * Wrap a function Call to uPy code in nlr_push/nlr_pop: use this to safely invoke
    * uPy code outside of the standard interpreter, to make sure nlr_raise is properly handled.
    * If an exception is raised it is passed to the HandleEx instance - better make sure the
    * latter doesn't raise an exception itself since it is outside of the nlr_push/pop.
    * Returns true if no exception was raised.
    */
  template< class Call, class HandleEx >
  bool WrapMicroPythonCall( Call f, HandleEx ex )
  {
#ifdef _MSC_VER
  #pragma warning ( disable : 4611 ) //interaction between '_setjmp' and C++ object destruction is non-portable
#endif
    nlr_buf_t nlr;
    if( nlr_push( &nlr ) == 0 )
    {
      f();
      nlr_pop();
      return true;
    }
    else
    {
      ex( nlr.ret_val );
    }
    return false;
#ifdef _MSC_VER
  #pragma warning ( default : 4611 )
#endif
  }
}

#endif //#ifndef MICROPYTHON_WRAP_UTIL_H
