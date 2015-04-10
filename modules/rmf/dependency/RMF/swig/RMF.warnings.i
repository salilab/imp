// turn off the warning as it mostly triggers on methods (and lots of them)
%warnfilter(321);

%{
#include <RMF/compiler_macros.h>

RMF_PUSH_WARNINGS
RMF_GCC_PRAGMA(diagnostic ignored "-Wunused-but-set-variable")
RMF_GCC_PRAGMA(diagnostic ignored "-Wunused-value")
RMF_GCC_PRAGMA(diagnostic ignored "-Wmissing-declarations")
RMF_GCC_PRAGMA(diagnostic ignored "-Wunused-but-set-variable")
RMF_CLANG_PRAGMA(diagnostic ignored "-Wunused-parameter")
RMF_CLANG_PRAGMA(diagnostic ignored "-Wused-but-marked-unused")
RMF_CLANG_PRAGMA(diagnostic ignored "-Wunused-function")

/* SWIG generates long class names with wrappers that use certain Boost classes,
   longer than the 255 character name length for MSVC. This shouldn't affect
   the code, but does result in a lot of warning output, so disable this warning
   for clarity. */
RMF_VC_PRAGMA( warning( disable: 4503 ) )
%}
