/**
 *  \file helper/covers.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPHELPER_COVERS_H
#define IMPHELPER_COVERS_H

#include "config.h"
#include "internal/version_info.h"
#include <IMP/core/XYZR.h>

IMP_BEGIN_NAMESPACE
class ScoreState;
class SingletonContainer;
class Refiner;
IMP_END_NAMESPACE

IMPHELPER_BEGIN_NAMESPACE

//! Set up a set of particles as covers of their refined particles
/** This method creates the score stated needed to maintain the cover
    and to propagate derivatives from the cover particle to the
    constituent particles. The resulting XYZR particles' location is
    not optimized. Add the score state to the model to enforce
    maintain the cover.

    \relatesalso core::CoverRefined
 */
IMPHELPEREXPORT ScoreState* create_covers(SingletonContainer *sc,
                                Refiner *pr,
                 FloatKey radius_key= core::XYZR::get_default_radius_key(),
                                Float slack=0);


//! Setup a particle to be the cetroid of a set of aprticles
/** This method creates the score state need to maintain the cover and
    to propagate derivatives from the cover particle to the
    constituent particles. The resulting XYZR particle's location is
    not optimized. Add the score state to the model to enforce
    maintain the cover.

    \relatesalso core::CoverRefined
 */
IMPHELPEREXPORT ScoreState* create_cover(Particle *p,
                                       Refiner *pr,
                  FloatKey radius_key= core::XYZR::get_default_radius_key(),
                                       Float slack=0);



//! Set up a set of particles as centroids of their refined constituents
/** This method creates the score stated needed to make things work
    and to propagate derivatives from the cover particle to the
    constituent particles. Add the ScoreState to the model to maintain
    the centroid.
    \relatesalso core::CentroidOfRefined
 */
IMPHELPEREXPORT ScoreState* create_centroids(SingletonContainer *sc,
                                    Refiner *pr,
                                    FloatKey weight=FloatKey(),
                                    FloatKeys ks
                                    = core::XYZ::get_xyz_keys());



//! Initialize the particle to be the cetroid of a set of particles
/** This method creates the score state need to make things work and
    to propagate derivatives from the centroid particle to the
    constituent particles. Add the ScoreState to the model to maintain
    the centroid.

    \relatesalso core::CentroidOfRefined
 */
IMPHELPEREXPORT ScoreState* create_centroid(Particle *p,
                                   Refiner *pr,
                                   FloatKey weight=FloatKey(),
                                   FloatKeys ks
                                   = core::XYZ::get_xyz_keys());


IMPHELPER_END_NAMESPACE

#endif  /* IMPHELPER_COVERS_H */
