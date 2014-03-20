/**
 *  \file IMP/container/PredicateClassnamesRestraint.h
 *  \brief Apply a ClassnameScore to each Classname in a list.
 *
 *  BLURB
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_PREDICATE_CLASSNAMES_RESTRAINT_H
#define IMPCONTAINER_PREDICATE_CLASSNAMES_RESTRAINT_H

#include <IMP/container/container_config.h>
#include <boost/unordered_map.hpp>
#include <IMP/ClassnamePredicate.h>
#include <IMP/restraint_macros.h>
#include "generic.h"

#include <iostream>

IMPCONTAINER_BEGIN_NAMESPACE

//! Applies a ClassnameScore to each Classname in a list based on a predicate
/** This restraint uses a passed predicate to choose which score to apply
    to each tuple in the input container. The selections are cached, making it
    substantially faster than using a core::TypedPairScore.

    \note The ordering of particles within a tuple may vary depending on the
    input container used. You may need to call set_score() with several
    different predicate values for different orderings.
*/
class IMPCONTAINEREXPORT PredicateClassnamesRestraint : public Restraint {
  base::PointerMember<kernel::ClassnamePredicate> predicate_;
  base::PointerMember<kernel::ClassnameContainer> input_;
  mutable boost::unordered_map<int, PLURALINDEXTYPE> lists_;
  boost::unordered_map<int, base::PointerMember<ClassnameScore> > scores_;
  mutable std::size_t input_version_;
  bool error_on_unknown_;
  void update_lists_if_necessary() const;

 public:
  PredicateClassnamesRestraint(ClassnamePredicate *pred,
                               ClassnameContainerAdaptor input,
                               std::string name =
                                   "PredicateClassnamesRestraint %1%");

  /** Apply the passed score to all pairs whose predicate values match
      the passed value.*/
  void set_score(int predicate_value, kernel::ClassnameScore *score);

  /** Apply this score to any pair whose predicate value does not match
      one passed to set_score().*/
  void set_unknown_score(kernel::ClassnameScore *score);

  /** By default, it is an error if the predicate returns a value that is
      not known. If this is false, then they are silently skipped.
  */
  void set_is_complete(bool tf) { error_on_unknown_ = tf; }

  /** return the indexes of all particles for  a given predicate value.*/
  PLURALINDEXTYPE get_indexes(int predicate_value) const {
    return lists_.find(predicate_value)->second;
  }

 public:
  void do_add_score_and_derivatives(IMP::ScoreAccumulator sa) const
      IMP_OVERRIDE;
  IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(PredicateClassnamesRestraint);

 private:
  Restraints do_create_current_decomposition() const;
};

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_PREDICATE_CLASSNAMES_RESTRAINT_H */
