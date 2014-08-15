/**
 *  \file IMP/atom/CenterOfMass.h
 *  \brief A decorator for particles with mass
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_CENTER_OF_MASS_H
#define IMPATOM_CENTER_OF_MASS_H

#include <IMP/atom/atom_config.h>

#include <IMP/core/CentroidOfRefined.h>
#include <IMP/core/XYZ.h>
#include <IMP/atom/Mass.h>
#include <IMP/Decorator.h>
#include <IMP/decorator_macros.h>


IMPATOM_BEGIN_NAMESPACE

//! A particle that is the center of mass of other particles.
/** A decorator which constrains a particle to be the center of mass of a
    set of other particles, and whose mass is the sum of their masses.
    The c.o.m. is updated before model
    evaluation and its derivatives are copied to its children,
    using a constraint that is created at setup time. The derivatives
    propagated to each particle are scaled based on its mass relative
    to the total mass.

    \usesconstraint
*/
class IMPATOMEXPORT CenterOfMass : public IMP::Decorator {
  IMP_CONSTRAINT_DECORATOR_DECL(CenterOfMass);
 private:
  /** Sets up CenterOfMass over particles in pis.
      pi is decorated with core::XYZ and atom::Mass decorators, its
      coordinates are set to the current center of mass of pis, and
      its mass is set to the sum of their masses.
   */
  static void do_setup_particle(kernel::Model *m, ParticleIndex pi,
                                const ParticleIndexes &pis);

  /** Sets up CenterOfMass over particles passed by applying the refiner
      over the particle pi. pi is set to have core::XYZ and atom::Mass
      decorators, its coordinates are set to the current center of
      mass of ref->get_refined_indexes(m, pi), and its mass is set to
      the sum of their masses.
  */
  static void do_setup_particle(kernel::Model *m, ParticleIndex pi,
                                Refiner *ref);

 public:
    Float get_mass() const {
      IMP::atom::Mass pm( get_particle() );
      return pm.get_mass();
    }

    Float get_coordinate(int i) const {
      IMP::core::XYZ pxyz( get_particle() );
      return pxyz.get_coordinate(i);
    }

    const algebra::Vector3D &get_coordinates() const {
      IMP::core::XYZ pxyz( get_particle() );
      return pxyz.get_coordinates();
    }


   public:
  IMP_DECORATOR_METHODS(CenterOfMass, IMP::Decorator);

    /** Sets up CenterOfMass over members, and constrains CenterOfMass to be
        computed before model evaluation and to propagate derivatives
        following model evaluation.

        pi is decorated with core::XYZ and atom::Mass decorators, its
        coordinates are set to the current center of mass of pis, and
        its mass is set to the sum of their masses.
    */
    IMP_DECORATOR_SETUP_1(CenterOfMass, ParticleIndexesAdaptor, members);

    /** Sets up CenterOfMass over particles passed by applying the refiner
        over the particle pi, and constrains CenterOfMass to be computed before
        model evaluation and to propagate derivatives following model
        evaluation.

        pi is decorated with the core::XYZ and atom::Mass decorators,
        its coordinates are set to the current center of mass of
        refiner->get_refined_indexes(m, pi), and its mass is set to
        the sum of their masses.
    */
    IMP_DECORATOR_SETUP_1(CenterOfMass, Refiner *, refiner);

    static bool get_is_setup(kernel::Model *m, ParticleIndex pi) {
      return m->get_has_attribute(get_constraint_key(), pi) &&
        IMP::atom::Mass::get_is_setup(m, pi) &&
        IMP::core::XYZ::get_is_setup(m, pi);    }



    IMP_NO_DOXYGEN(typedef boost::false_type DecoratorHasTraits);

  };


IMP_DECORATORS( CenterOfMass, CenterOfMasses, UNUSED );

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_CENTER_OF_MASS_H */
