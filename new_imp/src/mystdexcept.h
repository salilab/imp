/**
 *  \file mystdexcept.h   \brief Simple wrapper for the stdexcept header.
 *
 *  This header is to work around a g++4 bug (stdexcept does not declare its
 *  symbol visibility, preventing std exceptions from being thrown outside
 *  of the shared library when compiling with -fvisibility=hidden). Include
 *  this file rather than stdexcept in any C++ code.
 *
 *  References:
 *  - http://gcc.gnu.org/bugzilla/show_bug.cgi?id=26217
 *  - http://www.dribin.org/dave/blog/archives/2006/02/10/gcc_exception_bug_2/
 *
 *  Do NOT include stdexcept in any of your own headers, because even one
 *  copy of stdexcept with the 'default' symbol visibility in your program
 *  will force default symbol visibility and break throwing of the exceptions
 *  outside of the shared library.
 *
 *  Do NOT include mystdexcept.h either in your own headers, because it is an
 *  internal header (relies on preprocessor defines set by the build system)
 *  and so is not available as part of the public API.
 *
 *  These two restrictions mean that right now, std exceptions cannot be
 *  thrown from public header files. Either hide these throws in the
 *  corresponding C++ code, or use IMP-specific exceptions.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#if defined(GCC_VISIBILITY) && !defined(_MSC_VER)
#pragma GCC visibility push(default)
#endif

#include <stdexcept>

#if defined(GCC_VISIBILITY) && !defined(_MSC_VER)
#pragma GCC visibility pop
#endif
