#ifndef MICROPYTHON_WRAP_CLASSWRAPPER
#define MICROPYTHON_WRAP_CLASSWRAPPER

#include "detail/callreturn.h"
#include "detail/functioncall.h"
#include "detail/index.h"
#include "detail/util.h"
#include <cstdint>
#include <vector>
#if UPYWRAP_SHAREDPTROBJ
#include <memory>
#endif

namespace upywrap
{
  inline bool FullTypeCheck()
  {
    return UPYWRAP_FULLTYPECHECK == 1;
  }

  //Main logic for registering classes and their functions.
  //Usage:
  //
  //struct SomeClass
  //{
  //  SomeClass();
  //  int Foo( int );
  //  double Bar( const std::string&, int );
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
  //wrap.Def< Funcs::Bar >( &SomeClass::Bar, Kwargs( "a", mp_const_none )( "b", 0 ) );
  //
  //This will register type "SomeClass" and given functions in dict,
  //so if dict is the global dict of a module "mod", the class
  //can be used in uPy like this:
  //
  //import mod;
  //x = mod.SomeClass();
  //x.Foo();
  //x.Bar();
  //x.Bar(a="a", b=2);
  //
  //For supported arguments and return values see FromPyObj and ToPyObj classes.
  template< class T >
  class ClassWrapper
  {
  public:
#if UPYWRAP_SHAREDPTROBJ
    using native_obj_t = std::shared_ptr< T >;
#else
    using native_obj_t = T*;
#endif
    //Just to make it clear what the intent is.
    enum class ConstructorOptions
    {
      RegisterInStaticPyObjectStore
    };

    //Initialize the type and store it in the module's globals dict so it's accessible as <mod>.<name>.
    ClassWrapper( const char* name, mp_obj_module_t* mod, decltype( mp_obj_type_t::flags ) flags = 0 ) :
      ClassWrapper( name, mod->globals, flags )
    {
    }

    //Initialize the type and store it in the given dict, normally a module's globals dict.
    //Stores the type with the given name, but also stores the type's locals dict: assuming the
    //given dict is allocated on the MicroPython heap this makes our members reachable by the GC
    //mark phase, or in other words: crucial to prevent the GC from sweeping it.
    ClassWrapper( const char* name, mp_obj_dict_t* dict, decltype( mp_obj_type_t::flags ) flags = 0 ) :
      ClassWrapper( name, flags )
    {
      mp_obj_dict_store( dict, new_qstr( name ), &type );
      mp_obj_dict_store( dict, new_qstr( ( std::string( name ) + "_locals" ).data() ), MP_OBJ_FROM_PTR( MP_OBJ_TYPE_GET_SLOT( &type, locals_dict ) ) );
    }

    //Initialize the type, storing the locals in StaticPyObjectStore to prevent GC collection.
    ClassWrapper( const char* name, ConstructorOptions, decltype( mp_obj_type_t::flags ) flags = 0 ) :
      ClassWrapper( name, flags )
    {
      StaticPyObjectStore::Store( MP_OBJ_FROM_PTR( MP_OBJ_TYPE_GET_SLOT( &type, locals_dict ) ) );
    }

    static const mp_obj_type_t& Type()
    {
      return *((const mp_obj_type_t*) &type);
    }

