/**
 *  \file bond_geometry.h
 *  \brief Represent an XYZRDecorator particle with a sphere
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDISPLAY_BOND_GEOMETRY_H
#define IMPDISPLAY_BOND_GEOMETRY_H

#include "config.h"
#include "macros.h"

#include "internal/version_info.h"
#include <IMP/PairContainer.h>
#include <IMP/atom/bond_decorators.h>
#include <IMP/display/geometry.h>

IMPDISPLAY_BEGIN_NAMESPACE

//! Represent a bond with segment
/**
 */
class IMPDISPLAYEXPORT BondGeometry: public Geometry
{
  atom::BondDecorator d_;
  Float radius_;
public:
  //! Get the individual particles from the passed SingletonContainer
  BondGeometry(atom::BondDecorator d, Float radius=0);

  virtual ~BondGeometry();

  virtual Float get_size() const;

  std::string get_name() const {
    return d_.get_particle()->get_name();
  }

  IMP_GEOMETRY(internal::version_info)
};

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_BOND_GEOMETRY_H */
