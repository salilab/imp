/**
 *  \file internal/ListLikeContainer.h
 *  \brief a templated list-like container of Base::ContainedIndexTypes
 * (e.g. ParticlePairIndexes)
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_LIST_LIKE_CONTAINER_H
#define IMPKERNEL_INTERNAL_LIST_LIKE_CONTAINER_H

#include <IMP/kernel_config.h>
#include "../scoped.h"
#include "container_helpers.h"
#include <IMP/thread_macros.h>
#include <algorithm>
#include <cereal/access.hpp>
#include <cereal/types/base_class.hpp>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

//! a list-like container of Base::ContainedIndexTypes (e.g. ParticlePairIndexes)
template <class Base>
class ListLikeContainer : public Base {
 private:
  std::size_t version_;
  typename Base::ContainedIndexTypes data_;
  virtual std::size_t do_get_contents_hash() const override {
    return version_;
  }

  friend class cereal::access;

  template<class Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<Base>(this), data_);
    // reset version (cache)
    if (std::is_base_of<cereal::detail::InputArchiveBase, Archive>::value) {
      version_ = 0;
    }
  }

 protected:
  //! swap internal data_ with data in cur - effectively make the container contain cur
  void swap(typename Base::ContainedIndexTypes &cur) {
    ++version_;
    IMP::swap(data_, cur);
  }

  ListLikeContainer(Model *m, std::string name)
      : Base(m, name), version_(0) {}

  ListLikeContainer() {}

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

  template <class F>
  void apply_generic_moved(const F *f, const ParticleIndexes &moved_pis,
                           const ParticleIndexes &reset_pis) const {
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
        IMP_TASK_SHARED((lb, ub, m, f), (moved_pis, reset_pis),
                 f->apply_indexes_moved(m, data_, lb, ub, moved_pis, reset_pis),
                 "apply");
      }
      IMP_OMP_PRAGMA(taskwait)
    } else {
      f->apply_indexes_moved(Base::get_model(), data_, 0, data_.size(),
                             moved_pis, reset_pis);
    }
  }

  //! apply sm->apply_indexes to data. Use parallel mode using IMP_TASK
  //! if get_number_of_threads()>=2
  void do_apply(const typename Base::Modifier *sm) const override {
    apply_generic(sm);
  }

  void do_apply_moved(const typename Base::Modifier *sm,
                      const ParticleIndexes &moved_pis,
                      const ParticleIndexes &reset_pis) const override {
    apply_generic_moved(sm, moved_pis, reset_pis);
  }

  //! returns a copy of list indexes of appropriate type
  typename Base::ContainedIndexTypes get_indexes() const override {
    return data_;
  }
  bool do_get_provides_access() const override { return true; }

  //! direct access to contained data (which may or may not be properly updated)
  const typename Base::ContainedIndexTypes &get_access() const override {
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
