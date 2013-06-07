/**
 *  \file IMP/display/writer_macros.h
 *  \brief macros for display classes
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_WRITER_MACROS_H
#define IMPDISPLAY_WRITER_MACROS_H
#include "Writer.h"

//! Define information for an TextWriter object
/** This macro declares the methods do_open, do_close, add_geometry
    and show, and defines the destructor and get_version_info.
*/
#define IMP_TEXT_WRITER(Name)                                           \
  Name(base::TextOutput of): TextWriter(of)                             \
  {do_open();}                                                          \
  Name(std::string name): TextWriter(name){                             \
    if (name.find("%1%") == std::string::npos) {                        \
      TextWriter::open();                                               \
    }                                                                   \
  }                                                                     \
  Name(const char* name): TextWriter(std::string(name)){                \
    if (std::string(name).find("%1%") == std::string::npos) {           \
      TextWriter::open();                                               \
    }                                                                   \
  }                                                                     \
  IMP_OBJECT_METHODS(Name);                                             \
protected:                                                              \
virtual void do_destroy() IMP_OVERRIDE {do_close();}                    \
virtual void do_open();                                                 \
 virtual void do_close()


#define IMP_WRITER(Name)                                                \
  IMP_OBJECT_METHODS(Name);                                             \
protected:                                                              \
virtual void do_destroy() IMP_OVERRIDE {do_close();}                    \
virtual void do_open();                                                 \
 virtual void do_close()

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
#define IMP_REGISTER_WRITER(Name, suffix)                               \
  namespace {                                                           \
    internal::WriterFactoryRegistrar Name##registrar(suffix,            \
                            new internal::WriterFactoryHelper<Name>()); \
  }
#endif


#endif /* IMPDISPLAY_WRITER_MACROS_H */
