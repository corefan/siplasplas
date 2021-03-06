# siplasplas [![Build Status](https://travis-ci.org/Manu343726/siplasplas.svg?branch=master)](https://travis-ci.org/Manu343726/siplasplas) [![Build status](https://ci.appveyor.com/api/projects/status/d395bonrvrduwl6a?svg=true)](https://ci.appveyor.com/project/AlvarBer/siplasplas) [![Documentation](https://img.shields.io/badge/Documentation-latest-green.svg)](https://Manu343726.github.io/siplasplas/doc) [![GitHub issues Open](https://img.shields.io/github/issues/Manu343726/siplasplas.svg?maxAge=2592000)]() [![](https://img.shields.io/github/issues-closed-raw/Manu343726/siplasplas.svg?maxAge=2592000)]()

A library for C++ reflection and introspection

## Features

### Reflection metadata

All reflection metadata is processed by DRLParser, a python script
that takes input about a proejct (Compilation options, include dirs, etc)
and scans the project headers, generating C++ header files with
the reflection information of the corresponding input header. All
generated code is C++11 compatible.

### CMake integration

Users should not worry about DRLParser and its input, a set of cmake
scripts is given to simplify reflection in user projects. Just include
`siplasplas.cmake` and invoke `configure_siplasplas_reflection()` with your target:

``` cmake
add_library(MyLibrary myLib.cpp)

target_include_directories(MyLibrary PUBLIC include/)
target_compile_options(MyLibrary PRIVATE -std=c++11 -Wall)

configure_siplasplas_reflection(MyLibrary)
```

This will add a custom pre-build target that automatically runs
DRLParser and generates reflection metadata headers before building your
library.

### Static reflection

SIplasplas provides a template-based API to access to static reflection
information of user defined types:

``` cpp
// particle.hpp

class Particle
{
public:
    struct Position
    {
        float x, y, z;
    };

    struct Color
    {
        float a, r, g, b;
    };

    enum class State
    {
        Alive,
        Dead
    };

    Position position;
    Color color;
    State state;
};
```

siplasplas uses a libclang based script to generate C++ code with
all the metadata. After running this script, include both
the user header and the generated header:

``` cpp
#include <particle.hpp>
#include <reflection/particle.hpp> // Reflection data (generated code)

std::vector<Particle> particles;

std::ostream& operator<<(std::ostream& os, const Particle::Position& position)
{
    using PositionClass = cpp::static_reflection::Class<Particle::Position>;

    os << "{";

    // For each coordinate in the Position class...
    cpp::foreach_type<PositionClass::Fields>([&](auto type)
    {
        using Field = cpp::meta::type_t<decltype(type)>;

        os << Field::spelling() << ": "            // Field name ("x", "y", "z")
           << cpp::invoke(Field::get(), position)  // Field value (Like C++17 invoke with member object ptr)
           << " ";
    });

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const Particle::Color& color)
{
    using ColorClass = cpp::static_reflection::Class<Particle::Color>;

    os << "{";

    // For each channel in the Color class...
    cpp::foreach_type<ColorClass::Fields>([&](auto type)
    {
        using Field = cpp::meta::type_t<decltype(type)>;

        os << Field::spelling() << ": "             // channel name (r, g, b, ...)
           << cpp::invoke(Field::get(), color)*255  // channel value
           << " ";
    });

    return os << "}";
}

std::ostream& operator<<(std::ostream& os, const Particle::State& state)
{
    // Use static reflection to get the name of the enum value:
    return os << cpp::static_reflection::Enum<Particle::State>::toString(state);
}

int main()
{
    for(const auto& particle : particles)
    {
        std::cout << "position: " << particle.position << std::endl;
        std::cout << "color: " << particle.color << std::endl;
        std::cout << "state: " << particle.state << std::endl;
    }
}
```

The static reflection API currently supports:

 - User defined class types: Source information, set of public non-static member objects and functions, member types.
 - User defined enumeration types: Set of enum constants values, enum constants names, to/from string methods
 - User defined functions

### Dynamic reflection

Siplasplas also supports dynamic reflection in the form of a simple entity based component
system:

``` cpp
cpp::dynamic_reflection::Runtime runtime = loadDynamicReflection();

// Get dynamic reflection info of the class ::Particle::Position:
cpp::dynamic_reflection::Class&  positionClass = runtime.class_("Particle").class_("Position");

// Manipulate a particle object using dynamic reflection:
Particle particle;
positionCLass.field_("x").get(particle.position) = 42.0f; // particle.position.x = 42

// You can also create objects dynamically:
auto particle2 = runtime.class_("Particle").create();

// Returned objects are dynamically manipulable too:
particle2["color"]["r"] = 0.5f;
```

The dynamic reflection API can be used to load APIs from external libraries at
runtime in a straightforward way:

``` cpp
int main()
{
    cpp::DynamicLibrary lib{"libmylibrary.so"};
    cpp::dynamic_reflection::Runtimeloader loader{lib};
    cpp::dynamic_reflection::Runtime& runtime = loader.runtime();

    auto myObject = runtime.class_("MyClass").create();

    // Invoke MyClass::function with params 1 and "hello":
    myObject("function")(1, std::string("hello!"));
}
```
### Other features:

