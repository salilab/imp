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
    \code
    IMP::atom::write_pdb(particles, "path/to/file.pdb");
    IMP::atom::write_pdb(particles, my_fstream);
    \endcode
    \ingroup null_default
    \ingroup comparable
*/
class IMPEXPORT TextOutput
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  : public internal::TextIO<std::ostream, std::ofstream, 0>
#endif
{
  typedef internal::TextIO<std::ostream, std::ofstream, 0> P;
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
  TextOutput(const TextOutput &o): P(static_cast<const P&>(o)) {}
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
    \ingroup null_default
    \ingroup comparable
*/
class IMPEXPORT TextInput
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  : public internal::TextIO<std::istream, std::ifstream, 1>
#endif
{
  typedef internal::TextIO<std::istream, std::ifstream, 1> P;
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
  TextInput(const TextInput &o): P(static_cast<const P&>(o)) {}
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
  TextOutput old_;
public:
  IMP_RAII(SetLogTarget, (TextOutput to),
           old_=get_log_target();,
           set_log_target(to);,
           set_log_target(old_););
};
IMP_END_NAMESPACE

#endif /* IMP_FILE_H */
