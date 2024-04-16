#ifndef MICROPYTHON_WRAP_MODULE_H
#define MICROPYTHON_WRAP_MODULE_H

#include "detail/micropythonc.h"

/**
  * Declare the module initialization function when building a dynamic module
  * (the ones enabled with with MICROPY_MODULE_LOADDYNLIB).
  * Usage:
  * UPY_MODULE(mymodule)
  * {
  *   auto mod = upywrap::CreateModule("mymodule");
  *   RegsiterFunctions(mod);
  *   return mod;
  * }
  */
#ifdef _MSC_VER
#define UPY_MODULE( name ) extern "C" __declspec( dllexport ) mp_obj_module_t* init_##name()
#else
#define UPY_MODULE( name ) extern "C" mp_obj_module_t* init_##name()
#endif


/**
  * Support for building modules which work with the USER_C_MODULES feature of MicroPython.
  * To get this working we need to work around the fact that such module requires
  * a static definition (i.e. there must be a compile-time mp_obj_module_t available),
  * however our modules get populated dynamically at runtime.
  * There are a basically two ways to do this:
  * 1. Use MICROPY_MODULE_ATTR_DELEGATION to forward all module attribute loading to a function
  *    which looks up attributes in a map (defined as root pointer), that map having been populated by
  *    the upywrap function/class registration functions. That registration has to be triggered
  *    somehow, which can be done once in the function which loads attributes.
  * 2. Use MICROPY_MODULE_BUILTIN_INIT and point it to a function which replaces the module's
  *    global dict with a new one, populated by the upywrap function/class registration functions.
  *    That's a bit of a hack but should work without issues.
  *
  * The below functions and macros define the different ways described above so one can pick
  * whatever is most suitable.
  */

/**
  * Initialize a dict and populate again via another function.
  * This is for use with a module's globals dict, so __name__ gets set for consistency.
  * Additionally the dict's table gets registered as root pointer, i.e. the rootptr argument is
  * supposed to point to a pointer reachable by the GC, and gets set to the table so that all
  * its entries are reachable; this is required for micropython-wrap as it might store micropython
  * heap-allocated objects in this table which should never be GC'd.
  */
static inline void init_module_globals(mp_obj_dict_t *globals, const mp_map_elem_t** rootptr, qstr name, void (*initter)(mp_obj_dict_t *)) {
    mp_map_init(&globals->map, 1);
    mp_obj_dict_store(MP_OBJ_FROM_PTR(globals), MP_ROM_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(name));
    initter(globals);
    *rootptr = globals->map.table;
}

/**
  * Lookup attr in dict and store in dest if found.
  */
static inline void dict_lookup(mp_map_t *src, qstr attr, mp_obj_t *dest) {
    mp_map_elem_t *el = mp_map_lookup(src, MP_OBJ_NEW_QSTR(attr), MP_MAP_LOOKUP);
    if (el != NULL) {
        *dest = el->value;
    }
}

/**
  * Define a module which finds attributes in a dict which is initialized once in the same function.
  * This is option 1 mentioned above, so requires MICROPY_MODULE_ATTR_DELEGATION.
  * Usage:
  * extern void init_mymodule(mp_obj_dict_t *); //Actual function/class registration with upywrap.
  * UPYWRAP_DEFINE_ATTR_MODULE(mymodule, init_mymodule);
  * MP_REGISTER_MODULE(MP_QSTR_mymodule, mymodule_module);
  * MP_REGISTER_ROOT_POINTER(const mp_map_elem_t* mymodule_globals_table);
  */
#define UPYWRAP_DEFINE_ATTR_MODULE(name, initter) \
  void name##_module_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) { \
      if (name##_module.globals->map.table == NULL) { \
        init_module_globals(name##_module.globals, &MP_STATE_VM(name##_module_globals_table), MP_QSTR_##name, initter); \
      } \
      dict_lookup(&name##_module.globals->map, attr, dest); \
  }\
  MP_REGISTER_MODULE_DELEGATION(name##_module, name##_module_attr); \
  static mp_obj_dict_t name##_module_globals = { \
    .base = { &mp_type_dict },  \
  };\
  const mp_obj_module_t name##_module = { \
      .base = { &mp_type_module }, \
      .globals = (mp_obj_dict_t *)&name##_module_globals, \
  };

/**
  * Define a module which gets initialized with its __init__ function forwarding to init_module.
  * This is option 2 mentioned above, so requires MICROPY_MODULE_BUILTIN_INIT.
  * Usage:
  * extern void init_mymodule(mp_obj_dict_t *); //Actual function/class registration with upywrap.
  * UPYWRAP_DEFINE_INIT_MODULE(mymodule, init_mymodule);
  * MP_REGISTER_MODULE(MP_QSTR_mymodule, mymodule_module);
  * MP_REGISTER_ROOT_POINTER(const mp_map_elem_t* mymodule_globals_table);
  */
#define UPYWRAP_DEFINE_INIT_MODULE(name, initter) \
  extern const struct _mp_obj_module_t name##_module; \
  static mp_obj_t init_##name##_module(void) { \
      init_module_globals(name##_module.globals, &MP_STATE_VM(name##_module_globals_table), MP_QSTR_##name, initter); \
      return mp_const_none; \
  } \
  static MP_DEFINE_CONST_FUN_OBJ_0(init_##name##_module_obj, init_##name##_module); \
  static mp_rom_map_elem_t name##_module_globals_table[] = { \
      { MP_ROM_QSTR(MP_QSTR___init__), MP_ROM_PTR(&init_##name##_module_obj) } \
  }; \
  mp_obj_dict_t name##_module_globals = { \
      .base = {&mp_type_dict}, \
      .map = { \
          .all_keys_are_qstrs = 1, \
          .is_fixed = 1, \
          .is_ordered = 0, \
          .used = MP_ARRAY_SIZE(name##_module_globals_table), \
          .alloc = MP_ARRAY_SIZE(name##_module_globals_table), \
          .table = (mp_map_elem_t*)(mp_rom_map_elem_t*) name##_module_globals_table, \
      } \
  }; \
  const mp_obj_module_t name##_module = { \
      .base = { &mp_type_module }, \
      .globals = (mp_obj_dict_t *)&name##_module_globals, \
  }; \


#endif //#ifndef MICROPYTHON_WRAP_MODULE_H
