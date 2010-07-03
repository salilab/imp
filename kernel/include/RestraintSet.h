/**
 *  \file RestraintSet.h     \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_RESTRAINT_SET_H
#define IMP_RESTRAINT_SET_H

#include "kernel_config.h"
#include "Restraint.h"
#include "container_macros.h"
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

  IMP_RESTRAINT(RestraintSet);
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
  friend class Model;
  void set_model(Model *m);
private:
  Float weight_;
};

IMP_OBJECTS(RestraintSet, RestraintSets);

/** \name Gathering restraints
    It is sometimes useful to extract all the non-RestraintSet restraints
    from a hierarchy involving RestraintSets mixed with Restraints.
    @{
 */
typedef std::pair<RestraintsTemp, Floats> RestraintsAndWeights;

IMPEXPORT RestraintsAndWeights get_restraints(const RestraintsTemp &rs,
                                              double initial_weight=1);

template <class It>
RestraintsAndWeights get_restraints(It b, It e,
                                    double initial_weight=1) {
  RestraintsAndWeights ret;
  for (It c=b; c!= e; ++c) {
    RestraintSet *rs=dynamic_cast<RestraintSet*>(*c);
    if (rs) {
      IMP_LOG(TERSE, "Restraint set " << rs->get_name()
              << " has weight " << rs->get_weight() << std::endl);
      RestraintsAndWeights rw=get_restraints(rs->restraints_begin(),
                                             rs->restraints_end(),
                                             initial_weight*rs->get_weight());
      ret.first.insert(ret.first.end(), rw.first.begin(), rw.first.end());
      ret.second.insert(ret.second.end(), rw.second.begin(), rw.second.end());
    } else {
      ret.first.push_back(*c);
      ret.second.push_back(initial_weight);
    }
  }
  return ret;
}

          /** @} */
IMP_END_NAMESPACE

#endif  /* IMP_RESTRAINT_SET_H */
