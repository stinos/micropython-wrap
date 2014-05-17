Provide convenient wrapping of C and C++ functions and classes so they can be called from
Micro Python (http://github.com/micropython/micropython).

Though fully operational (example below tested on 32/64 bit unix/windows ports)
the code should still be considered beta and todos include:
- refactor uPy specifics away from main code, so we can reuse this for eg CPython
- check if it would be needed to pass certain mutable py types (list etc) by reference
  (which would translate to converting uPy argument to native type, call native function,
  then convert back to uPy type)
- check with real-life code if there are any performance problems that can be solved
  (for instance see which arguments can be rvalue references)
- write some tests to verify functionality; easiest would be to use the code below,
  run it and check output.
  Ideally that requires a working dynamic import system for Micro Python,
  though we can get away with a custom main and modifications to makefiles since
  they only compile C code now.

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
        std::cout << "ctor: " << this << std::endl;
      }

      static SomeClass* Factory()
      {
        return new SomeClass();
      }

      std::string Foo( const std::string& a, int n )
      {
        std::cout << a << n << std::endl;
        return "hello";
      }

      void Bar( const std::vector< double >& vec )
      {
        std::for_each( vec.cbegin(), vec.cend(), [] ( double a ) { std::cout << a; } );
        std::cout << std::endl;
      }

      void Use( SomeClass* p )
      {
        std::cout << "inst: " << p << std::endl;
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

    std::vector< std::string > List( std::vector< std::string > i )
    {
      i.push_back( "abcdef" );
      return i;
    }

    std::map< int, int > Dict( std::map< int, int > arg )
    {
      arg[ 0 ] = 1;
      arg[ 1 ] = 2;
      return arg;
    }

    std::tuple< int, std::string > Tuple( std::tuple< int, std::string > arg )
    {
      std::get< 0 >( arg ) += 35;
      std::get< 1 >( arg ) = std::get< 1 >( arg ) + "tuple";
      return arg;
    }

    struct Funcs
    {
      func_name_def( Foo )
      func_name_def( Bar )
      func_name_def( Use )
      func_name_def( Factory )
      func_name_def( Func )
      func_name_def( List )
      func_name_def( Dict )
      func_name_def( Tuple )
    };

    extern "C"
    {
      void RegisterMyModule()
      {
        auto mod = upywrap::CreateModule( "mod" );

        upywrap::ClassWrapper< SomeClass > wrapclass( "SomeClass", mod->globals );
        wrapclass.DefInit<>();
        wrapclass.Def< Funcs::Foo >( &SomeClass::Foo );
        wrapclass.Def< Funcs::Bar >( &SomeClass::Bar );
        wrapclass.Def< Funcs::Use >( &SomeClass::Use );
        wrapclass.Def< Funcs::Func >( Func );

        upywrap::ClassWrapper< ContextManager > wrapcman( "ContextManager", mod->globals );
        wrapcman.DefInit< int >();
        wrapcman.DefExit( &ContextManager::Dispose );

        upywrap::FunctionWrapper wrapfunc( mod->globals );
        wrapfunc.Def< Funcs::Func >( Func );
        wrapfunc.Def< Funcs::Factory >( SomeClass::Factory );
        wrapfunc.Def< Funcs::List >( List );
        wrapfunc.Def< Funcs::Dict >( Dict );
        wrapfunc.Def< Funcs::Tuple >( Tuple );
      }
    }

After calling RegisterMyModule from uPy's main for instance,
module mod can be used in Python like this:

    import mod

    x = mod.SomeClass()
    x.Bar( [ 0.0, 1.0, 2.0 ] )
    print( x.Foo( 'abc', 1 ) )
    x.Func( 'abc', 1 )
    mod.Func( x, 'glob', 2 )
    x.Use( mod.Factory() )

    with mod.ContextManager( 1 ) as p :
      pass

    print( mod.List( [ 'a', 'b' ] ) )
    print( mod.Dict( { 3: 4 } ) )
    print( mod.Tuple( [ 0, 'upy' ] ) )

And the output is something like:

    ctor: 004FDDD0
    012
    abc1
    hello
    abc1
    glob2
    ctor: 004FF1F8
    inst: 004FF1F8
    ContextManager 1
    __exit__ called
    ['a', 'b', 'abcdef']
    {0: 1, 1: 2, 3: 4}
    (35, 'upytuple')