    template< class A >
    void StoreClassVariable( const char* name, const A& value )
    {
      mp_obj_dict_store( MP_OBJ_FROM_PTR( MP_OBJ_TYPE_GET_SLOT( &type, locals_dict ) ), new_qstr( name ), ToPy( value ) );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret( *f ) ( T*, A... ), typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, conv );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret( *f ) ( T&, A... ), typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, conv );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret( *f ) ( const T&, A... ), typename SelectRetvalConverter< Ret >::type conv = nullptr )
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

    template< index_type name, class Base, class Ret, class... A >
    typename std::enable_if< std::is_base_of< Base, T >::value >::type Def( Ret( Base::*f ) ( A... ), typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, conv );
    }

    template< index_type name, class Base, class Ret, class... A >
    typename std::enable_if< std::is_base_of< Base, T >::value >::type Def( Ret( Base::*f ) ( A... ) const, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, conv );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret ( *f )( T*, A... ), Arguments arguments, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, std::move( arguments ), conv );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret ( *f )( T&, A... ), Arguments arguments, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, std::move( arguments ), conv );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret ( *f )( const T&, A... ), Arguments arguments, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, std::move( arguments ), conv );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret ( T::*f )( A... ), Arguments arguments, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, std::move( arguments ), conv );
    }

    template< index_type name, class Ret, class... A >
    void Def( Ret ( T::*f )( A... ) const, Arguments arguments, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, std::move( arguments ), conv );
    }

    template< index_type name, class Base, class Ret, class... A >
    typename std::enable_if< std::is_base_of< Base, T >::value >::type Def( Ret ( Base::*f )( A... ), Arguments arguments, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, std::move( arguments ), conv );
    }

    template< index_type name, class Base, class Ret, class... A >
    typename std::enable_if< std::is_base_of< Base, T >::value >::type Def( Ret ( Base::*f )( A... ) const, Arguments arguments, typename SelectRetvalConverter< Ret >::type conv = nullptr )
    {
      DefImpl< name, Ret, decltype( f ), A... >( f, std::move( arguments ), conv );
    }

    template< class A >
    void Setter( const char* name, void( *f )( T*, A ) )
    {
      SetterImpl< decltype( f ), A >( name, f );
    }

    template< class A >
    void Setter( const char* name, void( T::*f )( A ) )
    {
      SetterImpl< decltype( f ), A >( name, f );
    }

    template< class A >
    void Getter( const char* name, A( *f )( T* ) )
    {
      GetterImpl< decltype( f ), A >( name, f );
    }

    template< class A >
    void Getter( const char* name, A( T::*f )() const )
    {
      GetterImpl< decltype( f ), A >( name, f );
    }

    template< class A >
    void Property( const char* name, void( *fset )( T*, A ), A( *fget )( T* ) )
    {
      SetterImpl< decltype( fset ), A >( name, fset );
      GetterImpl< decltype( fget ), A >( name, fget );
    }

    template< class A >
    void Property( const char* name, void( T::*fset )( A ), A( T::*fget )() const )
    {
      SetterImpl< decltype( fset ), A >( name, fset );
      GetterImpl< decltype( fget ), A >( name, fget );
    }

    void DefInit()
    {
      DefInit<>();
    }

    template< class... A >
    void DefInit()
    {
      DefInit( ConstructorFactoryFunc< A... > );
    }

    template< class... A >
    void DefInit( Arguments arguments )
    {
      DefInit( ConstructorFactoryFunc< A... >, std::move( arguments ) );
    }

    template< class... A >
    void DefInit( T*( *f ) ( A... ) )
    {
      InitImpl< FixedFuncNames::Init, decltype( f ), T*, A... >( f );
    }

    template< class... A >
    void DefInit( T* ( *f )( A... ), Arguments arguments )
    {
      InitImpl< FixedFuncNames::Init, decltype( f ), T*, A... >( f, std::move( arguments ) );
    }

#if UPYWRAP_SHAREDPTROBJ
    template< class... A >
    void DefInit( std::shared_ptr< T >( *f ) ( A... ) )
    {
      InitImpl< FixedFuncNames::Init, decltype( f ), std::shared_ptr< T >, A... >( f );
    }

    template< class... A >
    void DefInit( std::shared_ptr< T >( *f ) ( A... ), Arguments arguments )
    {
      InitImpl< FixedFuncNames::Init, decltype( f ), std::shared_ptr< T >, A... >( f, std::move( arguments ) );
    }
#endif

    void DefExit( void( T::*f ) () )
    {
      ExitImpl< FixedFuncNames::Exit, decltype( f ) >( f );
    }

    void DefExit( void( *f ) ( T* ) )
    {
      ExitImpl< FixedFuncNames::Exit, decltype( f ) >( f );
    }

    void DefExit( void( *f ) ( T& ) )
    {
      ExitImpl< FixedFuncNames::Exit, decltype( f ) >( f );
    }

    //If argument looks like a ClassWrapper< T > return it as such.
    //With UPYWRAP_SHAREDPTROBJ this will essentially increase ref count and return
    //a new ClassWrapper with the same shared_ptr control block as another one,
    //but possibly with different functions registered.
    //As such it's useful if you e.g. have an instance of ClassWrapper< A > and this is
    //type ClassWrapper< B > and B derives from A, then B.Cast(a) gives the expected thing.
    //Also see classnt.py test.
    //Use with caution: see comments in AsNativeObjChecked.
    static mp_obj_t Cast( mp_obj_t other )
    {
      return AsPyObj( AsNativeObjChecked( other )->obj );
    }

#if UPYWRAP_SHAREDPTROBJ
    static mp_obj_t AsPyObj( T* p, bool own )
    {
      if( own )
      {
        return AsPyObj( native_obj_t( p ) );
      }
      return AsPyObj( native_obj_t( p, NoDelete ) );
    }
#else
    static mp_obj_t AsPyObj( T* p, bool )
    {
      return AsPyObj( p );
    }
