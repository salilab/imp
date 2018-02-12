/**
 *  \file IMP/container/PredicateClassnamesRestraint.h
 *  \brief Apply a ClassnameScore to each Classname in a list.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCONTAINER_PREDICATE_CLASSNAMES_RESTRAINT_H
#define IMPCONTAINER_PREDICATE_CLASSNAMES_RESTRAINT_H

#include <IMP/container/container_config.h>
#include <IMP/ClassnamePredicate.h>
#include "generic.h"

#if IMP_CONTAINER_HAS_GOOGLE_DENSE_HASH_MAP==1
#define IMP_CONTAINER_PREDICATE_USE_GOOGLE_DENSE_HASH_MAP
#include <google/dense_hash_map>
#elif IMP_CONTAINER_HAS_ROBIN_MAP==1
#define IMP_CONTAINER_PREDICATE_USE_ROBIN_MAP
#include <tsl/robin_map.h>
#else
#define IMP_CONTAINER_PREDICATE_USE_STL_MAP
#include <boost/unordered_map.hpp>
#endif

#include <iostream>

//#define IMP_CONTAINER_TEASLE_ROBIN_MAP

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
  PointerMember<ClassnamePredicate> predicate_;
  PointerMember<ClassnameContainer> input_;
#if IMP_CONTAINER_HAS_GOOGLE_DENSE_HASH_MAP==1
  mutable google::dense_hash_map<int, PLURALINDEXTYPE> lists_;
  google::dense_hash_map<int, PointerMember<ClassnameScore> > scores_;
#elif IMP_CONTAINER_PREDICATE_USE_ROBIN_MAP==1
  mutable tsl::robin_map<int, PLURALINDEXTYPE> lists_;
  tsl::robin_map<int, PointerMember<ClassnameScore> > scores_;
#else
  mutable std::unordered_map<int, PLURALINDEXTYPE> lists_;
  std::unordered_map<int, PointerMember<ClassnameScore> > scores_;
#endif

  bool is_get_inputs_ignores_individual_scores_;
  mutable std::size_t input_version_;
  bool is_unknown_score_set_;
  bool error_on_unknown_;
  void update_lists_if_necessary() const;

 public:
  PredicateClassnamesRestraint(ClassnamePredicate *pred,
                               ClassnameContainerAdaptor input,
                               std::string name =
                                   "PredicateClassnamesRestraint %1%");

  /** Apply the passed score to all pairs whose predicate values match
      the passed value.*/
  void set_score(int predicate_value, ClassnameScore *score);

  /** Apply this score to any pair whose predicate value does not match
      one passed to set_score().*/
  void set_unknown_score(ClassnameScore *score);

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
  IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;

#ifndef SWIG
  //! use at own risk - for efficiency in case e.g. all scores use same inputs
  //! If true, do_get_inputs() includes all inputs in input container
  //! but not individual inputs from individual scores
  void set_is_get_inputs_ignores_individual_scores(bool is_ignore){
    is_get_inputs_ignores_individual_scores_= is_ignore;
  }
#endif
  IMP_OBJECT_METHODS(PredicateClassnamesRestraint);

 private:
  Restraints do_create_current_decomposition() const;
};

IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_PREDICATE_CLASSNAMES_RESTRAINT_H */
