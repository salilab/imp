/**
 *  \file IMP/base/tracking.h    \brief IO support.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_TRACKING_H
#define IMPBASE_TRACKING_H

#include <IMP/base/base_config.h>
#include "Object.h"
#include <IMP/base/set.h>
#include <IMP/base/Vector.h>
#include "WeakPointer.h"
#include <sstream>
IMPBASE_BEGIN_NAMESPACE
template <class Type, class Tracker>
class TrackedObject;

/** By inheriting from this, an Object can keep track of
    a list of objects as long as they are alive.
*/
template <class Type, class Tracked>
class Tracker {
  base::set<Tracked *> tracked_;

  friend class TrackedObject<Tracked, Type>;
  void add_tracked(Tracked *tr) {
    IMP_USAGE_CHECK(tr, "Can't track nullptr object");
    IMP_CHECK_OBJECT(tr);
    tracked_.insert(tr);
    do_add_tracked();
  }
  void remove_tracked(Tracked *tr) {
    IMP_USAGE_CHECK(tr, "Can't untrack nullptr");
    IMP_CHECK_OBJECT(tr);
    IMP_USAGE_CHECK(tracked_.find(tr) != tracked_.end(),
                    "Tracked object " << (tr ? tr->get_name() : "nullptr")
                                      << " not found.");
    tracked_.erase(tr);
    do_remove_tracked();
  }
 protected:
  virtual void do_add_tracked() {}
  virtual void do_remove_tracked() {}
  Vector<Tracked *> get_tracked() {
    return base::Vector<Tracked *>(tracked_.begin(), tracked_.end());
  }
  typedef typename base::set<Tracked *>::const_iterator TrackedIterator;
  TrackedIterator tracked_begin() const { return tracked_.begin(); }
  TrackedIterator tracked_end() const { return tracked_.end(); }
 public:
  Tracker() {}
  virtual ~Tracker() {
    base::Vector<Tracked *> tracked(tracked_begin(), tracked_end());
    for (unsigned int i = 0; i < tracked_.size(); ++i) {
      IMP_CHECK_OBJECT(tracked[i]);
      tracked[i]->set_no_tracker();
    }
  }
};

/** By inheriting from this, a record of the Object will be
    maintained as long as it is alive.*/
template <class Type, class Tracker>
class TrackedObject : public Object {
  typedef IMP::base::Tracker<Tracker, Type> T;
  UncheckedWeakPointer<Tracker> tracker_;
  UncheckedWeakPointer<Type> me_;

  friend class IMP::base::Tracker<Tracker, Type>;
  void set_no_tracker() {
    tracker_ = nullptr;
    me_ = nullptr;
  }
 protected:
  void set_tracker(Type *me, Tracker *tracker) {
    IMP_USAGE_CHECK(!tracker || me,
                    "Can't pass a null oject with a non-null tacker.");
    if (tracker == tracker_) return;
    if (tracker_) {
      static_cast<T *>(tracker_.get())->remove_tracked(me_);
    }
    tracker_ = tracker;
    me_ = me;
    if (tracker_) {
      static_cast<T *>(tracker_)->add_tracked(me_);
    }
  }
  bool get_is_tracked() const { return tracker_; }
  Tracker *get_tracker() const { return tracker_; }
 public:
  TrackedObject(Type *me, Tracker *tracker, std::string name) : Object(name) {
    IMP_USAGE_CHECK(tracker, "Must pass non-null tracker in constructor.");
    IMP_INTERNAL_CHECK(me, "The passed this pointer is null, bad.");
    set_tracker(me, tracker);
  }
  TrackedObject(std::string name) : Object(name) {}
  virtual ~TrackedObject() { set_tracker(nullptr, nullptr); }
};

IMPBASE_END_NAMESPACE

#endif /* IMPBASE_TRACKING_H */
