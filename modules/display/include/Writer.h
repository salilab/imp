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
#include <IMP/RefCountedObject.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Base class for writing geometry to a file.
/** A writer object accumulates geometry and then
    writes a file when the object is destroyed. You must set the name
    of the file being written to before writing using the
    IMP::display::Writer::set_file_name() method.
 */
class IMPDISPLAYEXPORT Writer: public RefCountedObject
{
  std::ofstream out_;
 protected:
  //! Get the stream for inhereting classes to write to
  std::ostream &get_stream() {
    IMP_check(out_.is_open(), "You must set the file name before writing",
               InvalidStateException);
    return out_;
  }

 public:
  //! Create a writer to a file
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
    if (!name.empty()) {
      out_.open(name.c_str());
      on_open(name);
    }
  }

  //! Close the stream. You shouldn't need this, but it doesn't hurt
  void close() {
    set_file_name("");
  }

  //! Write the data and close the file
  virtual ~Writer();

  //! Add the geometry given a Geometry objec with the particle set
  virtual void add_geometry(Geometry *g)=0;

  //! Add the geometry given a Geometry object with the particle set
  virtual void add_geometry(const Geometries &g) {
    for (unsigned int i=0; i< g.size(); ++i) {
      IMP_CHECK_OBJECT(g[i]);
      add_geometry(g[i]);
    }
  }


  //! Add the geometry given a CompoundGeometry object
  virtual void add_geometry(CompoundGeometry *cg) {
    Geometries g= cg->get_geometry();
    for (unsigned int i=0; i< g.size(); ++i) {
      IMP_CHECK_OBJECT(g[i]);
      add_geometry(g[i]);
      delete g[i];
    }
  }


  //! Add a set of CompoundGeometry objects
  virtual void add_geometry(const CompoundGeometries &g) {
    for (unsigned int i=0; i< g.size(); ++i) {
      IMP_CHECK_OBJECT(g[i]);
      add_geometry(g[i]);
    }
  }


  //! Get the version info
  virtual VersionInfo get_version_info() const=0;

  //! Write info about the object
  virtual void show(std::ostream &out=std::cout) const=0;

  //! Take actions before a file closes
  virtual void on_close()=0;
  //! Take actions after a file opens
  virtual void on_open(std::string name)=0;
};

IMP_OUTPUT_OPERATOR(Writer)


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_WRITER_H */
