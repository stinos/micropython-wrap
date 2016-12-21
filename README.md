[![Unix Build Status](https://travis-ci.org/stinos/micropython-wrap.svg?branch=master)](https://travis-ci.org/stinos/micropython-wrap)
[![Windows Build status](https://ci.appveyor.com/api/projects/status/3a7gmffr0mpfv9va?svg=true)](https://ci.appveyor.com/project/stinos/micropython-wrap)

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
Has been tested under Unix with gcc and Windows with msvc; Windows with msys should be no problem either.

Example
-------
Complete usage examples covering all aspects can be found in the the [tests](tests) directory which also serves as documentation:
 in [module.cpp](tests/module.cpp) a micropython module is created and a bunch of C++ classes and functions are added to the module.
Consequently when running the [python test code](tests/py) using the standard MicroPython test runner the module is imported and all registered functions are called.

Just to get an idea here is a short sample of C++ code registration; code achieving the same using just the MicroPython API is not shown here but would likely be around 50 lines:

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

And the MicroPython code making use of this looks like:

    import foo

    print(foo.TransformList(['a', 'b']))  # Prints ['aTRANSFORM', 'bTRANSFORM']

Type Conversion
---------------
Conversion between standard native types and `mp_obj_t`, the MicroPython opaque object type
is declared in two template classes aptly named [ToPyObj](detail/topyobj.h) and [FromPyObj](detail/frompyobj.h).

Currently these conversions are supported:

    uPy double <-> double/float
    uPy int <-> std::int32_t/std::int64_t/std::uint32_t/std::uint64_t with overflow checks
    uPy bool <-> bool
    uPy str <-> std::string
    uPy str <-> const char* (optional)
    uPy tuple <-> std::tuple
    uPy list <-> std::vector (each element must be of the same type)
    uPy dict <-> std::map (each key/value must be of the same type)
    uPy callable -> std::function

Function and class wrapping
---------------------------
Wrapping code is provided for:

    uPy functions <-> free functions via upywrap::FunctionWrapper
    uPy class <-> C++ class via upywrap::ClasssWrapper
    uPy __init__ <-> C++ class constructor or factory function of choice
    uPy __del__ <-> C++ class destructor (called only when instance is grabage collected!)
    uPy __exit__ <-> C++ class method with void() signature
    uPy class methods <-> C++ class methods
    uPy class attributes <-> C++ class methods

For builtin types listed under 'type conversion', the native function must take the argument by value, const value or const reference,
and only values can be returned.
ClassWrapper types can be passed by pointer, value, reference or std::shared_ptr and returned as pointer,
reference or std::shared_ptr. See tests for ownership rules.

Furthermore there is optional support for wrapping each native call in a try/catch for std::exception,
and re-raise it as a uPy RuntimeError

Integrating and Building
------------------------
First clone this repository alongside the MicroPython repository, then refer to the way the tests module
is built and create your own modules in the same way: see [Travis config](.travis.yml) and [Makefile](Makefile) for Unix,
and the [Appveyor config](.appveyor.yml) and [Project file](micropython-wrap.vcxproj) for Windows.

- the [Unix Makefile](Makefile) shows one way of integration: all native code (including class/function registration) is
  compiled into a static library. MicroPython's main() function is modified to call the module registration function and then built.
  Alternatively, instead of building a static library, one could modify the MicroPython Makefile to allow C++ compilation and add all C++ source and wrapper code directly to it.
- for Windows with Visual Studio (2013 or up) some extra work has already been done in [this MicroPython fork](https://github.com/stinos/micropython/tree/windows-pyd).
  Create an empty C++ dll project, import [extmodule.props](https://github.com/stinos/micropython/blob/windows-pyd/windows/msvc/extmodule.props)
  and all code is built into a dll with a .pyd extension which is discovered automatically by the fork's micropython.exe when using
  an import statement.
