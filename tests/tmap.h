#ifndef MICROPYTHON_WRAP_TESTS_TMAP_H
#define MICROPYTHON_WRAP_TESTS_TMAP_H

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include "../maptype.h"

namespace upywrap
{
  struct TypeSink
  {
    void operator ()( int a )
    {
      std::cout << "int" << a << std::endl;
    }

    void operator ()( double a )
    {
      std::cout << "double" << a << std::endl;
    }

    void operator ()( bool a )
    {
      std::cout << "bool" << a << std::endl;
    }

    void operator ()( std::string a )
    {
      std::cout << "string" << a << std::endl;
    }

    void operator ()( const std::vector< int >& a )
    {
      std::cout << "int" << a[ 0 ] << std::endl;
    }

    void operator ()( const std::vector< double >& a )
    {
      std::cout << "double" << a[ 0 ] << std::endl;
    }

    void operator ()( const std::vector< bool >& a )
    {
      std::cout << "bool" << a[ 0 ] << std::endl;
    }

    void operator ()( const std::vector< std::string >& a )
    {
      std::cout << "string" << a[ 0 ] << std::endl;
    }
  };

  void UseTypeMap( void* o )
  {
    MapType( o, TypeSink() );
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_MAP_H