Siplasplas offers other features, building blocks for the APIs explained
above, including:

 - **Type erasure**: A module dedicated to type erasure, with classes designed to
   manipulate type erased functions, member object pointers, and objects.

 - **Signals**: Siplasplas implements a simple message passing system for inter-thread
   communication.

 - **CMake API**: With the ultimate goal of providing the basis for a work in progress
   runtime C++ compilation module, siplasplas implements a C++ API to configure and build existing CMake
   projects.

 - **Utilities**: Dynamic library loading, aligned malloc, assertions, function type introspection,
   metaprogramming, hashing, etc. Lots of stuff!

## Supported compilers

siplasplas has been tested in GCC 5.1/5.2/6.1, Clang 3.6/3.7/3.8, and Visual Studio 2015.

## Documentation

Documentation is available [here](https://manu343726.github.io/siplasplas/doc)

The documentation is available in Doxygen and Standardese format, each one
with multiple versions corresponding to the latest documentation of each
siplasplas release and active branch.

## Installation

> **NOTE**: siplasplas is a work in progress project
subject to changes. We don't currently provide any kind of
API or ABI stability guarantee, nor a production-ready installation
process. The following instructions are to build siplasplas from sources.

### TL;DR

You can build siplasplas from sources:

``` bash
$ git clone https://github.com/Manu343726/siplasplas --recursive
$ cd siplasplas
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

Or download the [`bootstrap cmake script`](https://github.com/Manu343726/siplasplas/blob/master/bootstrap.cmake)
and point it to one of the siplasplas releases:

``` cmake
set(SIPLASPLAS_PACKAGE_URL <url to siplasplas release package>)
set(SIPLASPLAS_INSTALL_DRLPARSER_DEPENDENCIES ON) # Download DRLparser deps with pip
set(SIPLASPLAS_LIBCLANG_VERSION 3.8.0) # libclang version
set(SIPLASPLAS_DOWNLOAD_LIBCLANG ON) # Download and configure libclang automatically

include(bootstrap.cmake)
```

This will download and configure a siplasplas installation in your buildtree. After including
`bootstrap.cmake`, a `FindSiplasplas.cmake` module is available in your module path to link against
the different siplasplas modules:

``` cmake
find_package(Siplasplas)

target_link_libraries(MyLibrary PUBLIC siplasplas-reflection-dynamic)
```

The module defines one imported library for each siplasplas module. All inter-module dependencies are already
solved.

### Prerequisites

 - **Python 2.7**: The siplasplas relfection engine uses a libclang-based
   parser witten in python. Python 2.7 and pip for Python 2.7 are
   neccesary. All dependencies are handled automatically
   (See*configuration* bellow).

 - **Mercurial**: The [Entropia Filesystem Watcher](https://bitbucket.org/SpartanJ/efsw) dependency
   is hosted on bitbucket using Mercurial for source control. Mercurial is needed to download the
   dependency.

 - **Doxygen**: Needed only if documentation build is enabled. See *configuration* bellow.

 - **Libclang**: Siplasplas will use the libclang library distributed as
   part of the system clang installation by default, but it can be
   configured to download and build libclang automatically. See
   *configuration*.

### Dependencies

All siplasplas dependencies are managed automatically through CMake, users
don't have to worry about installing deps. Anyway, here is the list of the
thrid party dependencies of siplasplas:

 - [backward-cpp](https://github.com/bombela/backward-cpp) for exception
   stack traces
 - [chaiscript](http://chaiscript.com/) (For examples only)
 - [cmake](https://github.com/Manu343726/cmake) tools for the cmake
   scripts
 - [ctti](https://github.com/Manu343726/ctti) for type indexing and
   debugging
 - [efsw](https://bitbucket.org/SpartanJ/efsw) for runtime C++ compilation
 - [fmt](http://fmtlib.net/latest/index.html) for diagnostic messages
 - [googletest](https://github.com/google/googletest) (For tests only)
 - [imgui](https://github.com/ocornut/imgui) (For examples only)
 - [SFML](http://www.sfml-dev.org/) (For examples only)
 - [JSON For Modern C++](https://github.com/nlohmann/json) for cmake
   target properties and serialization
 - [libexecstream](http://libexecstream.sourceforge.net/) for cmake
   invocation
 - [readerwriterqueue](https://github.com/cameron314/readerwriterqueue)
   for inter-thread message passing
 - [spdlog](https://github.com/gabime/spdlog) for logging
 - [standardese](https://github.com/foonathan/standardese) (For documentation only)

siplasplas also depends on some python modules:

 - [clang](https://pypi.python.org/pypi/clang) for C++ parsing
 - [colorama](https://pypi.python.org/pypi/colorama) for parser logging
 - [asciitree](https://pypi.python.org/pypi/asciitree/0.3.2) for AST
   debugging
 - [jinja2](http://jinja.pocoo.org/) for code generation

### Download and configure the project

Clone the [siplasplas repository]({{site.project.url}})

``` bash
$ git clone https://github.com/Manu343726/siplasplas --recursive
```

Create a `build/` directory inside the local repository

``` bash
$ cd siplasplas
$ mkdir build
```

Run cmake in the build directory

``` bash
$ cd build
$ cmake ..
```

> Make sure you installed all the requirements before running cmake,
> siplasplas configuration may fail if one or more of that requirements is
> missing.

To build the library, invoke the default build target:

``` bash
$ cmake --build . # Or just "make" if using Makefiles generator
```

### Configuration

The default cmake invocation will build siplasplas as dynamic libraries
(one per module) using the default generator. Also, siplasplas
configuration can be modified using some options and variables:

> The syntax to pass variables to cmake during configuration is
> `-D<VARIABLE>=<VALUE>`, for example:
>
> `$ cmake .. -DSIPLASPLAS_VERBOSE_CONFIG=ON`

 - `CMAKE_BUILD_TYPE`: Build type to be used to build the project (Debug,
   Release, etc). Set to `Debug` by default.
 - `SIPLASPLAS_VERBOSE_CONFIG`: Configure siplasplas using detailed
   output. `OFF` by default.
 - `SIPLASPLAS_LIBRARIES_STATIC`: Build static libraries. `FALSE` by
   default.
 - `SIPLASPLAS_BUILD_EXAMPLES`: Build siplasplas examples in addition to
   libraries. `OFF` by default.
 - `SIPLASPLAS_BUILD_TESTS`: Build siplasplas unit tests. `OFF` by default.
 - `SIPLASPLAS_BUILD_DOCS`: Generate targets to build siplasplas
   documentation. `OFF` by default.
 - `SIPLASPLAS_INSTALL_DRLPARSER_DEPENDENCIES`: Install reflection parser
   python dependencies. `ON` by default. This needs pip version 2.7
   installled.  Dependencies can be manually installed too, there's is
   a `requirements.txt` file in `<siplasplas
   sources>/src/reflection/parser/`. The requirements file doesn't cover
   the `clang` dependency, you must install the clang package **with the
   same version of your installed libclang**. For example, given:

   ``` bash
   $ clang --version
   clang version 3.8.0 (tags/RELEASE_380/final)
   ...
   ```

   you must install `clang==3.8.0` package for Python 2.7.

 - `SIPLASPLAS_DOWNLOAD_LIBCLANG`: Download libclang from LLVM repository.
   If enabled, siplasplas will download LLVM+Clang version
   `${SIPLASPLAS_LIBCLANG_VERSION}` from the LLVM repositories. This
   overrides `SIPLASPLAS_LIBCLANG_INCLUDE_DIR`,
   `SIPLASPLAS_LIBCLANG_SYSTEM_INCLUDE_DIR`, and
   `SIPLASPLAS_LIBCLANG_LIBRARY` variables. `OFF` by default.

 - `SIPLASPLAS_LIBCLANG_VERSION`: Version of libclang used by the
   reflection parser. Inferred from the installed clang version by
   default.
   > **NOTE:** siplasplas has been tested with libclang 3.7 and 3.8 only.
   > siplasplas sources use C++14 features, a clang version with C++14
   > support is needed. *Actually, the siplasplas configuration uses
   > `-std=c++14` option, which limits the range of supported versions.*

 - `SIPLASPLAS_LIBCLANG_INCLUDE_DIR`: Path to the LLVM includes. When
   building docs, Standardese tool is built using this configuraton too.
   Inferred by default.
 - `SIPLASPLAS_LIBCLANG_SYSTEM_INCLUDE_DIR`: Path to the installed clang
   includes. When building docs, Standardese tool is built using this
   configuraton too. Inferred by default.
 - `SIPLASPLAS_LIBCLANG_LIBRARY`: Path to the libclang library. When
   building docs, Standardese tool is built using this configuraton too.
   Inferred by default.

## Acknowledgements

Many thanks to:

 - Jonathan "foonathan" Müller, [as always](https://github.com/foonathan/standardese#acknowledgements)
 - George "Concepticide" Makrydakis, for feedback, debates, and "Guys, what the fuck is gitter?"
 - Diego Rodriguez Losada, for feedback, palmeritas, and blue boxes
 - Asier González, for holding on for six months in my C++ course, which eventually became this project
 - To all my [ByThech
   WM&IP](http://www.by.com.es/watch-mochi/watch-mochi-ip-video-intercomunicacion-digital-con-tecnologia-ip/)
   team mates, for having to suffer me saying "this with reflection would
   be so easy!" every single day, and specifically to Javier Martín and
   Antonio Pérez for feedback
 - All my twitter followers, still there even with docens of tweets a day
   about reflection! Seriously, some of the best people of the C++
   community are there and give me a lot of feedback and ideas
 - Jens Weller and the Fortune God, thanks for accepting my Meeting C++
   2016 talk about siplasplas

## License

siplasplas project is released under the MIT open source license. This
license applies to all C++ sources, CMake scripts, and any other file
except explicitly stated.
