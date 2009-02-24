/**
 *  \file CMMWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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
 */
class IMPDISPLAYEXPORT CMMWriter: public Writer
{
  unsigned int marker_index_;
public:
  //! write to a file
  CMMWriter();

  virtual ~CMMWriter();

  IMP_WRITER(internal::version_info)

  IMP_WRITER_ADD_GEOMETRY
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CMM_WRITER_H */
