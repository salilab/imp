/**
 *  \file atom/Diffusion.h
 *  \brief A decorator for a diffusing particle.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPATOM_DIFFUSION_H
#define IMPATOM_DIFFUSION_H

#include "config.h"
#include <IMP/core/internal/utility.h>

#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/internal/constants.h>

#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A a decorator for a diffusing particle.
/** \ingroup helper
    \ingroup decorators
    \see BrownianDynamics
    \unstable{Diffusion} The name really should be fixed.
 */
class IMPATOMEXPORT Diffusion:
  public IMP::core::XYZ
{
 public:
  IMP_DECORATOR(Diffusion, IMP::core::XYZ)

  /** Create a decorator with the passed coordinates and D.
      D is assumed to be in cm2/sec.
  */
  static Diffusion create(Particle *p,
                          const algebra::Vector3D &v,
                          Float D) {
    XYZ::create(p, v);
    p->add_attribute(get_D_key(), D);
    return Diffusion(p);
  }

  /** Create a decorator with the a given D.
      D is assumed to be in cm2/sec and the particle
      is assumed to already have x,y,z attributes
  */
  static Diffusion create(Particle *p,
                          Float D=0) {
    IMP_check(XYZ::is_instance_of(p),
              "Particle must already be an XYZ particle",
              ValueException);
    p->add_attribute(get_D_key(), D);
    return Diffusion(p);
  }

  IMP_DECORATOR_GET_SET(D_in_cm2_per_second, get_D_key(), Float, Float);

  //! Set D from the radius in angstroms
  /** Use default temperature.
   */
  void set_D_from_radius_in_angstroms(Float r);

  //! Set D from the radius in angstroms
  /** t is in kelvin
   */
  void set_D_from_radius_in_angstroms(Float r, Float t);

  //! Return true if the particle is an instance of an Diffusion
  static bool is_instance_of(Particle *p) {
    return XYZ::is_instance_of(p)
      && p->has_attribute(get_D_key());
  }

#ifndef SWIG
#ifndef IMP_DOXYGEN
  unit::SquareCentimeterPerSecond get_D() const {
    return
      unit::SquareCentimeterPerSecond(get_particle()->get_value(get_D_key()));
  }

  void set_D(unit::SquareCentimeterPerSecond D) {
    set_D_in_cm2_per_second(D.get_value());
  }

  unit::Angstrom
    get_sigma(unit::Femtosecond dt) const {
    return sqrt(2.0*dt*get_D());
  }
#endif
#endif

  //! Get the D key
  static FloatKey get_D_key();
};

IMP_OUTPUT_OPERATOR(Diffusion);

typedef Decorators<Diffusion, core::XYZs> Diffusions;

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_DIFFUSION_H */
