#ifndef MICROPYTHONMODULE_FUNCTIONCALL_H
#define MICROPYTHONMODULE_FUNCTIONCALL_H

namespace upywrap
{
  //Base function object for instance function calls
  template< class T, class Ret, class... A >
  struct InstanceFunctionCall
  {
    typedef Ret( *func_type )( T*, A... );
    typedef Ret( T::*mem_func_type )( A... );
    typedef Ret( T::*const_mem_func_type )( A... ) const;
    virtual ~InstanceFunctionCall() {}
    virtual Ret Call( T* p, A... ) = 0;
  };

  //Normal member function call
  template< class T, class Ret, class... A >
  struct MemberFunctionCall : public InstanceFunctionCall< T, Ret, A... >
  {
    typedef typename InstanceFunctionCall< T, Ret, A... >::mem_func_type mem_func_type;
    mem_func_type func;
    MemberFunctionCall( mem_func_type func ) : func( func ) {}
    virtual Ret Call( T* p, A... a ) { return ( p->*func )( a... ); }
  };

  //Normal const member function call (const is ignored btw)
  template< class T, class Ret, class... A >
  struct ConstMemberFunctionCall : public InstanceFunctionCall< T, Ret, A... >
  {
    typedef typename InstanceFunctionCall< T, Ret, A... >::const_mem_func_type const_mem_func_type;
    const_mem_func_type func;
    ConstMemberFunctionCall( const_mem_func_type func ) : func( func ) {}
    virtual Ret Call( T* p, A... a ) { return ( p->*func )( a... ); }
  };

  //Non-member function taking T* as first argument
  template< class T, class Ret, class... A >
  struct NonMemberFunctionCall : public InstanceFunctionCall< T, Ret, A... >
  {
    typedef typename InstanceFunctionCall< T, Ret, A... >::func_type func_type;
    func_type func;
    NonMemberFunctionCall( func_type func ) : func( func ) {}
    virtual Ret Call( T* p, A... a ) { return func( p, a... ); }
  };

  //Standard function call object
  template< class Ret, class... A >
  struct FunctionCall
  {
    typedef Ret( *func_type )( A... );
    func_type func;
    FunctionCall( func_type func ) : func( func ) {}
    virtual Ret Call( A... a ) { return func( a... ); }
  };
}

#endif
