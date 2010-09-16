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

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

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

/** The class temporarily transforms the restraints,
    in a model to make it better suited for the
    DominoSampler. Transformations include
    - replacing certain container::PairsRestraint and
      container::SingletonRestraint objects by a set a set of
      core::PairRestraint/core::SingletonRestraint objects
    \throw ModelException if the model contains any non-static
    containers other than container::ClosePairContainer and its
    ilk. Examples include container::ConnectedPairContainer.
*/
class IMPDOMINO2EXPORT OptimizeRestraints {
  boost::ptr_vector<ScopedRemoveRestraint> removed_;
  boost::ptr_vector<ScopedRestraint> added_;
  Pointer<RestraintSet> m_;

  void optimize_model(RestraintSet *m, const ParticlesTemp &particles);
public:
  IMP_RAII(OptimizeRestraints, (RestraintSet *m,
                                const ParticlesTemp &particles), {},
           {
             m_=m;
             optimize_model(m, particles);
           },
           {
             if (m_&& m_->get_is_part_of_model()) {
               removed_.clear();
               added_.clear();
               m_=NULL;
             }
           });
};

/** The class temporarily transforms the
    containers in a model to make it better suited for the
    DominoSampler. Transformations include
    - replacing container::ClosePairContainer and
      core::ExcludedVolumeRestraint by a static interaction list based
      on the ParticleStatesTable.
    \throw ModelException if the model contains any non-static
    containers other than container::ClosePairContainer and its
    ilk. Examples include container::ConnectedPairContainer.
*/
class IMPDOMINO2EXPORT OptimizeContainers {
  core::internal::CoreClosePairContainers staticed_;
  Pointer<RestraintSet> m_;

  void optimize_model(RestraintSet *m, const ParticleStatesTable *pst);
  void unoptimize_model();
public:
  IMP_RAII(OptimizeContainers, (RestraintSet *m,
                                const ParticleStatesTable *pst), {},
           {
             m_=m;
             optimize_model(m, pst);
           },
           {
             if (m_ && m_->get_is_part_of_model()) {
               unoptimize_model();
               m_=NULL;
             }
           });
};
/**@} */

/** Load the appropriate state for each particle in a Subset. */
IMPDOMINO2EXPORT void load_particle_states(const Subset &s,
                                           const SubsetState &ss,
                                           const ParticleStatesTable *pst);



/** Create a restraint set containing all the restraints do not depend on
    particles in other but not in s. The restraint set is added to the model.*/
IMPDOMINO2EXPORT RestraintSet* create_restraint_set(const Subset &s,
                                                    ParticlesTemp other,
                                                    const DependencyGraph &dg,
                                                    RestraintSet *rs);
IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_UTILITY_H */
