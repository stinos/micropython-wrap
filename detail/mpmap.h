#pragma once

#include "micropythonc.h"
#include <type_traits>

namespace upywrap {
/**
 * @brief A wrapper around the MicroPython map type.
 *
 * This class provides a convenient way to interact with the MicroPython map type.
 * Objects of this type do not own the map, and are not responsible for its lifetime.
 */
template <class Key, class Value>
class MPyMapView {
    static_assert(std::is_pointer<Value>::value, "upywrap::MpyMapView: Value must be a pointer type.");
    static_assert(std::is_same<Key, qstr>::value, "upywrap::MPyMapView: Currently only qstr keys are supported.");

public:
    using mapped_type = Value;
    explicit MPyMapView(mp_map_t* map_ptr) : map_ { map_ptr } {}

    Value& operator[](const qstr key) {
        mp_map_elem_t* elem = mp_map_lookup(map_, MP_OBJ_NEW_QSTR(key), MP_MAP_LOOKUP_ADD_IF_NOT_FOUND);
        return reinterpret_cast<Value&>(elem->value);
    }

    bool contains(const qstr key) const {
        mp_map_elem_t* elem = mp_map_lookup(map_, MP_OBJ_NEW_QSTR(key), MP_MAP_LOOKUP);
        return elem != nullptr;
    }

private:
    mp_map_t* map_;
};
} // namespace upywrap