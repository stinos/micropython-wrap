#ifndef MICROPYTHON_WRAP_TESTS_STRING_H
#define MICROPYTHON_WRAP_TESTS_STRING_H

#include <string>

namespace upywrap
{
  std::string StdString( std::string a )
  {
    return a;
  }

  const char* CharString( const char* a )
  {
    return a;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_STRING_H
