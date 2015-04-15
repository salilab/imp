/**
 *  \file IMP/core/MinimumRestraint.h
 *  \brief Score based on the k minimum restraints.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_MINIMUM_RESTRAINT_H
#define IMPCORE_MINIMUM_RESTRAINT_H

#include <IMP/core/core_config.h>
#include <IMP/Restraint.h>
#include <IMP/Model.h>
#include <IMP/macros.h>
#include <IMP/generic.h>

IMPCORE_BEGIN_NAMESPACE

//! Score based on the minimum scoring members of a set of restraints
/**
 */
class IMPCOREEXPORT MinimumRestraint : public Restraint {
  unsigned int k_;

 public:
  /** Score based on the num minimum restraints from rs.
   */
  MinimumRestraint(unsigned int num,
                   const Restraints &rs = Restraints(),
                   std::string name = "MinimumRestraint %1%");

  void clear_caches();

 public:
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const
      IMP_OVERRIDE;
  IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(MinimumRestraint);
  ;

  IMP_LIST_ACTION(public, Restraint, Restraints, restraint, restraints,
                  Restraint *, Restraints, , {}, {});
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_MINIMUM_RESTRAINT_H */
