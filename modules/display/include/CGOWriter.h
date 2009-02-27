/**
 *  \file CGOWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_CGO_WRITER_H
#define IMPDISPLAY_CGO_WRITER_H

#include "config.h"
#include "macros.h"

#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/Writer.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Write a CGO file with the geometry
/** The cgo file format is a simple format for displaying geometry in Pymol.
    The cgo writer supports points, spheres, cyliners, and segments.
 */
class IMPDISPLAYEXPORT CGOWriter: public Writer
{
  std::string name_;
  unsigned int count_;
public:
  //! write to a file using the name to  name the files
  CGOWriter(std::string name);

  virtual ~CGOWriter();

  IMP_WRITER(internal::version_info)

  IMP_WRITER_ADD_GEOMETRY
};



IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CGO_WRITER_H */
