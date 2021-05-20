/**
 *  \file IMP/spb/RigidBodyPackingScore.h
 *  \brief A Score on the crossing angle between two rigid bodies
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_RIGID_BODY_PACKING_SCORE_H
#define IMPSPB_RIGID_BODY_PACKING_SCORE_H

#include <IMP/spb/spb_config.h>
#include <IMP/Pointer.h>
#include <IMP/PairScore.h>
#include <IMP/core/TableRefiner.h>
#include <IMP/pair_macros.h>

IMPSPB_BEGIN_NAMESPACE

//! Score on the crossing angles between two helices
class IMPSPBEXPORT RigidBodyPackingScore : public PairScore {
  IMP::PointerMember<core::TableRefiner> tbr_;
  Floats omb_, ome_, ddb_, dde_;
  double kappa_;

 public:
  RigidBodyPackingScore(core::TableRefiner *tbr, Floats omb, Floats ome,
                        Floats ddb, Floats dde, double kappa);

  virtual double evaluate_index(IMP::Model *m, const IMP::ParticleIndexPair &p,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;

  virtual IMP::ModelObjectsTemp do_get_inputs(
      IMP::Model *m, const IMP::ParticleIndexes &pis) const IMP_OVERRIDE;

  //  void show(std::ostream &out) const ;

  IMP_PAIR_SCORE_METHODS(RigidBodyPackingScore);
  IMP_SHOWABLE(RigidBodyPackingScore);
};

IMP_OBJECTS(RigidBodyPackingScore, RigidBodyPackingScores);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_RIGID_BODY_PACKING_SCORE_H */
