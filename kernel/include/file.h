/**
 *  \file IMP/file.h
 *  \brief Handling of file input/output
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_FILE_H
#define IMP_FILE_H

#include "kernel_config.h"
#include "Pointer.h"
#include "macros.h"
#include "internal/ifile.h"
#include <fstream>
#include <iostream>

IMP_BEGIN_NAMESPACE

class Object;

#if !defined(IMP_DOXYGEN)
template <class Stream>
struct TextProxy {
  Stream *str_;
  Pointer<Object> ptr_;
  TextProxy(Stream *str, Object *ptr): str_(str), ptr_(ptr){}
};
#endif


/** A TextOutput can be implicitly constructed from a C++ stream, a
    Python filelike object or a path to a file. As a result, those can be
    passed directly to functions which take a TextOutput as an
    argument.

    Files are created lazily, so TextOutput can be passed as
    arguments to functions that might not produce output.
    \code
    IMP::atom::write_pdb(particles, "path/to/file.pdb");
    IMP::atom::write_pdb(particles, my_fstream);
    \endcode
    \see TextInput
*/
class IMPEXPORT TextOutput
{
  Pointer<internal::IOStorage<std::ostream> > out_;
 public:
#ifndef IMP_DOXYGEN
  // SWIG needs these here for some bizarre reason
  TextOutput(int);
  TextOutput(double);
  TextOutput(const char *c);
  TextOutput(TextProxy<std::ostream> p);
#endif
  TextOutput(){}
  TextOutput(std::string file_name);
#ifndef SWIG
  TextOutput(std::ostream &out, std::string name="C++ stream");
#endif

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  operator std::ostream &() {
    return get_stream();
  }
  operator bool () {
    return out_ && out_->get_stream();
  }
  std::ostream &get_stream() {
    if (!out_) {
      IMP_THROW("Attempting to write to uninitialized text input",
                IOException);
    }
    return out_->get_stream();
  }
#endif
  IMP_SHOWABLE_INLINE(TextOutput, out << get_name());
  std::string get_name() const {
    return out_->get_name();
  }
};


/** A TextInput can be implicitly constructed from a C++ stream, a
    Python filelike object or a path to a file. As a result, those can be
    passed directly to functions which take a TextInput as an
    argument.
    \code
    IMP::atom::read_pdb("path/to/file.pdb", m);
    IMP::atom::read_pdb(my_fstream, m);
    \endcode
    \see TextOutput
*/
class IMPEXPORT TextInput
{
  Pointer<internal::IOStorage<std::istream> > in_;
 public:
#ifndef IMP_DOXYGEN
  // SWIG needs these here for some bizarre reason
  TextInput(int);
  TextInput(double);
  TextInput(const char *c);
  TextInput(TextProxy<std::istream> p);
#endif
  TextInput(){}
  TextInput(std::string file_name);
#ifndef SWIG
  TextInput(std::istream &out, std::string name="C++ stream");
#endif

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
  operator std::istream &() {
    return get_stream();
  }
  operator bool () {
    return in_ && in_->get_stream();
  }
  std::istream &get_stream() {
    if (!in_) {
      IMP_THROW("Attempting to read from uninitialized text input",
                IOException);
    }
    return in_->get_stream();
  }
#endif
  IMP_SHOWABLE_INLINE(TextOutput, out << get_name());
  std::string get_name() const {
    return in_->get_name();
  }
};

//! Set the target for the log.
/** See TextOutput for options. Python users should use
    SetLogTarget instead.
    \ingroup logging
 */
#ifndef SWIG
IMPEXPORT void set_log_target(TextOutput l);
IMPEXPORT TextOutput get_log_target();
#endif

/** Set the log target to a given value and reset it
    when the object is destroyed. Use this in Python
    to set the target of logs.
    \ingroup logging
*/
class SetLogTarget {
  /* Python deletes all Python objects before static
     destructors are called. As a result, having static
     C++ objects point to Python objects is problematic.
     This class makes sure that the pointer to the
     Python class gets cleaned up when Python exits.
  */
  TextOutput old_;
public:
  IMP_RAII(SetLogTarget, (TextOutput to),
           old_=get_log_target();,
           set_log_target(to);,
           set_log_target(old_););
};


IMP_VALUES(TextInput,TextInputs);
IMP_VALUES(TextOutput,TextOutputs);

/** Create a temporary file. The path can be extracted from the TextOutput.

    If suffix is non-empty, there is some small chance of a collision on
    non-BSD systems as a unique temporary file is first created, and then
    a file with that suffix appended is created.*/
IMPEXPORT TextOutput create_temporary_file(std::string prefix="imp_temp",
                                           std::string suffix="");


/** Return a path to a file relative to another file. For example
    if base is path/to/config.file and relative is data/image0.jpg
    then the return value would be path/to/data/image0.jpg. This
    function should be used when processing configuration files so
    that the meaning of the configuration file does not change if
    current working directory changes.
*/
IMPEXPORT std::string get_relative_path(std::string base,
                                   std::string relative);

/** Set the directory where IMP data is to be found. The directory
    should have a subdirectory for each module containing
    that module's data.
*/
IMPEXPORT void set_data_path(std::string dp);

/** Set the directory where IMP examples are to be found. The directory
    should have a subdirectory for each module containing
    that module's examples.
*/
IMPEXPORT void set_example_path(std::string dp);

IMP_END_NAMESPACE

#endif /* IMP_FILE_H */
