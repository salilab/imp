/**
 *  \file PymolWriter.h
 *  \brief Implement PymolWriter
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_PYMOL_WRITER_H
#define IMPDISPLAY_PYMOL_WRITER_H

#include "display_config.h"
#include "display_macros.h"

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
class IMPDISPLAYEXPORT PymolWriter: public Writer
{
  friend class CGOAnimationWriter;
  void setup(std::string name);
  void cleanup(std::string name, bool close=true);
  bool handle(SphereGeometry *g,
               Color color, std::string name);
  bool handle(CylinderGeometry *g,
               Color color, std::string name);
  bool handle(PointGeometry *g,
               Color color, std::string name);
  bool handle(SegmentGeometry *g,
               Color color, std::string name);
  bool handle(PolygonGeometry *g,
               Color color, std::string name);
  bool handle(TriangleGeometry *g,
               Color color, std::string name);
  bool handle(LabelGeometry *g,
               Color color, std::string name);

public:

  IMP_WRITER(PymolWriter);
};



IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_PYMOL_WRITER_H */
