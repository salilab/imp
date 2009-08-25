/**
 *  \file CGOWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_CGO_WRITER_H
#define IMPDISPLAY_CGO_WRITER_H

#include "config.h"
#include "macros.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/Writer.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write a CGO file with the geometry
/** The cgo file format is a simple format for displaying geometry in Pymol.
    The cgo writer supports points, spheres, cyliners, and segments.
    The file name should end in ".pym".

    The geometry is assembled into objects in pymol based on the passed
    names. For example, all geometry named "box" becomes one pymol object.
    If many files are loaded into python defining the same objects, they
    become sequential frames in a movie. The frame numbers are determined
    sequentially from the file load order (so they can form a subset of
    the generated files).
 */
class IMPDISPLAYEXPORT CGOWriter: public Writer
{
  friend class CGOAnimationWriter;
  unsigned int count_;

  static void write_geometry(Geometry *g, std::ostream &out);
public:
  //! write to a file using the name to  name the files
  CGOWriter(std::string file_name=std::string());

  virtual void add_geometry(CompoundGeometry* cg);
  virtual void add_geometry(const IMP::display::Geometries &g) {
    Writer::add_geometry(g);
  }
  virtual void add_geometry(const IMP::display::CompoundGeometries &g) {
    Writer::add_geometry(g);
  }


  IMP_WRITER(CGOWriter, get_module_version_info())
};



IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CGO_WRITER_H */
