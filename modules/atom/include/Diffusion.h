/**
 *  \file IMP/atom/Diffusion.h
 *  \brief A decorator for a diffusing particle.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_DIFFUSION_H
#define IMPATOM_DIFFUSION_H

#include <IMP/atom/atom_config.h>

#include <IMP/core/XYZR.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/internal/constants.h>

#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a diffusing particle with a diffusion coefficient.
/** Diffusion is used to decorate a particle with XYZ coordinates (since it
    inhertis from XYZ) and a translational diffusion coefficient D.
    D is specified in units of \f$A^2/fs\f$, and it is used by eg 
    IMP's Brownian dynamics simulator.

    \ingroup helper
    \ingroup decorators
    \include Diffusion_decorator.py
    \see RigidBodyDiffusion
    \see BrownianDynamics
 */
class IMPATOMEXPORT Diffusion : public IMP::core::XYZ {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                Float D) {
    if(!XYZ::get_is_setup(m,pi))
      {
	XYZ::setup_particle(m, pi, algebra::Vector3D(0,0,0));
      }
    m->add_attribute(get_diffusion_coefficient_key(), pi, D);
  }
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                const algebra::Vector3D &v, Float D) {
    if(XYZ::get_is_setup(m,pi))
      {
	XYZ(m,pi).set_coordinates(v);
      }
    else
      {
	XYZ::setup_particle(m, pi, v);
      }
    do_setup_particle(m, pi, D);
  }

  /**
     Same as do_setup_particle(m, pi, D) but D is automatically set
     to IMP::atom::get_einstein_diffusion_coefficient(radius) for radius
     IMP::core::XYZR(m, pi).get_radius(). 

     Note a different D should probably be set manually if the temperature is
     not the default IMP temperature (297.15).
   */
  static void do_setup_particle(Model *m, ParticleIndex pi);

 public:
  IMP_DECORATOR_METHODS(Diffusion, IMP::core::XYZ);
  //! Setup the particle with the specified diffusion coefficient
  /**
     Sets the diffusion coefficient to D. If the particle does not have
     coordinates, it is decorated as XYZ and its coordinates are set to
     [0,0,0], otherwise the coordinates remain the same.
   */
  IMP_DECORATOR_SETUP_1(Diffusion, Float, D);
  //! Setup the particle with the specified coordinates and diffusion coefficient
  /**
      Sets the XYZ coordinates of this particle to v and the diffusion
      coefficient to D.
  */
  IMP_DECORATOR_SETUP_2(Diffusion, algebra::Vector3D, v, Float, D);
  //! Setup the particle with a diffusion coefficient automatically
  //! inferred from its radius using the Stokes-Einstein equation
  /** 
      Setup this particle with an automaticall-computed diffusion coefficient,
      computed using the Stokes-Einstein equation for a Stokes radius
      of core::XYZR(m, pi).get_radius()  at the default IMP temperature (297.15K).
      Note this default must change for different temperatures.
      
      \Note: The particle must have been decorated with core::XYZR. 
  */
  IMP_DECORATOR_SETUP_0(Diffusion);

  //! Return true if the particle is an instance of Diffusion
  static bool get_is_setup(Model *m, ParticleIndex p) {
    return m->get_has_attribute(get_diffusion_coefficient_key(), p);
  }
  //! set diffusion coefficient in \f$A^2/fs\f$
  void set_diffusion_coefficient(double d) {
    get_particle()->set_value(get_diffusion_coefficient_key(), d);
  
  } 
  //! get diffusion coefficient in \f$A^2/fs\f$
  double get_diffusion_coefficient() const {
    return get_model()->get_attribute(get_diffusion_coefficient_key(),
                                      get_particle_index());
  }
  //! Get the D key
  static FloatKey get_diffusion_coefficient_key();
};

IMPATOMEXPORT double get_diffusion_coefficient_from_cm2_per_second(double din);

IMP_DECORATORS(Diffusion, Diffusions, core::XYZs);

/** A rigid body that is diffusing, so it also has a rotation diffusion
    coefficient. The units on the rotational coefficient are
    \f$radians^2/fs\f$.

    The translational and rotational diffusion coefficients are set automatically
    using IMP::atom::get_einstein_diffusion_coefficient(radius) and 
    IMP::atom::get_einstein_rotational_diffusion_coefficient(radius) for radius
    IMP::core::XYZR(m, pi).get_radius(). 

    Note that different translational and rotational coefficients should probably 
    be set manually if the temperature is not the default IMP temperature.
*/
class IMPATOMEXPORT RigidBodyDiffusion : public Diffusion {
  static void do_setup_particle(Model *m, ParticleIndex pi);

 public:
  IMP_DECORATOR_METHODS(RigidBodyDiffusion, Diffusion);
  //! All diffusion coefficients are determined from the radius
  IMP_DECORATOR_SETUP_0(RigidBodyDiffusion);

  //! returns the rotational diffusion coefficient in \f$radians^2/fs\f$
  double get_rotational_diffusion_coefficient() const {
    return get_model()->get_attribute
      (get_rotational_diffusion_coefficient_key(),
       get_particle_index());
  }


  //! sets the rotational diffusion coefficient in \f$radians^2/fs\f$
  void set_rotational_diffusion_coefficient(double d) const {
    return get_particle()->set_value(get_rotational_diffusion_coefficient_key(),
                                     d);
  }

  //! Return true if the particle is an instance of an Diffusion
  static bool get_is_setup(Model *m, ParticleIndex p) {
    return m->get_has_attribute(get_rotational_diffusion_coefficient_key(), p);
  }

  //! Get the D key
  static FloatKey get_rotational_diffusion_coefficient_key();
};

IMP_DECORATORS(RigidBodyDiffusion, RigidBodyDiffusions, Diffusions);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DIFFUSION_H */