#endif

    static mp_obj_t AsPyObj( native_obj_t p )
    {
      assert( p );
      CheckTypeIsRegistered();
      auto o = (this_type*) m_malloc_with_finaliser( sizeof( this_type ) );
      o->base.type = (const mp_obj_type_t*) & type;
      o->cookie = defCookie;
#if UPYWRAP_FULLTYPECHECK
      o->typeId = &typeid( T );
#endif
#if UPYWRAP_SHAREDPTROBJ
      new( &o->obj ) native_obj_t( std::move( p ) );
#else
      o->obj = p;
#endif
      return o;
    }

    static ClassWrapper< T >* AsNativeObjCheckedImpl( mp_obj_t arg )
    {
      auto native = (this_type*) MP_OBJ_TO_PTR( arg );
      if( !mp_obj_is_exact_type( arg, (const mp_obj_type_t*) &type ) )
      {
        //If whatever gets passed in doesn't remotely look like an object bail out.
        //Otherwise it's possible we're being passed an arbitrary 'opaque' ClassWrapper (so the cookie mathches)
        //which has not been registered or has been registered elsewhere (e.g. another dll, which makes
        //mp_obj_is_exact_type fail since that just compares pointers)
        //but if it's the same C++ type (or that check is disabled) we're good to go after all.
        //With UPYWRAP_FULLTYPECHECK off, another possibility which makes sense is this gets called from
        //ClassWrapper< B > and arg is actually a ClassWrapper< A >, but B derives from A or vice-versa:
        //in that case, as long as the memory layout of A and B is similar, i.e. for
        //auto b = new B();
        //auto a = static_cast< A* >( b );
        //auto c = static_cast< B* >( a );
        //all 3 pointers are the same, this actually also works as tested with both gcc and msvc.
        //Still, it's not exactly the safest option: AFAICT it's still UB but just happens to work, plus
        //in multiple inheritance cases where B derives from C and A - in that order - it will segfault
        //in no time because in that case the 3 pointers shown above will not be the same.
        //Eventually we might fix this which requires doing it at compile-time because the C++ type
        //system doesn't have anything at runtime to check if one type_info has a relationship to another.
        //An option would be to have an std::map< type_info, custom_dynamic_cast_fun > which gets populated
        //in the constructor by passing the type_info of the class(es) from which T derives - or in
        //case of an opaque wrapper, which effectively is the same as T - and a function for casting,
        //preferrably using dynamic_cast or dynamic_pointer_cast to double-check errors.
        if( !mp_obj_is_obj( arg ) || native->cookie != defCookie
#if UPYWRAP_FULLTYPECHECK
            || typeid( T ) != *native->typeId
#endif
            )
        {
          return nullptr;
        }
      }
      return native;
    }

    static ClassWrapper< T >* AsNativeObjChecked( mp_obj_t arg )
    {
      if( auto native = AsNativeObjCheckedImpl( arg ) )
      {
        return native;
      }
      //Could be a Python class inheriting from us.
      //Can't use something like mp_obj_cast_to_native_base because that will try to compare
      //pointers to type and arg's type but those don't match if this_type is for a C++ class
      //which is a parent of arg's class i.e. this is ClassWrapper<A> and arg is ClassWrapper<B>
      //where B derives from A. Which also means this will never work if UPYWRAP_FULLTYPECHECK is
      //enabled, and if it's not you have to take care to only use this for types which actually
      //derive from each other else it's UB.
      if( mp_obj_is_obj( arg ) )
      {
        mp_obj_base_t* base = (mp_obj_base_t*) MP_OBJ_TO_PTR( arg );
        if( mp_obj_is_instance_type( base->type ) && MP_OBJ_TYPE_GET_SLOT( base->type, parent ) != nullptr )
        {
          if( auto native = AsNativeObjCheckedImpl( ( (mp_obj_instance_t*) base )->subobj[ 0 ] ) )
          {
            return native;
          }
        }
      }
      CheckTypeIsRegistered(); //since we want to access type.name
      RaiseTypeException( arg, qstr_str( type.name ) );
#if !defined( _MSC_VER ) || defined( _DEBUG )
      return nullptr;
#endif
    }

    static T* AsNativeNonNullPtr( mp_obj_t arg )
    {
      return AsNativeObjChecked( arg )->GetPtr();
    }

    static T* AsNativePtr( mp_obj_t arg )
    {
      return arg == mp_const_none ? nullptr : AsNativeNonNullPtr( arg );
    }

    static native_obj_t AsNativeObj( mp_obj_t arg )
    {
      return arg == mp_const_none ? nullptr : AsNativeObjChecked( arg )->obj;
    }

#if UPYWRAP_SHAREDPTROBJ
    static native_obj_t& AsNativeObjRef( mp_obj_t arg ) //in case the native side wants a reference, avoid extra ptr copies
    {
      return AsNativeObjChecked( arg )->obj;
    }
