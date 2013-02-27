/**
 *  \file IMP/display/ChimeraWriter.h
 *  \brief A writer for Chimera python files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_CHIMERA_WRITER_H
#define IMPDISPLAY_CHIMERA_WRITER_H

#include <IMP/display/display_config.h>
#include <IMP/display/Writer.h>
#include <IMP/display/writer_macros.h>

IMPDISPLAY_BEGIN_NAMESPACE
#if IMP_HAS_DEPRECATED
//! Write geometry to a python file for Chimera to read
/** The writer writes a python file which can handle markers, edges
    and surfaces.  Since these are native chimera objects, they are
    handled a bit better than vrml ones.

    This format creates one file per frame. So if you want to use frames
    and save them all, make sure there is a "%1%" in the file name string
    passed to the constructor.

    \deprecated Use IMP::rmf instead when interfacing with Chimera.
 */
class IMPDISPLAYEXPORT ChimeraWriter: public TextWriter
{
  void cleanup(std::string name,
               bool need_ms, bool need_surf=false);
  bool handle_sphere(SphereGeometry *g,
               Color color, std::string name);
  bool handle_cylinder(CylinderGeometry *g,
               Color color, std::string name);
  bool handle_point(PointGeometry *g,
               Color color, std::string name);
  bool handle_segment(SegmentGeometry *g,
               Color color, std::string name);
  bool handle_polygon(PolygonGeometry *g,
               Color color, std::string name);
  bool handle_triangle(TriangleGeometry *g,
               Color color, std::string name);
  bool handle_ellipsoid(EllipsoidGeometry *g,
               Color color, std::string name);
public:
  IMP_TEXT_WRITER(ChimeraWriter);

  //! Add some arbitrary python code to the chimera file
  /** You should import the bits of Chimera that you need.
      At the moment, you should not name a variable surf_sets
      or marker_sets.
   */
  void add_python_code(std::string code);
};
#endif

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CHIMERA_WRITER_H */
