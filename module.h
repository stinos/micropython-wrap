#ifndef MICROPYTHON_WRAP_MODULE_H
#define MICROPYTHON_WRAP_MODULE_H

#ifdef _MSC_VER
#define UPY_MODULE( name ) extern "C" __declspec( dllexport ) mp_obj_module_t* init_##name()
#else
#define UPY_MODULE( name ) extern "C" mp_obj_module_t* init_##name()
#endif

#endif //#ifndef MICROPYTHON_WRAP_MODULE_H
