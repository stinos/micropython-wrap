Easy wrapping of C and C++ functions and classes so they can be called from
Micro Python (http://github.com/micropython/micropython).

This is mainly proof-of-concept at the moment, todos include:
- add more types (tuple, map)
- allow passing native pointers around
- refactor uPy specifics away from main code, so we can reuse this for eg CPython
- check if it would be needed to pass certain mutable py types (list etc) by reference

Usage
-----

    #include <micropython-wrap/classwrapper.h>
    #include <micropython-wrap/functionwrapper.h>
    #include <iostream>

    class SomeClass
    {
    public:
      SomeClass()
      {
        std::cout << "ctor" << std::endl;
      }

      std::string Foo( const std::string& a, int n )
      {
        std::cout << a << n << std::endl;
        return "hello";
      }

      void Bar( const std::vector< double >& vec )
      {
        std::for_each( cbegin( vec ), cend( vec ), [] ( double a ) { std::cout << a; } );
        std::cout << std::endl;
      }
    };

    class ContextManager
    {
    public:
      ContextManager( int a )
      {
        std::cout << "ContextManager " << a << std::endl;
      }

      void Dispose()
      {
        std::cout << "__exit__ called" << std::endl;
      }
    };

    void Func( SomeClass* p, const std::string& a, int n )
    {
      p->Foo( a, n );
    }

    std::vector< std::string > OtherFunc( std::vector< std::string > i )
    {
      i.push_back( "abcdef" );
      return i;
    }

    extern "C"
    {
      void RegisterMyModule()
      {
        struct Funcs
        {
          func_name_def( Foo )
          func_name_def( Bar )
          func_name_def( Foo2 )
          func_name_def( SomeFunc )
        };

        auto mod = upywrap::CreateModule( "mod" );

        upywrap::ClassWrapper< SomeClass > wrapclass( "SomeClass", mod->globals );
        wrapclass.DefInit<>();
        wrapclass.Def< Funcs::Foo >( &SomeClass::Foo );
        wrapclass.Def< Funcs::Bar >( &SomeClass::Bar );
        wrapclass.Def< Funcs::Foo2 >( Func );

        upywrap::ClassWrapper< ContextManager > wrapcman( "ContextManager", mod->globals );
        wrapcman.DefInit< int >();
        wrapcman.DefExit( &ContextManager::Dispose );

        upywrap::FunctionWrapper wrapfunc( mod->globals );
        wrapfunc.Def< Funcs::SomeFunc >( OtherFunc );
      }
    }

After calling RegisterMyModule from uPy's main for instance,
module mod can be used in Python like this:

    import mod

    x = mod.SomeClass()
    x.Bar( [ 0.0, 1.0, 2.0 ] )
    print( x.Foo( 'abc', 1 ) )
    print( x.Foo2( 'abc', 1 ) )

    print( mod.SomeFunc( [ 'a', 'b' ] ) )

    with mod.ContextManager( 1 ) as p :
      pass

And the output is:

    ctor
    012
    abc1
    hello
    abc1
    None
    ['a', 'b', 'abcdef']
    ContextManager 1
    __exit__ called
