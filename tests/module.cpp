#include "../classwrapper.h"
#include "../functionwrapper.h"
#include "../variable.h"
#include "exception.h"
#include "map.h"
#include "function.h"
#include "tuple.h"
#include "vector.h"
#include "class.h"
#include "context.h"
#include "string.h"
#include "qualifier.h"
#include "nargs.h"
#include "numeric.h"
#if UPYWRAP_HAS_CPP17
#include "optional.h"
#endif
#if UPYWRAP_THROW_ERROR_CODE
#include "errorcode.h"
#endif
using namespace upywrap;

struct F
{
  func_name_def( __eq__ )
  func_name_def( __ne__ )

  func_name_def( Pair )
  func_name_def( Tuple1 )
  func_name_def( Tuple2 )
  func_name_def( Vector1 )
  func_name_def( Vector2 )
  func_name_def( Map1 )
  func_name_def( Map2 )
  func_name_def( Func1 )
  func_name_def( Func2 )
  func_name_def( Func3 )
  func_name_def( Func4 )
  func_name_def( Func5 )
  func_name_def( Func6 )
  func_name_def( Func7 )
  func_name_def( Func8 )
  func_name_def( NoFunc )
  func_name_def( ToFunc1 )
  func_name_def( ToFunc2 )
  func_name_def( ToFunc3 )
  func_name_def( IsNullPtr )
  func_name_def( IsNullSharedPtr )
  func_name_def( IsEmptyFunction )
  func_name_def( CallbackWithNativeArg )
  func_name_def( BuiltinValue )
  func_name_def( BuiltinConstValue )
  func_name_def( BuiltinReference )
  func_name_def( BuiltinConstReference )
  func_name_def( BuiltinPointer )
  func_name_def( BuiltinConstPointer )
  func_name_def( ReturnBuiltinValue )
  func_name_def( ReturnBuiltinReference )
  func_name_def( ReturnBuiltinConstReference )
  func_name_def( ReturnBuiltinPointer )
  func_name_def( Pointer )
  func_name_def( ConstPointer )
  func_name_def( SharedPointer )
  func_name_def( ConstSharedPointer )
  func_name_def( ConstSharedPointerRef )
  func_name_def( Reference )
  func_name_def( ConstReference )
  func_name_def( ReturnPointer )
  func_name_def( ReturnReference )
  func_name_def( ReturnConstReference )
  func_name_def( ReturnSharedPointer )
  func_name_def( ReturnNullPtr )
  func_name_def( ReturnValue )
  func_name_def( Get )
  func_name_def( Address )
  func_name_def( HasExceptions )
  func_name_def( FullTypeCheck )
  func_name_def( Throw )
  func_name_def( StdString )
  func_name_def( StdStringView )
  func_name_def( HasCharString )
  func_name_def( CharString )
  func_name_def( HasFinaliser )
  func_name_def( Three )
  func_name_def( Four )
  func_name_def( Eight )
  func_name_def( Int )
  func_name_def( Unsigned )
  func_name_def( Int16 )
  func_name_def( Unsigned16 )
  func_name_def( Int64 )
  func_name_def( Unsigned64 )
  func_name_def( Double )
  func_name_def( Float )
  func_name_def( UseTypeMap )

  func_name_def( Add )
  func_name_def( Value )
  func_name_def( Name )
  func_name_def( Plus )
  func_name_def( SimpleFunc )

  func_name_def( NullOpt )
  func_name_def( OptionalInt )
  func_name_def( OptionalVector )
  func_name_def( OptionalArgument )
  func_name_def( NoErrorCode )
  func_name_def( SomeErrorCode )

  func_name_def( TestVariables )
};

void TestVariables()
{
  std::cout << GetVariable< int >( "a" ) << std::endl;
  SetVariable( 0, "a" );
  std::cout << GetVariable< int >( "x", "a" ) << std::endl;
  SetVariable( 2, "x", "a" );
  std::cout << GetVariable< int >( "x2", "x", "a" ) << std::endl;
  SetVariable( 4, "x2", "x", "a" );
}

//#define TEST_STATIC_ASSERTS_FOR_UNSUPPORTED_TYPES

