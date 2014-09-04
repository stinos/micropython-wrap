Provide convenient wrapping of C and C++ functions and classes so they can be called from
Micro Python (http://github.com/micropython/micropython).

Currently supports these conversions for function arguments and return values:

    uPy double <-> double
    uPy int <-> int
    uPy bool <-> bool
    uPy str <-> std::string
    uPy str <-> const char* (optional)
    uPy tuple <-> std::tuple
    uPy array <-> std::vector
    uPy dict <-> std::map
    uPy callable -> std::function

    uPy functions <-> C++ free functions
    uPy type <-> C++ T (if T is registered with ClassWrapper, see Usage)
    uPy class methods <-> getter/setter methods of T
    uPy class attributes <-> getter/setter methods of T

For the latter, native functions accepting pointers, values and references are allowed,
and pointers and referenecs can be returned.
For the 'builtin' types, the native function must take the argument by value, const value or const reference,
and only values can be returned.

Some typical Python concepts are supported for class types:

    uPy __init__ = native class constructor or factory function of choice
    uPy __del__ = native class destructor
    uPy __exit__ = can be registered to call a void() method

Furthermore there is optional support for wrapping each native call in a try/catch for std::exception,
and re-raise it as a uPy RuntimeError

The tests cover pretty much everything that is supported so they serve as
the main documentationf the possibilities.

Though fully operational (all tests are ok on 32/64 bit unix/windows ports)
the code should still be considered beta and todos include:
- check if it would be needed to pass certain mutable py types (list etc) by reference
  (which would translate to converting uPy argument to native type, call native function,
  then convert back to uPy type)
- check with real-life code if there are any performance problems that can be solved
- the tests require a native module to be registered with uPy;
  Ideally that requires a working dynamic import system for Micro Python, but as long as
  that is not available, we manually build with tests/module.cpp included as source and
  call InitUpyWrapTest() in main()..
- if we refactor uPy specifics away from main code, we can reuse this lib for eg CPython

Usage Sample
------------

    #include <micropython-wrap/classwrapper.h>
    #include <micropython-wrap/functionwrapper.h>
    #include <iostream>

    class SomeClass
    {
    public:
      SomeClass();

      static SomeClass* Factory();

      std::string Foo( const std::string& a, int n );

      void Bar( const std::vector< double >& vec );

      void Use( SomeClass* p );

      int GetValue() const;

      void SetValue();
    };

    class ContextManager
    {
    public:
      ContextManager( int a );

      void Dispose();
    };

    void Func( SomeClass* p, const std::string& a, int n );

    std::vector< std::string > List( std::vector< std::string > i );

    std::map< int, int > Dict( std::map< int, int > arg );

    std::tuple< int, std::string > Tuple( std::tuple< int, std::string > arg );

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

        upywrap::ClassWrapper< SomeClass > wrapclass( "SomeClass", mod );
        wrapclass.DefInit<>();
        wrapclass.Def< Funcs::Foo >( &SomeClass::Foo );
        wrapclass.Def< Funcs::Bar >( &SomeClass::Bar );
        wrapclass.Def< Funcs::Use >( &SomeClass::Use );
        wrapclass.Def< Funcs::Func >( Func );
        wrapClass.Property( "value", &SomeClass::SetValue, &SomeClass::GetValue );

        upywrap::ClassWrapper< ContextManager > wrapcman( "ContextManager", mod );
        wrapcman.DefInit< int >();
        wrapcman.DefExit( &ContextManager::Dispose );

        upywrap::FunctionWrapper wrapfunc( mod );
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
    x.value = 0
    print( x.value )

    with mod.ContextManager( 1 ) as p :
      pass

    print( mod.List( [ 'a', 'b' ] ) )
    print( mod.Dict( { 3: 4 } ) )
    print( mod.Tuple( [ 0, 'upy' ] ) )
