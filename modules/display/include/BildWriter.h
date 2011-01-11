/**
 *  \file BildWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_BILD_WRITER_H
#define IMPDISPLAY_BILD_WRITER_H

#include "display_config.h"
#include "display_macros.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/Writer.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write a Bild file with the geometry
/** The bild file format is a simple format for displaying geometry in Chimera.
    The bild writer supports points, spheres, cyliners, and segments.

    You are probably better off using the IMP::display::ChimeraWriter unless
    you want a human-readable file. ChimeraWriter supports better
    interactivity and more types of geometry.
 */
class IMPDISPLAYEXPORT BildWriter: public Writer
{
 protected:
  void handle(Color color) {
    get_stream() << ".color " << color << "\n";
  }
  bool handle(PointGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".dotat " <<  algebra::spaces_io(*g)
                 << "\n";
    return true;
  }
  bool handle(SegmentGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".move "
                 << algebra::spaces_io(g->get_point(0)) << "\n";
    get_stream() << ".draw "
                 << algebra::spaces_io(g->get_point(1))
                 << "\n";
    return true;
  }
  bool handle(PolygonGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".polygon ";
    for (unsigned int i=0; i< g->size(); ++i) {
      get_stream() << " " << algebra::spaces_io(g->at(i));
    }
    get_stream() << "\n";
    return true;
  }
  bool handle(TriangleGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".polygon ";
    for (unsigned int i=0; i< 3; ++i) {
      get_stream() << " " << algebra::spaces_io(g->at(i));
    }
    get_stream() << "\n";
    return true;
  }
  bool handle(SphereGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".sphere "
                 << algebra::spaces_io(g->get_center()) << " "
                 << g->get_radius() << "\n";
    return true;
  }
  bool handle(CylinderGeometry *g, Color color, std::string) {
    handle(color);
    get_stream() << ".cylinder "
                 << algebra::spaces_io(g->get_segment().get_point(0))
                 << " "
                 << algebra::spaces_io(g->get_segment().get_point(1)) << " "
                 << g->get_radius() << "\n";
    return true;
  }
public:

  IMP_WRITER(BildWriter);
};



IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_BILD_WRITER_H */
