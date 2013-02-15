/**
 *  \file IMP/domino/utility.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO_UTILITY_H
#define IMPDOMINO_UTILITY_H

#include <IMP/domino/domino_config.h>
#include "Assignment.h"
#include "Subset.h"
#include "particle_states.h"
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>
#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/core/internal/CoreClosePairContainer.h>
#include <IMP/display/Writer.h>
#include <IMP/dependency_graph.h>
#include <IMP/statistics/metric_clustering.h>

#ifdef IMP_DOMINO_USE_IMP_RMF
#include <RMF/HDF5/Group.h>
#endif


IMPKERNEL_BEGIN_NAMESPACE
class Model;
class Particle;
IMPKERNEL_END_NAMESPACE

IMPDOMINO_BEGIN_NAMESPACE

class AssignmentsTable;
class AssignmentContainer;
class SubsetFilterTable;

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



/** Return the list of interactions implied by the passed balls
    given the allowed positions specified by the ParticleStatesTable.
*/
IMPDOMINOEXPORT
ParticlePairsTemp get_possible_interactions(const ParticlesTemp &ps,
                                            double max_distance,
                                            ParticleStatesTable *pst);

//! Return an embedding for an assignment
IMPDOMINOEXPORT algebra::VectorKD get_embedding(const Subset &s,
                                                const Assignment &a,
                                                ParticleStatesTable *pst);

//! Return the nearest assignment from an embedding
IMPDOMINOEXPORT Assignment
get_nearest_assignment(const Subset &s,
                       const algebra::VectorKD &embedding,
                       ParticleStatesTable *pst);


/** Return a distance between two assignments if they are less than
    a threshold. The distance returned is the l2 norm on the distances
    between each state as given by the corresponding metric. If no
    metric is passed, then the l2 norm on the embedding is used.
   \unstable{get_distance_if_close}
 */
inline double get_distance_if_smaller_than(const Subset &s,
                                           const Assignment &a,
                                           const Assignment &b,
                                           ParticleStatesTable *pst,
                                           const statistics::Metrics &metrics,
                                           double max) {
  IMP_USAGE_CHECK(a.size()==b.size(),
                  "Dimensions of embeddings don't match.");
  double d=0;
  for (unsigned int i=0; i< a.size(); ++i) {
    double cur;
    if (!metrics.empty() && metrics[i]) {
      cur= square(metrics[i]->get_distance(a[i], b[i]));
    } else {
      algebra::VectorKD ea
        = pst->get_particle_states(s[i])->get_embedding(a[i]);
      algebra::VectorKD eb
        = pst->get_particle_states(s[i])->get_embedding(b[i]);
      cur= (ea-eb).get_squared_magnitude();
    }
    d+= cur;
    if (d > square(max)) {
      IMP_LOG_VERBOSE( "Returning " << std::sqrt(d) << " > " << max
              << " for " << a << " and " << b
              << std::endl);
      return std::sqrt(d);
    }
  }
  IMP_LOG_VERBOSE( "Distance between " << a << " and "
          << b << " is " << std::sqrt(d) << std::endl);
  return std::sqrt(d);
}

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_UTILITY_H */
