#ifndef MICROPYTHON_WRAP_DETAIL_MICROPYTHON_H
#define MICROPYTHON_WRAP_DETAIL_MICROPYTHON_H

#include "configuration.h"

#ifdef _MSC_VER
#pragma warning ( disable : 4200 ) //nonstandard extension used : zero-sized array in struct/union
#endif
extern "C"
{
#include <py/objfun.h>
#include <py/objint.h>
#include <py/objmodule.h>
#include <py/objtype.h>
#include <py/runtime.h>
}
#ifdef _MSC_VER
#pragma warning ( default : 4200 )
#endif

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <type_traits>

namespace upywrap
{
  inline mp_obj_t new_qstr( qstr what )
  {
    return MP_OBJ_NEW_QSTR( what );
  }

  inline mp_obj_t new_qstr( const char* what )
  {
    return new_qstr( qstr_from_str( what ) );
  }

  inline mp_obj_t MakeFunction( mp_obj_t (*fun) ( void ) )
  {
    auto o = m_new_obj( mp_obj_fun_builtin_fixed_t );
    o->base.type = &mp_type_fun_builtin_0;
    o->fun._0 = fun;
    return o;
  }

  inline mp_obj_t MakeFunction( mp_obj_t (*fun) ( mp_obj_t ) )
  {
    auto o = m_new_obj( mp_obj_fun_builtin_fixed_t );
    o->base.type = &mp_type_fun_builtin_1;
    o->fun._1 = fun;
    return o;
  }

  inline mp_obj_t MakeFunction( mp_obj_t (*fun) ( mp_obj_t, mp_obj_t ) )
  {
    auto o = m_new_obj( mp_obj_fun_builtin_fixed_t );
    o->base.type = &mp_type_fun_builtin_2;
    o->fun._2 = fun;
    return o;
  }

  inline mp_obj_t MakeFunction( mp_obj_t (*fun) ( mp_obj_t, mp_obj_t, mp_obj_t ) )
  {
    auto o = m_new_obj( mp_obj_fun_builtin_fixed_t );
    o->base.type = &mp_type_fun_builtin_3;
    o->fun._3 = fun;
    return o;
  }

  inline mp_obj_t MakeFunction( mp_uint_t numArgs, mp_obj_t (*fun) ( mp_uint_t, const mp_obj_t* ) )
  {
    auto o = m_new_obj( mp_obj_fun_builtin_var_t );
    o->base.type = &mp_type_fun_builtin_var;
    o->sig = static_cast< uint32_t >( MP_OBJ_FUN_MAKE_SIG( numArgs, numArgs, false ) );
    o->fun.var = fun;
    return o;
  }

  inline mp_obj_t MakeFunction( mp_uint_t numArgsMin, mp_uint_t numArgsMax, mp_obj_t( *fun ) ( mp_uint_t, const mp_obj_t* ) )
  {
    auto o = m_new_obj( mp_obj_fun_builtin_var_t );
    o->base.type = &mp_type_fun_builtin_var;
    o->sig = static_cast< uint32_t >( MP_OBJ_FUN_MAKE_SIG( numArgsMin, numArgsMax, false ) );
    o->fun.var = fun;
    return o;
  }

  //See mp_obj_fun_builtin_fixed_t: for up to 3 arguments there's a builtin function signature
  //this is reflected in MakeFunction
  //VS2013 hasn't constexpr yet so fall back to a macro..
  #define FitsBuiltinNativeFunction( numArgs ) ( (numArgs) < 4 )

  /**
    * Select the appropriate MakeFunction overload based on the arguments.
    */
  template< class... Args >
  struct CreateFunction
  {
    template< bool NativeArgs, size_t NumArgs >
    struct FunctionSelector
    {
      template< class BuiltinFixedT, class BuiltinVarT >
      static mp_obj_t Create( BuiltinFixedT call, BuiltinVarT ) { return MakeFunction( call ); }
    };

    template< size_t NumArgs >
    struct FunctionSelector< false, NumArgs >
    {
      template< class BuiltinFixedT, class BuiltinVarT >
      static mp_obj_t Create( BuiltinFixedT, BuiltinVarT call ) { return MakeFunction( NumArgs, call ); }
    };

    template< class BuiltinFixedT, class BuiltinVarT >
    static mp_obj_t Create( BuiltinFixedT fixed, BuiltinVarT var )
    {
      static const auto numArgs = sizeof...( Args );
      return FunctionSelector< FitsBuiltinNativeFunction( numArgs ), numArgs >::Create( fixed, var );
    }
  };

  /**
    * Use in place of mp_obj_new_exception_msg if the message string needs to be copied
    * (mp_obj_new_exception_msg assumes the message passed to it is in ROM so just stores the char pointer)
    */
  inline mp_obj_t RaiseException( const mp_obj_type_t* exc_type, const char* msg )
  {
    mp_obj_exception_t* o = m_new_obj_var( mp_obj_exception_t, mp_obj_t, 0 );
    if( !o )
    {
      throw std::bad_alloc();
    }
    o->base.type = exc_type;
    o->traceback_data = nullptr;
    o->args = reinterpret_cast< mp_obj_tuple_t* >( MP_OBJ_TO_PTR( mp_obj_new_tuple( 1, nullptr ) ) );
    o->args->items[ 0 ] = mp_obj_new_str( msg, std::strlen( msg ) );
    nlr_raise( MP_OBJ_FROM_PTR( o ) );
  }

