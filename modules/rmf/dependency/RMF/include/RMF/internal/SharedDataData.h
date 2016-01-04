/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_DATA_H
#define RMF_INTERNAL_SHARED_DATA_DATA_H

#include "RMF/config.h"
#include "RMF/keys.h"
#include "RMF/types.h"
#include "RMF/infrastructure_macros.h"
#include "small_set_map.h"
#include "large_set_map.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

template <class Traits>
struct KeyData : public RMF_LARGE_UNORDERED_MAP<NodeID, typename Traits::Type> {
  typedef RMF_LARGE_UNORDERED_MAP<NodeID, typename Traits::Type> P;
  KeyData(const P& d) : P(d) {}
  KeyData() {}
};

template <class Traits>
struct TypeData : RMF_SMALL_UNORDERED_MAP<ID<Traits>, KeyData<Traits> > {
  typedef RMF_SMALL_UNORDERED_MAP<ID<Traits>, KeyData<Traits> > P;
  // Mac OS 10.8 and earlier clang needs this for some reason
  TypeData() {}
  TypeData(const TypeData& o) : P(o) {}
  TypeData<Traits> operator=(const TypeData& o) {
    P::operator=(o);
    return *this;
  }
  void swap(TypeData<Traits>& o) { std::swap<P>(*this, o); }
};

#define RMF_SHARED_DATA_TYPE_PARENT(Traits, UCName) , public TypeData<Traits>

#define RMF_SHARED_DATA_TYPE_CLEAR(Traits, UCName) TypeData<Traits>::clear();

#define RMF_SHARED_DATA_TYPE_LIFT(Traits, UCName) \
  using TypeData<Traits>::operator[];             \
  using TypeData<Traits>::erase;                  \
  using TypeData<Traits>::find;

struct Nothing {};
class SharedDataDataTypes : public Nothing RMF_FOREACH_TYPE(
                                RMF_SHARED_DATA_TYPE_PARENT) {
 public:
  void clear() { RMF_FOREACH_TYPE(RMF_SHARED_DATA_TYPE_CLEAR); }
  RMF_FOREACH_TYPE(RMF_SHARED_DATA_TYPE_LIFT);
};

class SharedDataData {
  SharedDataDataTypes static_, frame_;
  bool static_dirty_;
  template <class Traits>
  typename Traits::ReturnType get_value(const SharedDataDataTypes& data,
                                        NodeID node, ID<Traits> k) const {
    typename TypeData<Traits>::const_iterator it0 = data.find(k);
    if (it0 == static_cast<const TypeData<Traits>&>(data).end())
      return Traits::get_null_value();
    typename KeyData<Traits>::const_iterator it1 = it0->second.find(node);
    if (it1 == it0->second.end()) return Traits::get_null_value();
    return it1->second;
  }

  template <class Traits>
  void unset_value(SharedDataDataTypes& data, NodeID node, ID<Traits> k) {
    data[k].erase(node);
    if (data[k].empty()) data.erase(k);
  }

 public:
  SharedDataData() : static_dirty_(false) {}
  void clear_static_values() { static_.clear(); }
  void clear_loaded_values() { frame_.clear(); }
  bool get_static_is_dirty() const { return static_dirty_; }
  void set_static_is_dirty(bool tf) { static_dirty_ = tf; }
  template <class Traits>
  const TypeData<Traits>& get_loaded_data(Traits) const {
    return static_cast<const TypeData<Traits>&>(frame_);
  }
  template <class Traits>
  const TypeData<Traits>& get_static_data(Traits) const {
    return static_cast<const TypeData<Traits>&>(static_);
  }
  template <class Traits>
  TypeData<Traits>& access_loaded_data(Traits) {
    return static_cast<TypeData<Traits>&>(frame_);
  }
  template <class Traits>
  TypeData<Traits>& access_static_data(Traits) {
    return static_cast<TypeData<Traits>&>(static_);
  }

  template <class Traits>
  typename Traits::ReturnType get_static_value(NodeID node,
                                               ID<Traits> k) const {
    return get_value(static_, node, k);
  }
  template <class Traits>
  typename Traits::ReturnType get_loaded_value(NodeID node,
                                               ID<Traits> k) const {
    return get_value(frame_, node, k);
  }
  template <class Traits>
  void set_static_value(NodeID node, ID<Traits> k,
                        typename Traits::ArgumentType v) {
    static_[k][node] = v;
    static_dirty_ = true;
  }
  template <class Traits>
  void set_loaded_value(NodeID node, ID<Traits> k,
                        typename Traits::ArgumentType v) {
    frame_[k][node] = v;
  }
  template <class Traits>
  void unset_static_value(NodeID node, ID<Traits> k) {
    unset_value(static_, node, k);
    static_dirty_ = true;
  }
  template <class Traits>
  void unset_loaded_value(NodeID node, ID<Traits> k) {
    unset_value(frame_, node, k);
  }
  template <class Traits>
  typename Traits::Type& access_static_value(NodeID node, ID<Traits> k) {
    return static_[k][node];
  }
  template <class Traits>
  typename Traits::Type& access_loaded_value(NodeID node, ID<Traits> k) {
    return frame_[k][node];
  }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_DATA_H */
