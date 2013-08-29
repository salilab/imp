/**
 *  \file IMP/core/DihedralRestraint.h
 *  \brief Dihedral restraint between four particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_DIHEDRAL_RESTRAINT_H
#define IMPCORE_DIHEDRAL_RESTRAINT_H

#include <IMP/core/core_config.h>

#include <IMP/UnaryFunction.h>
#include <IMP/kernel/Restraint.h>
#include <IMP/Particle.h>
#include <IMP/generic.h>

IMPCORE_BEGIN_NAMESPACE

//! Dihedral restraint between four particles
class IMPCOREEXPORT DihedralRestraint : public kernel::Restraint {
 public:
  //! Create the dihedral restraint.
  /** \param[in] score_func Scoring function for the restraint.
      \param[in] p1 Pointer to first particle in dihedral restraint.
      \param[in] p2 Pointer to second particle in dihedral restraint.
      \param[in] p3 Pointer to third particle in dihedral restraint.
      \param[in] p4 Pointer to fourth particle in dihedral restraint.
   */
  DihedralRestraint(UnaryFunction* score_func, Particle* p1, Particle* p2,
                    Particle* p3, Particle* p4);

  virtual double unprotected_evaluate(
      IMP::kernel::DerivativeAccumulator* accum) const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DihedralRestraint);

 private:
  IMP::base::PointerMember<UnaryFunction> score_func_;
  IMP::base::PointerMember<Particle> p_[4];
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_DIHEDRAL_RESTRAINT_H */