#endif

  private:
    ClassWrapper( const char* name, decltype( mp_obj_type_t::flags ) flags )
    {
      //Initialize the static parts; note this will set the type's name, once, so while it's
      //possible to call the constructor again with a different name that has no effect on the
      //stored type. It is allowed so the other constructors can always delegate to this one,
      //making it possible to store the same type with different names in another dict for instance.
      //Explicitly disable calling this with different flags though since that yields
      //a type which is actually different.
      static bool init = false;
      if( !init )
      {
        OneTimeInit( name );
        type.flags = flags;
        init = true;
      }
      else if( type.flags != flags )
      {
        RaiseTypeException( "ClassWrapper's type flags can only be set once" );
      }
    }

    struct FixedFuncNames
    {
      func_name_def( Init )
      func_name_def( Exit )
    };

#if UPYWRAP_SHAREDPTROBJ
    template< class... Args >
    static std::shared_ptr< T > ConstructorFactoryFunc( Args... args )
    {
      return std::make_shared< T >( std::forward< Args >( args )... );
    }

    T* GetPtr()
    {
      return obj.get();
    }

    static void NoDelete( T* )
    {
    }
#else
    template< class... Args >
    static T* ConstructorFactoryFunc( Args... args )
    {
      return new T( std::forward< Args >( args )... );
    }

    T* GetPtr()
    {
      return obj;
    }
