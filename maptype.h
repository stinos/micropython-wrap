#ifndef MICROPYTHONWRAP_MAPTYPE
#define MICROPYTHONWRAP_MAPTYPE

#include "classwrapper.h"

namespace upywrap
{
    /**
      * Looks up type of python object and calls template fun with converted value.
      * Only some common types are supported: int/string/double/bool and non-empty lists of those.
      */
  template< class Fun >
  void MapType( mp_obj_t obj, Fun f )
  {
    if( MP_OBJ_IS_INT( obj ) )
    {
      f( FromPyObj< int >::Convert( obj ) );
    }
    else if( MP_OBJ_IS_STR( obj ) )
    {
      f( FromPyObj< std::string >::Convert( obj ) );
    }
    else if( MP_OBJ_IS_TYPE( obj, &mp_type_bool ) )
    {
      f( FromPyObj< bool >::Convert( obj ) );
    }
    else if( MP_OBJ_IS_TYPE( obj, &mp_type_float ) )
    {
      f( FromPyObj< mp_float_t >::Convert( obj ) );
    }
    else if( MP_OBJ_IS_TYPE( obj, &mp_type_list ) )
    {
      size_t len;
      mp_obj_t* items;
      mp_obj_get_array( obj, &len, &items );
      if( !len )
      {
        RaiseTypeException( "MapType cannot deduce type from empty list" );
      }
      else
      {
        auto firstItem = *items;
        if( MP_OBJ_IS_INT( firstItem ) )
        {
          f( FromPyObj< std::vector< int > >::Convert( obj ) );
        }
        else if( MP_OBJ_IS_STR( firstItem ) )
        {
          f( FromPyObj< std::vector< std::string > >::Convert( obj ) );
        }
        else if( MP_OBJ_IS_TYPE( firstItem, &mp_type_bool ) )
        {
          f( FromPyObj< std::vector< bool > >::Convert( obj ) );
        }
        else if( MP_OBJ_IS_TYPE( firstItem, &mp_type_float ) )
        {
          f( FromPyObj< std::vector< mp_float_t > >::Convert( obj ) );
        }
        else
        {
          RaiseTypeException( "MapType does not support this type" );
        }
      }
    }
    else
    {
      RaiseTypeException( "MapType does not support this type" );
    }
  }
}

#endif //#ifndef MICROPYTHONWRAP_MAPTYPE
