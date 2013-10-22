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
#include <IMP/kernel/internal/constants.h>

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
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                const algebra::Vector3D &v, Float charge) {
    XYZ::setup_particle(m, pi, v);
    do_setup_particle(m, pi, charge);
  }
  static void do_setup_particle(kernel::Model *m, kernel::ParticleIndex pi,
                                Float charge = 0) {
    IMP_USAGE_CHECK(XYZ::get_is_setup(m, pi),
                    "Particle must already be an XYZ particle");
    m->add_attribute(get_charge_key(), pi, charge);
  }

 public:
  IMP_DECORATOR_METHODS(Charged, IMP::core::XYZ);
  IMP_DECORATOR_SETUP_1(Charged, Float, charge);
  IMP_DECORATOR_SETUP_2(Charged, algebra::Vector3D, v, Float, charge);

  IMP_DECORATOR_GET_SET(charge, get_charge_key(), Float, Float);

  //! Return true if the particle is an instance of a Charged
  static bool get_is_setup(kernel::Model *m, kernel::ParticleIndex pi) {
    return XYZ::get_is_setup(m, pi) &&
           m->get_has_attribute(get_charge_key(), pi);
  }

  static FloatKey get_charge_key();
};

IMP_DECORATORS(Charged, Chargeds, core::XYZs);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CHARGED_H */
