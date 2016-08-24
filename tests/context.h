#ifndef MICROPYTHON_WRAP_TESTS_CONTEXT_H
#define MICROPYTHON_WRAP_TESTS_CONTEXT_H

#include <iostream>

namespace upywrap
{
  class Context
  {
  public:
    Context()
    {
      std::cout << "__init__" << std::endl;
    }

    ~Context()
    {
      std::cout << "__del__" << std::endl;
    }

    void Dispose()
    {
      std::cout << "__exit__" << std::endl;
    }
  };
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_CONTEXT_H
