/**
 *  \file RestraintSet.h     \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMP_RESTRAINT_SET_H
#define IMP_RESTRAINT_SET_H

#include "config.h"

#include "Restraint.h"
#include "Model.h"

#include <string>

IMP_BEGIN_NAMESPACE

//! Container used to hold a set of restraints
/** RestraintSets allow one to define a tree of restraints
    and to weight various restraints. Upon evaluation,
    all of the restraints in the RestraintSet are evaluated
    using the provided weight (weights are multiplicative
    when RestraintSets are nested).

    If the weight is 0, the restraints are not evaluated.

    \advanceddoc

    Talk to Daniel if you want to inherit from RestraintSet.
 */
class IMPEXPORT RestraintSet : public Restraint
{
public:
  //! Create an empty set
  RestraintSet(double weight,
               const std::string& name="RestraintSet %1%");

  //! Create an empty set
  RestraintSet(const std::string& name="RestraintSet %1%");

  IMP_RESTRAINT(RestraintSet, get_module_version_info())
 /** @name Methods to control the nested Restraint objects

     This container manages a set of Restraint objects. To
     manipulate the stored set use the methods below.
  */
  /**@{*/
    IMP_LIST(public, Restraint, restraint, Restraint*, Restraints);
  /**@}*/
 public:
  void set_weight(Float weight);
  Float get_weight() const { return weight_; }
 protected:
  void set_model(Model *m);
private:
  Float weight_;
};

IMP_END_NAMESPACE

#endif  /* IMP_RESTRAINT_SET_H */
