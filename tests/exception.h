#ifndef MICROPYTHON_WRAP_TESTS_EXCEPTION_H
#define MICROPYTHON_WRAP_TESTS_EXCEPTION_H

#include <string>
#include <stdexcept>

namespace upywrap
{
  void Throw( const std::string& what )
  {
    throw std::runtime_error( what.data() );
  }
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_EXCEPTION_H
