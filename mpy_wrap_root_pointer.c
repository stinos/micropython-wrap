//Add micropython-wrap as a "USERMOD" in micropython to register this root pointer
//Alternative, this line can also be copy-pasted to user code.
//Only required when using the `UPYWRAP_STATICPYOBJ_USE_ROOTPTR` configuration option (opt-in):
//Needs to be done once: Add global type map for micropython-wrap to the micropython state as root pointer
//This enables micropython-wrap to store anonymous type information while preventing visibility of the _StaticPyObjectStore
//in user modules.
MP_REGISTER_ROOT_POINTER(mp_map_t* micropython_wrap_global_storage);