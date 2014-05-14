#ifndef MICROPYTHONMODULE_UTIL_H
#define MICROPYTHONMODULE_UTIL_H

#include <type_traits>

namespace upywrap
{
  //Remove cv qualifiers and reference
  template< class T >
  struct remove_all
  {
    typedef typename std::remove_cv< typename std::remove_reference< T >::type >::type type;
  };

  //Take two template arguments and return the second one
  template< class A, class B >
  struct project2nd
  {
    typedef B type;
  };

}

#endif
