/**
 *  \file CentroidOfRefined.h
 *  \brief Set the coordinates of the particle to be the centoid of the
 *  refined particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CENTROID_OF_REFINED_H
#define IMPCORE_CENTROID_OF_REFINED_H

#include "config.h"
#include "internal/version_info.h"

#include "XYZDecorator.h"
#include <IMP/ParticleRefiner.h>
#include <IMP/Pointer.h>
#include <IMP/SingletonModifier.h>


IMPCORE_BEGIN_NAMESPACE

//! Set the coordinates of the particle to be the centoid of the particles.
/** An exception is thrown if there are no refined particles. The weight
    FloatKey can be FloatKey and then all the weights will be equal.

    \see CoverRefined
    \see DerivativesToRefined
    \see DerivativesFromRefined
 */
class IMPCOREEXPORT CentroidOfRefined:
public SingletonModifier
{
  Pointer<ParticleRefiner> r_;
  FloatKeys ks_;
  FloatKey w_;
public:
  //! Set the keys ks to be the average of the refined particles.
  CentroidOfRefined(ParticleRefiner *r,
                                     FloatKey weight=FloatKey(),
                                     FloatKeys ks
                                      = XYZDecorator::get_xyz_keys());
  ~CentroidOfRefined();

  IMP_SINGLETON_MODIFIER(internal::version_info);
};


//! Set up a set of particles as centroids of their refined constituents
/** This method adds score states as needed to make things work and to propagate
    derivatives from the cover particle to the constituent particles.
    \relates CoverRefined
 */
IMPCOREEXPORT void setup_centroids(Model *m, SingletonContainer *sc,
                                   ParticleRefiner *pr,
                                   FloatKey weight=FloatKey(),
                                   FloatKeys ks
                                   = XYZDecorator::get_xyz_keys());



//! Create a particle which which is kept to be the centroid of the passed ones
/** This method adds score states as needed to make things work and to propagate
    derivatives from the centroid particle to the constituent particles.
    \relates CentroidOfRefined
 */
IMPCOREEXPORT Particle* create_centroid(Model *m, const Particles &p,
                                        FloatKey weight=FloatKey(),
                                        FloatKeys ks
                                        = XYZDecorator::get_xyz_keys());


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CENTROID_OF_REFINED_H */
