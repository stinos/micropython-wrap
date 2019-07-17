#ifndef MICROPYTHON_WRAP_TESTS_VECTOR_H
#define MICROPYTHON_WRAP_TESTS_VECTOR_H

#include <algorithm>
#include <iostream>
#include <vector>

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