extern "C"
{
  void doinit_upywraptest( mp_obj_module_t* mod )
  {
    upywrap::InitializePyObjectStore( *mod );

    upywrap::ClassWrapper< Simple > wrap1( "Simple", mod, MP_TYPE_FLAG_EQ_HAS_NEQ_TEST ); //Need this flag since we implement __ne__
    wrap1.DefInit< int >();
    wrap1.Def< F::__eq__ >( &Simple::operator == );
    wrap1.Def< F::__ne__ >( &Simple::operator != );
    wrap1.Def< F::Add >( &Simple::Add );
    wrap1.Def< F::Value >( &Simple::Value );
    wrap1.Def< F::Plus >( &Simple::Plus );
    wrap1.Def< F::SimpleFunc >( SimpleFunc );
    wrap1.Def< upywrap::special_methods::__str__ >( &Simple::Str );
    wrap1.Def< upywrap::special_methods::__call__ >( &Simple::operator bool );
    wrap1.Property( "val", &Simple::SetValue, &Simple::Value );
    wrap1.StoreClassVariable( "x", 0 );
    wrap1.StoreClassVariable( "y", 0.0 );
    wrap1.StoreClassVariable( "z", std::string( "z" ) );

    upywrap::ClassWrapper< NewSimple > wrapNewSimple( "Simple2", mod );
    wrapNewSimple.DefInit( ConstructNewSimple );
    wrapNewSimple.Def< F::Value >( &NewSimple::Value );
    wrapNewSimple.Def< F::Name >( &NewSimple::Name );
    wrapNewSimple.Def< upywrap::special_methods::__str__ >( &NewSimple::Str );

    upywrap::ClassWrapper< SharedSimple > wrapSharedSimple( "Simple3", mod );
    wrapSharedSimple.DefInit( ConstructSharedSimple );
    wrapSharedSimple.Def< F::Value >( &SharedSimple::Value );

    upywrap::ClassWrapper< SimpleCollection > wrapSimpleCollection( "SimpleCollection", mod );
    wrapSimpleCollection.DefInit();
    wrapSimpleCollection.Def< F::Add >( &SimpleCollection::Add );
    wrapSimpleCollection.Def< F::Get >( &SimpleCollection::At );
    wrapSimpleCollection.Def< F::Reference >( &SimpleCollection::RefCount );

    upywrap::ClassWrapper< Context > wrap2( "Context", mod );
    wrap2.DefInit<>();
    wrap2.DefExit( &Context::Dispose );

    upywrap::ClassWrapper< Q > wrap3( "Q", mod );
    wrap3.DefInit<>();
    wrap3.Def< F::Get >( &Q::Get );
    wrap3.Def< F::Address >( &Q::Address );

    upywrap::ClassWrapper< NargsTest > nargs( "NargsTest", mod );
    nargs.DefInit();
    nargs.Def< F::Three >( &NargsTest::Three );
    nargs.Def< F::Four >( &NargsTest::Four );

    upywrap::FunctionWrapper fn( mod );
    fn.Def< F::Pair >( Pair );
    fn.Def< F::Tuple1 >( Tuple1 );
    fn.Def< F::Tuple2 >( Tuple2 );
    fn.Def< F::Vector1 >( Vector< int > );
    fn.Def< F::Vector2 >( Vector< std::string > );
    fn.Def< F::Map1 >( Map1 );
    fn.Def< F::Map2 >( Map2 );
    fn.Def< F::Func1 >( Func1 );
    fn.Def< F::Func2 >( Func2 );
    fn.Def< F::Func3 >( Func3 );
    fn.Def< F::Func4 >( Func4 );
    fn.Def< F::Func5 >( Func5 );
    fn.Def< F::Func6 >( Func6 );
    fn.Def< F::Func7 >( Func7 );
    fn.Def< F::Func8 >( Func8 );
    fn.Def< F::NoFunc >( NoFunc );
    fn.Def< F::ToFunc1 >( ToFunc1 );
    fn.Def< F::ToFunc2 >( ToFunc2 );
    fn.Def< F::ToFunc3 >( ToFunc3 );
    fn.Def< F::IsNullPtr >( IsNullPtr );
    fn.Def< F::IsNullSharedPtr >( IsNullSharedPtr );
    fn.Def< F::IsEmptyFunction >( IsEmptyFunction );
    fn.Def< F::CallbackWithNativeArg >( CallbackWithNativeArg );
    fn.Def< F::BuiltinValue >( BuiltinValue );
    fn.Def< F::BuiltinConstValue >( BuiltinConstValue );
    fn.Def< F::BuiltinConstReference >( BuiltinConstReference );
    fn.Def< F::ReturnBuiltinValue >( ReturnBuiltinValue );
    fn.Def< F::ReturnBuiltinConstReference >( ReturnBuiltinConstReference );
    fn.Def< F::Value >( Value );
    fn.Def< F::Pointer >( Pointer );
    fn.Def< F::ConstPointer >( ConstPointer );
    fn.Def< F::SharedPointer >( SharedPointer );
    fn.Def< F::ConstSharedPointer >( ConstSharedPointer );
    fn.Def< F::ConstSharedPointerRef >( ConstSharedPointerRef );
    fn.Def< F::Reference >( Reference );
    fn.Def< F::ConstReference >( ConstReference );
    fn.Def< F::ReturnReference >( ReturnReference );
    fn.Def< F::ReturnSharedPointer >( ReturnSharedPointer );
    fn.Def< F::ReturnNullPtr >( ReturnNullPtr );
    fn.Def< F::HasExceptions >( HasExceptions );
    fn.Def< F::FullTypeCheck >( FullTypeCheck );
#if UPYWRAP_USE_EXCEPTIONS
    fn.Def< F::Throw >( Throw );
#endif
    fn.Def< F::StdString >( StdString );
    fn.Def< F::StdStringView >( StdStringView );
    fn.Def< F::HasCharString >( HasCharString );
#if UPYWRAP_USE_CHARSTRING
    fn.Def< F::CharString >( CharString );
#endif
    fn.Def< F::Four >( Four );
    fn.Def< F::Eight >( Eight );
    fn.Def< F::Int >( Int );
    fn.Def< F::Int16 >( Int16 );
    fn.Def< F::Int64 >( Int64 );
    fn.Def< F::Unsigned >( Unsigned );
    fn.Def< F::Unsigned16 >( Unsigned16 );
    fn.Def< F::Unsigned64 >( Unsigned64 );
    fn.Def< F::Float >( Float );
    fn.Def< F::Double >( Double );

    fn.Def< F::TestVariables >( TestVariables );

#if UPYWRAP_HAS_CPP17
    fn.Def< F::NullOpt >( NullOpt );
    fn.Def< F::OptionalInt >( OptionalInt );
    fn.Def< F::OptionalVector >( OptionalVector );
    fn.Def< F::OptionalArgument >( OptionalArgument );
#endif

#if UPYWRAP_THROW_ERROR_CODE
    fn.Def< F::NoErrorCode >( NoErrorCode );
    fn.Def< F::SomeErrorCode >( SomeErrorCode );
#endif

    //these are all not suported so should yield compiler errors
#ifdef TEST_STATIC_ASSERTS_FOR_UNSUPPORTED_TYPES
    fn.Def< F::ReturnPointer >( ReturnPointer );
    fn.Def< F::BuiltinReference >( BuiltinReference );
    fn.Def< F::BuiltinPointer >( BuiltinPointer );
    fn.Def< F::BuiltinConstPointer >( BuiltinConstPointer );
    fn.Def< F::ReturnBuiltinPointer >( ReturnBuiltinPointer );
    fn.Def< F::ReturnBuiltinReference >( ReturnBuiltinReference );
    fn.Def< F::ReturnValue >( ReturnValue );
    fn.Def< F::ReturnConstReference >( ReturnConstReference );
#endif
  }

#ifdef _MSC_VER
  _declspec( dllexport )
#endif
  mp_obj_module_t* init_upywraptest()
  {
    auto mod = upywrap::CreateModule( "upywraptest", false );
    doinit_upywraptest( mod );
    return mod;
  }
}
