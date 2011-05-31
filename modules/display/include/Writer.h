/**
 *  \file Writer.h
 *  \brief Base class for writing geometry to a file
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_WRITER_H
#define IMPDISPLAY_WRITER_H

#include "display_config.h"
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
  int frame_;
 public:
  //! Create a writer opening the file with the passed name
  Writer(std::string name);

  //! Write the data and close the file
  virtual ~Writer();

  /** \name Frames
      The writer has a concept of the current frame. Depending on
      the implementation, each frame might be stored in a separate
      file or they might all be in one file. If using a writer that
      stores frames in multiple files, you should include a %1% in the
      filename which will get replaced by the frame number.
      @{
  */
  void set_frame(unsigned int i);
  int get_frame() const {
    return frame_;
  }
  /** @} */


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
  virtual void do_close()=0;
  //! A hook for implementation classes to use to take actions on file open
  virtual void do_open()=0;

  //! in case you want to take action on a new frame
  virtual void do_set_frame() {}
};


/** A base class for writers which write to text files. By default,
    separate frames are stored in separate files. To change this,
    override the do_set_frame() method.
 */
class IMPDISPLAYEXPORT TextWriter: public Writer
{
  std::string file_name_;
  TextOutput out_;
 protected:
  //! Get the stream for inhereting classes to write to
  std::ostream &get_stream() {
    return out_;
  }

  void do_set_frame();

 public:
  //! Create a writer opening the file with the passed name
  TextWriter(TextOutput fn);
  TextWriter(std::string name);

  //! Write the data and close the file
  virtual ~TextWriter();
};


/** Create an appropriate writer based on the file suffix. */
IMPDISPLAYEXPORT Writer *create_writer(std::string filename,
                                       bool append=false);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
#define IMP_REGISTER_WRITER(Name, suffix)                               \
  namespace {                                                           \
    internal::WriterFactoryRegistrar Name##registrar(suffix,            \
                            new internal::WriterFactoryHelper<Name>()); \
  }
#endif

IMP_OBJECTS(Writer, Writers);
IMP_OBJECTS(TextWriter, TextWriters);

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_WRITER_H */
