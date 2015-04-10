/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_SHARED_DATA_HIERARCHY_H
#define RMF_INTERNAL_SHARED_DATA_HIERARCHY_H

#include "RMF/config.h"
#include "RMF/enums.h"
#include "small_set_map.h"
#include <string>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

template <class ID, class Type>
struct HierarchyNode {
  std::string name;
  Type type;
  std::vector<ID> parents;
  std::vector<ID> children;
  HierarchyNode() : type(Type(-1)) {}
};

class SharedDataHierarchy {
  typedef std::vector<HierarchyNode<NodeID, NodeType> > Data;
  Data hierarchy_;
  bool dirty_;

  void remove(std::vector<NodeID>& list, NodeID to_remove) {
    RMF_INTERNAL_CHECK(
        std::find(list.begin(), list.end(), to_remove) != list.end(),
        "Note in list");
    list.erase(std::remove(list.begin(), list.end(), to_remove), list.end());
    RMF_INTERNAL_CHECK(
        std::find(list.begin(), list.end(), to_remove) == list.end(),
        "Still in list");
  }

 public:
  SharedDataHierarchy() { clear(); }

  unsigned int get_number_of_nodes() const { return hierarchy_.size(); }

  std::string get_name(NodeID node) const {
    return hierarchy_[node.get_index()].name;
  }
  NodeType get_type(NodeID node) const {
    return hierarchy_[node.get_index()].type;
  }
  void set_name(NodeID node, std::string name) {
    hierarchy_[node.get_index()].name = name;
  }
  void set_type(NodeID node, NodeType t) {
    hierarchy_[node.get_index()].type = t;
  }

  NodeID add_node(std::string name, NodeType t) {
    NodeID ret(hierarchy_.size());
    hierarchy_.resize(hierarchy_.size() + 1);
    hierarchy_.back().name = name;
    hierarchy_.back().type = t;
    return ret;
  }

  NodeID add_child(NodeID id, std::string name, NodeType t) {
    NodeID ret = add_node(name, t);
    add_child(id, ret);
    return ret;
  }

  void add_child(NodeID parent, NodeID child) {
    RMF_USAGE_CHECK(
        parent != NodeID() && parent != NodeID(-1, NodeID::SpecialTag()),
        "Bad parent");
    hierarchy_.resize(
        std::max<std::size_t>(hierarchy_.size(), parent.get_index()));
    hierarchy_.resize(
        std::max<std::size_t>(hierarchy_.size(), child.get_index()));
    hierarchy_[parent.get_index()].children.push_back(child);
    hierarchy_[child.get_index()].parents.push_back(parent);
    dirty_ = true;
  }

  void remove_child(NodeID parent, NodeID child) {
    remove(hierarchy_[parent.get_index()].children, child);
    remove(hierarchy_[child.get_index()].parents, parent);
    dirty_ = true;
  }

  std::vector<NodeID> get_children(NodeID node) const {
    static std::vector<NodeID> missing;
    if (node.get_index() >= hierarchy_.size()) return missing;
    return hierarchy_[node.get_index()].children;
  }

  const std::vector<NodeID>& get_parents(NodeID node) const {
    static std::vector<NodeID> missing;
    if (node.get_index() >= hierarchy_.size()) return missing;
    return hierarchy_[node.get_index()].parents;
  }

  bool get_is_dirty() const { return dirty_; }
  void set_is_dirty(bool tf) { dirty_ = tf; }

  void clear() {
    hierarchy_.clear();
    dirty_ = true;
    hierarchy_.resize(1);
    hierarchy_[0].name = "root";
    hierarchy_[0].type = NodeType(0);
  }

  std::vector<HierarchyNode<NodeID, NodeType> >& access_node_hierarchy() {
    return hierarchy_;
  }

  const std::vector<HierarchyNode<NodeID, NodeType> >& get_node_hierarchy()
      const {
    return hierarchy_;
  }
};

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_SHARED_DATA_HIERARCHY_H */
