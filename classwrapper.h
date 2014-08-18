#ifndef MICROPYTHONMODULE_CLASSWRAPPER
#define MICROPYTHONMODULE_CLASSWRAPPER

#include "detail/index.h"
#include "detail/util.h"
#include "detail/functioncall.h"
#include "detail/callreturn.h"
#include <vector>
#include <cstdint>

namespace upywrap
{
  //Main logic for registering classes and their functions.
  //Usage:
  //
  //struct SomeClass
  //{
  //  SomeClass();
  //  int Foo( int );
  //  double Bar( const std::string& );
  //};
  //
  //struct Funcs
  //{
  //  func_name_def( Foo )
  //  func_name_def( Bar )
  //};
  //
  //ClassWrapper< SomeClass > wrap( "SomeClass", dict );
  //wrap.Def< Funcs::Foo >( &SomeClass::Foo );
  //wrap.Def< Funcs::Bar >( &SomeClass::Bar );
  //
  //This will register type "SomeClass" and given functions in dict,
  //so if dict is the global dict of a module "mod", the class
  //can be used in uPy like this:
  //
  //import mod;
  //x = mod.SomeClass();
  //x.Foo();
  //x.Bar();
  //
  //For supported arguments and return values see FromPyObj and ToPyObj classes.
  template< class T >
  class ClassWrapper
  {
  public:
    ClassWrapper( const char* name, mp_obj_module_t* mod ) :
      ClassWrapper( name, mod->globals )
    {
    }

