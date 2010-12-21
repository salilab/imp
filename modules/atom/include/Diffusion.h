/**
 *  \file atom/Diffusion.h
 *  \brief A decorator for a diffusing particle.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_DIFFUSION_H
#define IMPATOM_DIFFUSION_H

#include "atom_config.h"

#include <IMP/core/XYZR.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/internal/constants.h>

#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a diffusing particle.
/** \ingroup helper
    \ingroup decorators
    \see BrownianDynamics
    \unstable{Diffusion} The name really should be fixed.
 */
class IMPATOMEXPORT Diffusion:
  public IMP::core::XYZ
{
 public:
  IMP_DECORATOR(Diffusion, IMP::core::XYZ);

  /** Create a decorator with the passed coordinates and D.
      D is assumed to be in cm2/sec.
  */
  static Diffusion setup_particle(Particle *p,
                          const algebra::VectorD<3> &v,
                          Float D) {
    XYZ::setup_particle(p, v);
    p->add_attribute(get_d_key(), D);
    return Diffusion(p);
  }

  /** Create a decorator with the a given D.
      D is assumed to be in cm2/sec and the particle
      is assumed to already have x,y,z attributes
  */
  static Diffusion setup_particle(Particle *p,
                          Float D=0) {
    IMP_USAGE_CHECK(XYZ::particle_is_instance(p),
              "Particle must already be an XYZ particle");
    p->add_attribute(get_d_key(), D);
    return Diffusion(p);
  }

  IMP_DECORATOR_GET_SET(d_in_cm2_per_second, get_d_key(), Float, Float);

  //! Set D from the radius in angstroms
  /** Use default temperature.
   */
  void set_d_from_radius(Float r);

  //! Set D from the radius in angstroms
  /** t is in kelvin
   */
  void set_d_from_radius(Float r, Float t);

  //! Return true if the particle is an instance of an Diffusion
  static bool particle_is_instance(Particle *p) {
    return XYZ::particle_is_instance(p)
      && p->has_attribute(get_d_key());
  }

#ifndef SWIG
#ifndef IMP_DOXYGEN
  unit::SquareCentimeterPerSecond get_d() const {
    return
      unit::SquareCentimeterPerSecond(get_particle()->get_value(get_d_key()));
  }

  void set_d(unit::SquareCentimeterPerSecond D) {
    set_d_in_cm2_per_second(D.get_value());
  }

  static unit::SquareCentimeterPerSecond D_from_r(unit::Angstrom radius,
                                                  unit::Kelvin t
#ifndef _MSC_VER
                                                  // it ICEs on this.
                              = IMP::internal::DEFAULT_TEMPERATURE
#endif
                                                  );

  unit::Angstrom
    get_sigma(unit::Femtosecond dt) const {
    return sqrt(2.0*dt*get_d());
  }
  void set_d_from_radius(unit::Angstrom radius,
                         unit::Kelvin t
#ifndef _MSC_VER
                         // it ICEs on this.
                         = IMP::internal::DEFAULT_TEMPERATURE
#endif
) {
    set_d_from_radius(unit::strip_units(radius),
                                   unit::strip_units(t));
  }
#endif
#endif

  double get_time_step_from_sigma(double sigma) {
    //s^2= 2*dt*D;
    unit::Femtosecond fs= square(unit::Angstrom(sigma))/2.0/get_d();
    return unit::strip_units(fs);
  }

  void set_d_from_radius() {
    set_d_from_radius(core::XYZR(get_particle()).get_radius());
  }

  //! Get the D key
  static FloatKey get_d_key();
};

IMP_OUTPUT_OPERATOR(Diffusion);


IMP_DECORATORS(Diffusion, Diffusions, core::XYZs);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_DIFFUSION_H */
