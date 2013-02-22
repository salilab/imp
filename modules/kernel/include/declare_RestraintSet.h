/**
 *  \file IMP/kernel/declare_RestraintSet.h
 *  \brief Used to hold a set of related restraints.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DECLARE_RESTRAINT_SET_H
#define IMPKERNEL_DECLARE_RESTRAINT_SET_H

#include <IMP/kernel/kernel_config.h>
#include "declare_Restraint.h"
#include "container_macros.h"
#include <IMP/base/map.h>
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
class IMPKERNELEXPORT RestraintSet : public Restraint
{
  void on_add(Restraint*r);
  void on_change();
  static void on_remove(RestraintSet *container, Restraint *r);
  void show_it(std::ostream &out) const;
 public:
  //! Create an empty set that is registered with the model
  RestraintSet(Model *m, double weight,
               const std::string& name="RestraintSet %1%");
  //! Create a set that is registered with the model
  RestraintSet(const RestraintsTemp &rs, double weight,
               const std::string& name="RestraintSet %1%");
#ifndef IMP_DOXYGEN
  //! Create an empty set
  RestraintSet(double weight,
               const std::string& name="RestraintSet %1%");

  //! Create an empty set
  RestraintSet(const std::string& name="RestraintSet %1%");

  //! special for model
  RestraintSet(Restraint::ModelInitTag,
               const std::string& name="RestraintSet %1%");
#endif

  double unprotected_evaluate(DerivativeAccumulator *da) const;
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
  ModelObjectsTemp do_get_inputs() const;
  ScoringFunction* create_scoring_function(double weight=1.0,
                                           double max
                                           = std::numeric_limits<double>::max())
      const;
#endif
  IMP_IMPLEMENT(double get_last_score() const);
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  void set_model(Model *m);
  IMP_PROTECTED_METHOD(Restraints, do_create_decomposition,(), const,);;
  IMP_PROTECTED_METHOD(Restraints, do_create_current_decomposition,(), const,);
  IMP_PROTECTED_METHOD(void, do_add_score_and_derivatives,
                       (ScoreAccumulator sa), const,);

  friend class Model;
#endif
};

/** \name Gathering restraints
    It is sometimes useful to extract all the non-RestraintSet restraints
    from a hierarchy involving RestraintSets mixed with Restraints.
*/

IMPKERNELEXPORT RestraintsTemp get_restraints(const RestraintsTemp &rs);

IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_DECLARE_RESTRAINT_SET_H */
