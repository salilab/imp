/**
 *  \file MyRestraint.h   \brief A restraint on a list of particle pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMODULENAME_MY_RESTRAINT_H
#define IMPMODULENAME_MY_RESTRAINT_H

#include "config.h"
#include "internal/modulename_version_info.h"
#include <IMP/SingletonScore.h>
#include <IMP/Restraint.h>
#include <IMP/PairContainer.h>

IMPMODULENAME_BEGIN_NAMESPACE

//! Apply a PairScore to a list of particle pairs
/**
 */
class IMPMODULENAMEEXPORT MyRestraint : public Restraint
{
  Pointer<PairContainer> pc_;
  Pointer<PairScore> f_;
public:
  //! Create the restraint.
  MyRestraint(PairScore* score_func,
              PairContainer *pc);
  virtual ~MyRestraint() {}

  IMP_RESTRAINT(internal::modulename_version_info)

  ParticlesList get_interacting_particles() const;
};

IMPMODULENAME_END_NAMESPACE

#endif  /* IMPCORE_DISTANCE_RESTRAINT_H */

/**
 *  \file MyRestraint.cpp \brief Restrain a list of particle pairs.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/modulename/MyRestraint.h>
#include <IMP/log.h>

IMPMODULENAME_BEGIN_NAMESPACE

MyRestraint::MyRestraint(PairScore* score_func,
                         PairContainer *pc) : pc_(pc),
                                           f_(score_func) {}

PairsRestraint::~PairsRestraint() {}

Float PairsRestraint::evaluate(DerivativeAccumulator *accum)
{
  Float score=0;
  for (PairContainer::ParticlePairIterator
       it= pc_->particle_pairs_begin();
       it != pc_->particle_pairs_end(); ++it) {
    score += f_->evaluate(it->first, it->second, accum);
  }

  return score;
}

// the interacting sets are pairs
ParticlesList PairsRestraint::get_interacting_particles() const
{
  ParticlesList ret;
  for (PairContainer::ParticlePairIterator it
       = pc_->particle_pairs_begin();
       it != pc_->particle_pairs_end(); ++it) {
    Particles s;
    s.push_back(it->first);
    s.push_back(it->second);
    ret.push_back(s);
  }
  return ret;
}

void PairsRestraint::show(std::ostream& out) const
{
  out << "ContainerRestraint with score function ";
  ss_->show(out);
  out << " and container ";
  pc_->show(out);
  out << std::endl;
}

IMPMODULENAME_END_NAMESPACE
