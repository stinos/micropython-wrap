[![Build status](https://ci.appveyor.com/api/projects/status/3a7gmffr0mpfv9va?svg=true)](https://ci.appveyor.com/project/stinos/micropython-wrap)

MicroPython-Wrap
================

This header-only C++ library provides some interoperability between C/C++ and the [MicroPython](https://github.com/micropython/micropython) programming language.

The standard way of extending MicroPython with your own C or C++ modules involves a lot of boilerplate,
both for converting function arguments and return values between native types and the MicroPython object model and for
regsitering the function and type names so they can be discovered by MicroPython.
Using MicroPython-Wrap most of that boilerplate is avoided and instead one can focus on writing the actual C and/or C++ code
while the process of integration with MicroPython comes down to adding two lines of code for every function/method or type which needs to be
available in your scripts.

WARNING: while fully tested and daily in use without problems, this project should still be considered to be in beta stage and is subject to changes of the
code base, including project-wide name changes and API changes.
Furthermore the actual integration at the build level is not too straightforward, see below.

Platforms
---------
In principle any MicroPython port which has a working C++ compiler should work since the code is just standard C++.
Has been tested for the unix port with gcc, the esp32 port with ESP-IDF sdk v4, and the windows port with gcc (from MSYS2) and msvc.

Example
-------
Complete usage examples covering all aspects can be found in the the [tests](tests) directory which also serves as documentation:
 in [module.cpp](tests/module.cpp) a micropython module is created and a bunch of C++ classes and functions are added to the module.
Consequently when running the [python test code](tests/py) using the standard MicroPython test runner the module is imported and all registered functions are called.

Just to get an idea here is a short sample of C++ code registration; code achieving the same using just the MicroPython API is not shown here but would likely be around 50 lines:

```c++
#include <micropython-wrap/functionwrapper.h>

//function we want to call from within a MicroPython script
std::vector< std::string > SomeFunction( std::vector< std::string > vec )
{
  for( auto& v : vec )
    v += "TRANSFORM";
  return vec;
}

//function names are declared in structs
struct FunctionNames
{
  func_name_def( TransformList )
};

extern "C"
{
  void RegisterMyModule(void)
  {
    //register a module named 'foo'
    auto mod = upywrap::CreateModule( "foo" );

    //register our function with the name 'TransformList'
    //conversion of a MicroPython list of strings is done automatically
    upywrap::FunctionWrapper wrapfunc( mod );
    wrapfunc.Def< FunctionNames::TransformList >( SomeFunction );
  }
}

//now call RegisterMyModule() in MicroPython's main() for example
```

And the MicroPython code making use of this looks like:

```python
import foo

print(foo.TransformList(['a', 'b']))  # Prints ['aTRANSFORM', 'bTRANSFORM']
```

Type Conversion
---------------
Conversion between standard native types and `mp_obj_t`, the MicroPython opaque object type
is declared in two template classes aptly named [ToPyObj](detail/topyobj.h) and [FromPyObj](detail/frompyobj.h).

Currently these conversions are supported (depending on C++ standard used):

    uPy double <-> double/float
    uPy int <-> std::int16_t/std::int32_t/std::int64_t/std::uint16_t/std::uint32_t/std::uint64_t with overflow checks
    uPy bool <-> bool
    uPy str <-> std::string/std::string_view
    uPy str <-> const char* (optional)
    uPy tuple <-> std::tuple/std::pair
    uPy list <-> std::vector (each element must be of the same type)
    uPy dict <-> std::map (each key/value must be of the same type)
    uPy callable <-> std::function (None maps to empty std::function)
    uPy None <-> std::optional (i.e. std::nullopt <-> None, otherwise value gets converted)
    uPy None <- empty std::shared_ptr
    uPy None <- std::error_code (if empty, otherwise throws runtime_error)

Function and class wrapping
---------------------------
Wrapping code is provided for:

    uPy functions <-> free functions via upywrap::FunctionWrapper
    uPy class <-> C++ class via upywrap::ClasssWrapper
    uPy __init__ <-> C++ class constructor or factory function of choice
    uPy __del__ <-> C++ class destructor (called only when instance is grabage collected!)
    uPy __exit__ <-> C++ class method with void() signature
    uPy __call__ <-> any C++ class method
    uPy class methods <-> C++ class methods
    uPy class attributes <-> C++ class methods

For builtin types listed under 'type conversion', the native function must take the argument by value, const value or const reference,
and only values can be returned.
ClassWrapper types can be passed by pointer, value, reference or std::shared_ptr and returned as pointer,
reference or std::shared_ptr. See tests for ownership rules.

Furthermore there is optional support for wrapping each native call in a try/catch for std::exception,
and re-raise it as a uPy RuntimeError

Optional and keyword argument support
-------------------------------------
This is supported by naming the arguments and eventually supplying defaults when registering the function in the C++ code, example:

```c++
void Foo( int, std::string, const std::vector< int >& );

struct FunctionNames
{
  func_name_def( Foo )
};

extern "C"
{
  void RegisterMyModule(void)
  {
    upywrap::FunctionWrapper wrapfunc( upywrap::CreateModule( "foo" ) );
    //Make the first argument required and the rest optional.
    wrapfunc.Def< FunctionNames::Foo >( Foo, upywrap::Kwargs( "a" )( "b", "default" )( "c", {0, 1} ) );
  }
}

```

Calling code:

```python
import foo

foo.Foo(0)  # Calls Foo( 0, "default", std::vector< int >{ 0, 1 } ) in C++.
foo.Foo(a=1, c=[2])  # Calls Foo( 1, "default", std::vector< int >{ 2 } ) in C++.
```


Integrating and Building
------------------------
First clone this repository alongside the MicroPython repository, then refer to the way the tests module
is built and create your own modules in the same way. Also see the [Makefile](Makefile) for Unix and
[Project file](micropython-wrap.vcxproj) for Windows, and the [Appveyor config](.appveyor.yml) for how builds are done.

- the [Unix Makefile](Makefile) shows three ways way of integration (also explained in [the module code](module.h)):
    - combination of a static library and 'user C module': the C++ test code is compiled into a static library
      and tests/cmodule.c is built with MicroPython as a 'user C module', linking to the static library.
    - as a 'user C module': same as above but MicroPython builds both .c and .cpp files.
    - with a shared library: using [this MicroPython fork](https://github.com/stinos/micropython/tree/windows-pyd) adds CPython-like
      support for loading dynamic modules.
- the Makefile is also a starting point for other gcc-based ports, e.g.
  ```
  make usercmodule MICROPYTHON_PORT_DIR=../micropython/ports/esp32
  ```
  will build the tests into a user C module. Note this might require changing ports/esp32/partitions.csv to make room for the C++ code like
  ```
  factory,  app,  factory, 0x10000, 0x200000,
  vfs,      data, fat,     0x210000, 0x1F0000,
  ```
- for Windows with Visual Studio (2013 or up) some extra work has already been done in [this MicroPython fork](https://github.com/stinos/micropython/tree/windows-pyd).
  Create an empty C++ dll project, import [extmodule.props](https://github.com/stinos/micropython/blob/windows-pyd/ports/windows/msvc/extmodule.props)
  and all code is built into a dll with a .pyd extension which is discovered automatically by the fork's micropython.exe when using
  an import statement.