#endif

    //native attribute store interface
    struct NativeSetterCallBase
    {
      virtual void Call( mp_obj_t self_in, mp_obj_t value ) = 0;
    };

    //native attribute load interface
    struct NativeGetterCallBase
    {
      virtual mp_obj_t Call( mp_obj_t self_in ) = 0;
    };

    template< class Map >
    static typename Map::mapped_type FindAttrMaybe( Map& map, qstr attr )
    {
      auto ret = map.find( attr );
      if( ret == map.end() )
      {
        return nullptr;
      }
      return ret->second;
    }

    template< class Map >
    static typename Map::mapped_type FindAttrChecked( Map& map, qstr attr )
    {
      const auto attrValue = FindAttrMaybe( map, attr );
      if( !attrValue )
      {
        RaiseAttributeException( type.name, attr );
      }
      return attrValue;
    }

    static mp_map_elem_t* LookupLocal( qstr attr )
    {
      auto locals_map = &( (mp_obj_dict_t*) MP_OBJ_TYPE_GET_SLOT( &type, locals_dict ) )->map;
      return mp_map_lookup( locals_map, new_qstr( attr ), MP_MAP_LOOKUP );
    }

    static bool store_attr( mp_obj_t self_in, qstr attr, mp_obj_t value )
    {
      this_type* self = (this_type*) self_in;
      FindAttrChecked( self->setters, attr )->Call( self, value );
      return true;
    }

    static void load_attr( mp_obj_t self_in, qstr attr, mp_obj_t* dest )
    {
      //uPy calls load_attr to find methods as well, so we have no choice but to go through them.
      //However if we find one, it's more performant than uPy's lookup (see mp_load_method_maybe)
      //because we know we have a proper map with only functions so we don't need x checks
      if( auto elem = LookupLocal( attr ) )
      {
        dest[ 0 ] = elem->value;
        dest[ 1 ] = self_in;
      }
      else
      {
        this_type* self = (this_type*) self_in;
        const auto attrValue = FindAttrMaybe( self->getters, attr );
        if( attrValue )
        {
          *dest = attrValue->Call( self );
        }
      }
    }

    static void attr( mp_obj_t self_in, qstr attr, mp_obj_t* dest )
    {
      if( dest[ 0 ] == MP_OBJ_NULL )
      {
        load_attr( self_in, attr, dest );
      }
      else
      {
        if( store_attr( self_in, attr, dest[ 1 ] ) )
        {
          dest[ 0 ] = MP_OBJ_NULL;
        }
      }
    }

    static mp_obj_t binary_op( mp_binary_op_t op, mp_obj_t self_in, mp_obj_t other_in )
    {
      //First check if the type defines the op and call it if so.
      if( auto elem = LookupLocal( mp_binary_op_method_name[ op ] ) )
      {
        mp_obj_t args[] = { elem->value, self_in, other_in };
        auto res = mp_call_method_n_kw( 1, 0, args );
        if( res != MP_OBJ_NULL )
        {
          return res;
        }
      }

      //Otherwise just fall back to comparing pointers.
      if( op != MP_BINARY_OP_EQUAL )
      {
        return MP_OBJ_NULL; //not supported
      }
      const auto self = (this_type*) self_in;
      const auto other = (this_type*) other_in;
      return ToPy( self->GetPtr() == other->GetPtr() );
    }

    static void instance_print( const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind )
    {
      auto elem = LookupLocal( ( kind == PRINT_STR ) ? MP_QSTR___str__ : MP_QSTR___repr__ );
      if( !elem && kind == PRINT_STR )
      {
        elem = LookupLocal( MP_QSTR___repr__ );  //fall back to __repr__ if __str__ not found
      }
      if( elem )
      {
        mp_obj_print_helper( print, mp_call_function_1( elem->value, self_in ), PRINT_STR );
        return;
      }
      mp_printf( print, "<%s object at %p>", mp_obj_get_type_str( self_in ), MP_OBJ_TO_PTR( self_in ) );
    }

    static mp_obj_t instance_call( mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args )
    {
      if( auto elem = LookupLocal( MP_QSTR___call__ ) )
      {
        return mp_call_method_self_n_kw( elem->value, self_in, n_args, n_kw, args );
      }
      RaiseTypeException( "object isn't callable" );
#if !defined( _MSC_VER ) || defined( _DEBUG )
      return mp_const_none;
#endif
    }

    static mp_obj_t del( mp_obj_t self_in )
    {
      auto self = (this_type*) self_in;
#if UPYWRAP_SHAREDPTROBJ
      self->obj.~shared_ptr();
#else
      delete self->obj;
#endif
      return ToPyObj< void >::Convert();
    }

    void OneTimeInit( const char* name )
    {
      type.base.type = &mp_type_type;
      type.name = static_cast< decltype( type.name ) >( qstr_from_str( name ) );
      //The ones we use here (so make sure the other locations stay in sync!).
      MP_OBJ_TYPE_SET_SLOT( &type, make_new, nullptr, 0 );
      MP_OBJ_TYPE_SET_SLOT( &type, locals_dict, mp_obj_new_dict( 0 ), 1 );
      MP_OBJ_TYPE_SET_SLOT( &type, attr, attr, 2 );
      MP_OBJ_TYPE_SET_SLOT( &type, binary_op, binary_op, 3 );
      MP_OBJ_TYPE_SET_SLOT( &type, call, nullptr, 5 );
      MP_OBJ_TYPE_SET_SLOT( &type, print, instance_print, 6 );
      //The ones we don't use, for completeness.
      type.slot_index_unary_op = 0;
      type.slot_index_subscr = 0;
      type.slot_index_iter = 0;
      type.slot_index_buffer = 0;
      type.slot_index_protocol = 0;
      type.slot_index_parent = 0;

      AddFunctionToTable( MP_QSTR___del__, MakeFunction( del ) );
      auto caster = mp_obj_malloc( mp_rom_obj_static_class_method_t, &mp_type_staticmethod );
      caster->fun = MakeFunction( Cast );
      StoreClassVariable( "Cast", MP_OBJ_FROM_PTR( caster ) );
    }

    static void CheckTypeIsRegistered()
    {
      if( type.base.type == nullptr )
      {
#if UPYWRAP_HAS_TYPEID
        std::string errorMessage( std::string( "Native type " ) + typeid( T ).name() + " has not been registered" );
#else
        std::string errorMessage( "Native type has not been registered" );
#endif
        RaiseTypeException( errorMessage.c_str() );
      }
    }

    void AddFunctionToTable( const qstr name, mp_obj_t fun )
    {
      mp_obj_dict_store( MP_OBJ_TYPE_GET_SLOT( &type, locals_dict ), new_qstr( name ), fun );
    }

    void AddFunctionToTable( const char* name, mp_obj_t fun )
    {
      AddFunctionToTable( qstr_from_str( name ), fun );
    }

    template< index_type name, class Ret, class Fun, class... A >
    void DefImpl( Fun f, Arguments&& arguments, typename SelectRetvalConverter< Ret >::type conv )
    {
      typedef NativeMemberCall< name, Ret, A... > call_type;
      auto callerObject = call_type::CreateCaller( f );
      callerObject->convert_retval = conv;
      callerObject->arguments = std::move( arguments );
      functionPointers[ (void*) name ] = callerObject;
      AddFunctionToTable( name(), call_type::CreateUPyFunction( *callerObject ) );
      if( std::string( name() ) == "__call__" )
      {
        MP_OBJ_TYPE_SET_SLOT( &type, call, instance_call, 5 );
      }
    }

    template< index_type name, class Ret, class Fun, class... A >
    void DefImpl( Fun f, typename SelectRetvalConverter< Ret >::type conv )
    {
      DefImpl< name, Ret, Fun, A... >( f, Arguments(), conv );
    }

    template< class Fun, class A >
    void SetterImpl( const char* name, Fun f )
    {
      setters[ qstr_from_str( name ) ] = new NativeSetterCall< A >( f );
    }

    template< class Fun, class A >
    void GetterImpl( const char* name, Fun f )
    {
      getters[ qstr_from_str( name ) ] = new NativeGetterCall< A >( f );
    }

    template< index_type name, class Fun, class Ret, class... A >
    void InitImpl( Fun f, Arguments&& arguments )
    {
      typedef NativeMemberCall< name, Ret, A... > call_type;
      auto caller = call_type::CreateCaller( f );
      caller->arguments = std::move( arguments );
      functionPointers[ (void*) name ] = caller;
      MP_OBJ_TYPE_SET_SLOT( &type, make_new, call_type::MakeNew, 0 );
    }

    template< index_type name, class Fun, class Ret, class... A >
    void InitImpl( Fun f )
    {
      InitImpl< name, Fun, Ret, A... >( f, Arguments() );
    }

    template< index_type name, class Fun >
    void ExitImpl( Fun f )
    {
      typedef NativeMemberCall< name, void > call_type;
      functionPointers[ (void*) name ] = call_type::CreateCaller( f );
      AddFunctionToTable( MP_QSTR___enter__, (mp_obj_t) &mp_identity_obj );
      AddFunctionToTable( MP_QSTR___exit__, MakeFunction( 4, call_type::CallDiscard ) );
    }

    //wrap native setter in function with uPy store_attr compatible signature
    template< class A >
    struct NativeSetterCall : NativeSetterCallBase
    {
      typedef InstanceFunctionCall< T, void, A > call_type;

      NativeSetterCall( typename call_type::func_type f ) :
        f( new NonMemberFunctionCall< T, void, A >( f ) )
      {
      }

      NativeSetterCall( typename call_type::mem_func_type f ) :
        f( new MemberFunctionCall< T, void, A >( f ) )
      {
      }

      void Call( mp_obj_t self_in, mp_obj_t value )
      {
        auto self = (this_type*) self_in;
        CallReturn< void, A >::Call( f, self->GetPtr(), value );
      }

    private:
      call_type* f;
    };

    //wrap native getter in function with uPy load_attr compatible signature
    template< class A >
    struct NativeGetterCall : NativeGetterCallBase
    {
      typedef InstanceFunctionCall< T, A > call_type;

      NativeGetterCall( typename call_type::func_type f ) :
        f( new NonMemberFunctionCall< T, A >( f ) )
      {
      }

      NativeGetterCall( typename call_type::const_mem_func_type f ) :
        f( new ConstMemberFunctionCall< T, A >( f ) )
      {
      }

      mp_obj_t Call( mp_obj_t self_in )
      {
        auto self = (this_type*) self_in;
        return CallReturn< A >::Call( f, self->GetPtr() );
      }

    private:
      call_type* f;
    };

    //wrap native call in function with uPy compatible mp_obj_t( mp_obj_t self, mp_obj_t.... ) signature
    template< index_type index, class Ret, class... A >
    struct NativeMemberCall
    {
      typedef InstanceFunctionCall< T, Ret, A... > call_type;
      typedef FunctionCall< Ret, A... > init_call_type;
      typedef typename call_type::func_type func_type;
      typedef typename call_type::byref_func_type byref_func_type;
      typedef typename call_type::byconstref_func_type byconstref_func_type;
      typedef typename call_type::mem_func_type mem_func_type;
      typedef typename call_type::const_mem_func_type const_mem_func_type;
      typedef typename init_call_type::func_type init_func_type;

      static call_type* CreateCaller( func_type f )
      {
        return new NonMemberFunctionCall< T, Ret, A... >( f );
      }

      static call_type* CreateCaller( byref_func_type f )
      {
        return new NonMemberByRefFunctionCall< T, Ret, A... >( f );
      }

      static call_type* CreateCaller( byconstref_func_type f )
      {
        return new NonMemberByConstRefFunctionCall< T, Ret, A... >( f );
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

      static mp_obj_t CreateUPyFunction( const call_type& caller )
      {
        if( caller.arguments.HasArguments() )
        {
          if( caller.arguments.NumberOfArguments() != sizeof...( A ) )
          {
            RaiseTypeException( ( std::string( "Wrong number of arguments in definition of " ) + index() ).data() );
          }
          return MakeFunction( caller.arguments.MimimumNumberOfArguments(), CallKw );
        }
        return CreateFunction< mp_obj_t, A... >::Create( Call, CallN );
      }

      static mp_obj_t CallDiscard( mp_uint_t n_args, const mp_obj_t* args )
      {
        assert( n_args == 4 );
        static_assert( sizeof...( A ) == 0, "Arguments must be discarded" );
        auto self = (this_type*) args[ 0 ];
        auto f = (call_type*) this_type::functionPointers[ (void*) index ];
        return CallReturn< Ret, A... >::Call( f, self->GetPtr() );
      }

      static mp_obj_t MakeNew( const mp_obj_type_t*, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t* args )
      {
        auto f = (init_call_type*) this_type::functionPointers[ (void*) index ];
        if( f->arguments.HasArguments() )
        {
          if( f->arguments.NumberOfArguments() != sizeof...( A ) )
          {
            RaiseTypeException( ( std::string( "Wrong number of arguments in definition of " ) + index() ).data() );
          }
          Arguments::parsed_obj_t parsedArgs{};
          f->arguments.Parse( n_args, n_kw, args, parsedArgs );
          UPYWRAP_TRY
          return AsPyObj( native_obj_t( Apply( f, parsedArgs.data(), make_index_sequence< sizeof...( A ) >() ) ) );
          UPYWRAP_CATCH
        }
        else if( n_args != sizeof...( A ) || n_kw )
        {
          RaiseTypeException( ( std::string( "Wrong number of arguments in definition of " ) + index() ).data() );
        }
        UPYWRAP_TRY
        return AsPyObj( native_obj_t( Apply( f, args, make_index_sequence< sizeof...( A ) >() ) ) );
        UPYWRAP_CATCH
      }

    private:
      static mp_obj_t Call( mp_obj_t self_in, typename project2nd< A, mp_obj_t >::type... args )
      {
        auto self = (this_type*) self_in;
        auto f = (call_type*) this_type::functionPointers[ (void*) index ];
        return CallReturn< Ret, A... >::Call( f, self->GetPtr(), args... );
      }

      static mp_obj_t CallN( mp_uint_t n_args, const mp_obj_t* args )
      {
        if( n_args != sizeof...( A ) + 1 )
        {
          RaiseTypeException( "Wrong number of arguments" );
        }
        auto self = (this_type*) args[ 0 ];
        auto firstArg = &args[ 1 ];
        auto f = (call_type*) this_type::functionPointers[ (void*) index ];
        return CallVar( f, self->GetPtr(), firstArg, make_index_sequence< sizeof...( A ) >() );
      }

      static mp_obj_t CallKw( size_t n_args, const mp_obj_t* pos_args, mp_map_t* kw_args )
      {
        //Self is required.
        if( n_args < 1 )
        {
          RaiseTypeException( "Wrong number of arguments" );
        }
        auto f = (call_type*) this_type::functionPointers[ (void*) index ];
        Arguments::parsed_obj_t parsedArgs{};
        f->arguments.Parse( n_args - 1, pos_args + 1, kw_args, parsedArgs );
        auto self = (this_type*) pos_args[ 0 ];
        return CallVar( f, self->GetPtr(), parsedArgs.data(), make_index_sequence< sizeof...( A ) >() );
      }

      template< size_t... Indices >
      static Ret Apply( init_call_type* f, const mp_obj_t* args, index_sequence< Indices... > )
      {
        (void) args;
        return f->Call( FromPy< A >( args[ Indices ] )... );
      }

      template< size_t... Indices >
      static mp_obj_t CallVar( call_type* f, T* self, const mp_obj_t* args, index_sequence< Indices... > )
      {
        (void) args;
        return CallReturn< Ret, A... >::Call( f, self, args[ Indices ]... );
      }
    };

    typedef ClassWrapper< T > this_type;
    using store_attr_map = std::map< qstr, NativeSetterCallBase* >;
    using load_attr_map = std::map< qstr, NativeGetterCallBase* >;

    mp_obj_base_t base; //must always be the first member!
    std::int64_t cookie; //we'll use this to check if a pointer really points to a ClassWrapper
    const std::type_info* typeId; //and this will be used to check if types aren't being mixed
    native_obj_t obj;
    static mp_obj_full_type_t type;
    static function_ptrs functionPointers;
    static store_attr_map setters;
    static load_attr_map getters;
    static const std::int64_t defCookie;
  };

  template< class T >
  mp_obj_full_type_t ClassWrapper< T >::type =