    ClassWrapper( const char* name, mp_obj_dict_t* dict )
    {
      static bool init = false;
      if( !init )
      {
        OneTimeInit( name, dict );
        init = true;
      }
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret( *f ) ( T*, A... ), typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, conv );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret( T::*f ) ( A... ), typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, conv );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret( T::*f ) ( A... ) const, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, conv );
    }

    void DefInit()
    {
      DefInit<>();
    }

    template< class... A >
    void DefInit()
    {
      DefInit( ConstructorFactoryFunc< T, A... > );
    }

    template< class... A >
    void DefInit( T*( *f ) ( A... ) )
    {
      InitImpl< FixedFuncNames::Init, decltype( f ), A... >( f );
    }

    void DefExit( void( T::*f ) () )
    {
      ExitImpl< FixedFuncNames::Exit, decltype( f ) >( f );
    }

    static mp_obj_t AsPyObj( T* p )
    {
#ifdef UPYWRAP_NOFINALISER
      #define upywrap_new_obj m_new_obj
#else
      #define upywrap_new_obj m_new_obj_with_finaliser
#endif
      if( type.base.type == nullptr )
        RaiseTypeException( "Native type has not been registered" );
      auto o = upywrap_new_obj( this_type );
      o->base.type = &type;
      o->cookie = defCookie;
      o->obj = p;
      return o;
    }

    static T* AsNativePtr( mp_obj_t arg )
    {
      auto native = (this_type*) arg;
      if( native->cookie != defCookie )
        RaiseTypeException( "Cannot convert this object to a native class instance" );
      return native->obj;
    }

    mp_obj_base_t base; //must always be the first member!
    std::int64_t cookie; //we'll use this to check if a pointer really points to a ClassWrapper
    T* obj;
    static function_ptrs functionPointers;
    static const std::int64_t defCookie;

  private:
    struct FixedFuncNames
    {
      func_name_def( Init )
      func_name_def( Exit )
      func_name_def( __del__ )
    };

    void OneTimeInit( std::string name, mp_obj_dict_t* dict )
    {
      const auto qname = qstr_from_str( name.data() );
      type.base.type = &mp_type_type;
      type.name = qname;
      type.locals_dict = (mp_obj_dict_t*) mp_obj_new_dict( 0 );
      type.make_new = nullptr;

      mp_obj_dict_store( dict, MP_OBJ_NEW_QSTR( qname ), &type );
      //store our dict in the module's dict so it's reachable by the GC mark phase,
      //or in other words: prevent the GC from sweeping it!!
      mp_obj_dict_store( dict, MP_OBJ_NEW_QSTR( qstr_from_str( ( name + "_locals" ).data() ) ), type.locals_dict );

      DelImpl();
    }

    void AddFunctionToTable( const qstr name, mp_obj_t fun )
    {
      mp_obj_dict_store( type.locals_dict, MP_OBJ_NEW_QSTR( name ), fun );
    }

    void AddFunctionToTable( const char* name, mp_obj_t fun )
    {
      AddFunctionToTable( qstr_from_str( name ), fun );
    }

    template< index_type name, class Ret, class Fun, class... A >
    void DefImpl( Fun f, typename SelectRetvalConverter< Ret >::type conv )
    {
      typedef NativeMemberCall< name, Ret, A... > call_type;
      auto callerObject = call_type::CreateCaller( f );
      if( conv )
        callerObject->convert_retval = conv;
      functionPointers[ (void*) name ] = callerObject;
      auto call = sizeof...( A ) + 1 > UPYWRAP_MAX_NATIVE_ARGS ? (void*) call_type::CallN : (void*) call_type::Call;
      AddFunctionToTable( name(), mp_make_function_n( 1 + sizeof...( A ), call ) );
    }

    void DelImpl()
    {
      typedef NativeMemberCall< FixedFuncNames::__del__, void, T* > call_type;
      auto call = (void*) call_type::Delete;
      AddFunctionToTable( FixedFuncNames::__del__(), mp_make_function_n( 1, call ) );
    }

    template< index_type name, class Fun, class... A >
    void InitImpl( Fun f )
    {
      typedef NativeMemberCall< name, T*, A... > call_type;
      functionPointers[ (void*) name ] = call_type::CreateCaller( f );
      type.make_new = call_type::MakeNew;
    }

    template< index_type name, class Fun >
    void ExitImpl( Fun f )
    {
      typedef NativeMemberCall< name, void > call_type;
      functionPointers[ (void*) name ] = call_type::CreateCaller( f );
      AddFunctionToTable( MP_QSTR___enter__, (mp_obj_t) &mp_identity_obj );
      AddFunctionToTable( MP_QSTR___exit__, mp_make_function_n( 4, (void*) call_type::CallDiscard ) );
    }

    //wrap native call in function with uPy compatible mp_obj_t( mp_obj_t self, mp_obj_t.... ) signature
    template< index_type index, class Ret, class... A >
    struct NativeMemberCall
    {
      typedef InstanceFunctionCall< T, Ret, A... > call_type;
      typedef FunctionCall< T*, A... > init_call_type;
      typedef typename call_type::func_type func_type;
      typedef typename call_type::mem_func_type mem_func_type;
      typedef typename call_type::const_mem_func_type const_mem_func_type;
      typedef typename init_call_type::func_type init_func_type;

      static call_type* CreateCaller( func_type f )
      {
        return new NonMemberFunctionCall< T, Ret, A... >( f );
      }

      static call_type* CreateCaller( mem_func_type f )
      {
        return new MemberFunctionCall< T, Ret, A... >( f );
      }

      static call_type* CreateCaller( const_mem_func_type f )
      {
        return new ConstMemberFunctionCall< T, Ret, A... >( f );
      }

      static init_call_type* CreateCaller( init_func_type f )
      {
        return new init_call_type( f );
      }

      static mp_obj_t Call( mp_obj_t self_in, typename project2nd< A, mp_obj_t >::type... args )
      {
        auto self = (this_type*) self_in;
        auto f = (call_type*) this_type::functionPointers[ (void*) index ];
        return CallReturn< Ret, A... >::Call( f, self->obj, args... );
      }

      static mp_obj_t CallN( uint n_args, const mp_obj_t* args )
      {
        if( n_args != sizeof...( A ) + 1 )
          RaiseTypeException( "Wrong number of arguments" );
        auto self = (this_type*) args[ 0 ];
        auto firstArg = &args[ 1 ];
        auto f = (call_type*) this_type::functionPointers[ (void*) index ];
        return callvar( f, self->obj, firstArg, make_index_sequence< sizeof...( A ) >() );
      }

      static mp_obj_t CallDiscard( uint n_args, const mp_obj_t* args )
      {
        (void) n_args;
        static_assert( sizeof...( A ) == 0, "Arguments must be discarded" );
        auto self = (this_type*) args[ 0 ];
        auto f = (call_type*) this_type::functionPointers[ (void*) index ];
        return CallReturn< Ret, A... >::Call( f, self->obj );
      }

      static mp_obj_t MakeNew( mp_obj_t, uint n_args, uint, const mp_obj_t *args )
      {
        if( n_args != sizeof...( A ) )
          RaiseTypeException( "Wrong number of arguments for constructor" );
        auto f = (init_call_type*) this_type::functionPointers[ (void*) index ];
        return AsPyObj( apply( f, args, make_index_sequence< sizeof...( A ) >() ) );
      }

      static mp_obj_t Delete( mp_obj_t self_in )
      {
        auto self = (this_type*) self_in;
        delete self->obj;
        return ToPyObj< void >::Convert();
      }

    private:
      template< size_t... Indices >
      static T* apply( init_call_type* f, const mp_obj_t* args, index_sequence< Indices... > )
      {
        (void) args;
        return f->Call( SelectFromPyObj< A >::type::Convert( args[ Indices ] )... );
      }

      template< size_t... Indices >
      static mp_obj_t callvar( call_type* f, T* self, const mp_obj_t* args, index_sequence< Indices... > )
      {
        (void) args;
        return CallReturn< Ret, A... >::Call( f, self, args[ Indices ]... );
      }
    };

    typedef ClassWrapper< T > this_type;

    static mp_obj_type_t type;
  };

  template< class T >
  mp_obj_type_t ClassWrapper< T >::type = { nullptr };

  template< class T >
  function_ptrs ClassWrapper< T >::functionPointers;

  template< class T >
  const std::int64_t ClassWrapper< T >::defCookie = 0x12345678908765;


  //Get instance pointer out of mp_obj_t
  template< class T >
  struct ClassFromPyObj< T* >
  {
    static T* Convert( mp_obj_t arg )
    {
      return ClassWrapper< T >::AsNativePtr( arg );
    }
  };

  template< class T >
  struct ClassFromPyObj< T& >
  {
    static T& Convert( mp_obj_t arg )
    {
      return *ClassFromPyObj< T* >::Convert( arg );
    }
  };

  template< class T >
  struct ClassFromPyObj
  {
    static T Convert( mp_obj_t arg )
    {
      return *ClassFromPyObj< T* >::Convert( arg );
    }
  };

  //Wrap instance in a new mp_obj_t
  template< class T >
  struct ClassToPyObj< T* >
  {
    static mp_obj_t Convert( T* p )
    {
      return ClassWrapper< T >::AsPyObj( p );
    }
  };

  template< class T >
  struct ClassToPyObj< T& >
  {
    static mp_obj_t Convert( T& p )
    {
      return ClassToPyObj< T* >::Convert( &p );
    }
  };
}

#endif