  inline void RaiseTypeException( const char* msg )
  {
    RaiseException( &mp_type_TypeError, msg );
  }

  inline void RaiseTypeException( mp_const_obj_t source, const char* target )
  {
    mp_raise_msg_varg( &mp_type_TypeError, "can't convert %s to %s", mp_obj_get_type_str( source ), target );
  }

  inline void RaiseAttributeException( qstr name, qstr attr )
  {
    mp_raise_msg_varg( &mp_type_AttributeError, "'%s' object has no attribute '%s'", qstr_str( name ), qstr_str( attr ) );
  }

  inline mp_obj_t RaiseOverflowException( const char* msg )
  {
    return RaiseException( &mp_type_OverflowError, msg );
  }

  inline mp_obj_t RaiseRuntimeException( const char* msg )
  {
    return RaiseException( &mp_type_RuntimeError, msg );
  }

  inline mp_obj_t ImportName( const char* moduleName )
  {
    return mp_import_name( qstr_from_str( moduleName ), mp_const_none, MP_OBJ_NEW_SMALL_INT( 0 ) );
  }

#if UPYWRAP_USE_EXCEPTIONS
  #define UPYWRAP_TRY try {
  #define UPYWRAP_CATCH } catch( const std::exception& e ) { return upywrap::RaiseRuntimeException( e.what() ); }
  inline bool HasExceptions()
  {
    return true;
  }
#else
  #define UPYWRAP_TRY
  #define UPYWRAP_CATCH
  inline bool HasExceptions()
  {
    return false;
  }
#endif

  //Implement some casts used and check for overflow where trunctaion is needed.
  //Only implemented for conversions which are effectively used.
  template< class S, class T >
  struct safe_integer_caster
  {
  };

  template< class T >
  struct safe_integer_caster< T, T >
  {
    static T Convert( T src )
    {
      return src;
    }
  };

  template< class T, bool uns >
  struct abs_all
  {
    static T abs( T t )
    {
      return t;
    }
  };

  template< class T >
  struct abs_all< T, false >
  {
    static T abs( T t )
    {
      return std::abs( t );
    }
  };


#ifdef max
  #undef max
#endif

  template< class T, class S >
  static void IntegerBoundCheck( S src )
  {
    if( abs_all< S, std::is_unsigned< S >::value >::abs( src ) > static_cast< S >( std::numeric_limits< T >::max() ) )
    {
      RaiseOverflowException( "Integer overflow" );
    }
  }

  template< class T >
  static void PositiveIntegerCheck( T src )
  {
    if( src < 0 )
    {
      RaiseTypeException( "Source integer must be unsigned" );
    }
  }

  template<>
  struct safe_integer_caster< mp_int_t, std::int16_t >
  {
    static std::int16_t Convert( mp_int_t src )
    {
      IntegerBoundCheck< std::int16_t >( src );
      return static_cast< std::int16_t >( src );
    }
  };

  template<>
  struct safe_integer_caster< mp_uint_t, std::uint16_t >
  {
    static std::uint16_t Convert( mp_uint_t src )
    {
      IntegerBoundCheck< std::uint16_t >( src );
      return static_cast< std::uint16_t >( src );
    }
  };

  template<>
  struct safe_integer_caster< std::int64_t, int >
  {
    static int Convert( std::int64_t src )
    {
      IntegerBoundCheck< int >( src );
      return static_cast< int >( src );
    }
  };

  template<>
  struct safe_integer_caster< std::uint64_t, unsigned >
  {
    static unsigned Convert( std::uint64_t src )
    {
      IntegerBoundCheck< unsigned >( src );
      return static_cast< unsigned >( src );
    }
  };

  template<>
  struct safe_integer_caster< std::int32_t, std::uint32_t >
  {
    static std::uint32_t Convert( std::int32_t src )
    {
      PositiveIntegerCheck( src );
      return static_cast< std::uint32_t >( src );
    }
  };

  template<>
  struct safe_integer_caster< std::int32_t, std::uint64_t >
  {
    static std::uint64_t Convert( std::int32_t src )
    {
      PositiveIntegerCheck( src );
      return static_cast< std::uint64_t >( src );
    }
  };

  template<>
  struct safe_integer_caster< std::int64_t, std::uint64_t >
  {
    static std::uint64_t Convert( std::int64_t src )
    {
      PositiveIntegerCheck( src );
      return static_cast< std::uint64_t >( src );
    }
  };

  template<>
  struct safe_integer_caster< double, float >
  {
    static float Convert( double src )
    {
      IntegerBoundCheck< float >( src );
      return static_cast< float >( src );
    }
  };

