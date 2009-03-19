/**
 *  \file VRMLWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_VRML_WRITER_H
#define IMPDISPLAY_VRML_WRITER_H

#include "config.h"
#include "macros.h"

#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/Writer.h>
#include <iostream>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write a VRML file with the geometry
/** This writer currently only supports spheres. VRML supports other geometry
    so someone could add it.
 */
class IMPDISPLAYEXPORT VRMLWriter: public Writer
{
public:
  //! write to a file with the given name
  VRMLWriter(std::string name=std::string());

  IMP_WRITER(VRMLWriter, internal::version_info)

  IMP_WRITER_ADD_GEOMETRY
};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_VRML_WRITER_H */
