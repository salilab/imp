/**
 *  \file IMP/core/DihedralRestraint.h
 *  \brief Dihedral restraint between four particles.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_DIHEDRAL_RESTRAINT_H
#define IMPCORE_DIHEDRAL_RESTRAINT_H

#include <IMP/core/core_config.h>
#include <IMP/core/DihedralQuadScore.h>

#include <IMP/core/QuadRestraint.h>
#include <IMP/UnaryFunction.h>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Dihedral restraint between four particles
/** \see DihedralQuadScore
 */
class IMPCOREEXPORT DihedralRestraint : public QuadRestraint {
  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<QuadRestraint>(this));
  }

  IMP_OBJECT_SERIALIZE_DECL(DihedralRestraint);

 public:
  //! Create the dihedral restraint.
  /** \param[in] m Model.
      \param[in] score_func Scoring function for the restraint.
      \param[in] p1 First particle in dihedral restraint.
      \param[in] p2 Second particle in dihedral restraint.
      \param[in] p3 Third particle in dihedral restraint.
      \param[in] p4 Fourth particle in dihedral restraint.
   */
  DihedralRestraint(Model *m, UnaryFunction* score_func,
                    ParticleIndexAdaptor p1,
                    ParticleIndexAdaptor p2,
                    ParticleIndexAdaptor p3,
                    ParticleIndexAdaptor p4);
  DihedralRestraint() {}

  IMP_OBJECT_METHODS(DihedralRestraint);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DIHEDRAL_RESTRAINT_H */
