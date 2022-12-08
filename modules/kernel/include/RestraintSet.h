/**
 *  \file IMP/RestraintSet.h
 *  \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_RESTRAINT_SET_H
#define IMPKERNEL_RESTRAINT_SET_H

#include <IMP/kernel_config.h>
#include "Restraint.h"
#include "container_macros.h"
#include <string>

IMPKERNEL_BEGIN_NAMESPACE

//! Object used to hold a set of restraints
/** RestraintSets allow one to define a tree of restraints
    and to weight various restraints. Upon evaluation,
    all of the restraints in RestraintSets that have been
    added to the model are evaluated
    using the provided weight (weights are multiplicative
    when RestraintSets are nested).

    If the weight is 0, the restraints are not evaluated.

    \note Restraints can belong to multiple RestraintSets.
    The total effect is simply one of adding up the weights.

    \headerfile RestraintSet.h "IMP/RestraintSet.h"
    \advanceddoc

    Talk to Daniel if you want to inherit from RestraintSet.
*/
class IMPKERNELEXPORT RestraintSet : public Restraint {
  void on_add(Restraint *r);
  void on_change();
  static void on_remove(RestraintSet *container, Restraint *r);
  void show_it(std::ostream &out) const;

 public:
  //! Create an empty set that is registered with the model
  RestraintSet(Model *m, double weight,
               const std::string &name = "RestraintSet %1%");
  //! Create an empty set that is registered with the model
  RestraintSet(Model *m, const std::string &name = "RestraintSet %1%");
  //! Create a set that is registered with the model
  RestraintSet(const RestraintsTemp &rs, double weight,
               const std::string &name = "RestraintSet %1%");

  double unprotected_evaluate(DerivativeAccumulator *da) const override;

  IMP_OBJECT_METHODS(RestraintSet);
  /** @name Methods to control the nested Restraint objects

      This container manages a set of Restraint objects. To
      manipulate the stored set use the methods below.
  */
  /**@{*/
  IMP_LIST_ACTION(public, Restraint, Restraints, restraint, restraints,
                  Restraint *, Restraints, on_add(obj), on_change(),
                  if (container) on_remove(container, obj));
  /**@}*/

  //! Divide the list of contained restraints into non-sets and sets.
  /** \return a list of all contained Restraints that are not also
              RestraintSets, and another list of contained RestraintSets.
   */
  std::pair<RestraintsTemp, RestraintSetsTemp> get_non_sets_and_sets() const;

 public:
#ifndef IMP_DOXYGEN
  ModelObjectsTemp do_get_inputs() const override;
  ScoringFunction *create_scoring_function(
      double weight = 1.0,
      double max = std::numeric_limits<double>::max()) const override;
#endif
  double get_last_score() const override;
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
 protected:
  Restraints do_create_decomposition() const override;
  Restraints do_create_current_decomposition() const override;
  void do_add_score_and_derivatives(ScoreAccumulator sa) const override;
  void do_add_score_and_derivatives_moved(
                  ScoreAccumulator sa,
                  const ParticleIndexes &moved_pis,
                  const ParticleIndexes &reset_pis) const override;
#endif
};

/** \name Gathering restraints
    It is sometimes useful to extract all the non-RestraintSet restraints
    from a hierarchy involving RestraintSets mixed with Restraints.
*/

IMPKERNELEXPORT RestraintsTemp get_restraints(const RestraintsTemp &rs);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace {
template <class It>
void get_restraints_internal(It b, It e, RestraintsTemp &ret) {
  for (It c = b; c != e; ++c) {
    Restraint *cur = *c;
    RestraintSet *rs = dynamic_cast<RestraintSet *>(cur);
    if (rs) {
      get_restraints_internal(rs->restraints_begin(), rs->restraints_end(),
                              ret);
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
  get_restraints_internal(b, e, ret);
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_RESTRAINT_SET_H */
