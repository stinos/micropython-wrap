#ifndef MICROPYTHONMODULE_UTIL_H
#define MICROPYTHONMODULE_UTIL_H

#include <type_traits>
#include <tuple>

namespace upywrap
{
  //Remove const/reference/pointer
  template< class T, class U = typename std::remove_cv< typename std::remove_pointer< typename std::remove_reference< T >::type >::type >::type >
  struct remove_all : remove_all< U >
  {
  };

  template< class T >
  struct remove_all< T, T >
  {
    typedef T type;
  };

  //Remove const qualifier, also from references and pointers
  template< class T >
  struct remove_all_const
  {
    typedef typename std::remove_const< T >::type type;
  };

  template< class T >
  struct remove_all_const< const T& >
  {
    typedef T& type;
  };

  template< class T >
  struct remove_all_const< const T* >
  {
    typedef T* type;
  };

  //Take two template arguments and return the second one
  template< class A, class B >
  struct project2nd
  {
    typedef B type;
  };

  //Generic new/constructor caller
  template< class T, class... Args >
  T* ConstructorFactoryFunc( Args... args )
  {
    return new T( args... );
  }

  //Generic destructor/delete caller
  template< class T >
  void DestructorFactoryFunc( T* p )
  {
    delete p;
  }

  //Compile-time generated sequence of size_t (will be in C++14)
  template< std::size_t... >
  struct index_sequence { };

  //Generator of index_sequence< Is >
  template< std::size_t N, std::size_t... Is >
  struct make_index_sequence : make_index_sequence< N - 1, N - 1, Is... > { };

  template< std::size_t... Is>
  struct make_index_sequence< 0, Is... > : index_sequence< Is... > { };

  //Recursive helper for apply
  template< std::size_t N, class... Args >
  struct apply_tuple
  {
    template< class Fun >
    static void apply( Fun&& f, const std::tuple< Args... >& args )
    {
      apply_tuple< N - 1, Args... >::apply( f, args );
      f( std::get< N >( args ) );
    }
  };

  template< class... Args >
  struct apply_tuple< 0, Args... >
  {
    template< class Fun >
    static void apply( Fun&& f, const std::tuple< Args... >& args )
    {
      f( std::get< 0 >( args ) );
    }
  };

  //Recursively apply each element of a tuple to the given function
  template< class Fun, class... Args >
  void apply( Fun&& f, const std::tuple< Args... >& args )
  {
    apply_tuple< std::tuple_size< std::tuple< Args... > >::value - 1, Args... >::apply( f, args );
  }
}

#endif
