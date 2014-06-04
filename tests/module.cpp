#include "../classwrapper.h"
#include "../functionwrapper.h"
#include "exception.h"
#include "map.h"
#include "function.h"
#include "tuple.h"
#include "vector.h"
#include "class.h"
#include "context.h"
#include "string.h"
#include "qualifier.h"
using namespace upywrap;

struct F
{
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
  func_name_def( BuiltinValue )
  func_name_def( BuiltinConstValue )
  func_name_def( BuiltinReference )
  func_name_def( BuiltinConstReference )
  func_name_def( BuiltinPointer )
  func_name_def( BuiltinConstPointer )
  func_name_def( ReturnBuiltinReference )
  func_name_def( ReturnBuiltinPointer )
  func_name_def( Pointer )
  func_name_def( ConstPointer )
  func_name_def( Reference )
  func_name_def( ConstReference )
  func_name_def( ReturnPointer )
  func_name_def( ReturnReference )
  func_name_def( ReturnValue )
  func_name_def( Get )
  func_name_def( Address )
  func_name_def( HasExceptions )
  func_name_def( Throw )
  func_name_def( StdString )
  func_name_def( HasCharString )
  func_name_def( CharString )
  func_name_def( HasFinaliser )

  func_name_def( Add )
  func_name_def( Value )
  func_name_def( Plus )
  func_name_def( SimpleFunc )
};

//#define TEST_STATIC_ASSERTS_FOR_UNSUPPORTED_TYPES

extern "C"
{
  void InitUpyWrapTest()
  {
    auto mod = upywrap::CreateModule( "upywraptest", true );

    upywrap::ClassWrapper< Simple > wrap1( "Simple", mod );
    wrap1.DefInit< int >();
    wrap1.Def< F::Add >( &Simple::Add );
    wrap1.Def< F::Value >( &Simple::Value );
    wrap1.Def< F::Plus >( &Simple::Plus );
    wrap1.Def< F::SimpleFunc >( SimpleFunc );

    upywrap::ClassWrapper< Context > wrap2( "Context", mod );
    wrap2.DefInit<>();
    wrap2.DefDel();
    wrap2.DefExit( &Context::Dispose );

    upywrap::ClassWrapper< Q > wrap3( "Q", mod );
    wrap3.DefInit<>();
    wrap3.Def< F::Get >( &Q::Get );
    wrap3.Def< F::Address >( &Q::Address );

    upywrap::FunctionWrapper fn( mod );
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
    fn.Def< F::BuiltinValue >( BuiltinValue );
    fn.Def< F::BuiltinConstValue >( BuiltinConstValue );
    fn.Def< F::BuiltinConstReference >( BuiltinConstReference );
    fn.Def< F::Value >( Value );
    fn.Def< F::Pointer >( Pointer );
    fn.Def< F::ConstPointer >( ConstPointer );
    fn.Def< F::Reference >( Reference );
    fn.Def< F::ConstReference >( ConstReference );
    fn.Def< F::ReturnPointer >( ReturnPointer );
    fn.Def< F::ReturnReference >( ReturnReference );
    fn.Def< F::HasExceptions >( HasExceptions );
#ifndef UPYWRAP_NOEXCEPTIONS
    fn.Def< F::Throw >( Throw );
#endif
    fn.Def< F::StdString >( StdString );
    fn.Def< F::HasCharString >( HasCharString );
#ifndef UPYWRAP_NOCHARSTRING
    fn.Def< F::CharString >( CharString );
#endif
    fn.Def< F::HasFinaliser >( HasFinaliser );

    //these are all not suported so should yield compiler errors
#ifdef TEST_STATIC_ASSERTS_FOR_UNSUPPORTED_TYPES
    fn.Def< F::BuiltinReference >( BuiltinReference );
    fn.Def< F::BuiltinPointer >( BuiltinPointer );
    fn.Def< F::BuiltinConstPointer >( BuiltinConstPointer );
    fn.Def< F::ReturnBuiltinPointer >( ReturnBuiltinPointer );
    fn.Def< F::ReturnBuiltinReference >( ReturnBuiltinReference );
    fn.Def< F::ReturnValue >( ReturnValue );
#endif
  }
}
