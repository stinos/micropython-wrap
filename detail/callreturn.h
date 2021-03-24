#ifndef MICROPYTHON_WRAP_DETAIL_CALLRETURN_H
#define MICROPYTHON_WRAP_DETAIL_CALLRETURN_H

#include "frompyobj.h"
#include "topyobj.h"

namespace upywrap
{
  template< class Ret >
  struct SelectRetvalConverter
  {
    typedef mp_obj_t( *type )( Ret );
  };

  template<>
  struct SelectRetvalConverter< void >
  {
    typedef mp_obj_t( *type )();
  };

  //Convert arguments, call native function and return converted return value - handles void properly
  //First arg is always InstanceFunctionCall or FunctionCall, and if it's convert_retval is not nullptr
  //it will be used instead of the default return value conversion
  template< class Ret, class... A >
  struct CallReturn
  {
    template< class Fun >
    static mp_obj_t Call( Fun f, typename project2nd< A, mp_obj_t >::type... args )
    {
      UPYWRAP_TRY
      if( f->convert_retval )
      {
        return f->convert_retval( f->Call( FromPy< A >( args )... ) );
      }
      return ToPy( f->Call( FromPy< A >( args )... ) );
      UPYWRAP_CATCH
    }

    template< class Fun, class Self >
    static mp_obj_t Call( Fun f, Self self, typename project2nd< A, mp_obj_t >::type... args )
    {
      UPYWRAP_TRY
      if( f->convert_retval )
      {
        return f->convert_retval( f->Call( self, FromPy< A >( args )... ) );
      }
      return ToPy( f->Call( self, FromPy< A >( args )... ) );
      UPYWRAP_CATCH
    }
  };

  template< class... A >
  struct CallReturn< void, A... >
  {
    template< class Fun >
    static mp_obj_t Call( Fun f, typename project2nd< A, mp_obj_t >::type... args )
    {
      UPYWRAP_TRY
      f->Call( FromPy< A >( args )... );
      return ToPyObj< void >::Convert();
      UPYWRAP_CATCH
    }

    template< class Fun, class Self >
    static mp_obj_t Call( Fun f, Self self, typename project2nd< A, mp_obj_t >::type... args )
    {
      UPYWRAP_TRY
      f->Call( self, FromPy< A >( args )... );
      return ToPyObj< void >::Convert();
      UPYWRAP_CATCH
    }
  };
}

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_CALLRETURN_H
