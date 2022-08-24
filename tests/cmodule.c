#include "py/obj.h"
#include "py/objmodule.h"

/**
Create a C module which works with the USER_C_MODULES feature of MicroPython.
To get this working we need to work around the fact that such module requires
a static definition (i.e. there must be a compile-time mp_obj_module_t available),
however our module (see module.cpp) gets populated dynamically at runtime.
So what we do is:
- supply a compile-time module with just an __init__ function pointing to init_module
- build MicroPython with the MICROPY_MODULE_BUILTIN_INIT feature enabled,
  which leads to the __init__ function getting called when the module is imported
- init_module replaces the module's global dict with a new one, then calls
  doinit_upywraptest from module.cpp to populate it
*/

extern void doinit_upywraptest(mp_obj_module_t*);
extern mp_obj_module_t module;

STATIC mp_obj_t init_module() {
    mp_map_init(&module.globals->map, 1);
    mp_obj_dict_store(MP_OBJ_FROM_PTR(module.globals), MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_upywraptest));
    doinit_upywraptest(&module);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(init_module_obj, init_module);

STATIC mp_rom_map_elem_t module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_upywraptest) },
    { MP_ROM_QSTR(MP_QSTR___init__), MP_ROM_PTR(&init_module_obj) }
};

mp_obj_dict_t module_globals = {
    .base = {&mp_type_dict},
    .map = {
        .all_keys_are_qstrs = 1,
        .is_fixed = 1,
        .is_ordered = 0,
        .used = MP_ARRAY_SIZE(module_globals_table),
        .alloc = MP_ARRAY_SIZE(module_globals_table),
        .table = (mp_map_elem_t*)(mp_rom_map_elem_t*) module_globals_table,
    }
};

mp_obj_module_t module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_upywraptest, module);
