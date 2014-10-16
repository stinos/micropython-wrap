#ifndef MICROPYTHON_WRAP_DETAIL_INDEX_H
#define MICROPYTHON_WRAP_DETAIL_INDEX_H

#include <map>

namespace upywrap
{
  //Use a function pointer as key for static function pointer maps,
  //this is the most convenient way to make unique instantiations from templates.
  //The function returns the string used as function name.
  //an extern or static const char[] works as well, but:
  //extern is in the global scope which severly limits the names used
  //static class members just involves too much typing
  typedef const char* index_type();

  //Use for creating index_type with same name as function name
  #define func_name_def( n ) static const char* n(){ return #n; }

  //Map type used to store functions
  typedef std::map< void*, void* > function_ptrs;
}

#endif //#ifndef MICROPYTHON_WRAP_DETAIL_INDEX_H
