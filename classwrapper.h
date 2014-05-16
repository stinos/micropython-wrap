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
    void Def( Ret( *f ) ( T*, A... ) )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret( T::*f ) ( A... ) )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f );
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
      auto o = m_new_obj( this_type );
      o->base.type = &type;
      o->cookie = defCookie;
      o->obj = p;
      return o;
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
    };

    static void OneTimeInit( const char* name, mp_obj_dict_t* dict )
    {
      locals = (mp_obj_dict_t*) mp_obj_new_dict( 0 );

      const auto qname = qstr_from_str( name );
      type.base.type = &mp_type_type;
      type.name = qname;
      type.locals_dict = locals;
      type.make_new = nullptr;

      mp_obj_dict_store( dict, MP_OBJ_NEW_QSTR( qname ), &type );
    }

    static void AddFunctionToTable( const qstr name, mp_obj_t fun )
    {
      const mp_map_elem_t elem = { MP_OBJ_NEW_QSTR( name ), fun };
      localsTable.push_back( elem );
      locals->map.table = (mp_map_elem_t*) this_type::localsTable.data();
      ++locals->map.used;
      ++locals->map.alloc;
    }

    static void AddFunctionToTable( const char* name, mp_obj_t fun )
    {
      AddFunctionToTable( qstr_from_str( name ), fun );
    }

    template< index_type name, class Ret, class Fun, class... A >
    static void DefImpl( Fun f )
    {
      typedef NativeMemberCall< name, Ret, A... > call_type;
      functionPointers[ (void*) name ] = call_type::CreateCaller( f );
      AddFunctionToTable( name(), mp_make_function_n( 1 + sizeof...( A ), (void*) call_type::Call ) );
    }

    template< index_type name, class Fun, class... A >
    static void InitImpl( Fun f )
    {
      typedef NativeMemberCall< name, T*, A... > call_type;
      functionPointers[ (void*) name ] = call_type::CreateCaller( f );
      type.make_new = call_type::MakeNew;
    }

    template< index_type name, class Fun >
    static void ExitImpl( Fun f )
    {
      typedef NativeMemberCall< name, void > call_type;
      functionPointers[ (void*) name ] = call_type::CreateCaller( f );
      AddFunctionToTable( MP_QSTR___enter__, (mp_obj_t) &mp_identity_obj );
      AddFunctionToTable( MP_QSTR___exit__, mp_make_function_var_between( 4, 4, call_type::CallVar ) );
    }

    //wrap native call in function with uPy compatible mp_obj_t( mp_obj_t self, mp_obj_t.... ) signature
    template< index_type index, class Ret, class... A >
    struct NativeMemberCall
    {
      typedef InstanceFunctionCall< T, Ret, A... > call_type;
      typedef FunctionCall< T*, A... > init_call_type;
      typedef typename call_type::func_type func_type;
      typedef typename call_type::mem_func_type mem_func_type;
      typedef typename init_call_type::func_type init_func_type;

      static call_type* CreateCaller( func_type f )
      {
        return new NonMemberFunctionCall< T, Ret, A... >( f );
      }

      static call_type* CreateCaller( mem_func_type f )
      {
        return new MemberFunctionCall< T, Ret, A... >( f );
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

      static mp_obj_t CallVar( uint n_args, const mp_obj_t* args )
      {
        static_assert( sizeof...( A ) == 0, "TODO expand args array if you want this with multiple arguments" );
        auto self = (this_type*) args[ 0 ];
        auto f = (call_type*) this_type::functionPointers[ (void*) index ];
        return CallReturn< Ret, A... >::Call( f, self->obj );
      }

      static mp_obj_t MakeNew( mp_obj_t type_in, uint n_args, uint n_kw, const mp_obj_t *args )
      {
        if( n_args != sizeof...( A ) )
          RaiseTypeException( "Wrong number of arguments for constructor" );
        auto f = (init_call_type*) this_type::functionPointers[ (void*) index ];
        return AsPyObj( apply( f, args, typename make_indices< A... >::type() ) );
      }

    private:
      template< size_t... Indices >
      static T* apply( init_call_type* f, const mp_obj_t* args, index_tuple< Indices... > )
      {
        return f->Call( FromPyObj< typename remove_all< A >::type >::Convert( args[ Indices ] )... );
      }
    };

    typedef ClassWrapper< T > this_type;
    typedef std::vector< mp_map_elem_t > map_type;

    static mp_obj_type_t type;
    static mp_obj_dict_t* locals;
    static map_type localsTable;
  };

  template< class T >
  mp_obj_type_t ClassWrapper< T >::type;

  template< class T >
  mp_obj_dict_t* ClassWrapper< T >::locals;

  template< class T >
  typename ClassWrapper< T >::map_type ClassWrapper< T >::localsTable;

  template< class T >
  function_ptrs ClassWrapper< T >::functionPointers;

  template< class T >
  const std::int64_t ClassWrapper< T >::defCookie = 0x12345678908765;


  //Get intance pointer out of mp_obj_t
  template< class T >
  struct FromPyObj< T* >
  {
    typedef ClassWrapper< T > wrap_type;

    static T* Convert( mp_obj_t arg )
    {
      auto native = (wrap_type*) arg;
      if( native->cookie != wrap_type::defCookie )
        RaiseTypeException( "Cannot convert this object to a native class instance" );
      return native->obj;
    }
  };

  //Wrap instance in a new mp_obj_t
  template< class T >
  struct ToPyObj< T* >
  {
    static mp_obj_t Convert( T* p )
    {
      return ClassWrapper< T >::AsPyObj( p );
    }
  };
}

#endif
