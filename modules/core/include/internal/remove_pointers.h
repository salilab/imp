/**
 *  \file core/internal/remove_pointers.h
 *  \brief Various important functionality
 *         for implementing decorators.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_INTERNAL_REMOVE_POINTERS_H
#define IMPCORE_INTERNAL_REMOVE_POINTERS_H

#include <IMP/core/core_config.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/ScoreState.h>
#include <IMP/base/Pointer.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

#define IMP_MANAGER(UCName, lcname)              \
  class Manage##UCName : boost::noncopyable {    \
    typedef IMP::base::OwnerPointer<UCName> P;   \
    P r_;                                        \
   public:                                       \
    Manage##UCName() {}                          \
    ~Manage##UCName() { set(nullptr, nullptr); } \
    void set(UCName *r, Model *m) {              \
      if (r_) {                                  \
        r_->get_model()->remove_##lcname(r_);    \
      }                                          \
      if (r) {                                   \
        r_ = r;                                  \
        if (m) {                                 \
          m->add_##lcname(r);                    \
        }                                        \
      }                                          \
    }                                            \
    operator UCName *() const { return r_; }     \
  };

IMP_MANAGER(ScoreState, score_state)

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_REMOVE_POINTERS_H */
