/**
 *  \file IMP/core/DihedralRestraint.h
 *  \brief Dihedral restraint between four particles.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_DIHEDRAL_RESTRAINT_H
#define IMPCORE_DIHEDRAL_RESTRAINT_H

#include <IMP/core/core_config.h>

#include <IMP/UnaryFunction.h>
#include <IMP/Restraint.h>
#include <IMP/Particle.h>
#include <IMP/generic.h>

IMPCORE_BEGIN_NAMESPACE

//! Dihedral restraint between four particles
class IMPCOREEXPORT DihedralRestraint : public Restraint {
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

  virtual double unprotected_evaluate(IMP::DerivativeAccumulator* accum)
      const IMP_OVERRIDE;
  virtual IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DihedralRestraint);

 private:
  IMP::PointerMember<UnaryFunction> score_func_;
  ParticleIndex p_[4];
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DIHEDRAL_RESTRAINT_H */
