#ifndef MICROPYTHON_WRAP_TESTS_MAP_H
#define MICROPYTHON_WRAP_TESTS_MAP_H

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

namespace upywrap
{
  auto Map1( std::map< std::string, int > x ) -> decltype( x )
  {
    std::for_each( x.cbegin(), x.cend(), [] ( decltype( *x.cend() ) i ) { std::cout << i.first << i.second; } );
    std::cout << std::endl;
    return x;
  }

  auto Map2( std::map< std::string, std::vector< int > > x ) -> decltype( x )
  {
    std::for_each( x.cbegin(), x.cend(), [] ( decltype( *x.cend() ) i ) { std::cout << i.first << i.second[ 0 ]; } );
    std::cout << std::endl;
    return x;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_MAP_H
