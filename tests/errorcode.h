#ifndef MICROPYTHON_WRAP_TESTS_ERRORCODE_H
#define MICROPYTHON_WRAP_TESTS_ERRORCODE_H

#include <system_error>

namespace upywrap
{
std::error_code NoErrorCode()
{
  return std::error_code{};
}

std::error_code SomeErrorCode()
{
  return std::make_error_code( std::errc::timed_out );
}
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_ERRORCODE_H
