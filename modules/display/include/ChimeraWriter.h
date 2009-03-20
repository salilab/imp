/**
 *  \file ChimeraWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_CHIMERA_WRITER_H
#define IMPDISPLAY_CHIMERA_WRITER_H

#include "config.h"

#include "internal/version_info.h"
#include <IMP/display/Writer.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write geometry to a python file for Chimera to read
/** The writer writes a python file which can handle markers, edges
    and surfaces.  Since these are native chimera objects, they are
    handled a bit better than vrml ones.
 */
class IMPDISPLAYEXPORT ChimeraWriter: public Writer
{
  bool has_ms_;
  bool has_surf_;

  void add_geometry_internal(IMP::display::Geometry *g, std::string name);
public:
  //! Create a writer with the given file name (or no open file)
  ChimeraWriter(std::string name=std::string());

  virtual void add_geometry(const IMP::display::Geometries &g) {
    Writer::add_geometry(g);
  }
  virtual void add_geometry(IMP::display::CompoundGeometry *cg);
  virtual void add_geometry(const IMP::display::CompoundGeometries &g) {
    Writer::add_geometry(g);
  }


  IMP_WRITER(ChimeraWriter, internal::version_info)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CHIMERA_WRITER_H */
