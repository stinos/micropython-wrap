#ifndef MICROPYTHON_WRAP_TESTS_FUNCTION_H
#define MICROPYTHON_WRAP_TESTS_FUNCTION_H

#include "class.h"
#include <functional>
#include <iostream>

namespace upywrap
{
  void Func1( std::function< void() > a )
  {
    a();
  }

  void Func2( std::function< void( int ) > a )
  {
    a( 2 );
  }

  void Func3( std::function< int() > a )
  {
    std::cout << a() << std::endl;
  }

  void Func4( const std::function< std::string( int, const std::string& ) >& a )
  {
    std::cout << a( 4, "hello" ) << std::endl;
  }

  void Func5( std::function< double( double ) > a )
  {
    std::cout << a( -5.0 ) << std::endl;
  }

  double Func6( double x )
  {
    return x < 0 ? -x : x;
  }

  int Func7( int a, int b, int c, int d )
  {
    return a + b + c + d;
  }

  int Func8( std::function< int( int, int, int, int ) > a )
  {
    return a( 1, 2, 3, 4 );
  }

  bool IsEmptyFunction( std::function< void() > f )
  {
    return !f;
  }

  int CallbackWithNativeArg( std::function< void( Simple& ) > callback )
  {
    Simple s( 32 );
    callback( s );
    return s.Value();
  }

  std::function< void() > NoFunc()
  {
    return std::function< void() >();
  }

  std::function< void() > ToFunc1()
  {
    return [] () { std::cout << "Func1" << std::endl; };
  }

  std::function< int( Simple& ) > ToFunc2()
  {
    return [] ( Simple& a ) { return a.Value(); };
  }

  std::function< bool( std::function< void() > ) > ToFunc3()
  {
    return IsEmptyFunction;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_FUNCTION_H
