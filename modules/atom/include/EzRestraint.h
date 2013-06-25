/**
 *  \file EzRestraint.h
 *  \brief Ez potential. A statistical scoring function for atom proteins
 *
 *  Copyright 2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_EZ_RESTRAINT_H
#define IMPATOM_EZ_RESTRAINT_H

#include <IMP/atom/atom_config.h>
#include <IMP/Restraint.h>
#include <IMP/restraint_macros.h>
#include <IMP/Particle.h>
#include <IMP/UnaryFunction.h>
#include <IMP/base_types.h>
#include <string>

IMPATOM_BEGIN_NAMESPACE

//! Ez Potential Restraint
/** Ez, a Depth-dependent Potential for Assessing the Energies of
    Insertion of Amino Acid Side-chains into Membranes.
    Senes et al. J. Mol. Biol. (2007) 366, 436–448
 */
class IMPATOMEXPORT EzRestraint : public Restraint
{

 Particles ps_;
 UnaryFunctions ufs_;
 Floats get_parameters(std::string restype);
protected:
  virtual double
      unprotected_evaluate(DerivativeAccumulator *da) const IMP_OVERRIDE;
  virtual  ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
public:
  EzRestraint(Particles ps);

  IMP_OBJECT_METHODS(EzRestraint);
};


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_EZ_RESTRAINT_H */
