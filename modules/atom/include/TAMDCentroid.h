/**
 *  \file IMP/atom/TAMDCentroid.h
 *  \brief A decorator for a particle that is a centroid of TAMD simulations.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_TAMD_CENTROID_H
#define IMPATOM_TAMD_CENTROID_H

#include <IMP/atom/atom_config.h>

#include <IMP/Decorator.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/CentroidOfRefined.h>
#include <IMP/atom/Diffusion.h>
#include <IMP/kernel/internal/constants.h>

#include <vector>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a center of mass particle that is part of a TAMD
//! simulation (so simulators can recognize it as such)
/** \ingroup helper
    \ingroup decorators
    \see BrownianDynamicsBarak

    TAMDCentroid is almost identical to IMP::core::Centroid, used to
    decorate particles that represet centers of mass of their children.
    It also contains some flags that are useful for TAMD simulations
    (Temperature-Accelerated MD).

    \see IMP::core::Centroid
 */
class IMPATOMEXPORT TAMDCentroid : public IMP::core::Centroid  {
    /** Sets up TAMDCentroid over particles in pis */
    static void do_setup_particle(kernel::Model *m, ParticleIndex pi,
                                  const ParticleIndexes &pis) {
      Centroid::setup_particle(m, pi, pis);
      m->add_attribute(get_is_tamd_centroid_key(), pi, true);
    }

    /** Sets up TAMDCentroid over particles passed by applying the refiner
        over the particle pi
    */
    static void do_setup_particle(kernel::Model *m, ParticleIndex pi,
                                  Refiner *ref) {
      Centroid::setup_particle(m, pi, ref);
      m->add_attribute(get_is_tamd_centroid_key(), pi, true);
    }

   public:
    IMP_DECORATOR_METHODS(TAMDCentroid, IMP::core::Centroid);

    /** Sets up TAMDCentroid over members, and constrains TAMDCentroid to be
        computed before model evaluation and to propogate derivatives
        folllowing model evaulation.
        SetupDoc
    */
    IMP_DECORATOR_SETUP_1(TAMDCentroid, ParticleIndexesAdaptor, members);

    /** Sets up TAMDCentroid over particles passed by applying the refiner
        over the particle pi, and constrains TAMDCentroid to be computed before
        model evaluation and to propogate derivatives following model
        evaluation.
        SetupDoc
    */
    IMP_DECORATOR_SETUP_1(TAMDCentroid, Refiner *, refiner);

    static bool get_is_setup(kernel::Model *m, ParticleIndex pi) {
      return m->get_has_attribute(get_is_tamd_centroid_key(), pi);
    }

 private:
    /** Returns whether this tamd centroid is marked as such. */
    bool get_is_tamd_centroid() const {
      return static_cast<bool>
        ( get_model()->get_attribute( get_is_tamd_centroid_key(),
                                      get_particle_index() ) );
    }

    void set_is_tamd_centroid(bool t) {
      get_model()->set_attribute( get_is_tamd_centroid_key(),
                                  get_particle_index(),
                                  static_cast<Int>(t) );
  }

    //! Get the is_tamd_centroid key
    static IntKey get_is_tamd_centroid_key();

};

IMP_DECORATORS(TAMDCentroid, TAMDCentroids, IMP::core::Centroid);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_TAMD_CENTROID_H */
