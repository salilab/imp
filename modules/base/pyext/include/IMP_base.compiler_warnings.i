%{
IMP_CLANG_PRAGMA(diagnostic ignored "-Wdeprecated")
IMP_CLANG_PRAGMA(diagnostic ignored "-Wdeprecated-declarations")
IMP_CLANG_PRAGMA(diagnostic ignored "-Wunused-function")

IMP_GCC_PRAGMA(diagnostic ignored "-Wstrict-aliasing")
IMP_GCC_PRAGMA(diagnostic ignored "-Wmissing-prototypes")
IMP_GCC_PRAGMA(diagnostic ignored "-Wmissing-declarations")
IMP_GCC_PRAGMA(diagnostic ignored "-Wunused-function")
IMP_GCC_PRAGMA(diagnostic ignored "-Wdeprecated-declarations")
IMP_GCC_PRAGMA(diagnostic ignored "-Wsign-compare")

/* SWIG generates long class names with wrappers that use certain Boost classes,
   longer than the 255 character name length for MSVC. This shouldn't affect
   the code, but does result in a lot of warning output, so disable this warning
   for clarity. */
IMP_VC_PRAGMA(warning( disable: 4503 ))

%}
