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


#if IMP_CONTAINER_HAS_ROBIN_MAP==1
#define IMP_CONTAINER_PREDICATE_USE_ROBIN_MAP
#include <tsl/robin_map.h>
#elif IMP_CONTAINER_HAS_GOOGLE_DENSE_HASH_MAP==1
#define IMP_CONTAINER_PREDICATE_USE_GOOGLE_DENSE_HASH_MAP
#include <google/dense_hash_map>
#else
#define IMP_CONTAINER_PREDICATE_USE_KERNEL_MAP
#include <IMP/set_map_macros.h>
//#include <boost/unordered_map.hpp>
#endif

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
  PointerMember<ClassnamePredicate> predicate_;
  PointerMember<ClassnameContainer> input_;
#if IMP_CONTAINER_HAS_ROBIN_MAP==1
  typedef tsl::robin_map<int, PLURALINDEXTYPE> t_lists_map;
  typedef tsl::robin_map<int, PointerMember<ClassnameScore> > t_score_map;
#elif IMP_CONTAINER_HAS_GOOGLE_DENSE_HASH_MAP==1
  typedef google::dense_hash_map<int, PLURALINDEXTYPE> t_lists_map;
  typedef google::dense_hash_map<int, PointerMember<ClassnameScore> > t_score_map;
#else
  typedef IMP_KERNEL_LARGE_UNORDERED_MAP<int, PLURALINDEXTYPE> t_lists_map;
  typedef IMP_KERNEL_LARGE_UNORDERED_MAP<int, PointerMember<ClassnameScore> > t_score_map;
#endif
  mutable t_lists_map lists_;
  t_score_map scores_;

  bool is_get_inputs_ignores_individual_scores_;
  mutable std::size_t input_version_;
  //  bool is_unknown_score_set_;
  bool error_on_unknown_;
  PointerMember<ClassnameScore> unknown_score_;
  //  void update_lists_with_item(INDEXTYPE const &it) const;
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
      not known. If this is false, then they are silently skipped even
      if set_unknown_score() was not called.
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
  /**
     Returns the score for specified predicate based on set_score()

     Unknown scores:
     If predicate was not added with set_score(), return the default unknown score
     set by set_unknown_score() if applicable. If not, then if set_is_complete(false)
     was called, return nullptr. Otherwise, undefined behavior (will fail when checks
     are on)
  */
  inline ClassnameScore*
    get_score_for_predicate(int predicate) const;


  Restraints do_create_current_decomposition() const;
};


#ifndef SWIG
ClassnameScore*
PredicateClassnamesRestraint::get_score_for_predicate
(int predicate) const
{
  ClassnameScore* score;
  t_score_map::const_iterator it= scores_.find(predicate);
  if(it!=scores_.end()){
    score= it->second;
  } else {
    IMP_USAGE_CHECK(!error_on_unknown_, // is_unknown_score_set_,
                    "unknown interaction type cannot be scored when error_on_unknown is true");
    score = unknown_score_;
  }
  return score;
}
#endif


IMPCONTAINER_END_NAMESPACE

#endif /* IMPCONTAINER_PREDICATE_CLASSNAMES_RESTRAINT_H */
