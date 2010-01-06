/**
 *  \file BildWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_BILD_WRITER_H
#define IMPDISPLAY_BILD_WRITER_H

#include "config.h"
#include "macros.h"

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
  void process(Color color) {
    get_stream() << ".color " << color << "\n";
  }
  bool process(PointGeometry *g, Color color, std::string name) {
    process(color);
    get_stream() << ".dotat " <<  algebra::spaces_io(*g)
                 << "\n";
    return true;
  }
  bool process(SegmentGeometry *g, Color color, std::string name) {
    process(color);
    get_stream() << ".move "
                 << algebra::spaces_io(g->get_point(0)) << "\n";
    get_stream() << ".draw "
                 << algebra::spaces_io(g->get_point(1))
                 << "\n";
    return true;
  }
  bool process(PolygonGeometry *g, Color color, std::string name) {
    process(color);
    get_stream() << ".polygon ";
    for (unsigned int i=0; i< g->size(); ++i) {
      get_stream() << " " << algebra::spaces_io(g->at(i));
    }
    get_stream() << "\n";
    return true;
  }
  bool process(TriangleGeometry *g, Color color, std::string name) {
    process(color);
    get_stream() << ".polygon ";
    for (unsigned int i=0; i< 3; ++i) {
      get_stream() << " " << algebra::spaces_io(g->at(i));
    }
    get_stream() << "\n";
    return true;
  }
  bool process(SphereGeometry *g, Color color, std::string name) {
    process(color);
    get_stream() << ".sphere "
                 << algebra::spaces_io(g->get_center()) << " "
                 << g->get_radius() << "\n";
    return true;
  }
  bool process(CylinderGeometry *g, Color color, std::string name) {
    process(color);
    get_stream() << ".cylinder "
                 << algebra::spaces_io(g->get_segment().get_point(0))
                 << " "
                 << algebra::spaces_io(g->get_segment().get_point(1)) << " "
                 << g->get_radius() << "\n";
    return true;
  }
public:
  //! write to a file with the given name
  BildWriter(std::string file_name=std::string());

  IMP_WRITER(BildWriter, get_module_version_info())
};



IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_BILD_WRITER_H */
