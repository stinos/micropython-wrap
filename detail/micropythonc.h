#ifndef MICROPYTHON_WRAP_DETAIL_MICROPYTHONC_H
#define MICROPYTHON_WRAP_DETAIL_MICROPYTHONC_H

#include "configuration.h"

#ifdef _MSC_VER
#pragma warning ( disable : 4200 ) //nonstandard extension used : zero-sized array in struct/union
#endif
#ifdef __cplusplus
extern "C"
{
#endif
#include <py/objfun.h>
#include <py/objint.h>
#include <py/objmodule.h>
#include <py/objtype.h>
#include <py/runtime.h>
#ifdef __cplusplus
}
#endif
#ifdef _MSC_VER
#pragma warning ( default : 4200 )
#endif

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_MICROPYTHONC_H
