#pragma once

#include "micropythonc.h"
#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace upywrap {
/**
 * @brief A wrapper around the MicroPython map type.
 *
 * This class provides a convenient way to interact with the MicroPython map type.
 * Objects of this type do not own the map, and are not responsible for its lifetime.
 * The observer functions and Iterator are crafted specifically to support the micropython-wrap library
 * with as little changes as possible. As such, `void *` is not treated as mp_obj_t, but rather as an integer type when used as key.
 */
template <class Key, class Value>
requires std::is_pointer_v<Value>
class MPyMapView {
public:
    using mapped_type = Value;
    explicit MPyMapView(mp_map_t* map_ptr) : map_ { map_ptr } {}

    Value& operator[](const qstr key) requires std::same_as<Key, qstr> {
        mp_map_elem_t* elem = mp_map_lookup(map_, MP_OBJ_NEW_QSTR(key), MP_MAP_LOOKUP_ADD_IF_NOT_FOUND);
        return reinterpret_cast<Value&>(elem->value);
    }

    bool contains(const qstr key) const requires std::same_as<Key, qstr> {
        mp_map_elem_t* elem = mp_map_lookup(map_, MP_OBJ_NEW_QSTR(key), MP_MAP_LOOKUP);
        return elem != nullptr;
    }

    /**
     * @brief Converts a void pointer to an mp_obj_t.
     * @warning While an mp_obj_t is technically a void*, this will rather convert the void* to an integer type.
     */
    mp_obj_t uint_obj_from_void_ptr(const void* ptr) const { 
        auto key_uintptr = (uintptr_t)ptr;
        auto key_mpuint = (mp_uint_t)key_uintptr;
        mp_obj_t key_obj = mp_obj_new_int_from_uint(key_mpuint);
        return key_obj;
    }

    /**
     * @brief Access the map with a void pointer key. Micropytho-wrap uses this to use the name() functions (index_type) as keys.
     * 
     * We convert the void* here to an mp_uint_t, which is then used as the key in the micropython map.
     */
    Value& operator[](const void* key) requires std::same_as<Key, void*> {
        mp_map_elem_t* elem = mp_map_lookup(map_, uint_obj_from_void_ptr(key), MP_MAP_LOOKUP_ADD_IF_NOT_FOUND);
        return reinterpret_cast<Value&>(elem->value);
    }

    /**
     * @brief Access the map with a void pointer key. Micropytho-wrap uses this to use the name() functions (index_type) as keys.
     * 
     * We convert the void* here to an mp_uint_t, which is then used as the key in the micropython map.
     */
    bool contains(const void* key) const requires std::same_as<Key, void*> {
        mp_map_elem_t* elem = mp_map_lookup(map_, uint_obj_from_void_ptr(key), MP_MAP_LOOKUP);
        return elem != nullptr;
    }

    /**
     * @brief Minimal iterator for the MPyMapView.
     *
     * Micropython-wrap uses only find(), end() and ->, so we do not need to implement incrementing or dereferencing.
     */
    class FakeIterator {
    public:
        FakeIterator() = default;
        FakeIterator(Key key, Value val) : val_{ { key, val } } {}

        std::pair<Key, Value>* operator->() {
            return &*val_;
        }

        bool operator==(const FakeIterator& other) const = default;

    private:
        std::optional<std::pair<Key, Value>> val_ {};
    };

    FakeIterator find(const Key& key) {
        if (contains(key)) {
            return FakeIterator {key, (*this)[key] };
        }
        return {};
    }

    FakeIterator end() {
        return {};
    }

private:
    mp_map_t* map_;
};
} // namespace upywrap