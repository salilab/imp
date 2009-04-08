/**
 *  \file Writer.h
 *  \brief Base class for writing geometry to a file
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_WRITER_H
#define IMPDISPLAY_WRITER_H

#include "config.h"
#include "geometry.h"

#include "internal/version_info.h"
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
class IMPDISPLAYEXPORT Writer: public Object
{
  std::ofstream out_;
  std::string file_name_;
 protected:
  //! Get the stream for inhereting classes to write to
  std::ostream &get_stream() {
    if (!out_.is_open() && !file_name_.empty()) {
      out_.open(file_name_.c_str());
      on_open();
    }
    IMP_check(out_.is_open(), "Error opening file '" << file_name_ << "'",
               InvalidStateException);
    return out_;
  }

  //! Get the name of the file
  std::string get_file_name() const {
    return file_name_;
  }

 public:
  //! Create a writer opening the file with the passed name
  Writer(std::string name);

  // Ideally this would be const, but std::ostream::is_open is unfortunately
  // defined non-const in older versions of the C++ standard, so need to leave
  // this as non-const until we can require g++ later than 3.5.
  //! Return whether a file is open for writing
  bool get_stream_is_open() {
    return out_.is_open();
  }

  //! Open a new file with the given name
  /** Set it to "" to close. */
  void set_file_name(std::string name) {
    if (get_stream_is_open()) {
      on_close();
      out_.close();
    }
    file_name_=name;
  }

  //! Close the stream. You shouldn't need this, but it doesn't hurt
  void close() {
    set_file_name("");
  }

  //! Write the data and close the file
  virtual ~Writer();

  virtual void add_geometry(Geometry *g)=0;

  virtual void add_geometry(const Geometries &g) {
    for (unsigned int i=0; i< g.size(); ++i) {
      IMP_CHECK_OBJECT(g[i]);
      add_geometry(g[i]);
    }
  }

  virtual void add_geometry(CompoundGeometry *cg) {
    Geometries g= cg->get_geometry();
    for (unsigned int i=0; i< g.size(); ++i) {
      IMP_CHECK_OBJECT(g[i]);
      Pointer<Geometry> gi(g[i]);

      add_geometry(g[i]);
    }
  }

  virtual void add_geometry(const CompoundGeometries &g) {
    for (unsigned int i=0; i< g.size(); ++i) {
      IMP_CHECK_OBJECT(g[i]);
      add_geometry(g[i]);
    }
  }

  virtual VersionInfo get_version_info() const=0;

  virtual void show(std::ostream &out=std::cout) const=0;

 protected:
  //! A hook for implementation classes to use to take actions on file close
  virtual void on_close()=0;
  //! A hook for implementation classes to use to take actions on file open
  virtual void on_open()=0;
};

IMP_OUTPUT_OPERATOR(Writer)


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_WRITER_H */
