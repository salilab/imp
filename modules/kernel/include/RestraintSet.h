/**
 *  \file RestraintSet.h     \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_SET_H
#define IMPKERNEL_RESTRAINT_SET_H

#include "kernel_config.h"
#include "Restraint.h"
#include "container_macros.h"
#include "compatibility/map.h"
#include <string>

IMP_BEGIN_NAMESPACE
class RestraintSet;
IMP_OBJECTS(RestraintSet, RestraintSets);

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
  void on_add(Restraint*r);
  void on_change();
  static void on_remove(RestraintSet *container, Restraint *r);
  void show_it(std::ostream &out) const;
 public:
  //! Create an empty set that is registered with the model
  RestraintSet(Model *m, double weight,
               const std::string& name="RestraintSet %1%");
#ifndef IMP_DOXYGEN
  //! Create an empty set
  RestraintSet(double weight,
               const std::string& name="RestraintSet %1%");

  //! Create an empty set
  RestraintSet(const std::string& name="RestraintSet %1%");
#endif

  double unprotected_evaluate(DerivativeAccumulator *accum) const;
  double unprotected_evaluate_if_good(DerivativeAccumulator *accum,
                                      double max) const;
  double unprotected_evaluate_if_below(DerivativeAccumulator *accum,
                                       double max) const;
  ContainersTemp get_input_containers() const;
  ParticlesTemp get_input_particles() const;
  IMP_OBJECT_INLINE(RestraintSet,show_it(out),
                    Restraint::set_model(nullptr));
  /** @name Methods to control the nested Restraint objects

      This container manages a set of Restraint objects. To
      manipulate the stored set use the methods below.
  */
  /**@{*/
  IMP_LIST_ACTION(public, Restraint, Restraints, restraint, restraints,
                  Restraint*, Restraints,
                  on_add(obj), on_change(),
                  if (container) on_remove(container, obj));
  /**@}*/

  /** Divide the list of contained restraints into sets and non-sets.*/
  std::pair<RestraintsTemp, RestraintSetsTemp> get_non_sets_and_sets() const;
 public:
#ifndef IMP_DOXYGEN
  ScoringFunction* create_scoring_function(double weight=1.0,
                                           double max
                                           = std::numeric_limits<double>::max())
      const;
#endif
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  void set_model(Model *m);
 protected:
  Restraints do_create_decomposition() const;
  Restraints do_create_current_decomposition() const;
  Restraints do_create_incremental_decomposition(unsigned int n) const;
  friend class Model;
#endif
};

/** \name Gathering restraints
    It is sometimes useful to extract all the non-RestraintSet restraints
    from a hierarchy involving RestraintSets mixed with Restraints.
    @{
*/


IMPEXPORT RestraintsTemp get_restraints(const RestraintsTemp &rs);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace {
  template <class It>
  void get_restraints_internal(It b, It e,
                               RestraintsTemp &ret) {
    for (It c=b; c!= e; ++c) {
      Restraint *cur= *c;
      RestraintSet *rs=dynamic_cast<RestraintSet*>(cur);
      if (rs) {
        get_restraints_internal(rs->restraints_begin(),
                                rs->restraints_end(), ret);
      } else {
        ret.push_back(cur);
      }
    }
  }
}
#endif

template <class It>
inline RestraintsTemp get_restraints(It b, It e) {
  RestraintsTemp ret;
  get_restraints_internal(b,e, ret);
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

/** @} */

//! Print the hierarchy of restraints
IMPEXPORT void show_restraint_hierarchy(Restraint *rs,
                                     std::ostream &out=std::cout);

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_RESTRAINT_SET_H */
