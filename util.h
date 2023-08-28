#ifndef MICROPYTHON_WRAP_UTIL_H
#define MICROPYTHON_WRAP_UTIL_H

#include "detail/micropython.h"

namespace upywrap
{
  inline mp_print_t PrintToString( std::string& dest )
  {
    return mp_print_t{ &dest, [] ( void* data, const char* str, mp_uint_t len ) { ( (std::string*) data )->append( str, len ); } };
  }

#if MICROPY_PY_JSON_SEPARATORS
  inline mp_print_ext_t PrintToJSonString( std::string& dest, const char* itemSeparator, const char* keySeparator )
  {
    mp_print_ext_t printExt;
    const auto printToString( PrintToString( dest ) );
    printExt.base.data = printToString.data;
    printExt.base.print_strn = printToString.print_strn;
    printExt.item_separator = itemSeparator;
    printExt.key_separator = keySeparator;
    return printExt;
  }
#endif

  inline std::string ExceptionToString( mp_obj_t ex )
  {
    std::string exMessage;
    const auto mp_my_print( PrintToString( exMessage ) );
    mp_obj_print_exception( &mp_my_print, ex );
    return exMessage;
  }

  inline std::string VariableValueToString( mp_obj_t obj, mp_print_kind_t kind = PRINT_REPR )
  {
    std::string var;
    const auto mp_my_print( PrintToString( var ) );
    mp_obj_print_helper( &mp_my_print, obj, kind );
    return var;
  }

#if MICROPY_PY_JSON_SEPARATORS
  inline std::string VariableValueToJSonString( mp_obj_t obj, const char* itemSeparator, const char* keySeparator )
  {
    std::string var;
    const auto mp_my_print( PrintToJSonString( var, itemSeparator, keySeparator ) );
    mp_obj_print_helper( &mp_my_print.base, obj, PRINT_JSON );
    return var;
  }
#endif

  /**
    * Wrap a function Call to uPy code in nlr_push/nlr_pop: use this to safely invoke
    * uPy code outside of the standard interpreter, to make sure nlr_raise is properly handled.
    * If an exception is raised it is passed to the HandleEx instance - better make sure the
    * latter doesn't raise an exception itself since it is outside of the nlr_push/pop.
    * Returns true if no exception was raised.
    */
#ifdef _MSC_VER
  //4611: interaction between '_setjmp' and C++ object destruction is non-portable
  //4702: In release builds the compiler/optimizer sometimes is able to figure
  //out the call to ex() will lead to this code never being reached in cases
  //where ex() contains nlr_jump or a throw statement.
  //Moreover, disabling this warning only works for VS2013 when this pragma is
  //outside of the function.
  #pragma warning ( disable : 4702 4611 )
#endif
  template< class Call, class HandleEx >
  bool WrapMicroPythonCall( Call f, HandleEx ex )
  {
    nlr_buf_t nlr;
    if( nlr_push( &nlr ) == 0 )
    {
      f();
      nlr_pop();
      return true;
    }
    else
    {
      ex( nlr.ret_val );
    }
    return false;
  }
#ifdef _MSC_VER
  #pragma warning ( default : 4702 4611 )
#endif

  /**
    * Provide scopeguard-like functionality (for nlr_raise, not for C++ exceptions) by
    * wrapping a uPy function Call using nlr_push/nlr_pop, and assure that no matter if
    * an exception got raised or not, the 'guard' function g gets called.
    * Returns the value f returns, though that point will not get reached when f raises an exception.
    */
  template< class Call, class Guard >
  mp_obj_t GuardMicroPythonCall( Call f, Guard g )
  {
    mp_obj_t returnValue;
    auto exceptionHandler = [&] ( void* exception )
    {
      g();
      nlr_jump( exception ); //re-raise
    };
    WrapMicroPythonCall( [&] () { returnValue = f(); }, exceptionHandler );
    g();
    return returnValue;
  }

  /**
    * Add an object to the given module's global dict.
    */
  template< class T >
  void StoreGlobal( mp_obj_module_t* mod, const char* name, const T& obj )
  {
    mp_obj_dict_store( MP_OBJ_FROM_PTR( mod->globals ), new_qstr( name ), ToPy( obj ) );
  }
}

#endif //#ifndef MICROPYTHON_WRAP_UTIL_H
