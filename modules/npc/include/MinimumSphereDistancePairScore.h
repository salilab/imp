/**
 *  \file IMP/npc/MinimumSphereDistancePairScore.h
 *  \brief Apply a UnaryFunction to the minimum transformed distance
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPNPC_MINIMUM_SPHERE_DISTANCE_PAIR_SCORE_H
#define IMPNPC_MINIMUM_SPHERE_DISTANCE_PAIR_SCORE_H

#include <IMP/npc/npc_config.h>
#include <IMP/PairScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/pair_macros.h>
#include <IMP/algebra/Transformation3D.h>

IMPNPC_BEGIN_NAMESPACE

//! Apply a UnaryFunction to the minimum transformed sphere-sphere distance
/** This is like a SphereDistancePairScore in that it calculates the
    inter-surface distance between two XYZR Particles (spheres) and then
    scores it with a provided UnaryFunction. However, it also considers
    the distance between the two particles with the second particle
    transformed by each of the provided transformations. The transformation
    which results in the minimum interparticle distance (not necessarily the
    minimum score) is then used for scoring. This can be used for periodic
    boundaries or other symmetric systems.
 */
class IMPNPCEXPORT MinimumSphereDistancePairScore : public PairScore {
  IMP::PointerMember<UnaryFunction> f_;
  algebra::Transformation3Ds transforms_;

 public:
  MinimumSphereDistancePairScore(UnaryFunction *f,
                                 algebra::Transformation3Ds transforms)
                 : f_(f), transforms_(transforms) {}

  virtual double evaluate_index(Model *m, const ParticleIndexPair &pi,
                                DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const IMP_OVERRIDE {
    return IMP::get_particles(m, pis);
  }
  IMP_PAIR_SCORE_METHODS(MinimumSphereDistancePairScore);
  IMP_OBJECT_METHODS(MinimumSphereDistancePairScore);
};

IMPNPC_END_NAMESPACE

#endif /* IMPNPC_MINIMUM_SPHERE_DISTANCE_PAIR_SCORE_H */
