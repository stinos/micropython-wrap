#ifndef MICROPYTHON_WRAP_TESTS_CLASS_H
#define MICROPYTHON_WRAP_TESTS_CLASS_H

#include <iostream>
#include <memory>
#include <string>

namespace upywrap
{
  class Simple
  {
  public:
    Simple( int a ) :
      a( a )
    {
    }

    virtual ~Simple()
    {
    }

    void Add( int x )
    {
      a += x;
    }

    int Value() const
    {
      return a;
    }

    void SetValue( int x )
    {
      a = x;
    }

    void Plus( Simple& rh )
    {
      a += rh.a;
    }

    virtual std::string Str() const
    {
      return "Simple " + std::to_string( a );
    }

    bool operator == ( const Simple& rh ) const
    {
      return a == rh.a;
    }

    bool operator != ( const Simple& rh ) const
    {
      std::cout << "__ne__" << std::endl;
      return !this->operator == ( rh );
    }

    explicit operator bool() const
    {
      return a != 0;
    }

  private:
    int a;
  };

  Simple& SimpleFunc( Simple& p1, Simple& p2 )
  {
    p1.Plus( p2 );
    return p1;
  }

  class NewSimple : public Simple
  {
  public:
    NewSimple( int v ) :
      Simple( v )
    {
    }

    std::string Str() const override
    {
      return "Simple2 " + std::to_string( Value() );
    }

    std::string Name() const
    {
      return "Simple2";
    }
  };

  NewSimple* ConstructNewSimple()
  {
    return new NewSimple( 33 );
  }

  class SharedSimple : public Simple
  {
  public:
    SharedSimple( int v ) :
      Simple( v )
    {
    }
  };

  std::shared_ptr< SharedSimple > ConstructSharedSimple( int val )
  {
    return std::make_shared< SharedSimple >( val );
  }

  bool IsNullPtr( Simple* p )
  {
    return p == nullptr;
  }

  bool IsNullSharedPtr( std::shared_ptr< Simple > p )
  {
    return p == nullptr;
  }

  class SimpleCollection
  {
  public:
    using simple_t = std::shared_ptr< Simple >;

    SimpleCollection()
    {
    }

    void Add( simple_t s )
    {
      simples.push_back( s );
    }

    simple_t At( size_t index )
    {
      return simples.at( index );
    }

    int RefCount( size_t index )
    {
      return simples.at( index ).use_count();
    }

  private:
    std::vector< simple_t > simples;
  };
}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_CLASS_H
