/**
 *  \file ListClassnameContainer.h
 *  \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_DYNAMIC_LIST_CONTAINER_H
#define IMPKERNEL_INTERNAL_DYNAMIC_LIST_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeContainer.h"
#include <IMP/kernel/container_base.h>
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

template <class Base>
class DynamicListContainer
  : public ListLikeContainer<Base> {
  typedef ListLikeContainer<Base> P;
  // use this to define the set of all possible particles when it is dynamic
  base::Pointer<Container> scope_;
  bool check_list(const ParticleIndexes &cp) const {
    ParticleIndexes app = scope_->get_all_possible_indexes();

    base::set<ParticleIndex> all(app.begin(), app.end());
    for (unsigned int i = 0; i < cp.size(); ++i) {
      IMP_USAGE_CHECK(
          all.find(cp[i]) != all.end(),
          "Particle " << cp[i]
                      << " is not in the list of all possible particles");
    }
    return true;
  }

 public:
  DynamicListContainer(Container *m, std::string name)
      : P(m->get_model(), name), scope_(m) {}
  void add(typename Base::PassContainedIndexType vt) {
    typename Base::ContainedIndexTypes cur;
    P::swap(cur);
    cur.push_back(vt);
    P::swap(cur);
  }
  void add(const typename Base::ContainedIndexTypes &c) {
    if (c.empty()) return;
    typename Base::ContainedIndexTypes cur;
    P::swap(cur);
    cur += c;
    P::swap(cur);
  }
  void set(typename Base::ContainedIndexTypes cp) { P::swap(cp); }
  void clear() {
    typename Base::ContainedIndexTypes t;
    P::swap(t);
  }
  virtual ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE {
    return scope_->get_all_possible_indexes();
  }
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return kernel::ModelObjectsTemp(1, scope_);
  }
  virtual typename Base::ContainedIndexTypes get_range_indexes() const
      IMP_OVERRIDE {
    return P::get_indexes();
  }
  IMP_OBJECT_METHODS(DynamicListContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_DYNAMIC_LIST_CONTAINER_H */
