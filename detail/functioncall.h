#ifndef MICROPYTHON_WRAP_DETAIL_FUNCTIONCALL_H
#define MICROPYTHON_WRAP_DETAIL_FUNCTIONCALL_H

#include "micropython.h"
#include <array>
#include <cassert>

namespace upywrap
{
  template< class Ret >
  struct DefineRetvalConverter
  {
    typedef void* (*type)( Ret );
  };

  template<>
  struct DefineRetvalConverter< void >
  {
    typedef void* (*type)();
  };

  //Return value 'converter' which ignores the C++ return value and just returns None.
  //Use to set convert_retval when the return value isn't needed and/or isn't registered,
  //instead of having to write a wrapper function. For example if your C++ function is
  //Foo& Bar();
  //and Foo is not a registered ClassWrapper this results in
  //"TypeError: Native type class Foo has not been registered" at runtime.
  //So if you don't need the return value anyway you can make a wrapper
  //void VoidVersionOfBar()
  //{
  //  Bar();
  //}
  //and register that instead, or skip that and instead just use
  //Def<Name>( Bar, upywrap::Ignore );
  template< class Ret >
  mp_obj_t Ignore( Ret&& )
  {
    return mp_const_none;
  }

  //Optional/keyword argument support is configured and parsed via this class.
  //Function objects (InstanceFunctionCall etc) with empty arguments, i.e. !HasArguments(),
  //are treated as not having any optional/keyword arguments.
  //Maximum number of arguments is UPYWRAP_MAXNUMKWARGS.
  class Arguments
  {
  public:
    using parsed_obj_t = std::array< mp_obj_t, UPYWRAP_MAXNUMKWARGS >;

    Arguments()
    {
    }

    Arguments( const Arguments& ) = default;
    Arguments( Arguments&& ) = default;
    Arguments& operator = ( Arguments&& ) = default;

    //Add an argument; if the second argument is MP_OBJ_NULL, the default,
    //this is a required argument.
    Arguments& Add( const char* name, mp_obj_t defaultValue = MP_OBJ_NULL )
    {
      if( args.size() == UPYWRAP_MAXNUMKWARGS )
      {
        RaiseTypeException( "maximum number of arguments reached" );
      }
      mp_arg_t arg{};
      arg.qst = static_cast< qstr_short_t >( qstr_from_str( name ) );
      arg.flags = MP_ARG_OBJ;
      if( defaultValue == MP_OBJ_NULL )
      {
        if( !args.empty() && !IsRequired( args.back() ) )
        {
          RaiseTypeException( "cannot add required argument after optional argument" );
        }
        arg.flags |= MP_ARG_REQUIRED;
      }
      else if( !(
          defaultValue == mp_const_none ||
          mp_obj_is_bool( defaultValue ) ||
          mp_obj_is_small_int( defaultValue ) ||
          mp_obj_is_qstr( defaultValue )
        ) )
      {
        //An actual object; this should not be garbage collected so pin it.
        pinnedDefaults.emplace_back( defaultValue );
      }
      arg.defval.u_obj = defaultValue;
      args.emplace_back( arg );
      return *this;
    }

    //Add an optional argument; note the type here should match with the
    //C++ function argument, or be convertible to it, else conversion will fail upon calling the function.
    template< class T >
    Arguments& Add( const char* name, T&& defaultValue )
    {
      return Add( name, ToPy( std::forward< T >( defaultValue ) ) );
    }

    //Shorthand for Add().
    template< class T = mp_obj_t >
    Arguments& operator () ( const char* name, T&& defaultValue = MP_OBJ_NULL )
    {
      return Add( name, std::forward< T >( defaultValue ) );
    }

    //Whether this is in use.
    bool HasArguments() const
    {
      return !args.empty();
    }

    //Number of arguments added.
    size_t NumberOfArguments() const
    {
      return args.size();
    }

    //For use in mp_obj_fun_builtin_var_t.sig.
    uint32_t MimimumNumberOfArguments() const
    {
      //Using this when there's nothing to do is a programmer error: should use the
      //faster ways of calling, without extra parsing.
      if( !HasArguments() )
      {
        RaiseTypeException( "function does not support keyword arguments" );
      }
      //Would seem logical here to return #required ones,
      //but then e.g. for a function with one required argument, calling that with a name like foo(a=1)
      //fails because mp_arg_check_num_sig treats it as 'there must be one positional argument'.
      //Doesn't matter much: mp_arg_parse_all handles the rest of the parsing correctly.
      return 0;
    }

    //Parse, returning NumberOfArguments() objects in order added.
    void Parse( size_t n_args, const mp_obj_t* pos_args, mp_map_t* kw_args, parsed_obj_t& parsedObj )
    {
      std::array< mp_arg_val_t, UPYWRAP_MAXNUMKWARGS > parsedArgs{};
      const auto numArgs = args.size();
      mp_arg_parse_all( n_args, pos_args, kw_args, numArgs, args.data(), parsedArgs.data() );
      for( size_t i = 0 ; i < numArgs ; ++i )
      {
        parsedObj[ i ] = parsedArgs[ i ].u_obj;
      }
    }

