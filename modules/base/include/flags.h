/**
 *  \file IMP/base/flags.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_FLAGS_H
#define IMPBASE_FLAGS_H

#include <IMP/base/base_config.h>
#include <boost/cstdint.hpp>
#include "types.h"
#include <string>

IMPBASE_BEGIN_NAMESPACE
/** \name Flags

    These methods add support for shared command line flags to \imp.
    Programs that use this have access to flags declared in modules
    which allow users to do things like control log level and turn
    on and off profiling to see what is going on. These functions
    are python accessible.

    In C++, you can also use the AddFloatFlag, AddStringFlag,
    AddBoolFlag and AddIntFlag classes to add flags statically.
    @{
*/

/** Return the name of the current executable.*/
IMPBASEEXPORT std::string get_executable_name();

#ifndef SWIG
/** Parse the command line flags and return the
    positional arguments.

    \param[in] argc argc
    \param[in] argv argv
    \param[in] description A message describing what the program does.
    */
IMPBASEEXPORT void
setup_from_argv(int argc,  char ** argv,
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
    */
IMPBASEEXPORT Strings
setup_from_argv(int argc,  char ** argv,
                std::string description,
                std::string positional_description,
                int num_positional);
#endif

/** Parse the command line flags and return the
    positional arguments. For python.

    \param[in] argv sys.argv
    \param[in] description A message describing what the program does.
    */
IMPBASEEXPORT
void setup_from_argv(const Strings& argv,
                     std::string description);

/** Parse the command line flags and return the
    positional arguments. For python.

    \param[in] argv sys.argv
    \param[in] description A message describing what the program does.
    \param[in] positional_description A message describing the positional
    arguments, eg "input.pdb output.pdb"
    \param[in] num_positional A positive integer to require that
    many positional arguments, or a negative integer to require at
    least that many.
    */
IMPBASEEXPORT Strings
setup_from_argv(const Strings& argv,
                std::string description,
                std::string positional_description,
                int num_positional);

#ifndef SWIG
/** Define one of these in C++ to add a new int flag storing
    into the passed variable.*/
struct IMPBASEEXPORT AddStringFlag {
  AddStringFlag(std::string name,
                std::string description,
                std::string *storage);
};
#endif

/** For python use.*/
IMPBASEEXPORT void add_string_flag(std::string name,
                                   std::string default_value,
                                   std::string description);
/** For python use.*/
IMPBASEEXPORT std::string get_string_flag(std::string name);

#ifndef SWIG
/** Define one of these in C++ to add a new boost::int64_t flag storing
    into the passed variable.*/
struct IMPBASEEXPORT AddIntFlag {
  AddIntFlag(std::string name,
             std::string description,
             boost::int64_t *storage);
};
#endif

/** For python use.*/
IMPBASEEXPORT void add_int_flag(std::string name,
                                size_t default_value,
                                std::string description);
/** For python use.*/
IMPBASEEXPORT size_t get_int_flag(std::string name);

#ifndef SWIG
/** Define one of these in C++ to add a new bool flag storing
    into the passed variable.*/
struct IMPBASEEXPORT AddBoolFlag {
  AddBoolFlag(std::string name,
             std::string description,
             bool *storage);
};
#endif

/** For python use. Default is always false.*/
IMPBASEEXPORT void add_bool_flag(std::string name,
                                 std::string description);
/** For python use.*/
IMPBASEEXPORT bool get_bool_flag(std::string name);

#ifndef SWIG
/** Define one of these in C++ to add a new float flag storing
    into the passed variable.*/
struct IMPBASEEXPORT AddFloatFlag {
  AddFloatFlag(std::string name,
             std::string description,
             double *storage);
};
#endif

/** For python use.*/
IMPBASEEXPORT void add_float_flag(std::string name,
                                 double default_value,
                                 std::string description);
/** For python use.*/
IMPBASEEXPORT double get_float_flag(std::string name);
/** @} */

/** Prints out the help message, useful if you have extra error checking
    and the flags don't pass it.*/
IMPBASEEXPORT void write_help(std::ostream &out= std::cerr);


/** Executables can inspect this flag and when it is true, run a shorter,
    simpler version of their code to just make sure things work.
*/
extern IMPBASEEXPORT bool run_quick_test;
// defined in static.cpp

IMPBASE_END_NAMESPACE


#endif  /* IMPBASE_FLAGS_H */
