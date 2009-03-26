/**
 *  \file CentroidOfRefined.h
 *  \brief Set the coordinates of the particle to be the centoid of the
 *  refined particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_CENTROID_OF_REFINED_H
#define IMPCORE_CENTROID_OF_REFINED_H

#include "config.h"
#include "internal/version_info.h"

#include "XYZDecorator.h"
#include <IMP/Refiner.h>
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
  Pointer<Refiner> r_;
  FloatKeys ks_;
  FloatKey w_;
public:
  //! Set the keys ks to be the average of the refined particles.
  CentroidOfRefined(Refiner *r,
                                     FloatKey weight=FloatKey(),
                                     FloatKeys ks
                                      = XYZDecorator::get_xyz_keys());
  ~CentroidOfRefined();

  IMP_SINGLETON_MODIFIER(internal::version_info);
};


//! Set up a set of particles as centroids of their refined constituents
/** This method creates the score stated needed to make things work
    and to propagate derivatives from the cover particle to the
    constituent particles. Add the ScoreState to the model to maintain
    the centroid.
    \relates CentroidOfRefined
 */
IMPCOREEXPORT ScoreState* create_centroids(SingletonContainer *sc,
                                    Refiner *pr,
                                    FloatKey weight=FloatKey(),
                                    FloatKeys ks
                                    = XYZDecorator::get_xyz_keys());



//! Initialize the particle to be the cetroid of a set of particles
/** This method creates the score state need to make things work and
    to propagate derivatives from the centroid particle to the
    constituent particles. Add the ScoreState to the model to maintain
    the centroid.

    \relates CentroidOfRefined
 */
IMPCOREEXPORT ScoreState* create_centroid(Particle *p,
                                   Refiner *pr,
                                   FloatKey weight=FloatKey(),
                                   FloatKeys ks
                                   = XYZDecorator::get_xyz_keys());


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_CENTROID_OF_REFINED_H */
