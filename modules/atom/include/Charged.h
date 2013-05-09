/**
 *  \file IMP/atom/Charged.h
 *  \brief A decorator for a point particle that has an electrostatic charge.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_CHARGED_H
#define IMPATOM_CHARGED_H

#include <IMP/atom/atom_config.h>

#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/internal/constants.h>

#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a point particle that has an electrostatic charge.
/** The charge itself should be given in atomic units; i.e. the charge on
    a proton is +1 and on an electron, -1.

    \ingroup helper
    \ingroup decorators
    \see CoulombPairScore
 */
class IMPATOMEXPORT Charged : public IMP::core::XYZ {
 public:
  IMP_DECORATOR(Charged, IMP::core::XYZ);

  /** Create a decorator with the passed coordinates and charge.
   */
  static Charged setup_particle(Particle *p, const algebra::Vector3D &v,
                                Float charge) {
    XYZ::setup_particle(p, v);
    p->add_attribute(get_charge_key(), charge);
    return Charged(p);
  }

  /** Create a decorator with the passed charge.
      The particle is assumed to already have x,y,z attributes.
   */
  static Charged setup_particle(Particle *p, Float charge = 0) {
    IMP_USAGE_CHECK(XYZ::particle_is_instance(p),
                    "Particle must already be an XYZ particle");
    p->add_attribute(get_charge_key(), charge);
    return Charged(p);
  }

  IMP_DECORATOR_GET_SET(charge, get_charge_key(), Float, Float);

  //! Return true if the particle is an instance of a Charged
  static bool particle_is_instance(Particle *p) {
    return XYZ::particle_is_instance(p) && p->has_attribute(get_charge_key());
  }

  static FloatKey get_charge_key();
};

IMP_DECORATORS(Charged, Chargeds, core::XYZs);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHARGED_H */
