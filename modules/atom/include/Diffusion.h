/**
 *  \file IMP/atom/Diffusion.h
 *  \brief A decorator for a diffusing particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_DIFFUSION_H
#define IMPATOM_DIFFUSION_H

#include <IMP/atom/atom_config.h>

#include <IMP/core/XYZR.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/kernel/internal/constants.h>

#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a diffusing particle.
/** \ingroup helper
    \ingroup decorators
    \see BrownianDynamics
    \unstable{Diffusion} The name really should be fixed.

    D is assumed to be in \f$A^2/fs\f$
 */
class IMPATOMEXPORT Diffusion : public IMP::core::XYZ {
  static void do_setup_particle(Model *m, kernel::ParticleIndex pi,
                                Float D) {
    IMP_USAGE_CHECK(XYZ::get_is_setup(m, pi),
                    "Particle must already be an XYZ particle");
    m->add_attribute(get_diffusion_coefficient_key(), pi, D);
  }
  static void do_setup_particle(Model *m, kernel::ParticleIndex pi,
                                const algebra::Vector3D &v,
                                Float D) {
    XYZ::setup_particle(m, pi, v);
    do_setup_particle(m, pi, D);
  }
  static void do_setup_particle(Model *m, kernel::ParticleIndex pi);
 public:
  IMP_DECORATOR_METHODS(Diffusion, IMP::core::XYZ);
  IMP_DECORATOR_SETUP_1(Diffusion, Float, D);
  IMP_DECORATOR_SETUP_2(Diffusion, algebra::Vector3D, v,
                        Float, D);
  /** Assume particle is already a core::XYZR particle. */
  IMP_DECORATOR_SETUP_0(Diffusion);

  //! Return true if the particle is an instance of an Diffusion
  static bool get_is_setup(Model *m, kernel::ParticleIndex p) {
    return m->get_has_attribute(get_diffusion_coefficient_key(), p);
  }
  void set_diffusion_coefficient(double d) {
    get_particle()->set_value(get_diffusion_coefficient_key(), d);
  }
  double get_diffusion_coefficient() const {
    return get_particle()->get_value(get_diffusion_coefficient_key());
  }
  //! Get the D key
  static FloatKey get_diffusion_coefficient_key();

  /** \deprecated_at{2.1} Use set_diffusion_coefficient(). */
  IMPATOM_DEPRECATED_METHOD_DECL(2.1)
    void set_d(double d);
  /** \deprecated_at{2.1} Use get_diffusion_coefficient(). */
  IMPATOM_DEPRECATED_METHOD_DECL(2.1)
    double get_d() const;
};

IMPATOMEXPORT double get_diffusion_coefficient_from_cm2_per_second(double din);

IMP_DECORATORS(Diffusion, Diffusions, core::XYZs);

/** A rigid body that is diffusing, so it also has a rotation diffusion
    coefficient. The units on the rotational coefficient are
    \f$radians^2/fs\f$.*/
class IMPATOMEXPORT RigidBodyDiffusion : public Diffusion {
  static void do_setup_particle(Model *m, kernel::ParticleIndex pi);
 public:
  IMP_DECORATOR_METHODS(RigidBodyDiffusion, Diffusion);
  /** All diffusion coefficients are determined from the radius */
  IMP_DECORATOR_SETUP_0(RigidBodyDiffusion);

  double get_rotational_diffusion_coefficient() const {
    return get_particle()->get_value(
        get_rotational_diffusion_coefficient_key());
  }
  void set_rotational_diffusion_coefficient(double d) const {
    return get_particle()->set_value(get_rotational_diffusion_coefficient_key(),
                                     d);
  }

  //! Return true if the particle is an instance of an Diffusion
  static bool get_is_setup(Model *m, kernel::ParticleIndex p) {
    return m->get_has_attribute(get_rotational_diffusion_coefficient_key(), p);
  }

  //! Get the D key
  static FloatKey get_rotational_diffusion_coefficient_key();
};

IMP_DECORATORS(RigidBodyDiffusion, RigidBodyDiffusions, Diffusions);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DIFFUSION_H */
