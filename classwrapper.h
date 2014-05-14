#ifndef MICROPYTHONMODULE_CLASSWRAPPER
#define MICROPYTHONMODULE_CLASSWRAPPER

#include "detail/index.h"
#include "detail/util.h"
#include "detail/functioncall.h"
#include "detail/callreturn.h"
#include <vector>

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

    mp_obj_base_t base; //must always be the first member!
    T* obj;
    static function_ptrs functionPointers;

  private:
    static void OneTimeInit( const char* name, mp_obj_dict_t* dict )
    {
      locals = (mp_obj_dict_t*) mp_obj_new_dict( 0 );

      const auto qname = qstr_from_str( name );
      type.base.type = &mp_type_type;
      type.name = qname;
      type.locals_dict = locals;
      type.make_new = MakeNew;

      mp_obj_dict_store( dict, MP_OBJ_NEW_QSTR( qname ), &type );
    }

    static mp_obj_t MakeNew( mp_obj_t type_in, uint n_args, uint n_kw, const mp_obj_t *args )
    {
      this_type* o = m_new_obj( this_type );
      o->base.type = &o->type;
      o->obj = new T();
      return o;
    }

    static void AddFunctionToTable( const char* name, mp_obj_t fun )
    {
      const mp_map_elem_t elem = { MP_OBJ_NEW_QSTR( qstr_from_str( name ) ), fun };
      localsTable.push_back( elem );
      locals->map.table = (mp_map_elem_t*) this_type::localsTable.data();
      ++locals->map.used;
      ++locals->map.alloc;
    }

    template< index_type name, class Ret, class Fun, class... A >
    static void DefImpl( Fun f )
    {
      typedef NativeMemberCall< name, Ret, A... > call_type;
      functionPointers[ (void*) name ] = call_type::CreateCaller( f );
      AddFunctionToTable( name(), mp_make_function_n( 1 + sizeof...( A ), (void*) call_type::Call ) );
    }

    //wrap native call in function with uPy compatible mp_obj_t( mp_obj_t self, mp_obj_t.... ) signature
    template< index_type index, class Ret, class... A >
    struct NativeMemberCall
    {
      typedef InstanceFunctionCall< T, Ret, A... > call_type;
      typedef typename call_type::func_type func_type;
      typedef typename call_type::mem_func_type mem_func_type;

      static call_type* CreateCaller( func_type f )
      {
        return new NonMemberFunctionCall< T, Ret, A... >( f );
      }

      static call_type* CreateCaller( mem_func_type f )
      {
        return new MemberFunctionCall< T, Ret, A... >( f );
      }

      static mp_obj_t Call( mp_obj_t self_in, typename project2nd< A, mp_obj_t >::type... args )
      {
        auto self = (this_type*) self_in;
        auto f = (call_type*) this_type::functionPointers[ (void*) index ];
        return CallReturn< Ret, A... >::Call( f, self->obj, args... );
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
}

#endif
