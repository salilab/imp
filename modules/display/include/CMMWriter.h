/**
 *  \file CMMWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_CMM_WRITER_H
#define IMPDISPLAY_CMM_WRITER_H

#include "config.h"
#include "macros.h"

#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/Writer.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write a CMM file with the geometry
/** The CMM writer supports points and spheres. Cylinders can be added
    at some point.

    You are probably better off using the IMP::display::ChimeraWriter
    which writes a python file, readable by Chimera as it supports
    more types of geometry.
 */
class IMPDISPLAYEXPORT CMMWriter: public Writer
{
  unsigned int marker_index_;
  bool process(SphereGeometry *g,
               Color color, std::string name);
  bool process(PointGeometry *g,
               Color color, std::string name);
public:
  //! write to a file
  CMMWriter(std::string name=std::string());

  IMP_WRITER(CMMWriter, get_module_version_info())
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CMM_WRITER_H */
