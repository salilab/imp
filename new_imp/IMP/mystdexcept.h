/* Simple wrapper for the <stdexcept> header, to work around a g++4 bug
 * (stdexcept does not declare its symbol visibility, preventing std exceptions
 * from being thrown outside of the shared library when compiling with
 * -fvisibility=hidden). Include this file rather than <stdexcept> in any
 * C++ code. Do not include either header in your own C++ headers. */

#if defined(GCC_VISIBILITY) && !defined(_MSC_VER)
#pragma GCC visibility push(default)
#endif

#include <stdexcept>

#if defined(GCC_VISIBILITY) && !defined(_MSC_VER)
#pragma GCC visibility pop
#endif
