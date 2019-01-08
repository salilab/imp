/**
 * \file direction_helpers.h
 * \brief Helper functions for directions.
 *
 * Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_DIRECTION_HELPERS_H
#define IMPSCORE_FUNCTOR_DIRECTION_HELPERS_H

#include <IMP/score_functor/score_functor_config.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>

IMPSCOREFUNCTOR_BEGIN_INTERNAL_NAMESPACE

//! Get key for direction attribute at coordinate i.
IMPSCOREFUNCTOREXPORT FloatKey get_direction_key(unsigned int i);

//! Get value of direction attribute of particle.
IMPSCOREFUNCTOREXPORT algebra::Vector3D get_direction(Model *m,
                                                      ParticleIndex pi);

IMPSCOREFUNCTOR_END_INTERNAL_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_DIRECTION_HELPERS_H */
