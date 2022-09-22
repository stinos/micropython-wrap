#ifndef MICROPYTHON_WRAP_TESTS_NARGS_H
#define MICROPYTHON_WRAP_TESTS_NARGS_H

#include <iostream>

namespace upywrap
{
  class NargsTest
  {
  public:
    NargsTest()
    {
    }

    void Three( int a, int b, int c )
    {
      std::cout << a << b << c << std::endl;
    }

    void Four( int a, int b, int c, int d )
    {
      std::cout << a << b << c << d << std::endl;
    }
  };

  class KwargsTest
  {
  public:
    //Note this is just for testing keyword behavior; actual argument conversion uses
    //the same calls as 'normal' functions so no need to test all of that again.
    KwargsTest( int a, std::string b, int c )
    {
      std::cout << a << b << c << std::endl;
    }

    void Two( int a, int b )
    {
      std::cout << a << b << std::endl;
    }
  };

  void Two( int a, int b )
  {
    std::cout << a << b << std::endl;
  }

  void Four( int a, int b, int c, int d )
  {
    std::cout << a << b << c << d << std::endl;
  }

  void Eight( int a, int b, int c, int d, int e, int f, int g, int h )
  {
    std::cout << a << b << c << d << e << f << g << h << std::endl;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_NARGS_H
