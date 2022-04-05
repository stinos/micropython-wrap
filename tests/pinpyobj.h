#pragma once

#include "../detail/micropython.h"
#include <exception>

//Simple macro for C++ tests: just throw descriptive exception when argument is false.
#ifdef CHECK
#undef CHECK
#endif

#define CHECK( condition ) \
  if( !( condition ) ) \
  { \
    const auto message = std::string( "FAILED: '" ) + #condition + "' in file " + __FILE__ + " line " + std::to_string( __LINE__ ); \
    throw std::runtime_error( message.c_str() ); \
  }

void TestPinPyObj()
{
  using upywrap::PinPyObj;
  using upywrap::StaticPyObjectStore;

  CHECK( StaticPyObjectStore::Initialized() );
  const auto objectStore = StaticPyObjectStore::BackEnd();
  const auto objectStoreLength = [objectStore] () { return objectStore->len; };
  const auto initialObjectStoreLength = objectStoreLength();
  const auto upyObject = mp_obj_new_int( 0 );

  //An empty object, not stored.
  {
    PinPyObj x;
    CHECK( *x == nullptr );
    CHECK( !x );
    CHECK( objectStoreLength() == initialObjectStoreLength );
    PinPyObj y( x );
    CHECK( *y == nullptr );
    CHECK( objectStoreLength() == initialObjectStoreLength );
  }

  //Constructor stores, destructor removes again.
  {
    PinPyObj x( upyObject );
    CHECK( *x == upyObject );
    CHECK( objectStoreLength() == initialObjectStoreLength + 1 );
  }
  CHECK( objectStoreLength() == initialObjectStoreLength );

  //Test assignment and copying; all refer to the same object so gets stored once only.
  {
    PinPyObj x;
    x = PinPyObj( upyObject );
    CHECK( *x == upyObject );
    CHECK( objectStoreLength() == initialObjectStoreLength + 1 );
    PinPyObj y( x );
    CHECK( *y == upyObject );
    CHECK( objectStoreLength() == initialObjectStoreLength + 1 );
    PinPyObj z( std::move( x ) );
    CHECK( !x );
    CHECK( *z == upyObject );
    CHECK( objectStoreLength() == initialObjectStoreLength + 1 );
    y = PinPyObj();
    z = y;
    CHECK( objectStoreLength() == initialObjectStoreLength );
  }
}
