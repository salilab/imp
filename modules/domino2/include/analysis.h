/**
 *  \file domino2/analysis.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO2_ANALYSIS_H
#define IMPDOMINO2_ANALYSIS_H

#include "domino2_config.h"
#include "SubsetState.h"
#include "Subset.h"

IMPDOMINO2_BEGIN_NAMESPACE
class ParticleStatesTable;
/** Return subset of the passed states which are representative.
    The states for each particle is clustered and then the
    passed states are reduced to only be on the centers
    of that clustering.

    The XYZ coordinates are used for clustering, there is not
    currently a nice mechanism to vary this, but it should be
    figured out. Probably by subclasses statistics::Embedding
    to statistics::ParticleEmbedding which has a method
    which takes a particle.
 */
IMPDOMINO2EXPORT SubsetStates get_state_clusters(const Subset &subset,
                                                 const SubsetStates &states,
                                                 ParticleStatesTable *pst,
                                                 double resolution);

IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_ANALYSIS_H */
