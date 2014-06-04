#ifndef MICROPYTHONMODULE_CALLRETURN
#define MICROPYTHONMODULE_CALLRETURN

#include "topyobj.h"
#include "frompyobj.h"

namespace upywrap
{
  //Convert arguments, call native function and return converted return value - handles void properly
  template< class Ret, class... A >
  struct CallReturn
  {
    template< class Fun >
    static mp_obj_t Call( Fun f, typename project2nd< A, mp_obj_t >::type... args )
    {
      UPYWRAP_TRY
      return SelectToPyObj< Ret >::type::Convert( f->Call( SelectFromPyObj< A >::type::Convert( args )... ) );
      UPYWRAP_CATCH
    }

    template< class Fun, class Self >
    static mp_obj_t Call( Fun f, Self self, typename project2nd< A, mp_obj_t >::type... args )
    {
      UPYWRAP_TRY
      return SelectToPyObj< Ret >::type::Convert( f->Call( self, SelectFromPyObj< A >::type::Convert( args )... ) );
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
      f->Call( SelectFromPyObj< A >::type::Convert( args )... );
      return ToPyObj< void >::Convert();
      UPYWRAP_CATCH
    }

    template< class Fun, class Self >
    static mp_obj_t Call( Fun f, Self self, typename project2nd< A, mp_obj_t >::type... args )
    {
      UPYWRAP_TRY
      f->Call( self, SelectFromPyObj< A >::type::Convert( args )... );
      return ToPyObj< void >::Convert();
      UPYWRAP_CATCH
    }
  };
}

#endif
