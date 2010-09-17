/**
 *  \file domino2/optimize_restraints.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO2_OPTIMIZE_RESTRAINTS_H
#define IMPDOMINO2_OPTIMIZE_RESTRAINTS_H

#include "domino2_config.h"
#include "SubsetState.h"
#include "Subset.h"
#include "particle_states.h"
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>
#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/core/internal/CoreClosePairContainer.h>
#include <IMP/display/Writer.h>
#include <boost/ptr_container/ptr_vector.hpp>

IMP_BEGIN_NAMESPACE
class Model;
class Particle;
class Object;
IMP_END_NAMESPACE

IMPDOMINO2_BEGIN_NAMESPACE

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
                                const ParticleStatesTable *pst), {},
           {
             m_=m;
             optimize_model(m, pst->get_particles());
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

IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_OPTIMIZE_RESTRAINTS_H */
