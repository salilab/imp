/**
 *  \file IMP/atom/EzRestraint.h
 *  \brief Ez potential. A statistical scoring function for atom proteins
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_EZ_RESTRAINT_H
#define IMPATOM_EZ_RESTRAINT_H

#include <IMP/atom/atom_config.h>
#include <IMP/kernel/Restraint.h>
#include <IMP/restraint_macros.h>
#include <IMP/kernel/Particle.h>
#include <IMP/UnaryFunction.h>
#include <IMP/base_types.h>
#include <string>

IMPATOM_BEGIN_NAMESPACE

//! Ez Potential kernel::Restraint
/** Ez, a Depth-dependent Potential for Assessing the Energies of
    Insertion of Amino Acid Side-chains into Membranes.
    Senes et al. J. Mol. Biol. (2007) 366, 436–448
 */
class IMPATOMEXPORT EzRestraint : public kernel::Restraint {

  kernel::Particles ps_;
  UnaryFunctions ufs_;
  Floats get_parameters(std::string restype);

 protected:
  virtual double unprotected_evaluate(DerivativeAccumulator *da) const
      IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

 public:
  EzRestraint(kernel::Particles ps);

  IMP_OBJECT_METHODS(EzRestraint);
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_EZ_RESTRAINT_H */
