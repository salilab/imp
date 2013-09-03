/**
 *  \file ListClassnameContainer.h    \brief Store a list of PLURALVARIABLETYPE
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_INTERNAL_LIST_CLASSNAME_CONTAINER_H
#define IMPKERNEL_INTERNAL_INTERNAL_LIST_CLASSNAME_CONTAINER_H

#include <IMP/kernel/kernel_config.h>
#include "container_helpers.h"
#include "ListLikeClassnameContainer.h"
#include <IMP/base/Pointer.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

class IMPKERNELEXPORT InternalListClassnameContainer
    : public ListLikeClassnameContainer {
  typedef ListLikeClassnameContainer P;

 public:
  InternalListClassnameContainer(kernel::Model *m, std::string name);
  InternalListClassnameContainer(kernel::Model *m, const char *name);
  void add(PASSINDEXTYPE vt);
  void add(const PLURALINDEXTYPE &c);
  void set(PLURALINDEXTYPE cp);
  void remove(PASSINDEXTYPE vt);
  void clear();
  virtual ParticleIndexes get_all_possible_indexes() const IMP_OVERRIDE;
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual void do_before_evaluate() IMP_OVERRIDE;
  virtual PLURALINDEXTYPE get_range_indexes() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(InternalListClassnameContainer);
};

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif /* IMPKERNEL_INTERNAL_INTERNAL_LIST_CLASSNAME_CONTAINER_H */
