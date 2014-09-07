#ifndef MICROPYTHONWRAP_VARIABLE
#define MICROPYTHONWRAP_VARIABLE

#include "classwrapper.h"

namespace upywrap
{
  namespace detail
  {
    mp_obj_t GetVariable( const char* name )
    {
      return mp_load_name( qstr_from_str( name ) );
    }

    mp_obj_t GetVariable( mp_obj_t instance, const char* name )
    {
      return mp_load_attr( instance, qstr_from_str( name ) );
    }

    mp_obj_t GetVariable( const char* instance, const char* name )
    {
      return GetVariable( GetVariable( instance ), name );
    }

    template< class... Names >
    mp_obj_t GetVariable( const char* instance, const char* name, Names... names )
    {
      return GetVariable( GetVariable( instance, name ), names... );
    }

    void SetVariable( mp_obj_t value, const char* name )
    {
      mp_store_name( qstr_from_str( name ), value );
    }

    template< class... Names, size_t... Indices >
    mp_obj_t GetVariableHelper( const std::tuple< Names... >& args, index_sequence< Indices... > )
    {
      return GetVariable( std::get< Indices >( args )... );
    }

    template< class... Names >
    void SetVariable( mp_obj_t value, const char* instance, Names... names )
    {
      //get all arguments, then pass all but the last one to GetVariableHelper
      const auto args = std::make_tuple( instance, names... );
      mp_store_attr( GetVariableHelper( args, make_index_sequence< sizeof...( Names ) >() ), qstr_from_str( split_last( names... ) ), value );
    }
  }

    /**
      * Get the value of any variable in scope, or an (possibly nested) attribute of an object in scope.
      * Since this uses mp_store_xxx it uses the lookup rules for it, being: locals/globals/builtins,
      * so it depends on the currently active scope which variabls will be found.
      * For example:
      * @code
        auto i = upywrap::GetVariable< int >( "a" ); //get a from scope
        auto i = upywrap::GetVariable< int >( "a", "b", "c" ); //get attribute c from attribute b from object a from scope, aka a.b.c
      * @endcode
      */
  template< class T, class... Names >
  T GetVariable( Names... names )
  {
    return SelectFromPyObj< T >::type::Convert( detail::GetVariable( names... ) );
  }

    /**
      * Set the value of any variable in scope, or an (possibly nested) attribute of an object in scope.
      * Adds the variable/attribute if it does not exist, but not nested (so setting a.b.c is an error if a has no member b).
      * Uses same lookup as GetVariable.
      */
  template< class T, class... Names >
  void SetVariable( const T& value, Names... names )
  {
    detail::SetVariable( SelectToPyObj< T >::type::Convert( value ), names... );
  }
}

#endif //#ifndef MICROPYTHONWRAP_VARIABLE
