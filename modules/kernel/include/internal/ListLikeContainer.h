/**
 *  \file internal/core_FUNCTIONNAME_helpers.h
 *  \brief a templated list-like container of Base::ContainedIndexTypes
 * (e.g. ParticlePairIndexes)
 *
 *  BLURB
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_LIST_LIKE_CONTAINER_H
#define IMPKERNEL_INTERNAL_LIST_LIKE_CONTAINER_H

#include <IMP/kernel_config.h>
#include "../scoped.h"
#include "container_helpers.h"
#include <IMP/thread_macros.h>
#include <algorithm>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! a list-like container of Base::ContainedIndexTypes (e.g. ParticlePairIndexes)
template <class Base>
class ListLikeContainer : public Base {
 private:
  std::size_t version_;
  typename Base::ContainedIndexTypes data_;
  virtual std::size_t do_get_contents_hash() const IMP_OVERRIDE {
    return version_;
  }

 protected:
  void swap(typename Base::ContainedIndexTypes &cur) {
    ++version_;
    IMP::swap(data_, cur);
  }

  ListLikeContainer(Model *m, std::string name)
      : Base(m, name), version_(0) {}

 public:
  //! apply f->apply_indexes to data_. Use parallel mode using IMP_TASK
  //! if get_number_of_threads()>=2
  template <class F>
  void apply_generic(const F *f) const {
    Base::validate_readable();
    if (get_number_of_threads() > 1) {
      unsigned int tasks = 2 * get_number_of_threads();
      unsigned int chunk_size =
          std::max<unsigned int>(1U, data_.size() / tasks) + 1;
      Model *m = Base::get_model();
      for (unsigned int i = 0; i < tasks; ++i) {
        unsigned int lb = i * chunk_size;
        unsigned int ub =
            std::min<unsigned int>(data_.size(), (i + 1) * chunk_size);
        IMP_TASK((lb, ub, m, f), f->apply_indexes(m, data_, lb, ub), "apply");
      }
      IMP_OMP_PRAGMA(taskwait)
    } else {
      f->apply_indexes(Base::get_model(), data_, 0, data_.size());
    }
  }

  //! apply sm->apply_indexes to data. Use parallel mode using IMP_TASK
  //! if get_number_of_threads()>=2
  void do_apply(const typename Base::Modifier *sm) const { apply_generic(sm); }

  //! returns a copy of list indexes of appropriate type
  typename Base::ContainedIndexTypes get_indexes() const IMP_OVERRIDE {
    return data_;
  }
  bool do_get_provides_access() const IMP_OVERRIDE { return true; }

  //! direct access to contained data (which may or may not be properly updated)
  const typename Base::ContainedIndexTypes &get_access() const IMP_OVERRIDE {
    return data_;
  }

  typedef typename Base::ContainedIndexTypes::const_iterator const_iterator;

  //! get first item in list
  const_iterator begin() const { return data_.begin(); }

  //! get last item in list
  const_iterator end() const { return data_.end(); }
  IMP_OBJECT_METHODS(ListLikeContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_LIST_LIKE_CONTAINER_H */
