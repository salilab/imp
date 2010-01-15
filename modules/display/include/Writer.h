/**
 *  \file Writer.h
 *  \brief Base class for writing geometry to a file
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_WRITER_H
#define IMPDISPLAY_WRITER_H

#include "config.h"
#include "geometry.h"
#include "internal/writers.h"
#include "GeometryProcessor.h"

#include <IMP/file.h>
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/RefCounted.h>

IMPDISPLAY_BEGIN_NAMESPACE


//! Base class for writing geometry to a file.
/** A writer object accumulates geometry and then
    writes a file when the object is destroyed. You must set the name
    of the file being written to before writing using the
    IMP::display::Writer::set_file_name() method.
 */
class IMPDISPLAYEXPORT Writer: public GeometryProcessor, public Object
{
  TextOutput out_;
  bool on_open_called_;
 protected:
  //! Get the stream for inhereting classes to write to
  std::ostream &get_stream() {
    if (! on_open_called_) {
      // can't call virtual functions from constructor reliably
      on_open_called_=true;
      on_open();
    }
    return out_;
  }

 public:
  //! Create a writer opening the file with the passed name
  Writer(TextOutput fn, std::string name);
  Writer(std::string name);

  // Ideally this would be const, but std::ostream::is_open is unfortunately
  // defined non-const in older versions of the C++ standard, so need to leave
  // this as non-const until we can require g++ later than 3.5.
  //! Return whether a file is open for writing
  bool get_stream_is_open() {
    return out_;
  }

  //! Open a new file with the given name
  /** Set it to "" to close. */
  virtual void set_file_name(std::string name) {
    if (get_stream_is_open()) on_close();
    out_= TextOutput(name);
    if (get_stream_is_open()) on_open();
  }

  virtual void set_output(TextOutput f) {
    if (get_stream_is_open()) on_close();
    out_= f;
    if (get_stream_is_open()) on_open();
  }

  //! Close the stream. You shouldn't need this, but it doesn't hurt
  void close() {
    if (get_stream_is_open()) on_close();
    out_= TextOutput();
  }

  //! Write the data and close the file
  virtual ~Writer();

  /** @name Geometry Addition methods
      These methods can be used to add geometry to the model.
      If you do not want the geometry objects to be destroyed
      upon addition, make sure you store an IMP::Pointer
      to them externally.

      \throws UsageException if it doesn't know
      how to write that particular sort of geometry.
      @{
  */
  void add_geometry(Geometry* g);

  void add_geometry(const Geometries &g) {
    for (unsigned int i=0; i< g.size(); ++i) {
      IMP_CHECK_OBJECT(g[i]);
      IMP::internal::OwnerPointer<Geometry> gp(g[i]);
      add_geometry(gp);
    }
  }
  /** @} */

 protected:
  //! A hook for implementation classes to use to take actions on file close
  virtual void on_close()=0;
  //! A hook for implementation classes to use to take actions on file open
  virtual void on_open()=0;
};

IMP_OUTPUT_OPERATOR(Writer)

/** Create an appropriate writer based on the file suffix. */
IMPDISPLAYEXPORT Writer *create_writer(std::string filename);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
#define IMP_REGISTER_WRITER(Name, suffix)                               \
  namespace {                                                           \
    internal::WriterFactoryRegistrar registrar(suffix,                  \
                            new internal::WriterFactoryHelper<Name>()); \
  }
#endif

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_WRITER_H */
