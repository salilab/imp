/**
 *  \file IMP/atom/internal/SelectionPredicate.h
 *  \brief Predicates for selecting a subset of a hierarchy.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_SELECTION_PREDICATE_H
#define IMPATOM_SELECTION_PREDICATE_H

#include <IMP/atom/atom_config.h>
#include <IMP/base/InputAdaptor.h>
#include <boost/dynamic_bitset.hpp>

IMPATOM_BEGIN_INTERNAL_NAMESPACE

//! Predicates for selecting a subset of a hierarchy.
/** \see Selection */
class SelectionPredicate : public ParticleInputs, public base::Object {
  int bitset_index_;
 public:

  IMP_REF_COUNTED_DESTRUCTOR(SelectionPredicate);

  SelectionPredicate(std::string name)
         : base::Object(name), bitset_index_(-1) {}

  //! Return a clone of this predicate
  /** By default, this returns the same object, since no state is kept */
  virtual SelectionPredicate *clone() {
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
      - 1:  particle matches (e.g. it is a Residue of the given type); all
            child particles will also automatically match (the result will be
            cached)
      - 2:  particle matches (e.g. it is a Residue of the given type) but
            child particles should still be examined, since they might not
            match (the result will not be cached)
      - -1: particle mismatches (e.g. it is a Residue but is of the wrong type);
            the search will be terminated here
      - 0:  no match (e.g. the particle is not a Residue, so a match
            cannot be attempted); the search will continue to child particles
     \see do_get_value_index
   */
  int get_value_index(kernel::Model *m, kernel::ParticleIndex vt,
                      boost::dynamic_bitset<> &bs) const {
    /* If a parent particle already matched sucessfully, no need to
       check this one */
    if (!bs[bitset_index_]) {
      return 1;
    } else {
      int v = do_get_value_index(m, vt, bs);
      /* Cache a successful match */
      if (v == 1) {
        bs.reset(bitset_index_);
      }
      return v;
    }
  }

 protected:
  //! Do the actual match for get_value_index()
  /** Should be overridden in subclasses */
  virtual int do_get_value_index(kernel::Model *m,
                                 kernel::ParticleIndex vt,
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
    IMP_FOREACH(SelectionPredicate *p, predicates_) {
      s->predicates_.push_back(p->clone());
    }
  }
public:
  ListSelectionPredicate(std::string name) : SelectionPredicate(name) {}

  //! Add a predicate to the list of subpredicates
  void add_predicate(SelectionPredicate *p) {
    predicates_.push_back(p);
  }

  virtual int setup_bitset(int index) IMP_OVERRIDE {
    index = SelectionPredicate::setup_bitset(index);
    /* Set indexes for subpredicates */
    IMP_FOREACH(SelectionPredicate *p, predicates_) {
      index = p->setup_bitset(index);
    }
    return index;
  }

  virtual kernel::ModelObjectsTemp do_get_inputs(
        kernel::Model *m, const kernel::ParticleIndexes &pis) const
        IMP_OVERRIDE {
    return IMP::get_particles(m, pis);
  }
};

IMPATOM_END_INTERNAL_NAMESPACE

#endif /* IMPATOM_SELECTION_PREDICATE_H */
