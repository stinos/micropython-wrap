#ifndef MICROPYTHON_WRAP_TESTS_STRING_H
#define MICROPYTHON_WRAP_TESTS_STRING_H

#include <string>

namespace upywrap
{
  std::string StdString( std::string a )
  {
    return a;
  }

#if UPYWRAP_HAS_CPP17
  std::string_view StdStringView( std::string_view a )
  {
    return a;
  }
#endif

  const char* CharString( const char* a )
  {
    return a;
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_STRING_H
