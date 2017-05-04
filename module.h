#ifndef MICROPYTHON_WRAP_MODULE_H
#define MICROPYTHON_WRAP_MODULE_H

#include "detail/topyobj.h"

#ifdef _MSC_VER
#define UPY_MODULE( name ) extern "C" __declspec( dllexport ) mp_obj_module_t* init_##name()
#else
#define UPY_MODULE( name ) extern "C" mp_obj_module_t* init_##name()
#endif

namespace upywrap
{
template< class T >
void StoreGlobal( mp_obj_module_t* mod, const char* name, const T& obj )
{
  mp_obj_dict_store( MP_OBJ_FROM_PTR( mod->globals ), new_qstr( name ), ToPyObj< T >::Convert( obj ) );
}
}

#endif //#ifndef MICROPYTHON_WRAP_MODULE_H
