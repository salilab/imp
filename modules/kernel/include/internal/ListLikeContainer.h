/**
 *  \file internal/core_FUNCTIONNAME_helpers.h
 *  \brief A container for Classnames.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_LIST_LIKE_CONTAINER_H
#define IMPKERNEL_INTERNAL_LIST_LIKE_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "../scoped.h"
#include "container_helpers.h"
#include <IMP/base/thread_macros.h>
#include <algorithm>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

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
    IMP::base::swap(data_, cur);
  }

  ListLikeContainer(kernel::Model *m, std::string name)
      : Base(m, name), version_(0) {}

 public:
  template <class F>
  void apply_generic(const F *f) const {
    Base::validate_readable();
    if (base::get_number_of_threads() > 1) {
      unsigned int tasks = 2 * base::get_number_of_threads();
      unsigned int chunk_size =
          std::max<unsigned int>(1U, data_.size() / tasks) + 1;
      kernel::Model *m = Base::get_model();
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

  void do_apply(const typename Base::Modifier *sm) const { apply_generic(sm); }

  IMP_OBJECT_METHODS(ListLikeContainer);

  typename Base::ContainedIndexTypes get_indexes() const IMP_OVERRIDE {
    return data_;
  }
  bool do_get_provides_access() const IMP_OVERRIDE { return true; }
  const typename Base::ContainedIndexTypes &get_access() const IMP_OVERRIDE {
    return data_;
  }

  typedef typename Base::ContainedIndexTypes::const_iterator const_iterator;
  const_iterator begin() const { return data_.begin(); }
  const_iterator end() const { return data_.end(); }
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_LIST_LIKE_CONTAINER_H */
