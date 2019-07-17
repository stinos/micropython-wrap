#ifndef MICROPYTHON_WRAP_TESTS_TUPLE_H
#define MICROPYTHON_WRAP_TESTS_TUPLE_H

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

namespace upywrap
{
  auto Pair( std::pair< std::pair< int, bool >, double > x ) -> decltype( x )
  {
    std::cout << x.first.first << x.first.second << x.second << std::endl;
    return x;
  }

  auto Tuple1( std::tuple< int, bool, double > x ) -> decltype( x )
  {
    std::cout << std::get< 0 >( x ) << std::get< 1 >( x ) << std::get< 2 >( x ) << std::endl;
    return x;
  }

  auto Tuple2( std::tuple< std::tuple< int, bool, double >, std::string, std::vector< std::tuple< int, bool, double > > > x ) -> decltype( x )
  {
    std::cout << std::get< 0 >( std::get< 0 >( x ) ) << std::get< 1 >( x ) << std::get< 0 >( std::get< 2 >( x )[ 0 ] ) << std::endl;
    return x;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_TUPLE_H
