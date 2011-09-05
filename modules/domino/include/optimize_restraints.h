/**
 *  \file domino/optimize_restraints.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO_OPTIMIZE_RESTRAINTS_H
#define IMPDOMINO_OPTIMIZE_RESTRAINTS_H

#include "domino_config.h"
#include "Assignment.h"
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

IMPDOMINO_BEGIN_NAMESPACE

/** The class temporarily transforms the restraints,
    in a model to make it better suited for the
    DominoSampler. Transformations include
    - replacing certain container::PairsRestraint and
      container::SingletonsRestraint objects by a set a set of
      core::PairRestraint or core::SingletonRestraint objects.
*/
class IMPDOMINOEXPORT OptimizeRestraints {
  Pointer<Model> mm_;
  boost::ptr_vector<ScopedRemoveRestraint> removed_;
  boost::ptr_vector<ScopedRestraint> added_;
  boost::ptr_vector<ScopedScoreState> addeds_;
  boost::ptr_vector<ScopedRemoveScoreState> removeds_;
  Pointer<RestraintSet> m_;

  void optimize_model(RestraintSet *m, const ParticleStatesTable *particles);
public:
  IMP_RAII(OptimizeRestraints, (RestraintSet *m,
                                const ParticleStatesTable *pst), {},
           {
             m_=m;
             mm_= m->get_model();
             optimize_model(m, pst);
           },
           {
             if (m_&& m_->get_is_part_of_model()) {
               removed_.clear();
               added_.clear();
               m_=static_cast<RestraintSet*>(nullptr);
             }
           },);
};

/**@} */

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_OPTIMIZE_RESTRAINTS_H */
