/**
 *  \file CMMWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_CMM_WRITER_H
#define IMPDISPLAY_CMM_WRITER_H

#include "config.h"
#include "macros.h"

#include "internal/version_info.h"
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
public:
  //! write to a file
  CMMWriter(std::string name=std::string());

  IMP_WRITER(CMMWriter, internal::version_info)

  IMP_WRITER_ADD_GEOMETRY
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CMM_WRITER_H */
