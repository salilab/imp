/**
 *  \file IMP/atom/internal/SelectionPredicate.h
 *  \brief Predicates for selecting a subset of a hierarchy.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_SELECTION_PREDICATE_H
#define IMPATOM_SELECTION_PREDICATE_H

#include <IMP/atom/atom_config.h>
#include <IMP/InputAdaptor.h>
#include <IMP/model_object_helpers.h>
#include <boost/dynamic_bitset.hpp>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

//! Predicates for selecting a subset of a hierarchy.
/** \see Selection */
class SelectionPredicate : public ParticleInputs, public Object {
  int bitset_index_;
 public:
  enum MatchType { MISMATCH = -1,
                   NO_MATCH = 0,
                   MATCH_WITH_CHILDREN = 1,
                   MATCH_SELF_ONLY = 2 };

  IMP_REF_COUNTED_DESTRUCTOR(SelectionPredicate);

  SelectionPredicate(std::string name)
         : Object(name), bitset_index_(-1) {}

  virtual unsigned get_number_of_children() const {
    return 0;
  }
  virtual SelectionPredicate *get_child(unsigned) const {
    return nullptr;
  }

  //! Return a clone of this predicate
  /** By default, this returns the same object, since no state is kept */
  virtual SelectionPredicate *clone(bool) {
    set_was_used(true);
    return this;
  };

  //! Set up unique index(es) into Selection's bitset.
  /** Each predicate needs a unique index so that matches can be cached using
      a bitset maintained by Selection. Should be called before using the
      predicate.

      Subclasses can override this to set up multiple indexes if they have
      child predicates.

      \return one greater than the largest index used.
   */
  virtual int setup_bitset(int index) {
    set_was_used(true);
    bitset_index_ = index++;
    return index;
  }

  //! Get match value for the given particle index.
  /** Calls do_get_value_index() or uses a cached result.
      Should return:
      - MATCH_WITH_CHILDREN: particle matches (e.g. it is a Residue of the
            given type); all child particles will also automatically match
            against this predicate (the result will be cached)
      - MATCH_SELF_ONLY: particle matches (e.g. it is a Residue of the given
            type) but child particles should still be examined, since they
            might not match (the result will not be cached)
      - MISMATCH: particle mismatches (e.g. it is a Residue but is of the
            wrong type); the search will be terminated here
      - NO_MATCH: no match (e.g. the particle is not a Residue, so a match
            cannot be attempted); the search will continue to child particles
     \see do_get_value_index
   */
  MatchType get_value_index(Model *m, ParticleIndex vt,
                            boost::dynamic_bitset<> &bs) const {
    /* If a parent particle already matched successfully, no need to
       check this one */
    if (!bs[bitset_index_]) {
      return MATCH_WITH_CHILDREN;
    } else {
      MatchType v = do_get_value_index(m, vt, bs);
      /* Cache a successful match */
      if (v == MATCH_WITH_CHILDREN) {
        bs.reset(bitset_index_);
      }
      return v;
    }
  }

 protected:
  //! Do the actual match for get_value_index()
  /** Should be overridden in subclasses */
  virtual MatchType do_get_value_index(Model *m, ParticleIndex vt,
                                       boost::dynamic_bitset<> &bs) const = 0;
};
IMP_OBJECTS(SelectionPredicate, SelectionPredicates);

//! A SelectionPredicate that delegates to a list of subpredicates
/** For example, an "and" predicate could return a match only if every
    subpredicate matches */
class ListSelectionPredicate : public SelectionPredicate {
protected:
  SelectionPredicates predicates_;
  void clone_predicates(ListSelectionPredicate *s) {
    s->predicates_.reserve(predicates_.size());
    for(SelectionPredicate *p : predicates_) {
      s->predicates_.push_back(p->clone(false));
    }
  }
public:
  ListSelectionPredicate(std::string name) : SelectionPredicate(name) {}

  virtual unsigned get_number_of_children() const override {
    return predicates_.size();
  }
  virtual SelectionPredicate *get_child(unsigned i) const override {
    return predicates_[i];
  }

  //! Add a predicate to the list of subpredicates
  void add_predicate(SelectionPredicate *p) {
    predicates_.push_back(p);
  }

  virtual int setup_bitset(int index) override {
    index = SelectionPredicate::setup_bitset(index);
    /* Set indexes for subpredicates */
    for(SelectionPredicate *p : predicates_) {
      index = p->setup_bitset(index);
    }
    return index;
  }

  virtual ModelObjectsTemp do_get_inputs(
        Model *m, const ParticleIndexes &pis) const
        override {
    return IMP::get_particles(m, pis);
  }
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_SELECTION_PREDICATE_H */
