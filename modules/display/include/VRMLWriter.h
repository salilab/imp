/**
 *  \file VRMLWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
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
/**
 */
class IMPDISPLAYEXPORT VRMLWriter: public Writer
{
public:
  //! write to a file with the given name
  VRMLWriter();

  virtual ~VRMLWriter();

  //IMP_WRITER(internal::version_info)
  virtual void add_geometry(Geometry *g);
  virtual void on_open(std::string name);
  virtual void on_close();
  virtual VersionInfo get_version_info() const {return internal::version_info;}
  virtual void show(std::ostream &out=std::cout) const;

};


IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_VRML_WRITER_H */
