#ifndef MICROPYTHON_WRAP_TESTS_CLASS_H
#define MICROPYTHON_WRAP_TESTS_CLASS_H

#include <string>

namespace upywrap
{
  class Simple
  {
  public:
    Simple( int a ) :
      a( a )
    {
    }

    void Add( int x )
    {
      a += x;
    }

    int Value() const
    {
      return a;
    }

    void SetValue( int x )
    {
      a = x;
    }

    void Plus( Simple* rh )
    {
      a += rh->a;
    }

    std::string Str() const
    {
      return "Simple " + std::to_string( a );
    }

  private:
    int a;
  };

  Simple& SimpleFunc( Simple& p1, Simple* p2 )
  {
    p1.Plus( p2 );
    return p1;
  }

}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_CLASS_H
