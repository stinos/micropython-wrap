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

  //Generic constructor caller
  template< class T, class... Args >
  T* ConstructorFactoryFunc( Args... args )
  {
    return new T( args... );
  }

  //Placeholder form compile-time indices array
  template< size_t... >
  struct index_tuple
  {
  };

  namespace detail
  {
    template< size_t N, class IndexTuple, class... Types >
    struct make_indices_impl;

    template< size_t N, size_t... Indices, class T, class... Types>
    struct make_indices_impl< N, index_tuple< Indices... >, T, Types...>
    {
      typedef typename make_indices_impl< N + 1, index_tuple< Indices..., N >, Types... >::type type;
    };

    template< size_t N, size_t... Indices >
    struct make_indices_impl< N, index_tuple< Indices... > >
    {
      typedef index_tuple< Indices... > type;
    };
  }

  //Create compile-time array with indices
  template< class... Types >
  struct make_indices : public detail::make_indices_impl< 0, index_tuple<>, Types... >
  {
  };
}

#endif
