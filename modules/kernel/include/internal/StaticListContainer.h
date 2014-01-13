/**
 *  \file ListClassnameContainer.h    \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_STATIC_LIST_CONTAINER_H
#define IMPKERNEL_INTERNAL_STATIC_LIST_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "ListLikeContainer.h"
#include "container_helpers.h"
#include <IMP/base/object_macros.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

template <class Base>
class IMPKERNELEXPORT StaticListContainer : public ListLikeContainer<Base> {
  typedef ListLikeContainer<Base> P;

 public:
  StaticListContainer(kernel::Model *m, std::string name) : P(m, name) {}
  void add(typename Base::PassContainedIndexType vt) {
    Base::set_has_dependencies(false);
    typename Base::ContainedIndexTypes cur;
    P::swap(cur);
    cur.push_back(vt);
    P::swap(cur);
  }
  void add(const typename Base::ContainedIndexTypes &c) {
    if (c.empty()) return;
    Base::set_has_dependencies(false);
    typename Base::ContainedIndexTypes cur;
    P::swap(cur);
    cur += c;
    P::swap(cur);
  }
  void set(typename Base::ContainedIndexTypes cp) {
    Base::set_has_dependencies(false);
    P::swap(cp);
  }
  void remove(typename Base::PassContainedIndexType vt) {
    Base::set_has_dependencies(false);
    typename Base::ContainedIndexTypes t;
    P::swap(t);
    t.erase(std::remove(t.begin(), t.end(), vt), t.end());
    P::swap(t);
  }
  void clear() {
    Base::set_has_dependencies(false);
    typename Base::ContainedIndexTypes t;
    P::swap(t);
  }
  virtual ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE {
    return flatten(P::get_indexes());
  }
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE {
    return ModelObjectsTemp();
  }
  virtual typename Base::ContainedIndexTypes get_range_indexes() const
      IMP_OVERRIDE {
    return P::get_indexes();
  }
  IMP_OBJECT_METHODS(StaticListContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_STATIC_LIST_CONTAINER_H */
