#ifndef MICROPYTHON_WRAP_TESTS_VECTOR_H
#define MICROPYTHON_WRAP_TESTS_VECTOR_H

#include <vector>
#include <algorithm>
#include <iostream>

namespace upywrap
{
  template< class T >
  auto Vector( std::vector< T > x ) -> decltype( x )
  {
    std::for_each( x.cbegin(), x.cend(), [] ( T i ) { std::cout << i; } );
    std::cout << std::endl;
    return x;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_VECTOR_H
