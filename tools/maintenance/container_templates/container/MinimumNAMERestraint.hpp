/**
 *  \file IMP/container/MinimumCLASSNAMERestraint.h
 *  \brief Score based on the minimum score over a set of CLASSNAMEs
 *
 *  BLURB
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_MINIMUM_HEADERNAME_RESTRAINT_H
#define IMPCONTAINER_MINIMUM_HEADERNAME_RESTRAINT_H

#include "container_config.h"
#include <IMP/Restraint.h>
#include <IMP/CLASSNAMEScore.h>
#include <IMP/CLASSNAMEContainer.h>
#include <IMP/restraint_macros.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Score based on the min or max CLASSNAMEScore over a set
/** The score is evaluated for each of the VALUETYPE in the container
    and the value of the min or max n scores is used. That is,
    if n is 1, the value of the restraint is the value of the min or max
    score over the container.
 */
class IMPCONTAINEREXPORT MinimumCLASSNAMERestraint
: public Restraint
{
  IMP::base::OwnerPointer<CLASSNAMEScore> f_;
  IMP::base::OwnerPointer<CLASSNAMEContainer> c_;
  unsigned int n_;
public:
  /** n is the number of LCMinimum scores to use.
   */
  MinimumCLASSNAMERestraint(CLASSNAMEScore *f,
                                 CLASSNAMEContainerAdaptor c,
                                 unsigned int n=1,
                                 std::string name
                                 ="MinimumCLASSNAMERestraint %1%");

  IMP_RESTRAINT(MinimumCLASSNAMERestraint);

  //! Set the number of lowest scores to use.
  void set_n(unsigned int n) { n_=n;}
#ifndef IMP_DOXYGEN
  Restraints do_create_current_decomposition() const;
  double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                      double max) const;
#endif
};


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_MINIMUM_HEADERNAME_RESTRAINT_H */
