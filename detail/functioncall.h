#ifndef MICROPYTHON_WRAP_DETAIL_FUNCTIONCALL_H
#define MICROPYTHON_WRAP_DETAIL_FUNCTIONCALL_H

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
  };
}

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_FUNCTIONCALL_H