#ifdef __GNUC__
    { { nullptr } }; //GCC bug 53119
#else
    { nullptr };
#endif

  template< class T >
  function_ptrs ClassWrapper< T >::functionPointers;

  template< class T >
  typename ClassWrapper< T >::store_attr_map ClassWrapper< T >::setters;

  template< class T >
  typename ClassWrapper< T >::load_attr_map ClassWrapper< T >::getters;

  template< class T >
  const std::int64_t ClassWrapper< T >::defCookie = 0x12345678908765;


  /**
    * Declare a bunch of common special method names.
    */
  struct special_methods
  {
    func_name_def( __str__ )
    func_name_def( __repr__ )
    func_name_def( __bytes__ )
    func_name_def( __format__ )
    func_name_def( __iter__ )
    func_name_def( __next__ )
    func_name_def( __reversed__ )
    func_name_def( __call__ )
  };


  template< class T >
  bool IsClassWrapperOfType( const mp_obj_type_t& type )
  {
    return &ClassWrapper< T >::Type() == &type;
  }

  //Get instance pointer (or nullptr) out of mp_obj_t.
  template< class T >
  struct ClassFromPyObj< T* >
  {
    static T* Convert( mp_obj_t arg )
    {
      return ClassWrapper< T >::AsNativePtr( arg );
    }
  };

