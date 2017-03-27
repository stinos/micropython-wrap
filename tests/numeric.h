#ifndef MICROPYTHON_WRAP_TESTS_NUMERIC_H
#define MICROPYTHON_WRAP_TESTS_NUMERIC_H

#include <cctype>

namespace upywrap
{
  int Int( int a )
  {
    return a;
  }

  unsigned Unsigned( unsigned a )
  {
    return a;
  }

  std::int16_t Int16( std::int16_t a )
  {
    return a;
  }

  std::uint16_t Unsigned16( std::uint16_t a )
  {
    return a;
  }

  std::int64_t Int64( std::int64_t a )
  {
    return a;
  }

  std::uint64_t Unsigned64( std::uint64_t a )
  {
    return a;
  }

  float Float( float a )
  {
    return a;
  }

  double Double( double a )
  {
    return a;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_NUMERIC_H
