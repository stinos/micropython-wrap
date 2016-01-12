#ifndef MICROPYTHON_WRAP_UTIL_H
#define MICROPYTHON_WRAP_UTIL_H

#include "detail/micropython.h"

namespace upywrap
{
  inline std::string ExceptionToString( mp_obj_t ex )
  {
    std::string exMessage;
    const mp_print_t mp_my_print{ &exMessage, [] ( void* data, const char* str, mp_uint_t len ) { ( (std::string*) data )->append( str, len ); } };
    mp_obj_print_exception( &mp_my_print, ex );
    return exMessage;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_UTIL_H
