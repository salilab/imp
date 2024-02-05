/**
 *  \file IMP/npcassembly/SlabWithToroidalPore.h
 *  \brief A decorator for a particle that's a slab with a toroidal pore.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPCASSEMBLY_SLAB_WITH_TOROIDAL_PORE_H
#define IMPNPCASSEMBLY_SLAB_WITH_TOROIDAL_PORE_H

#include "npcassembly_config.h"
#include "SlabWithPore.h"
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>

IMPNPCASSEMBLY_BEGIN_NAMESPACE

//! A decorator for a particle that represents a slab containing
//! a toroidal pore
/** \ingroup helper
    \ingroup decorators
 */
class IMPNPCASSEMBLYEXPORT SlabWithToroidalPore
: public SlabWithPore
{
  /** Decorate a particle that represents a slab (e.g. nuclear
      envelope) with specified thickness and a toroidal pore of
      specified major radius and thickness/2.0 minor radius.
      Note that the radius is controlled by set_pore_radius()
      as any other XYZR particle, but the XYZ coordinates are ignored for now
      (assumed to be 0,0,0).

      The slab is parallel to the x,y plain from z=-0.5*thickness to
      z=0.5*thickness, and the central axis of the pore lies on the
      origin.

      @param m the model
      @param pi the particle index
      @param thickness slab thickness, also twice the minor_radius
      @param major_radius pore major radius
  */
  static void do_setup_particle(IMP::Model* m,
                                ParticleIndex pi,
				double thickness,
				double major_radius,
                                double minor_radius_h2v_aspect_ratio=1.0);


 public:
  IMP_DECORATOR_METHODS(SlabWithToroidalPore, SlabWithPore);


  /** Decorate a particle that represents a slab (e.g. nuclear
      envelope) with specified thickness and a toroidal pore of
      specified major radius and minor radius of 0.5*thickness,
      with equal horizontal and vertical minor radii.

      The slab is parallel to the x,y plain from z=-0.5*thickness to
      z=0.5*thickness, and the central axis of the pore lies on the
      origin.

      @param m the model
      @param pi the particle index
      @param thickness slab thickness, also twice the minor radius
      @param major_radius pore major radius
  */
  IMP_DECORATOR_SETUP_2(SlabWithToroidalPore,
			double, thickness,
			double, major_radius);

  /** Decorate a particle that represents a slab (e.g. nuclear
      envelope) with specified thickness and a toroidal pore of
      specified major radius and minor radius of 0.5*thickness,
      and specified ratio between horizontal and vertical minor
      radii.

      The slab is parallel to the x,y plain from z=-0.5*thickness to
      z=0.5*thickness, and the central axis of the pore lies on the
      origin.

      @param m the model
      @param pi the particle index
      @param thickness slab thickness, also twice the vertical
                        minor radius
      @param major_radius pore major radius
      @param minor_radius_h2v_aspect_ratio
         ratio between horizontal and vertical minor radius
  */
  IMP_DECORATOR_SETUP_3(SlabWithToroidalPore,
			double, thickness,
			double, major_radius,
                        double, minor_radius_h2v_aspect_ratio);

  //! Return true if the particle is an instance of SlabWithToroidalPore
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    return SlabWithPore::get_is_setup(m, pi) &&
      m->get_has_attribute(get_minor_radius_h2v_aspect_ratio_key(), pi) &&
      m->get_has_attribute(get_toroidal_pore_key(), pi);
  }

  void set_minor_radius_h2v_aspect_ratio(double aspect_ratio){
    get_particle()->set_value
      (get_minor_radius_h2v_aspect_ratio_key(),
       aspect_ratio);
  }

  double get_minor_radius_h2v_aspect_ratio() const{
    return get_particle()->get_value
      (get_minor_radius_h2v_aspect_ratio_key());
  }

  double get_vertical_minor_radius() const{
    return get_thickness()*0.5;
  }

  void set_vertical_minor_radius(double rv) {
    set_thickness(2.0*rv);
  }

  double get_horizontal_minor_radius() const{
    return get_vertical_minor_radius()*get_minor_radius_h2v_aspect_ratio();
  }

  void set_horizontal_minor_radius(double rh) {
    set_minor_radius_h2v_aspect_ratio
      ( rh/ get_vertical_minor_radius() );
  }

  //! Get the decorator key for the ratio between horizontal and vertical aspect ratio
  static FloatKey get_minor_radius_h2v_aspect_ratio_key();

  //! Get the decorator key indicating a toroidal pore
  static IntKey get_toroidal_pore_key();

};



IMP_DECORATORS(SlabWithToroidalPore, SlabsWithToroidalPores, IMP::SlabsWithPores);

IMPNPCASSEMBLY_END_NAMESPACE

#endif /* IMPNPCASSEMBLY_SLAB_WITH_TOROIDAL_PORE_H */
