/**
 *  \file IMP/npc/SlabWithPore.h
 *  \brief A decorator for a particle that's a slab with a pore.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPC_SLAB_WITH_PORE_H
#define IMPNPC_SLAB_WITH_PORE_H

#include "npc_config.h"
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPNPC_BEGIN_NAMESPACE

//! A decorator for a particle that represents a slab containing
//! a cylindrical pore
/** \ingroup helper
    \ingroup decorators
 */
class IMPNPCEXPORT SlabWithPore
: public IMP::Decorator
{
  /** Decorate a particle that represents a slab (e.g. nuclear
      envelope) with specified thickness and a cylindrical pore of
      specified radius. Note that the radius is controlled by set_radius()
      as any other XYZR particle, but the XYZ coordinates are ignored for now
      (assumed to be 0,0,0).

      The slab is parallel to the x,y plain from z=-0.5*thickness to
      z=0.5*thickness, and the central axis of the pore lies on the
      origin.

      @param m the model
      @param pi the particle index
      @param thickness slab thickness
      @param pore_radius pore radius

      @note the pore radius is initially not optimizable
  */
  static void do_setup_particle(IMP::Model* m,
                                ParticleIndex pi,
				double thickness,
				double pore_radius);


 public:
  IMP_DECORATOR_METHODS(SlabWithPore, Decorator);


  /** Decorate a particle that represents a slab (e.g. nuclear
      envelope) with specified thickness and a cylindrical pore of
      specified pore_radius.

      The slab is parallel to the x,y plain from z=-0.5*thickness to
      z=0.5*thickness, and the central axis of the pore lies on the
      origin.

      @param m the model
      @param pi the particle index
      @param thickness slab thickness
      @param pore_radius pore radius

      @note the pore radius is initially not optimizable
  */
  IMP_DECORATOR_SETUP_2(SlabWithPore,
			double, thickness,
			double, pore_radius);

  //! Return true if the particle is an instance of SlabWithPore
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return m->get_has_attribute(get_pore_radius_key(), pi) &&
      m->get_has_attribute(get_thickness_key(), pi);
  }

  //! sets slab thickness
  void set_thickness(double thickness) {
    get_particle()->set_value(get_thickness_key(),
                              thickness);
  }

  //! returns whether the particle last entered the transport moiety from its
  //top
  Float get_thickness() const {
    return get_particle()->get_value(get_thickness_key());
  }

  //! get cylindrical pore radius
  Float get_pore_radius() const {
    return get_particle()->get_value(get_pore_radius_key());
  }

  //! set cylindrical pore radius
  void set_pore_radius(double r) const {
    get_particle()->set_value(get_pore_radius_key(), r);
  }

  //! add v to the derivative of the cylindrical pore radius,
  //! using derivative accumulator d
  void add_to_pore_radius_derivative(double v, DerivativeAccumulator &d) {
    get_particle()->add_to_derivative(get_pore_radius_key(), v, d);
  }

  bool get_pore_radius_is_optimized() const {
    return get_particle()->get_is_optimized(get_pore_radius_key());
  }
  //! Set whether the coordinates are optimized
  void set_pore_radius_is_optimized(bool tf) const {
    get_particle()->set_is_optimized(get_pore_radius_key(), tf);
  }

  //! Get the decorator key for is_last_entry_from_top
  static FloatKey get_thickness_key();

  //! Get the key for the pore radius.
  static FloatKey get_pore_radius_key();
};



IMP_DECORATORS(SlabWithPore, SlabsWithPores, IMP::Decorators);

IMPNPC_END_NAMESPACE

#endif /* IMPNPC_SLAB_WITH_PORE_H */
