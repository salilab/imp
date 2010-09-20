/**
 *  \file domino2/utility.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO2_UTILITY_H
#define IMPDOMINO2_UTILITY_H

#include "domino2_config.h"
#include "SubsetState.h"
#include "Subset.h"
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>
#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/core/internal/CoreClosePairContainer.h>
#include <IMP/display/Writer.h>


IMP_BEGIN_NAMESPACE
class Model;
class Particle;
class Object;
IMP_END_NAMESPACE

IMPDOMINO2_BEGIN_NAMESPACE

/** \name Debug tools

    We provide a number of different functions for helpering
    optimize and understand domino-based sampling. These functions
    are expose part of the implementation and are liable to change
    without notice.
    @{
 */

/** Returns the subset of particles that depend on p as input. This
    will include p.

    \note This function is here to aid in debugging of optimization
    protocols that use Domino2. As a result, its signature and
    functionality may change without notice.
 */
IMPDOMINO2EXPORT ParticlesTemp get_dependent_particles(Particle *p);

/** \copydoc get_dependent_particles(Particle*)
 */
IMPDOMINO2EXPORT ParticlesTemp
get_dependent_particles(Particle *p,
                        const DependencyGraph &dg);


class ParticleStatesTable;
/** A static container is one such as a container::ListSingletonContainer, whose
    contents will not change as optimized particles are moved around. More
    technically, it is one which either is not written by any score state or,
    any score state which writes it does not depend on optimized particles.
 */
IMPDOMINO2EXPORT bool
get_is_static_container(Container *c,
                        const DependencyGraph &dg,
                        const ParticlesTemp &particles);


/** Load the appropriate state for each particle in a Subset. */
IMPDOMINO2EXPORT void load_particle_states(const Subset &s,
                                           const SubsetState &ss,
                                           const ParticleStatesTable *pst);



/** Return a list of all restraints from rs that
    - do not depend on any particle in pst->get_particles() that is not in s
    The dependency graph is passed for efficiency.
*/
IMPDOMINO2EXPORT RestraintsTemp get_restraints(const Subset &s,
                                               const ParticleStatesTable *pst,
                                               const DependencyGraph &dg,
                                               RestraintSet *rs);

/** @} */


IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_UTILITY_H */
