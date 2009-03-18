/**
 *  \file core/DiffusionDecorator.h     \brief A
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_DIFFUSION_DECORATOR_H
#define IMPCORE_DIFFUSION_DECORATOR_H

#include "config.h"
#include "internal/utility.h"

#include <IMP/core/XYZDecorator.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/internal/constants.h>

#include <vector>
#include <limits>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

//! A a decorator for a diffusing particle.
/** \ingroup helper
    \ingroup decorators
    \see BrownianDynamics
    \deprecated Use atom::DiffusionDecorator instead
 */
class IMPCOREEXPORT DiffusionDecorator: public XYZDecorator
{
 public:
  IMP_DECORATOR(DiffusionDecorator, XYZDecorator)

  /** Create a decorator with the passed coordinates and D.
      D is assumed to be in cm2/sec.
  */
  static DiffusionDecorator create(Particle *p,
                             const algebra::Vector3D &v,
                                   Float D) {
    XYZDecorator::create(p, v);
    p->add_attribute(get_D_key(), D);
    return DiffusionDecorator(p);
  }

  /** Create a decorator with the a given D.
      D is assumed to be in cm2/sec and the particle
      is assumed to already have x,y,z attributes
  */
  static DiffusionDecorator create(Particle *p,
                                   Float D=0) {
    IMP_check(XYZDecorator::is_instance_of(p),
              "Particle must already be an XYZDecorator particle",
              ValueException);
    p->add_attribute(get_D_key(), D);
    return DiffusionDecorator(p);
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

  //! Return true if the particle is an instance of an DiffusionDecorator
  static bool is_instance_of(Particle *p) {
    return XYZDecorator::is_instance_of(p)
      && p->has_attribute(get_D_key());
  }

#ifndef SWIG
  unit::SquareCentimeterPerSecond get_D() const {
    return
      unit::SquareCentimeterPerSecond(get_particle()->get_value(get_D_key()));
  }
#endif

  //! Get the D key
  static FloatKey get_D_key();
};

IMP_OUTPUT_OPERATOR(DiffusionDecorator);

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_DIFFUSION_DECORATOR_H */
