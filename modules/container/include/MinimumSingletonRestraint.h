/**
 *  \file IMP/container/MinimumSingletonRestraint.h
 *  \brief Score based on the minimum score over a set of Singletons
 *
 *  WARNING This file was generated from MinimumNAMERestraint.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_MINIMUM_SINGLETON_RESTRAINT_H
#define IMPCONTAINER_MINIMUM_SINGLETON_RESTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/Restraint.h>
#include <IMP/SingletonScore.h>
#include <IMP/SingletonContainer.h>
#include <IMP/restraint_macros.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Score based on the min or max SingletonScore over a set
/** The score is evaluated for each of the VALUETYPE in the container
    and the value of the min or max n scores is used. That is,
    if n is 1, the value of the restraint is the value of the min or max
    score over the container.
 */
class IMPCONTAINEREXPORT MinimumSingletonRestraint
: public Restraint
{
  IMP::base::OwnerPointer<SingletonScore> f_;
  IMP::base::OwnerPointer<SingletonContainer> c_;
  unsigned int n_;
public:
  /** n is the number of LCMinimum scores to use.
   */
  MinimumSingletonRestraint(SingletonScore *f,
                                 SingletonContainerAdaptor c,
                                 unsigned int n=1,
                                 std::string name
                                 ="MinimumSingletonRestraint %1%");

  public:
   double unprotected_evaluate(IMP::DerivativeAccumulator *accum)
                 const IMP_OVERRIDE;
   IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
   IMP_OBJECT_METHODS(MinimumSingletonRestraint);;

  //! Set the number of lowest scores to use.
  void set_n(unsigned int n) { n_=n;}
#ifndef IMP_DOXYGEN
  Restraints do_create_current_decomposition() const IMP_OVERRIDE;
  double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                      double max) const IMP_OVERRIDE;
#endif
};


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_MINIMUM_SINGLETON_RESTRAINT_H */
