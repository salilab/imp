/**
 *  \file IMP/domino/analysis.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO_ANALYSIS_H
#define IMPDOMINO_ANALYSIS_H

#include <IMP/domino/domino_config.h>
#include "Assignment.h"
#include "Subset.h"

IMPDOMINO_BEGIN_NAMESPACE
class ParticleStatesTable;
//! Return subset of the passed states which are representative.
/** The states for each particle is clustered and then the
    passed states are reduced to only be on the centers
    of that clustering.

    The XYZ coordinates are used for clustering, there is not
    currently a nice mechanism to vary this, but it should be
    figured out. Probably by subclasses statistics::Embedding
    to statistics::ParticleEmbedding which has a method
    which takes a particle.
 */
IMPDOMINOEXPORT Assignments
    get_state_clusters(const Subset &subset, const Assignments &states,
                       ParticleStatesTable *pst, double resolution);

IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_ANALYSIS_H */
