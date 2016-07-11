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
}

#endif //#ifndef MICROPYTHON_WRAP_UTIL_H
