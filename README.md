# Utilities and Extensions for Standard C++ Library

This is a collection of useful (template) classes and functions developed upon standard C++ library.
It can be compiled using *C++11*, but *C++17* is more preferable.  This README file briefly
describes contents of this library and can be not up-do-date, it also gives guidelines on how to use
this stuff.  For more detailed information see [wiki](https://github.com/gbuzykin/std-ext/wiki)
pages.

The collection includes the following groups of helpers:

- implementation of base utility templates such as `std::enable_if_t<>`, `std::index_sequence<>`,
  and other missing stuff for *С++11*
- *iterator* helper classes and functions
- standard *algorithm* and *functional* extensions
- *C++20*-like `util::span<>` template type
- standard `std::basic_string_view<>` implementation for *C++11*
- some useful utility functions for *string* manipulation, such as *splitting*, *joining*, and
  *searching*
- functions for converting *UTF-8*, *UTF-16*, *UTF-32* to each other
- library for string *parsing*, *formatting* and *printing*
- library for *buffered input/output* (alternative to rather slow and consuming standard streams),
  which is compliant with the printing functions
- special *buffered input/output* derived classes to read and write files inside *zip* archives
  (*libzip* integration)
- dynamic *variant* object implementation `util::variant`, which can hold data of various types
  known at runtime and convert one to another (not a template with predefined set of types); it
  easily integrates with mentioned string parsers and formatters for to and from string conversion
- data structures `db::value` to store hierarchical records and arrays; `json` file reader and writer
- insert, erase and find algorithm implementations for *bidirectional lists* and *red-black trees*
  (in the form of functions to make it possible to implement either universal containers or
  intrusive data structures)
- implementation of `util::vector<>`, `util::list<>`, `util::set<>`, `util::multiset<>`,
  `util::map<>`, and `util::multimap<>`*С++17* specification compliant containers (but which can be
  compiled using *С++11*), build upon functions mentioned above (not so useful stuff, but it has
  academic value)
- intrusive bidirectional list implementation
- standard-compliant pool allocator
- *CRC32* calculator
- *COW* pointer `cow_ptr<>` implementation

The following groups of helpers are planned to be developed and added to the collection soon:

- *ZLib* inflation and deflation support for *buffered input/output* (*zlib* integration)
- `<xml>` file parser

## How to Use

This library is not a standalone library, so it doesn't have its own building script or
`CMakeLists.txt` file.  Instead, the common way to add this collection to the target project is to
use it as *git submodule*, say, in `std-ext/` folder.  Then, e.g.  these lines should be added to
the project's `CMakeLists.txt`:

```cmake
if(WIN32)
  set(platform_dir    std-ext/platform/win)
elseif(UNIX)
  set(platform_dir    std-ext/platform/posix)
endif()
file(GLOB_RECURSE std-ext_includes     std-ext/include/*.h)
file(GLOB_RECURSE std-ext_sources      std-ext/src/*.h;std-ext/src/*.cpp)
file(GLOB_RECURSE platform_includes    ${platform_dir}/include/*.h)
file(GLOB_RECURSE platform_sources     ${platform_dir}/src/*.h;${platform_dir}/src/*.cpp)

...

add_executable(<project-name>
  ...
  ${std-ext_includes}
  ${std-ext_sources}
  ${platform_includes}
  ${platform_sources}
  ...
)

...

target_include_directories(<project-name> PUBLIC ... std-ext/include ${platform_dir}/include ...)


```

Also, `std-ext` needs external file `config.h`, which could be found and included as
`#include "util/config.h"`.  The simplest way is to add this file in created `util` subfolder of
`include` folder of the project.  Here is the minimal contents of this file:

```cpp
#pragma once
#define UTIL_EXPORT
```

If the project is a library compiled for Windows, and it is needed to export symbols from `std-ext`
as well, the recommended contents is the following:

```cpp
#pragma once
#ifdef <library_name>_EXPORTS
#    define UTIL_EXPORT __declspec(dllexport)
#else
#    define UTIL_EXPORT __declspec(dllimport)
#endif
```

This [project](https://github.com/gbuzykin/std-ext-tests) can be referenced as `std-ext` usage
example.  It is a separate project for `std-ext` testing and contains lots of use- and test-cases.
A simple own test-suite is implemented to sort and organize tests; it involves verification tests
with predefined input data, brute-force-like tests with large amount of random-generated data, and
performance tests.  Tests from GCC test-suite are also ported for *containers* testing.
