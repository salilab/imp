/**
 *  \file IMP/container/MinimumQuadRestraint.h
 *  \brief Score based on the minimum score over a set of Quads
 *
 *  WARNING This file was generated from MinimumNAMERestraint.hpp
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_MINIMUM_QUAD_RESTRAINT_H
#define IMPCONTAINER_MINIMUM_QUAD_RESTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/Restraint.h>
#include <IMP/QuadScore.h>
#include <IMP/QuadContainer.h>
#include <IMP/restraint_macros.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Score based on the min or max QuadScore over a set
/** The score is evaluated for each of the VALUETYPE in the container
    and the value of the min or max n scores is used. That is,
    if n is 1, the value of the restraint is the value of the min or max
    score over the container.
 */
class IMPCONTAINEREXPORT MinimumQuadRestraint
: public Restraint
{
  IMP::base::OwnerPointer<QuadScore> f_;
  IMP::base::OwnerPointer<QuadContainer> c_;
  unsigned int n_;
public:
  /** n is the number of LCMinimum scores to use.
   */
  MinimumQuadRestraint(QuadScore *f,
                                 QuadContainerAdaptor c,
                                 unsigned int n=1,
                                 std::string name
                                 ="MinimumQuadRestraint %1%");

  IMP_RESTRAINT_2(MinimumQuadRestraint);

  //! Set the number of lowest scores to use.
  void set_n(unsigned int n) { n_=n;}
#ifndef IMP_DOXYGEN
  Restraints do_create_current_decomposition() const IMP_OVERRIDE;
  double unprotected_evaluate_if_good(DerivativeAccumulator *da,
                                      double max) const IMP_OVERRIDE;
#endif
};


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_MINIMUM_QUAD_RESTRAINT_H */
