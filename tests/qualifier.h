#ifndef MICROPYTHON_WRAP_TESTS_QUALIFIER_H
#define MICROPYTHON_WRAP_TESTS_QUALIFIER_H

#include <iostream>
#include <memory>
#include <string>

namespace upywrap
{
  struct Q
  {
    Q() :
      a( 0 )
    {
    }

    void Call()
    {
      a = 2;
    }

    void Call() const
    {
      a = 3;
    }

    int Get() const
    {
      return a;
    }

    int Address() const
    {
      return (int) (size_t) this;
    }

    mutable int a;
  };

  void BuiltinValue( int a )
  {
    std::cout << a << std::endl;
  }

  void BuiltinConstValue( const int a )
  {
    std::cout << a << std::endl;
  }

  void BuiltinReference( std::string& a ) //not supported
  {
    std::cout << a << std::endl;
  }

  void BuiltinConstReference( const std::string& a )
  {
    std::cout << a << std::endl;
  }

  void BuiltinPointer( std::string* a ) //not supported
  {
    std::cout << *a << std::endl;
  }

  void BuiltinConstPointer( const std::string* a ) //not supported
  {
    std::cout << *a << std::endl;
  }

  std::string ReturnBuiltinValue( std::string a )
  {
    return a;
  }

  std::string& ReturnBuiltinReference() //not supported
  {
    static std::string a;
    return a;
  }

  const std::string& ReturnBuiltinConstReference( const std::string& a ) //supported if UPYWRAP_PASSCONSTREF
  {
    return a;
  }

  std::string* ReturnBuiltinPointer() //not supported
  {
    static std::string a;
    return &a;
  }


  void Value( Q a )
  {
    a.Call();
  }

  void Pointer( Q* a )
  {
    a->Call();
  }

  void ConstPointer( const Q* a )
  {
    a->Call();
  }

  void SharedPointer( std::shared_ptr< Q > p )
  {
    p->Call();
  }

  void ConstSharedPointer( const std::shared_ptr< Q > p )
  {
    p->Call();
  }

  void ConstSharedPointerRef( const std::shared_ptr< Q >& p )
  {
    p->Call();
  }

  void Reference( Q& a )
  {
    a.Call();
  }

  void ConstReference( const Q& a )
  {
    a.Call();
  }

  Q ReturnValue( Q* a ) //not supported
  {
    return *a;
  }

  Q* ReturnPointer( Q* a )
  {
    return a;
  }

  Q& ReturnReference( Q* a )
  {
    return *a;
  }

  const Q& ReturnConstReference( const Q& a ) //not supported
  {
    return a;
  }

  std::shared_ptr< Q > ReturnSharedPointer()
  {
    return std::make_shared< Q >();
  }

  std::shared_ptr< Q > ReturnNullPtr()
  {
    return nullptr;
  }

}

#endif //#ifndef MICROPYTHON_WRAP_TESTS_QUALIFIER_H
