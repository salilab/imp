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
#include "internal/map.h"
#include <string>

IMP_BEGIN_NAMESPACE

//! Container used to hold a set of restraints
/** RestraintSets allow one to define a tree of restraints
    and to weight various restraints. Upon evaluation,
    all of the restraints in RestraintSets that have been
    added to the model are evaluated
    using the provided weight (weights are multiplicative
    when RestraintSets are nested).

    If the weight is 0, the restraints are not evaluated.

    \note Restraints can belong to multiple RestraintSets.
    The total effect is simply one of adding up the weights.

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
  double evaluate(bool deriv) const;
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

IMPEXPORT RestraintsAndWeights
get_restraints_and_weights(const RestraintsTemp &rs,
                           double initial_weight=1);

IMPEXPORT RestraintsAndWeights
get_restraints_and_weights(const RestraintSet *rs);

IMPEXPORT RestraintsTemp get_restraints(const RestraintsTemp &rs);

IMPEXPORT RestraintsTemp get_restraints(const RestraintSet *rs);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace {
  template <class It>
  void get_restraints_and_weights_internal(It b, It e,
                                           double initial_weight,
                                           RestraintsAndWeights &ret,
                                     internal::Map<Restraint*, int> &index) {
    if (initial_weight ==0) return;
    for (It c=b; c!= e; ++c) {
      RestraintSet *rs=dynamic_cast<RestraintSet*>(*c);
      if (rs) {
        get_restraints_and_weights_internal(rs->restraints_begin(),
                                             rs->restraints_end(),
                                             initial_weight*rs->get_weight(),
                                             ret, index);
      } else {
        if (index.find(*c) == index.end()) {
          index[*c]= ret.first.size();
          ret.first.push_back(*c);
          ret.second.push_back(initial_weight);
        } else {
          int i= index.find(*c)->second;
          ret.second[i]+=initial_weight;
        }
      }
    }
  }

  template <class It>
  void get_restraints_internal(It b, It e,
                               RestraintsTemp &ret) {
    for (It c=b; c!= e; ++c) {
      RestraintSet *rs=dynamic_cast<RestraintSet*>(*c);
      if (rs) {
        get_restraints_internal(rs->restraints_begin(),
                                rs->restraints_end(), ret);
      } else {
        ret.push_back(*c);
      }
    }
  }
}
#endif
template <class It>
RestraintsAndWeights get_restraints_and_weights(It b, It e,
                                             double initial_weight=1) {
  internal::Map<Restraint*, int> index;
  RestraintsAndWeights ret;
  get_restraints_and_weights_internal(b,e, initial_weight, ret,
                                      index);
  return ret;
}

template <class It>
RestraintsTemp get_restraints(It b, It e) {
  RestraintsTemp ret;
  get_restraints_internal(b,e, ret);
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

/** @} */

//! Print the hierarchy of restraints
IMPEXPORT void show_restraint_hierarchy(RestraintSet *rs,
                                     std::ostream &out=std::cout);

IMP_END_NAMESPACE

#endif  /* IMP_RESTRAINT_SET_H */
