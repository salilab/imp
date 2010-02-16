/**
 *  \file IMP/file.h
 *  \brief Control display of deprecation information.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_FILE_H
#define IMP_FILE_H

#include "config.h"
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
    python file or a path to a file. As a result, those can be
    passed directly to functions which take a TextOutput as an
    argument.

    Files are created lazily, so TextOutput can be passed as
    arguments to functions that might not produce output.
    \code
    IMP::atom::write_pdb(particles, "path/to/file.pdb");
    IMP::atom::write_pdb(particles, my_fstream);
    \endcode
*/
class IMPEXPORT TextOutput
{
  Pointer<internal::IOStorage<std::ostream> > out_;
 public:
#ifndef IMP_DOXYGEN
  // swig needs there here for some bizaare reason
  TextOutput(int);
  TextOutput(double);
  TextOutput(const char *c);
  TextOutput(TextProxy<std::ostream> p);
#endif
  TextOutput(){}
  TextOutput(std::string file_name);
#ifndef SWIG
  TextOutput(std::ostream &out);
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
};


/** A TextInput can be implicitly constructed from a C++ stream, a
    python file or a path to a file. As a result, those can be
    passed directly to functions which take a TextInput as an
    argument.
    \code
    IMP::atom::read_pdb("path/to/file.pdb", m);
    IMP::atom::read_pdb(my_fstream, m);
    \endcode
*/
class IMPEXPORT TextInput
{
  Pointer<internal::IOStorage<std::istream> > in_;
 public:
#ifndef IMP_DOXYGEN
  // swig needs there here for some bizaare reason
  TextInput(int);
  TextInput(double);
  TextInput(const char *c);
  TextInput(TextProxy<std::istream> p);
#endif
  TextInput(){}
  TextInput(std::string file_name);
#ifndef SWIG
  TextInput(std::istream &out);
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
    when the object is destroyed. Use this in python
    to set the target of logs.
    \ingroup logging
*/
class SetLogTarget: public RAII {
  /* Python deletes all python objects before static
     destructors are called. As a result, having static
     C++ objects point to python objects is problematic.
     This class makes sure that the pointer to the
     python class gets cleaned up when python exits.
  */
  TextOutput old_;
public:
  IMP_RAII(SetLogTarget, (TextOutput to),
           old_=get_log_target();,
           set_log_target(to);,
           set_log_target(old_););
};
IMP_END_NAMESPACE

#endif /* IMP_FILE_H */