  template< class T, class S >
  T safe_integer_cast( S src )
  {
    return safe_integer_caster< S, T >::Convert( src );
  }

  /**
    * Static list for storing mp_obj_t.
    * Used to prevent uPy objects from being GC'd when they are kept where the GC mark phase
    * cannot find them, for instance as a member of a C++ class when allocated on the standard heap.
    * Probably usage should be restricted to upywrap internals, or in any case avoided unless really needed:
    * - no conversions between uPy and C++ objects in upywrap, except functions, needs it because they
    *   essentially create new objects which are unrelated copies and do not store any state
    * - we don't know yet if this is the best solution; suppose the list ends up containing hundreds of
    *   items then adding/removing might become too slow for the application and it's an extra load
    *   on the GC as well.
    * For now counter this possible problem by allowing a maximum number of 50 items and
    * also add a bunch of checks to assure correct usage.
    * See PinPyObj to make use of this conveniently; InitBackEnd must be called exactly once before usage.
    */
  class StaticPyObjectStore
  {
  public:
    static void Store( mp_obj_t obj )
    {
      const auto list = *List();
      if( !list )
      {
        RaiseRuntimeException( "StaticPyObjectStore: not initialized" );
      }
      mp_obj_list_append( list, obj );

      static const size_t maxLen = 50;
      if( (*List())->len > maxLen )
      {
        RaiseRuntimeException( "StaticPyObjectStore: list is full" );
      }
    }

    static void Remove( mp_obj_t obj )
    {
      if( !Contains( obj ) )
      {
        RaiseRuntimeException( "StaticPyObjectStore: item not added" );
      }
      mp_obj_list_remove( *List(), obj );
    }

    static mp_obj_list_t* InitBackEnd()
    {
      auto list = List();
      if( *list )
      {
        RaiseRuntimeException( "StaticPyObjectStore: already initialized" );
      }
      *list = m_new_obj( mp_obj_list_t );
      mp_obj_list_init( *list, 0 );
      return *list;
    }

    static bool Initialized()
    {
      return !!*List();
    }

  private:
    static bool Contains( mp_obj_t obj )
    {
      const auto list = *List();
      if( !list )
      {
        RaiseRuntimeException( "StaticPyObjectStore: not initialized" );
      }
      size_t len;
      mp_obj_t* items;
      mp_obj_list_get( list, &len, &items );
      for( size_t i = 0 ; i < len ; ++i )
      {
        if( items[ i ] == obj )
        {
          return true;
        }
      }
      return false;
    }

    //just to avoid a global static and corresponding linking issues
    static mp_obj_list_t** List()
    {
      static mp_obj_list_t* list = nullptr;
      return &list;
    }
  };

  /**
    * Smart pointer which 'pins' a uPy object so it won't get GC'd during the lifetime of the PinPyObj it's held in.
    * Adds an mp_obj_t to StoreStaticPyObj when constructed and removes it again when all instances are destructed.
    * Store instances of this class instead of bare mp_obj_t to assure they don't get GC'd but also don't leak.
    * Note in some cases it might take 2 gc_collect calls before GC occurs: if a PinPyObj is stored in X,
    * and X's destructor gets called because it gets finalized in gc_collect, the stored object might already have
    * been marked if it happens to be at a lower memory address than X. As such only the second gc_collect call
    * will sweep it.
    */
  class PinPyObj
  {
  public:
    PinPyObj( mp_obj_t obj ) :
      obj( new mp_obj_t( obj ), [] ( mp_obj_t* o ) { StaticPyObjectStore::Remove( *o ); } )
    {
      StaticPyObjectStore::Store( Get() );
    }

    mp_obj_t Get() const
    {
      return *obj.get();
    }
  
  private:
    std::shared_ptr< mp_obj_t > obj;
  };


    /**
      * Initialize features for PinPyObj.
      * This creates the StaticPyObjectStore instance and stores it in the module's globals dict,
      * to ensure anything in the list will not be sweeped by the GC.
      * Doesn't do anything if called already, so all translation units in the binary use the same instance.
      */
  inline void InitializePyObjectStore( mp_obj_module_t& mod )
  {
    if( !StaticPyObjectStore::Initialized() )
    {
      mp_obj_dict_store( mod.globals, new_qstr( "_StaticPyObjectStore" ), StaticPyObjectStore::InitBackEnd() );
    }
  }

    /**
      * Wrapper around mp_obj_new_module/mp_module_register.
      * Also creates the StaticPyObjectStore backend so PinPyObj can be used.
      */
  inline mp_obj_module_t* CreateModule( const char* name, bool doRegister = false )
  {
    const qstr qname = qstr_from_str( name );
    mp_obj_module_t* mod = (mp_obj_module_t*) mp_obj_new_module( qname );
    if( doRegister )
    {
      mp_module_register( qname, mod );
    }
    InitializePyObjectStore( *mod );
    return mod;
  }

}

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_MICROPYTHON_H