#if UPYWRAP_SHAREDPTROBJ
  template< class T >
  struct ClassFromPyObj< std::shared_ptr< T > >
  {
    static std::shared_ptr< T > Convert( mp_obj_t arg )
    {
      return ClassWrapper< T >::AsNativeObj( arg );
    }
  };

  template< class T >
  struct ClassFromPyObj< std::shared_ptr< T >& >
  {
    static std::shared_ptr< T >& Convert( mp_obj_t arg )
    {
      return ClassWrapper< T >::AsNativeObjRef( arg );
    }
  };
#endif

  template< class T >
  struct ClassFromPyObj< T& >
  {
    static T& Convert( mp_obj_t arg )
    {
      //make sure ClassFromPyObj< std::shared_ptr< T >& > gets used instead
      static_assert( !is_shared_ptr< T >::value, "cannot convert object to shared_ptr&" );
      return *ClassWrapper< T >::AsNativeNonNullPtr( arg );
    }
  };

  template< class T >
  struct ClassFromPyObj
  {
    static T Convert( mp_obj_t arg )
    {
      return *ClassWrapper< T >::AsNativeNonNullPtr( arg );
    }
  };

  template< class T >
  struct False : std::integral_constant< bool, false >
  {
  };

  //Wrap instance in a new mp_obj_t
  template< class T >
  struct ClassToPyObj
  {
    static mp_obj_t Convert( T )
    {
      static_assert( False< T >::value, "Conversion from value to ClassWrapper is not allowed, pass a reference or shared_ptr instead" );
      return mp_const_none;
    }
  };

  template< class T >
  struct ClassToPyObj< T* >
  {
    static mp_obj_t Convert( T* p )
    {
      //Could return ClassWrapper< T >::AsPyObj( p, false ) here, but if p was allocated
      //it wouldn't ever get deallocated so disallow this to avoid memory leaks
      static_assert( False< T >::value, "Storing bare pointers in ClassWrapper is not allowed, return a reference or shared_ptr instead" );
      return mp_const_none;
    }
  };

