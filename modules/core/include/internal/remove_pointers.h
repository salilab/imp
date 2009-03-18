/**
 *  \file core/internal/remove_pointers.h
 *  \brief Various important functionality
 *         for implementing decorators.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_REMOVE_POINTERS_H
#define IMPCORE_INTERNAL_REMOVE_POINTERS_H

#include "../macros.h"
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/ScoreState.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

#define IMP_MANAGER(UCName, lcname)             \
  class Manage##UCName: boost::noncopyable {    \
    typedef Pointer<UCName> P;                  \
    P r_;                                       \
  public:                                       \
  Manage##UCName(){}                            \
  ~Manage##UCName() {                           \
    set(NULL, NULL);                            \
  }                                             \
  void set(UCName *r, Model *m) {               \
    if (r_) {                                   \
      r_->get_model()->remove_##lcname(r_);     \
    }                                           \
    if (r) {                                    \
      r_=r;                                     \
      if (m) {                                  \
        m->add_##lcname(r);                     \
      }                                         \
    }                                           \
  }                                             \
  };

IMP_MANAGER(Restraint, restraint)
IMP_MANAGER(ScoreState, score_state)


IMPCORE_END_INTERNAL_NAMESPACE

#endif  /* IMPCORE_INTERNAL_REMOVE_POINTERS_H */
