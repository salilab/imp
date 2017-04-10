/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_USER_DATA_H
#define RMF_INTERNAL_SHARED_DATA_USER_DATA_H

#include "RMF/config.h"
#include "RMF/ID.h"
#include "RMF/infrastructure_macros.h"
#include "large_set_map.h"
#include <boost/cstdint.hpp>
#include <boost/any.hpp>
#include <algorithm>

#include <boost/shared_ptr.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {

template <class P>
inline uintptr_t get_uint(const P* p) {
  return reinterpret_cast<uintptr_t>(p);
}
template <class P>
inline uintptr_t get_uint(boost::shared_ptr<P> p) {
  return reinterpret_cast<uintptr_t>(p.get());
}
inline uintptr_t get_uint(NodeID id) { return id.get_index(); }

namespace internal {

class SharedDataUserData {
  std::vector<boost::any> association_;
  std::vector<uintptr_t> back_association_value_;
  RMF_LARGE_UNORDERED_MAP<uintptr_t, NodeID> back_association_;
  RMF_LARGE_UNORDERED_MAP<int, boost::any> user_data_;

 public:
  template <class T>
  void set_user_data(int i, const T& d) {
    user_data_[i] = boost::any(d);
  }
  bool get_has_user_data(int i) const {
    return user_data_.find(i) != user_data_.end();
  }
  template <class T>
  T get_user_data(int i) const {
    RMF_USAGE_CHECK(user_data_.find(i) != user_data_.end(),
                    "No such data found");
    try {
      return boost::any_cast<T>(user_data_.find(i)->second);
    }
    catch (boost::bad_any_cast) {
      RMF_THROW(Message("Type mismatch when recovering user data"),
                UsageException);
    }
    RMF_NO_RETURN(T);
  }
  template <class T>
  void set_association(NodeID nid, const T& d, bool overwrite) {
    int id = nid.get_index();
    if (association_.size() <= static_cast<unsigned int>(id)) {
      association_.resize(id + 1, boost::any());
      back_association_value_.resize(id + 1);
    }
    RMF_USAGE_CHECK(overwrite || association_[id].empty(),
                    "Associations can only be set once");
    if (overwrite && !association_[id].empty()) {
      uintptr_t v = back_association_value_[id];
      back_association_.erase(v);
    }
    uintptr_t v = get_uint(d);
    back_association_value_[id] = v;
    association_[id] = boost::any(d);
    RMF_USAGE_CHECK(back_association_.find(v) == back_association_.end(),
                    "Collision on association keys.");
    back_association_[v] = nid;
  }
  template <class T>
  bool get_has_associated_node(const T& v) const {
    return back_association_.find(get_uint(v)) != back_association_.end();
  }
  boost::any get_association(NodeID nid) const {
    int id = nid.get_index();
    RMF_USAGE_CHECK(static_cast<unsigned int>(id) < association_.size(),
                    std::string("Unassociated node"));
    try {
      return association_[id];
    }
    catch (boost::bad_any_cast) {
      RMF_THROW(Message("Type mismatch when recovering node data"),
                UsageException);
    }
    RMF_NO_RETURN(boost::any);
  }
  bool get_has_association(NodeID nid) const {
    int id = nid.get_index();
    if (id >= static_cast<int>(association_.size())) return false;
    return !association_[id].empty();
  }
  template <class T>
  NodeID get_associated_node(const T& d) const {
    return back_association_.find(get_uint(d))->second;
  }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_USER_DATA_H */
