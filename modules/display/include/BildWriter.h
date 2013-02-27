/**
 *  \file IMP/display/BildWriter.h
 *  \brief Writer for the Chimera BILD file format.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_BILD_WRITER_H
#define IMPDISPLAY_BILD_WRITER_H

#include <IMP/display/display_config.h>
#include "writer_macros.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/Writer.h>

IMPDISPLAY_BEGIN_NAMESPACE

#if IMP_HAS_DEPRECATED
//! Write a Bild file with the geometry
/** The bild file format is a simple format for displaying geometry in Chimera.
    The bild writer supports points, spheres, cyliners, and segments.

    \deprecated Use IMP::rmf when interfacing with Chimera.
 */
class IMPDISPLAYEXPORT BildWriter: public TextWriter
{
  void handle(Color color) {
    get_stream() << ".color " << SpacesIO(color) << "\n";
  }
  bool handle_point(PointGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".dotat " <<  algebra::spaces_io(g->get_geometry())
                 << "\n";
    return true;
  }
  bool handle_segment(SegmentGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".move "
                 << algebra::spaces_io(g->get_geometry().get_point(0)) << "\n";
    get_stream() << ".draw "
                 << algebra::spaces_io(g->get_geometry().get_point(1))
                 << "\n";
    return true;
  }
  bool handle_polygon(PolygonGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".polygon ";
    for (unsigned int i=0; i< g->get_geometry().size(); ++i) {
      get_stream() << " " << algebra::spaces_io(g->get_geometry().at(i));
    }
    get_stream() << "\n";
    return true;
  }
  bool handle_triangle(TriangleGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".polygon ";
    for (unsigned int i=0; i< 3; ++i) {
      get_stream() << " " << algebra::spaces_io(g->get_geometry().get_point(i));
    }
    get_stream() << "\n";
    return true;
  }
  bool handle_sphere(SphereGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".sphere "
                 << algebra::spaces_io(g->get_geometry().get_center()) << " "
                 << g->get_geometry().get_radius() << "\n";
    return true;
  }
  bool handle_cylinder(CylinderGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".cylinder "
                 << algebra::spaces_io(g->get_geometry()
                                       .get_segment().get_point(0))
                 << " "
                 << algebra::spaces_io(g->get_geometry()
                                       .get_segment().get_point(1)) << " "
                 << g->get_geometry().get_radius() << "\n";
    return true;
  }
public:

  IMP_TEXT_WRITER(BildWriter);
};
#endif


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_BILD_WRITER_H */