    //Parse, returning NumberOfArguments() objects in order added.
    void Parse( size_t n_args, size_t n_kw, const mp_obj_t* all_args, parsed_obj_t& parsedObj )
    {
      std::array< mp_arg_val_t, UPYWRAP_MAXNUMKWARGS > parsedArgs{};
      const auto numArgs = args.size();
      mp_arg_parse_all_kw_array( n_args, n_kw, all_args, numArgs, args.data(), parsedArgs.data() );
      for( size_t i = 0 ; i < numArgs ; ++i )
      {
        parsedObj[ i ] = parsedArgs[ i ].u_obj;
      }
    }

  private:
    constexpr static bool IsRequired( const mp_arg_t& arg )
    {
      return arg.flags & MP_ARG_REQUIRED;
    }

    std::vector< mp_arg_t > args;
    std::vector< PinPyObj > pinnedDefaults;
  };

  //Shorthand for not having to write Arguments()(...) but instead Kwargs(...).
  template< class T = mp_obj_t >
  inline Arguments Kwargs( const char* name, T&& defaultValue = MP_OBJ_NULL )
  {
    return std::move( Arguments().Add( name, std::forward< T >( defaultValue ) ) );
  }

  //Base function object for instance function calls
  template< class T, class Ret, class... A >
  struct InstanceFunctionCall
  {
    typedef Ret( *func_type )( T*, A... );
    typedef Ret( *byref_func_type )( T&, A... );
    typedef Ret( *byconstref_func_type )( const T&, A... );
    typedef Ret( T::*mem_func_type )( A... );
    typedef Ret( T::*const_mem_func_type )( A... ) const;
    InstanceFunctionCall() : convert_retval( nullptr ) {}
    virtual ~InstanceFunctionCall() {}
    virtual Ret Call( T* p, A&&... ) = 0;

    //Storage for return value converter. Only used to enable converting
    //to native /types which are defined in other modules,
    //but if we don't keep this here we'd need an extra map for each type.
    //Also this makes the calling code much cleaner since we don't have to duplicate lots of code
    //for each function type (one which does take a converter and one which doesn't): now there's
    //just a single pointe to set the convertor (see ClassWrapper/FunctionWrapper)
    //and a single point to use it (see CallReturn)
    using convert_retval_type = typename DefineRetvalConverter< Ret >::type;
    convert_retval_type convert_retval;

    //Optional/keyword arguments.
    Arguments arguments;
  };

  //Normal member function call
  template< class T, class Ret, class... A >
  struct MemberFunctionCall : public InstanceFunctionCall< T, Ret, A... >
  {
    typedef typename InstanceFunctionCall< T, Ret, A... >::mem_func_type mem_func_type;
    mem_func_type func;
    MemberFunctionCall( mem_func_type func ) : func( func ) {}
    virtual Ret Call( T* p, A&&... a ) override { return ( p->*func )( std::forward< A >( a )... ); }
  };

  //Normal const member function call (const is ignored btw)
  template< class T, class Ret, class... A >
  struct ConstMemberFunctionCall : public InstanceFunctionCall< T, Ret, A... >
  {
    typedef typename InstanceFunctionCall< T, Ret, A... >::const_mem_func_type const_mem_func_type;
    const_mem_func_type func;
    ConstMemberFunctionCall( const_mem_func_type func ) : func( func ) {}
    virtual Ret Call( T* p, A&&... a ) override { return ( p->*func )( std::forward< A >( a )... ); }
  };

  //Non-member function taking T* as first argument
  template< class T, class Ret, class... A >
  struct NonMemberFunctionCall : public InstanceFunctionCall< T, Ret, A... >
  {
    typedef typename InstanceFunctionCall< T, Ret, A... >::func_type func_type;
    func_type func;
    NonMemberFunctionCall( func_type func ) : func( func ) {}
    virtual Ret Call( T* p, A&&... a ) override { return func( p, std::forward< A >( a )... ); }
  };

  //Non-member function taking T& as first argument, for convenience
  template< class T, class Ret, class... A >
  struct NonMemberByRefFunctionCall : public InstanceFunctionCall< T, Ret, A... >
  {
    typedef typename InstanceFunctionCall< T, Ret, A... >::byref_func_type byref_func_type;
    byref_func_type func;
    NonMemberByRefFunctionCall( byref_func_type func ) : func( func ) {}
    virtual Ret Call( T* p, A&&... a ) override { assert( p ); return func( *p, std::forward< A >( a )... ); }
  };

  //Non-member function taking const T& as first argument, for convenience
  template< class T, class Ret, class... A >
  struct NonMemberByConstRefFunctionCall : public InstanceFunctionCall< T, Ret, A... >
  {
    typedef typename InstanceFunctionCall< T, Ret, A... >::byconstref_func_type byconstref_func_type;
    byconstref_func_type func;
    NonMemberByConstRefFunctionCall( byconstref_func_type func ) : func( func ) {}
    virtual Ret Call( T* p, A&&... a ) override { assert( p ); return func( *p, std::forward< A >( a )... ); }
  };

  //Standard function call object
  template< class Ret, class... A >
  struct FunctionCall
  {
    typedef Ret( *func_type )( A... );
    func_type func;
    FunctionCall( func_type func ) : func( func ), convert_retval( nullptr ) {}
    virtual Ret Call( A&&... a ) { return func( std::forward< A >( a )... ); }

    using convert_retval_type = typename DefineRetvalConverter< Ret >::type;
    convert_retval_type convert_retval;

    //Optional/keyword arguments.
    Arguments arguments;
  };
}

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_FUNCTIONCALL_H
