/**
 *  \file IMP/base/tracking.h    \brief IO support.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPBASE_TRACKING_H
#define IMPBASE_TRACKING_H

#include "base_config.h"
#include "Object.h"
#include <IMP/compatibility/set.h>
#include <IMP/compatibility/vector.h>
IMPBASE_BEGIN_NAMESPACE

/** By inheriting from this, an Object can keep track of a list
    of objects as long as they are alive.
*/
template <class Tracked>
class Tracker {
  compatibility::set<Tracked*> tracked_;
  bool dirty_;
 public:
  Tracker(): dirty_(0){}
  compatibility::vector<Tracked*> get_tracked() {
    return compatibility::vector<Tracked*>(tracked_.begin(),
                                           tracked_.end());
  };
  void add_tracked(Tracked*tr) {
    IMP_USAGE_CHECK(tr, "Can't track nullptr object");
    IMP_CHECK_OBJECT(tr);
    tracked_.insert(tr);
    dirty_=true;
  }
  void remove_tracked(Tracked*tr) {
    IMP_USAGE_CHECK(tr, "Can't untrack nullptr");
    IMP_CHECK_OBJECT(tr);
    IMP_USAGE_CHECK(tracked_.find(tr) != tracked_.end(),
                    "Tracked object " << (tr ? tr->get_name():"nullptr")
                    << " not found.");
    tracked_.erase(tr);
    dirty_=true;
  }
  bool get_is_dirty() const {return dirty_;}
  void set_is_dirty(bool tf) {dirty_=tf;}
  typedef typename compatibility::set<Tracked*>::const_iterator TrackedIterator;
  TrackedIterator tracked_begin() const {
    return tracked_.begin();
  }
  TrackedIterator tracked_end() const {
    return tracked_.end();
  }
  ~Tracker() {
    base::Vector<Tracked*> tracked(tracked_begin(), tracked_end());
    for (unsigned int i=0; i< tracked_.size(); ++i) {
      IMP_CHECK_OBJECT(tracked[i]);
      tracked[i]->set_no_tracker();
    }
  }
};

/** By inheriting from this, a record of the Object will be
    maintained as long as it is alive.*/
template <class Type, class Tracker>
class TrackedObject: public Object {
  typedef IMP::base::Tracker<Type> T;
  UncheckedWeakPointer<Tracker> tracker_;
  UncheckedWeakPointer<Type> me_;
 public:
  TrackedObject(Type *me, Tracker *tracker,
                std::string name): Object(name) {
    set_tracker(me, tracker);
  }
  TrackedObject(std::string name): Object(name) {}
  void set_tracker(Type *me, Tracker *tracker) {
    IMP_USAGE_CHECK(!tracker || me,
                    "Can't pass a null oject with a non-null tacker.");
    if (tracker_) {
      static_cast<T*>(tracker_.get())
          ->remove_tracked(me_);
    }
    tracker_=tracker;
    me_=me;
    if (tracker_) {
      static_cast<T*>(tracker_)->add_tracked(me_);
    }
  }
  //! Used by the tracker when it is destroyed
  void set_no_tracker() {
    tracker_=nullptr;
    me_=nullptr;
  }
  bool get_is_tracked() const {return tracker_;}
  Tracker *get_tracker() const {return tracker_;}
  ~TrackedObject() {
    set_tracker(nullptr, nullptr);
  }
};

IMPBASE_END_NAMESPACE

#endif  /* IMPBASE_TRACKING_H */
