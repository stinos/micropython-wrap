#ifndef MICROPYTHON_WRAP_TESTS_FUNCTION_H
#define MICROPYTHON_WRAP_TESTS_FUNCTION_H

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
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_FUNCTION_H
