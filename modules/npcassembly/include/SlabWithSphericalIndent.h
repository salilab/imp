/**
 *  \file IMP/npcassembly/SlabWithSphericalIndent.h
 *  \brief A decorator for a particle representing a slab with a spherical cap indent. 
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPCASSEMBLY_SLAB_WITH_SPHERICAL_INDENT_H
#define IMPNPCASSEMBLY_SLAB_WITH_SPHERICAL_INDENT_H

#include <IMP/npcassembly/npcassembly_config.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>
#include <IMP/exception.h>

IMPNPCASSEMBLY_BEGIN_NAMESPACE

//! A decorator for a particle that represents a slab containing a spherical cap indent. 
/** \ingroup helper
    \ingroup decorators
*/
class IMPNPCASSEMBLYEXPORT SlabWithSphericalIndent : public Decorator {
  /* Decorate a particle that represents a slab with an indent 
   * that is modeled as a spherical cap. The spherical cap model is
   * specified by two paramters, a radius, R, that is the radius of the
   * underlying sphere and a height, h, that is the distance along the
   * perpendicular from the intersecting plane to the edge of the sphere.  
   */ 
  static StringKey get_name_key();
  //! Add a name to the particle
  /** The create function should take arguments which allow
      the initial state of the Decorator to be reasonable (i.e.
      make sure there is a non-empty name).
   */
  static void do_setup_particle(Model *m,
                                ParticleIndex pi,
                                double R,
				                double h);

 public:
  //! return true if the particle has a R and h defined
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_sphere_radius_key(), pi) && m->get_has_attribute(get_sphere_depth_key(), pi);
  }

  // set radius
  void set_sphere_radius(double R) {
      get_particle()->set_value(get_sphere_radius_key(), R);
  }

  // set sphere depth
  void set_sphere_depth(double h) {
      get_particle()->set_value(get_sphere_depth_key(), h);
  }

  //! return the sphere radius
  Float get_sphere_radius() const {
      return get_particle()->get_value(get_sphere_radius_key());
  }

  //! return the sphere depth
  Float get_sphere_depth() const {
      return get_particle()->get_value(get_sphere_depth_key());
  }

  //Float get_base_circle_radius() const {
      //return get_particle()->get_value(get_base_circle_radius());
  //}

  //! get the decorator key for sphere radius
  static FloatKey get_sphere_radius_key();

  //! get the decorator key for the sphere depth
  static FloatKey get_sphere_depth_key();


  //! Get the name added to the particle
  std::string get_decorator_name() const {
    return get_particle()->get_value(get_name_key());
  }

  //! Set the name added to the particle
  void set_decorator_name(std::string nm) {
    // use the usage check macro to check that functions are called properly
    IMP_USAGE_CHECK(!nm.empty(), "The name cannot be empty");
    get_particle()->set_value(get_name_key(), nm);
  }

  /* Declare the basic constructors and the cast function using the standard macrcos.*/
  IMP_DECORATOR_METHODS(SlabWithSphericalIndent, Decorator);
  IMP_DECORATOR_SETUP_2(SlabWithSphericalIndent, double, R, double, h);


}; // end class

IMP_DECORATORS(SlabWithSphericalIndent, SlabWithSphericalIndents, IMP::Decorators);

IMPNPCASSEMBLY_END_NAMESPACE

#endif /* IMPNPCASSEMBLY_SLAB_WITH_SPHERICAL_INDENT_H */
