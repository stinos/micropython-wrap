#ifndef MICROPYTHON_WRAP_TESTS_OPTIONAL_H
#define MICROPYTHON_WRAP_TESTS_OPTIONAL_H

#include <iostream>
#include <optional>
#include <vector>

namespace upywrap
{
  std::optional< int > NullOpt()
  {
    return std::nullopt;
  }

  std::optional< int > OptionalInt()
  {
    return 100;
  }

  std::optional< std::vector< int > > OptionalVector()
  {
    return std::vector< int >{ 1, 2, 3 };
  }

  void OptionalArgument( std::optional< std::vector< int > > arg )
  {
    if( arg )
    {
      for( const auto& a : *arg )
      {
        std::cout << a;
      }
    }
    else
    {
      std::cout << "nullopt";
    }
    std::cout << std::endl;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_OPTIONAL_H
