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

  private:
    int a;
  };

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