#if UPYWRAP_SHAREDPTROBJ
  template< class T >
  struct ClassToPyObj< std::shared_ptr< T > >
  {
    static mp_obj_t Convert( std::shared_ptr< T > p )
    {
      //Allow empty pointers, but don't convert them into ClassWrapper instances,
      //since trying to call functions on it would result in access violation anyway.
      //Return None instead which is either useful (e.g. used as 'optional' return value),
      //or will lead to an actual Python exception when used anyway.
      if( !p )
      {
        return mp_const_none;
      }
      return ClassWrapper< T >::AsPyObj( std::move( p ) );
    }
  };
#endif

  template< class T >
  struct ClassToPyObj< const T& >
  {
    static mp_obj_t Convert( const T& p )
    {
      static_assert( False< T >::value, "Conversion from const reference to ClassWrapper is not allowed since the const-ness cannot be guaranteed" );
      return mp_const_none;
    }
  };

  template< class T >
  struct ClassToPyObj< T& >
  {
    static mp_obj_t Convert( T& p )
    {
      //Make sure ClassToPyObj< std::shared_ptr< T > > gets used instead.
      static_assert( !is_shared_ptr< T >::value, "cannot convert object to shared_ptr&" );
      return ClassWrapper< T >::AsPyObj( &p, false );
    }
  };

  //Convert std::function into a callable.
  //Just forwards to ClassWrapper since that already contains the complete mechanism
  //for argument conversion and doing the actual call.
  //The type name is the std::type_info::name() value which might be affected by C++ name mangling.
  template< class R, class... Args >
  struct ClassToPyObj< std::function< R( Args... ) > >
  {
    using funct_t = std::function< R( Args... ) >;
    using wrapper_t = ClassWrapper< funct_t >;

    static mp_obj_t Convert( funct_t p )
    {
      if( !p )
      {
        return mp_const_none;
      }
      InitWrapper();
      return wrapper_t::AsPyObj( std::make_shared< funct_t >( std::move( p ) ) );
    }

    static void InitWrapper()
    {
      //Note: registered once, stays forever. Alternative would be to have a finalizer
      //but then when a new function is needed it again has to go through initialization.
      static wrapper_t reg( typeid( funct_t ).name(), wrapper_t::ConstructorOptions::RegisterInStaticPyObjectStore );
      static bool init = false;
      if( !init )
      {
        reg.template Def< special_methods::__call__ >( &funct_t::operator () );
        init = true;
      }
    }
  };
}

//In order for native instances to be returned to uPy, they must have been registered.
//However sometimes you just want to return a native instance to another module without
//defining any class methods for use in uPy, then use this macro to quickly register the class.
//Note the other way around (passing uPy object as native instance into another module, where
//the native class is not registered) is not a problem, since in order to get a uPy object in
//the first place it obviously must have been registered already somewhere
#define UPYWRAP_REGISTER_OPAQUE( className, module ) \
  { \
    upywrap::ClassWrapper< className > registerInstance( MP_STRINGIFY( className ), module ); \
  }

#endif //#ifndef MICROPYTHON_WRAP_CLASSWRAPPER
