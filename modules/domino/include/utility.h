/**
 *  \file domino/utility.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO_UTILITY_H
#define IMPDOMINO_UTILITY_H

#include "domino_config.h"
#include "Assignment.h"
#include "Subset.h"
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>
#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/core/internal/CoreClosePairContainer.h>
#include <IMP/display/Writer.h>
#include <IMP/dependency_graph.h>

#ifdef IMP_DOMINO_USE_IMP_RMF
#include <IMP/rmf/HDF5Group.h>
#endif


IMP_BEGIN_NAMESPACE
class Model;
class Particle;
class Object;
IMP_END_NAMESPACE

IMPDOMINO_BEGIN_NAMESPACE

/** \name Debug tools

    We provide a number of different functions for helpering
    optimize and understand domino-based sampling. These functions
    are expose part of the implementation and are liable to change
    without notice.
    @{
 */

class ParticleStatesTable;


/** Load the appropriate state for each particle in a Subset. */
IMPDOMINOEXPORT void load_particle_states(const Subset &s,
                                           const Assignment &ss,
                                           const ParticleStatesTable *pst);



/** Return a list of all restraints from rs that
    - do not depend on any particle in pst->get_particles() that is not in s
    The dependency graph is passed for efficiency.
*/
IMPDOMINOEXPORT RestraintsTemp get_restraints(const Subset &s,
                                               const ParticleStatesTable *pst,
                                               const DependencyGraph &dg,
                                               RestraintSet *rs);


/** @} */


/** If the passed particles are all contained in the Subset and are
    not contained any of the Subsets in excluded, then return a a list
    of indices given the location of each passed particle in the passed subset.
    That is
    \code
    particles[i]==subset[returned[i]];
    \endcode
    Otherwise return an empty list.

    This function is designed to be used for implementing SubsetFilterTable
    classes.
*/
IMPDOMINOEXPORT Ints get_index(const ParticlesTemp &particles,
                               const Subset &subset, const Subsets &excluded);

/** All of the passed particles are not contained in an ofthe Subsets
    in excluded, then return a a list of indices given the location of
    each passed particle in the passed subset or -1 if it is missing.

    This function is designed to be used for implementing SubsetFilterTable
    classes.
*/
IMPDOMINOEXPORT Ints get_partial_index(const ParticlesTemp &particles,
                               const Subset &subset, const Subsets &excluded);


#if defined(IMP_DOMINO_USE_IMP_RMF) || defined(IMP_DOXYGEN)
class AssignmentContainer;

/** \name HDF5 I/O
    \anchor hdf5io
    Lists of assignments can be written to HDF5 data sets and read back. The
    passed list of particles is used to figure out the order when reading things
    back, it must match across setting and getting for the results to make
    sense.

    The dimension of the data set must be 2.
    @{
*/
/** The existing data set is completely rewritten.
    See IMP::rmf.
 */
IMPDOMINOEXPORT void save_assignments(AssignmentContainer *ac,
                                      const Subset &s,
                                      const ParticlesTemp &all_particles,
                                      RMF::HDF5IndexDataSet2D dataset);
/** See \ref hdf5io "HDF5 I/O". */
IMPDOMINOEXPORT AssignmentContainer*
create_assignments_container(RMF::HDF5IndexDataSet2D dataset,
                             const Subset &s,
                             const ParticlesTemp &all_particles);
/** @} */
#endif

/** Return the list of interactions implied by the passed balls
    given the allowed positions specified by the ParticleStatesTable.
*/
IMPDOMINOEXPORT
ParticlePairsTemp get_possible_interactions(const ParticlesTemp &ps,
                                            double max_distance,
                                            ParticleStatesTable *pst);


/** Return a list of indexes into s, representing a permutation of the
    particles in s, so that they are ordered according to all_particles.
    This order can be used to write s to disk, as the order in s can
    change between domino runs.
*/
IMPDOMINOEXPORT Ints get_order(const Subset &s,
                               const ParticlesTemp &all_particles);
IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_UTILITY_H */
