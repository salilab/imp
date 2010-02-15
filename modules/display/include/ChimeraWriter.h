/**
 *  \file ChimeraWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_CHIMERA_WRITER_H
#define IMPDISPLAY_CHIMERA_WRITER_H

#include "config.h"

#include <IMP/display/Writer.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write geometry to a python file for Chimera to read
/** The writer writes a python file which can handle markers, edges
    and surfaces.  Since these are native chimera objects, they are
    handled a bit better than vrml ones.
 */
class IMPDISPLAYEXPORT ChimeraWriter: public Writer
{
  void cleanup(std::string name,
               bool need_ms, bool need_surf=false);
  bool process(SphereGeometry *g,
               Color color, std::string name);
  bool process(CylinderGeometry *g,
               Color color, std::string name);
  bool process(PointGeometry *g,
               Color color, std::string name);
  bool process(SegmentGeometry *g,
               Color color, std::string name);
  bool process(PolygonGeometry *g,
               Color color, std::string name);
  bool process(TriangleGeometry *g,
               Color color, std::string name);
  bool process(EllipsoidGeometry *g,
               Color color, std::string name);
public:
  IMP_WRITER(ChimeraWriter)
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CHIMERA_WRITER_H */
