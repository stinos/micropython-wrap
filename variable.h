#ifndef MICROPYTHONWRAP_VARIABLE
#define MICROPYTHONWRAP_VARIABLE

#include "classwrapper.h"
#include <stdexcept>

namespace upywrap
{
  using varname = std::vector< std::string >;

  namespace detail
  {
    inline mp_obj_t GetVariable( const char* name )
    {
      return mp_load_name( qstr_from_str( name ) );
    }

    inline mp_obj_t GetVariable( mp_obj_t instance, const char* name )
    {
      return mp_load_attr( instance, qstr_from_str( name ) );
    }

    inline mp_obj_t GetVariable( const char* instance, const char* name )
    {
      return GetVariable( GetVariable( instance ), name );
    }

    template< class... Names >
    mp_obj_t GetVariable( const char* instance, const char* name, Names... names )
    {
      return GetVariable( GetVariable( instance, name ), names... );
    }

    inline mp_obj_t GetVariable( const varname& names )
    {
      const auto numNames = names.size();
      if( !numNames )
      {
        throw std::runtime_error( "cannot get variable without a name" );
      }
      auto var = GetVariable( names[ 0 ].data() );
      for( size_t i = 1 ; i < numNames ; ++i )
      {
        var = GetVariable( var, names[ i ].data() );
      }
      return var;
    }

    inline void SetVariable( mp_obj_t value, const char* name )
    {
      mp_store_name( qstr_from_str( name ), value );
    }

    inline void SetVariable( mp_obj_t instance, mp_obj_t value, const char* name )
    {
      mp_store_attr( instance, qstr_from_str( name ), value );
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

    inline void SetVariable( mp_obj_t value, const varname& names )
    {
      const auto numNames = names.size();
      if( !numNames )
      {
        throw std::runtime_error( "cannot get variable without a name" );
      }
      if( numNames == 1 )
      {
        SetVariable( value, names[ 0 ].data() );
      }
      else
      {
        auto var = GetVariable( names[ 0 ].data() );
        for( size_t i = 1 ; i < numNames - 1 ; ++i )
        {
          var = GetVariable( var, names[ i ].data() );
        }
        SetVariable( var, value, names[ numNames - 1 ].data() );
      }
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
    return FromPy< T >( detail::GetVariable( names... ) );
  }

  template< class T >
  T GetVariable( const varname& names )
  {
    return FromPy< T >( detail::GetVariable( names ) );
  }

    /**
      * Set the value of any variable in scope, or an (possibly nested) attribute of an object in scope.
      * Adds the variable/attribute if it does not exist, but not nested (so setting a.b.c is an error if a has no member b).
      * Uses same lookup as GetVariable.
      */
  template< class T, class... Names >
  void SetVariable( const T& value, Names... names )
  {
    detail::SetVariable( ToPy( value ), names... );
  }

  template< class T >
  void SetVariable( const T& value, const varname& names )
  {
    detail::SetVariable( ToPy( value ), names );
  }
}

#endif //#ifndef MICROPYTHONWRAP_VARIABLE
