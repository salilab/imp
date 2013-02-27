/**
 *  \file IMP/display/CMMWriter.h
 *  \brief A writer for chimera CMM files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_CMM_WRITER_H
#define IMPDISPLAY_CMM_WRITER_H

#include <IMP/display/display_config.h>
#include "writer_macros.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/Writer.h>

IMPDISPLAY_BEGIN_NAMESPACE
#if IMP_HAS_DEPRECATED
//! Write a CMM file with the geometry
/** The CMM writer supports points and spheres. Cylinders can be added
    at some point.

    This format creates one file per frame. So if you want to use frames
    and save them all, make sure there is a "%1%" in the file name string
    passed to the constructor.

    You are probably better off using the IMP::display::ChimeraWriter
    which writes a python file, readable by Chimera as it supports
    more types of geometry.

    \deprecated Use IMP::rmf for interfacing with Chimera.
 */
class IMPDISPLAYEXPORT CMMWriter: public TextWriter
{
  unsigned int marker_index_;
  bool handle_sphere(SphereGeometry *g,
              Color color, std::string name);
  bool handle_point(PointGeometry *g,
              Color color, std::string name);
public:
  IMP_TEXT_WRITER(CMMWriter);
 public:
  unsigned int get_current_index() const {
    return marker_index_;
  }
};
#endif

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CMM_WRITER_H */
