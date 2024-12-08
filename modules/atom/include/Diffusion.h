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
    inherits from XYZ) and a translational diffusion coefficient D.
    D is specified in units of \f$A^2/fs\f$, and it is used by eg 
    IMP's Brownian dynamics simulator. It can be set explicitly or inferred
    implicitly from the radius.

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
     If the particle does not have coordinates, it is decorated as XYZ
     and its coordinates are set to [0,0,0], otherwise the coordinates
     remain the same.
   */
  IMP_DECORATOR_SETUP_1(Diffusion, Float, D);
  //! Setup the particle with the specified coordinates and diffusion coefficient
  IMP_DECORATOR_SETUP_2(Diffusion, algebra::Vector3D, v, Float, D);
  //! Setup the particle with a diffusion coefficient automatically
  //! inferred from its radius using the Stokes-Einstein equation.
  /** 
      The diffusion coefficient is computed implicitly using the
      Stokes-Einstein equation by calling
      IMP::atom::get_einstein_diffusion_coefficient(R), where the
      Stokes radius R is core::XYZR::get_radius() and the temperature
      is the default IMP temperature of 297.15K.

      \note If the simulation temperature or particle radius change,
      the diffusion coefficient must be changed explicitly e.g., using
      set_diffusion_coefficient()
      \note This constructor can be used only on particles that have
            a radius field, e.g. decorated as IMP::core::XYZR.
  */
  IMP_DECORATOR_SETUP_0(Diffusion);

  //! Return true if the particle is an instance of Diffusion
  static bool get_is_setup(Model *m, ParticleIndex p) {
    return m->get_has_attribute(get_diffusion_coefficient_key(), p) &&
      XYZ::get_is_setup(m, p);
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

/** A rigid body that is diffusing, so it also has coordinates and a
    rotational diffusion coefficient. The units on the rotational
    coefficient are \f$radians^2/fs\f$.

    The translational and rotational diffusion coefficients are computed
    automatically using
    IMP::atom::get_einstein_diffusion_coefficient(radius) and
    IMP::atom::get_einstein_rotational_diffusion_coefficient(radius)
    for radius IMP::core::XYZR(m, pi).get_radius() and default IMP
    temperature of 297.15, but they can be overridden explicitly.

    Note that different translational and rotational coefficients should probably 
    be set manually if the temperature is not the default IMP temperature.

    \see Diffusion
    \see BrownianDynamics
*/
class IMPATOMEXPORT RigidBodyDiffusion : public Diffusion {
  static void do_setup_particle(Model *m, ParticleIndex pi);

 public:
  IMP_DECORATOR_METHODS(RigidBodyDiffusion, Diffusion);
  //! Setup this particle with automatically inferred translation and
  //! rotational diffusion coefficients
  /**
      The diffusion coefficients are computed using
      core::XYZR::get_radius() for the Stokes radius and the
      default IMP temperature of 297.15K.

      \note If the simulation temperature or particle radius change,
      the diffusion coefficient must be changed explicitly e.g., using
      set_diffusion_coefficient()
      \note The particle must have been decorated with core::XYZR
            for this constructor to be used
     The diffusion coefficients are inferred automatically from the particle's radius
     using IMP::atom::get_einstein_diffusion_coefficient(radius) and
     IMP::atom::get_einstein_rotational_diffusion_coefficient(radius).
  */
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

  //! Return true if the particle is an instance of an RigidBodyDiffusion
  static bool get_is_setup(Model *m, ParticleIndex p) {
    return m->get_has_attribute(get_rotational_diffusion_coefficient_key(), p) &&
      Diffusion::get_is_setup(m, p);
  }

  //! Get the D key
  static FloatKey get_rotational_diffusion_coefficient_key();
};

IMP_DECORATORS(RigidBodyDiffusion, RigidBodyDiffusions, Diffusions);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_DIFFUSION_H */
