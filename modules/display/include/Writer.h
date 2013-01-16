/**
 *  \file IMP/display/Writer.h
 *  \brief Base class for writing geometry to a file
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_WRITER_H
#define IMPDISPLAY_WRITER_H

#include <IMP/display/display_config.h>
#include "declare_Geometry.h"
#include "internal/writers.h"
#include "GeometryProcessor.h"
#include <IMP/base/Pointer.h>
#include <IMP/base/InputAdaptor.h>
#include <IMP/file.h>
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/RefCounted.h>

#include <boost/format.hpp>

IMPDISPLAY_BEGIN_NAMESPACE


//! Base class for writing geometry to a file.
/** A writer object accumulates geometry and then
    writes a file when the object is destroyed. You must set the name
    of the file being written to before writing using the
    IMP::display::Writer::set_file_name() method.
 */
class IMPDISPLAYEXPORT Writer:
  public GeometryProcessor, public IMP::base::Object
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
      IMP::base::OwnerPointer<Geometry> gp(g[i]);
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

    \note If you inherit from this class, you must use the
    IMP_TEXT_WRITER() macro and not provide any additional
    constructors. Sorry, it is necessary in order to ensure
    that the derived classes handler for opening a file
    is successfully called, as it can't be directly
    called from the TextWriter constructor.
 */
class IMPDISPLAYEXPORT TextWriter: public Writer
{
  std::string file_name_;
  base::TextOutput out_;
  protected:
    void open();
  //! Get the stream for inhereting classes to write to
  std::ostream& get_stream() {
    return out_;
  }

  void do_set_frame();

 public:
  //! Create a writer opening the file with the passed sink
  /** Frames are not supported with this constructor when using a format
      such as CMM or Chimera that writes multiple frames to different
      files.
  */
  TextWriter(base::TextOutput fn);
  //! Create a write for a file or files with the passed name or pattern
  /** The name should contain %1% if you want to write different frames
      to separate files. Otherwise, it will either write all frames to the
      same file (Pymol) or overwrite the file with each new frame,
      Chimera, CMM.
  */
  TextWriter(std::string name);

  //! get the name of the current file being writter
  std::string get_current_file_name() const {
    if (file_name_.find("%1%") != std::string::npos) {
      IMP_USAGE_CHECK(get_frame()>=0, "No frame set");
      std::ostringstream oss;
      oss << boost::format(file_name_)%get_frame();
      return oss.str();
    } else {
      return file_name_;
    }
  }
  //! Write the data and close the file
  virtual ~TextWriter();
};


/** Create an appropriate writer based on the file suffix. */
IMPDISPLAYEXPORT Writer *create_writer(std::string filename);


IMP_OBJECTS(Writer, Writers);
IMP_OBJECTS(TextWriter, TextWriters);


/** An adaptor for functions that should take a writer as an input.
    It can be implicitly constructed from either a Writer or a string.
    In the later case it determines what type of writer is needed from
    the file suffix. */
class IMPDISPLAYEXPORT WriterAdaptor: public base::InputAdaptor {
  IMP::base::OwnerPointer<Writer> writer_;
 public:
  WriterAdaptor(std::string name): writer_(create_writer(name)){}
  WriterAdaptor(Writer *w): writer_(w){}
#ifndef SWIG
  Writer* operator->() const {
    return writer_;
  }
  operator Writer*() const {
    return writer_;
  }
#endif
  Writer* get_writer() const {
    return writer_;
  }
  IMP_SHOWABLE_INLINE(WriterAdaptor, out << writer_->get_name());
  ~WriterAdaptor();
};

IMP_VALUES(WriterAdaptor, WriterAdaptors);

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_WRITER_H */
