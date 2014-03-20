/**
 *  \file IMP/display/declare_Geometry.h
 *  \brief Implement geometry for the basic shapes from IMP.algebra.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_DECLARE_GEOMETRY_H
#define IMPDISPLAY_DECLARE_GEOMETRY_H

#include <IMP/display/display_config.h>
#include "Color.h"
#include <IMP/base/Object.h>
#include <IMP/base/object_macros.h>
#include <IMP/base/ref_counted_macros.h>

IMPDISPLAY_BEGIN_NAMESPACE
class Geometry;
IMP_OBJECTS(Geometry, Geometries);

//! The base class for geometry.
/** This class doesn't have much to say other than the color.

    \headerfile geometry.h "IMP/display/geometry.h"
 */
class IMPDISPLAYEXPORT Geometry : public IMP::base::Object {
  bool has_color_;
  Color color_;

 public:
  // Geometry();
  Geometry(std::string name);
  Geometry(Color c, std::string name);
  // Geometry(Color c);
  virtual Color get_color() const {
    IMP_USAGE_CHECK(has_color_, "Color not set");
    return color_;
  }

  virtual bool get_has_color() const { return has_color_; }
  void set_has_color(bool tf) { has_color_ = tf; }
  void set_color(Color c) {
    has_color_ = true;
    color_ = c;
  }

  //! Return a set of geometry composing this one
  virtual Geometries get_components() const { return Geometries(); }

  IMP_REF_COUNTED_DESTRUCTOR(Geometry);
};

IMPDISPLAY_END_NAMESPACE

#endif /* IMPDISPLAY_DECLARE_GEOMETRY_H */
