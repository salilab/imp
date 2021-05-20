/**
 *  \file IMP/flags.h
 *  \brief Support for shared command line flags.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_FLAGS_H
#define IMPKERNEL_FLAGS_H

#include <IMP/kernel_config.h>
#include <boost/cstdint.hpp>
#include "types.h"
#include <string>

IMPKERNEL_BEGIN_NAMESPACE
/** \name Flags

    These methods add support for shared command
    line flags to \imp.  Programs that use this have access to flags
    declared in modules which allow users to do things like control
    log level and turn on and off profiling to see what is going on.
    These functions are Python accessible.

    In C++, you can also use the AddFloatFlag, AddStringFlag,
    AddBoolFlag and AddIntFlag classes to add flags statically.  @{
*/
//! Return the name of the current executable.
IMPKERNELEXPORT std::string get_executable_name();

#ifndef SWIG
//! Parse the command line flags and return the positional arguments.
/**
    \param[in] argc argc
    \param[in] argv argv
    \param[in] description A message describing what the program does.
    \throws UsageException if a problem with the command line was found.
    */
IMPKERNELEXPORT void setup_from_argv(int argc, char **argv,
                                   std::string description);

/** Parse the command line flags and return the
    positional arguments returning unknown flags in a list. Use this version
    if some arguments are to be parsed by a different system.

    \param[in] argc argc
    \param[in] argv argv
    \param[in] description A message describing what the program does.
    \throws UsageException if a problem with the command line was found.
    */
IMPKERNELEXPORT Strings setup_from_argv_allowing_unknown(int argc, char **argv,
                                                       std::string description);

/** Parse the command line flags and return the
    positional arguments.

    \param[in] argc argc
    \param[in] argv argv
    \param[in] description A message describing what the program does.
    \param[in] positional_description A message describing the the
    positional arguments
    \param[in] num_positional A positive integer to require that
    many positional arguments, or a negative integer to require at
    least that many.
    \throws UsageException if a problem with the command line was found.
    */
IMPKERNELEXPORT Strings
    setup_from_argv(int argc, char **argv, std::string description,
                    std::string positional_description, int num_positional);
#endif

/** Parse the command line flags and return the
    positional arguments. For Python.

    \param[in] argv sys.argv
    \param[in] description A message describing what the program does.
    \throws UsageException if a problem with the command line was found.
    */
IMPKERNELEXPORT void setup_from_argv(const Strings &argv,
                                   std::string description);

/** Parse the command line flags and return the
    positional arguments. For Python.

    \param[in] argv sys.argv
    \param[in] description A message describing what the program does.
    \param[in] positional_description A message describing the positional
    arguments, eg "input.pdb output.pdb"
    \param[in] num_positional A positive integer to require that
    many positional arguments, or a negative integer to require at
    least that many.
    \throws UsageException if a problem with the command line was found.
    */
IMPKERNELEXPORT Strings
    setup_from_argv(const Strings &argv, std::string description,
                    std::string positional_description, int num_positional);

#ifndef SWIG
/** Define one of these in C++ to add a new int flag storing
    into the passed variable.

    \note You should consider using Flag<std::string> instead.
*/
struct IMPKERNELEXPORT AddStringFlag {
  AddStringFlag(std::string name, std::string description,
                std::string *storage);
};
#endif

/** For Python use.*/
IMPKERNELEXPORT void add_string_flag(std::string name, std::string default_value,
                                   std::string description);
/** For Python use.*/
IMPKERNELEXPORT std::string get_string_flag(std::string name);

#ifndef SWIG
/** Define one of these in C++ to add a new boost::int64_t flag storing
    into the passed variable.

    \note You should consider using Flag<boost::int64_t> instead.
*/
struct IMPKERNELEXPORT AddIntFlag {
  AddIntFlag(std::string name, std::string description,
             boost::int64_t *storage);
};
#endif

/** For Python use.*/
IMPKERNELEXPORT void add_int_flag(std::string name, size_t default_value,
                                std::string description);
/** For Python use.*/
IMPKERNELEXPORT size_t get_int_flag(std::string name);

#ifndef SWIG
/** Define one of these in C++ to add a new bool flag storing
    into the passed variable.

    \note You should consider using Flag<bool> instead.
*/
struct IMPKERNELEXPORT AddBoolFlag {
  AddBoolFlag(std::string name, std::string description, bool *storage);
};
#endif

/** For Python use. Default is always false.*/
IMPKERNELEXPORT void add_bool_flag(std::string name, std::string description);
/** For Python use.*/
IMPKERNELEXPORT bool get_bool_flag(std::string name);

#ifndef SWIG
/** Define one of these in C++ to add a new float flag storing
    into the passed variable.

    \note You should consider using Flag<double> instead.
*/
struct IMPKERNELEXPORT AddFloatFlag {
  AddFloatFlag(std::string name, std::string description, double *storage);
};
#endif

/** For Python use.*/
IMPKERNELEXPORT void add_float_flag(std::string name, double default_value,
                                  std::string description);
/** For Python use.*/
IMPKERNELEXPORT double get_float_flag(std::string name);
/** @} */

/** Prints out the help message, useful if you have extra error checking
    and the flags don't pass it.*/
IMPKERNELEXPORT void write_help(std::ostream &out = std::cerr);

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
/* Exposing global variables through swig is kind of broken,
   see issue #723. */
extern IMPKERNELEXPORT AdvancedFlag<bool> run_quick_test;
#endif
// defined in static.cpp

/** Executables can inspect this flag and when it is true, run a shorter,
    simpler version of their code to just make sure things work.
*/
inline bool get_is_quick_test() { return run_quick_test; }

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_FLAGS_H */
