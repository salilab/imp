/**
 *  \file IMP/core/TruncatedTorus.h
 *  \brief A decorator for a particle with a truncated torus representation.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_TRUNCATED_TORUS_H
#define IMPCORE_TRUNCATED_TORUS_H

#include "core_config.h"
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPCORE_BEGIN_NAMESPACE

//! A decorator for a particle with a truncated torus representation
/** \ingroup helper
    \ingroup decorators
 */
class IMPCOREEXPORT TruncatedTorus
: public IMP::Decorator
{
  /** Decorate a particle to represent a truncated torus with shape parameters
      $R$, $r$ and $\Theta$. 
      At its truncated ends, it is implicitly assumed that the torus is capped
      by half spheres of radius r (though the actual effect of capping depends
      on implementation of classed that rely on this decorator)
      
      @param m the model
      @param pi the particle index
      @param R the  major radius of the torus, from the center of its
               hole to the center of its tube
      @param r is the minor radius of the torus, from the center of its tube
               to the surface of its tube.
      @param theta the angle in radians of the truncated torus slice, following an
             arc of the same angle along its tube.
  */
  static void do_setup_particle(IMP::Model* m,
                                ParticleIndex pi,
				double R,
				double r,
                                double theta);


 public:
  IMP_DECORATOR_METHODS(TruncatedTorus, Decorator);


  /** Decorate a particle that represents a truncated torus. 

      @param R the  major radius of the torus, from the center of its
               hole to the center of its tube
      @param r is the minor radius of the torus, from the center of its tube
               to the surface of its tube.
      @param theta the angle in radians of the truncated torus slice, following an
             arc of the same angle along its tube.
  */
  IMP_DECORATOR_SETUP_3(TruncatedTorus,
			double, R,
			double, r,
			double, theta);

  //! Return true if the particle is an instance of TruncatedTorus
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return 
      m->get_has_attribute(get_major_radius_key(), pi) &&
      m->get_has_attribute(get_minor_radius_key(), pi) &&
      m->get_has_attribute(get_theta_key(), pi); 
  }

  //! return the distance from the origin (center of the torus hole) to the
  //! central axis of its tube
  double get_major_radius() const{
    return get_particle()->get_value(get_major_radius_key());
  }

  //! set the distance from the origin (center of the torus hole) to the
  //! central axis of its tube
  void set_major_radius(double R) {
    get_particle()->set_value(get_major_radius_key(), R);
  }

  //! return the distance from the central axis of the torus tube to its surface
  double get_minor_radius() const{
    return get_particle()->get_value(get_minor_radius_key());
  }

  //! set the distance from the central axis of the torus tube to its surface
  void set_minor_radius(double r) {
    get_particle()->set_value(get_minor_radius_key(), r);
  }

  //! return the angle in radians of the truncated torus slice
  double get_theta() const{
    return get_particle()->get_value(get_theta_key());
  }

  //! set the distance from the center of the torus hole to the center of its tube
  void set_theta(double theta) {
    get_particle()->set_value(get_theta_key(), theta);
  }
  
  static FloatKey get_major_radius_key();

  static FloatKey get_minor_radius_key();

  static FloatKey get_theta_key();

};



IMP_DECORATORS(TruncatedTorus, TruncatedTori, IMP::Decorator);

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_TRUNCATED_TORUS_H */
