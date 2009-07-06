/**
 *  \file CGOWriter.h
 *  \brief XXXXXXXXXXXXXX
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
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



//! Write a CGO file with an animation of the geometry
/** This write behaves a bit differently than the others in that it
    only writes one file during the lifetime of the object. Calls to
    Write::set_file_name() simply cause it to move on to the next
    frame rather than to actually open a new file.

    The frames will be numbered from 1.

    \note This class currently counts as a bit of a hack and so
    may not behave as expected always.
 */
class IMPDISPLAYEXPORT CGOAnimationWriter: public Writer
{
  std::string name_;
  unsigned int count_;
  bool initialized_;
public:
  //! write to a file using the name to  name the files
  /** The pymol_name parameter is the name used to identify it
      within pymol. */
  CGOAnimationWriter(std::string file_name);

  IMP_WRITER(CGOAnimationWriter, internal::version_info)

  IMP_WRITER_ADD_GEOMETRY

 //! Set the name used to identify the model in pymol
  void set_name(std::string name) {
    name_=name;
  }
};


//! Write a CGO file with the geometry
/** The cgo file format is a simple format for displaying geometry in Pymol.
    The cgo writer supports points, spheres, cyliners, and segments.
    The file name should end in ".py".
 */
class IMPDISPLAYEXPORT CGOWriter: public Writer
{
  friend class CGOAnimationWriter;
  std::string name_;
  unsigned int count_;

  static void write_geometry(Geometry *g, std::ostream &out);
public:
  //! write to a file using the name to  name the files
  CGOWriter(std::string file_name=std::string());

  IMP_WRITER(CGOWriter, internal::version_info)

    //! Set the name used to identify the model in pymol
  void set_name(std::string name) {
    name_=name;
  }

  IMP_WRITER_ADD_GEOMETRY
};



IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_CGO_WRITER_H */
