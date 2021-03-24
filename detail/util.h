#ifndef MICROPYTHON_WRAP_DETAIL_UTIL_H
#define MICROPYTHON_WRAP_DETAIL_UTIL_H

#include <memory>
#include <tuple>
#include <type_traits>

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

  //Get last item of a parameter pack
  template< class Last >
  Last split_last( Last t )
  {
    return t;
  }

  template< class First, class... Rest >
  First split_last( First, Rest... items )
  {
    return split_last( items... );
  }

  //Overload resolution, can be used to select 1 overload using template parameters when registering functions.
  //Suppose there's
  //int A(in);
  //int A(double);
  //then resolve< int >( A ) returns the first one, whereas just using A won't compile (unless
  //the template parameter is specified somewhere else)
  template< class... Args, class R >
  auto resolve( R( *m )( Args... ) ) -> decltype( m )
  {
    return m;
  }

  template< class... Args, class T, class R >
  auto resolve( R( T::*m )( Args... ) ) -> decltype( m )
  {
    return m;
  }

  template< class T >
  struct is_shared_ptr : std::false_type
  {
  };

  template< class T >
  struct is_shared_ptr< std::shared_ptr< T > > : std::true_type
  {
  };
}

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_UTIL_H
