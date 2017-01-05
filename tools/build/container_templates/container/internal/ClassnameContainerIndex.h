/**
 *  \file IMP/container/ClassnameContainerStatistics.h
 *  \brief A container for Classnames.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CLASSNAME_CONTAINER_INDEX_H
#define IMPCONTAINER_CLASSNAME_CONTAINER_INDEX_H

#include <IMP/container/container_config.h>
#include <IMP/ClassnameContainer.h>
#include <IMP/internal/container_helpers.h>
#include <IMP/ScoreState.h>
#include <boost/unordered_set.hpp>

IMPCONTAINER_BEGIN_INTERNAL_NAMESPACE

/** Store an index that allows one to quickly determine of something
    is in a container.*/
class IMPCONTAINEREXPORT ClassnameContainerIndex : public ScoreState {
  Pointer<ClassnameContainer> container_;
  std::size_t container_version_;
  boost::unordered_set<INDEXTYPE> contents_;
  bool handle_permutations_;

  void build();

 public:
  ClassnameContainerIndex(ClassnameContainerAdaptor c,
                          bool handle_permutations);
  bool get_contains(INDEXTYPE i) const {
    if (handle_permutations_) i = IMP::internal::get_canonical(i);
    return contents_.find(i) != contents_.end();
  }
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual void do_after_evaluate(DerivativeAccumulator *da) IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual ModelObjectsTemp do_get_outputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ClassnameContainerIndex);
};

IMPCONTAINER_END_INTERNAL_NAMESPACE

#endif /* IMPCONTAINER_CLASSNAME_CONTAINER_INDEX_H */
