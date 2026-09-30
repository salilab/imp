/**
 *  \file IMP/core/DihedralQuadScore.h
 *  \brief A Score on the dihedral angle between four particles.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_DIHEDRAL_QUAD_SCORE_H
#define IMPCORE_DIHEDRAL_QUAD_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/generic.h>
#include <IMP/QuadScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/quad_macros.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Apply a function to the dihedral angle between four particles.
/** */
class IMPCOREEXPORT DihedralQuadScore : public QuadScore {
  IMP::PointerMember<UnaryFunction> f_;

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<QuadScore>(this), f_);
  }
  IMP_OBJECT_SERIALIZE_DECL(DihedralQuadScore);

 public:
  //! Score the dihedral angle (in radians) using f
  DihedralQuadScore(UnaryFunction *f);
  DihedralQuadScore() {}
  virtual double evaluate_index(Model *m,
                                const ParticleIndexQuad &pi,
                                DerivativeAccumulator *da) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override;
  IMP_QUAD_SCORE_METHODS(DihedralQuadScore);
  IMP_OBJECT_METHODS(DihedralQuadScore);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DIHEDRAL_QUAD_SCORE_H */
