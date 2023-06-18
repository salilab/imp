/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_CLONE_SHARED_DATA_H
#define RMF_INTERNAL_CLONE_SHARED_DATA_H

#include "RMF/config.h"
#include "RMF/log.h"
#include "RMF/internal/shared_data_ranges.h"
#include "RMF/internal/SharedData.h"
#include "RMF/internal/paths.h"
#include "shared_data_maps.h"
#include "shared_data_equality.h"

#include <boost/range/distance.hpp>
#include <boost/algorithm/string/predicate.hpp>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

template <class OutType, class InType>
inline OutType get_as(InType in) {
  return OutType(in);
}
template <class Out>
inline Out get_as(NodeID ni) {
  if (ni == NodeID())
    return Out(-1);
  else
    return Out(ni.get_index());
}

template <>
inline NodeID get_as(int i) {
  if (i == -1)
    return NodeID();
  else
    return NodeID(i);
}
template <class OutType, class InType>
inline OutType get_as(const std::vector<InType> in) {
  OutType ret(in.size());
  for (unsigned int i = 0; i < ret.size(); ++i) {
    ret[i] = get_as<typename OutType::value_type>(in[i]);
  }
  return ret;
}

template <class SDA, class SDB>
void clone_hierarchy(SDA* sda, SDB* sdb) {
  RMF_INTERNAL_CHECK(boost::distance(get_nodes(sda)) >= 1,
                     "No root node found.");
  RMF_INTERNAL_CHECK(boost::distance(get_nodes(sdb)) >= 1,
                     "No root node found.");
  RMF_LARGE_UNORDERED_MAP<NodeID, NodeID> parents, deferred_parents;
  typedef RMF_LARGE_UNORDERED_MAP<NodeID, NodeID>::value_type NodeIDMapValue;

  for(NodeID na : get_nodes(sda)) {
    NodeIDs children = sda->get_children(na);
    for(NodeID c : children) {
      if (parents.find(c) == parents.end() &&
          c.get_index() >= sdb->get_number_of_nodes()) {
        parents[c] = na;
      }
    }
  }
  if (parents.empty()) return;
  RMF_LARGE_UNORDERED_SET<NodeID> existing;
  RMF_LARGE_UNORDERED_MAP<NodeID, RMF_SMALL_UNORDERED_SET<NodeID> >
      existing_parents;
  for(NodeID nb : get_nodes(sdb)) {
    existing.insert(nb);
    for(NodeID ch : sdb->get_children(nb)) {
      existing_parents[ch].insert(nb);
    }
  }
  for(NodeID na : get_nodes(sda)) {
    if (existing.find(na) != existing.end()) continue;
    if (parents.find(na) != parents.end()) {
      NodeID parent = parents.find(na)->second;
      if (parent.get_index() > na.get_index()) {
        /* If this node refers to a parent which doesn't exist yet, remember
           the relationship and add it later */
        deferred_parents[na] = parent;
        NodeID nid = sdb->add_node(sda->get_name(na), sda->get_type(na));
        RMF_UNUSED(nid);
        RMF_INTERNAL_CHECK(nid == na, "Don't match");
      } else {
        NodeID nid = sdb->add_child(parent, sda->get_name(na),
                                    sda->get_type(na));
        RMF_UNUSED(nid);
        RMF_INTERNAL_CHECK(nid == na, "Don't match");
      }
    } else {
      NodeID nid = sdb->add_node(sda->get_name(na), sda->get_type(na));
      RMF_UNUSED(nid);
      RMF_INTERNAL_CHECK(nid == na, "Don't match");
    }
  }
  for(NodeIDMapValue v : deferred_parents) {
    sdb->add_child(v.second, v.first);
  }

  for(NodeID na : get_nodes(sda)) {
    NodeIDs children = sda->get_children(na);
    for(NodeID c : children) {
      if (parents.find(c) != parents.end() && parents.find(c)->second != na &&
          existing_parents[c].find(na) == existing_parents[c].end()) {
        sdb->add_child(na, c);
      }
    }
  }
}

inline void clone_hierarchy(const SharedData* sda, SharedData* sdb) {
  sdb->access_node_hierarchy() = sda->get_node_hierarchy();
}

template <class SDA, class SDB>
void clone_file(SDA* sda, SDB* sdb) {
  sdb->set_description(sda->get_description());
  sdb->set_producer(sda->get_producer());
}

#define RMF_CLONE_KEYS(Traits, UCName) \
  sdb->access_key_data(Traits()) = sda->get_key_data(Traits());

inline void clone_file(const SharedData* sda, SharedData* sdb) {
  sdb->set_description(sda->get_description());
  sdb->set_producer(sda->get_producer());
  sdb->access_category_data() = sda->get_category_data();
  RMF_FOREACH_TYPE(RMF_CLONE_KEYS);
}

template <class TraitsA, class TraitsB, class SDA, class SDB, class H>
void clone_values_type(SDA* sda, Category cata, SDB* sdb, Category catb, H) {
  RMF_LARGE_UNORDERED_MAP<ID<TraitsA>, ID<TraitsB> > keys =
      get_key_map<TraitsA, TraitsB>(sda, cata, sdb, catb);
  if (keys.empty()) return;
  for(const auto &ks : keys) {
    for(NodeID n : get_nodes(sda)) {
      typename TraitsA::ReturnType rt = H::get(sda, n, ks.first);
      if (!TraitsA::get_is_null_value(rt)) {
        H::set(sdb, n, ks.second, get_as<typename TraitsB::Type>(rt));
      }
    }
  }
}

#define RMF_CLONE_VALUES(Traits, UCName) \
  clone_values_type<Traits, Traits>(sda, cata, sdb, catb, H());

template <class SDA, class SDB, class H>
void clone_values_category(SDA* sda, Category cata, SDB* sdb, Category catb,
                           H) {
  RMF_INTERNAL_CHECK(sda->get_number_of_nodes() <= sdb->get_number_of_nodes(),
                     "Number of nodes don't match.");
  RMF_INTERNAL_CHECK(boost::distance(get_nodes(sda)) >= 1,
                     "No root node found.");
  RMF_INTERNAL_CHECK(boost::distance(get_nodes(sdb)) >= 1,
                     "No root node found.");
  RMF_FOREACH_TYPE(RMF_CLONE_VALUES);
}

// Return true iff an attribute contains a filesystem path.
// Unfortunately RMF stores both paths and non-path strings in String(s)
// attributes (and we can't easily add a separate Path(s) type without
// breaking backwards compatibility), so we do this based on the name
// of the attribute. The convention is to end path attribute names in
// "filename" or "filenames" but we have to also include a few that
// predate this convention.
inline bool is_string_key_path(std::string name) {
  return boost::algorithm::ends_with(name, "filename") ||
         boost::algorithm::ends_with(name, "filenames") ||
         name == "cluster density" ||
         name == "image files" ||
         name == "path";
}

// Rewrite a String node attribute containing a relative path
template <class SDB>
void rewrite_node_path(SDB *sdb, NodeID n, ID<StringTraits> k,
                       StringTraits::ReturnType s,
                       const std::string &from_path,
                       const std::string &to_path) {
  sdb->set_static_value(
       n, k, get_relative_path(to_path, get_absolute_path(from_path, s)));
}

// Rewrite a Strings node attribute containing relative paths
template <class SDB>
void rewrite_node_path(SDB *sdb, NodeID n, ID<StringsTraits> k,
                       StringsTraits::ReturnType s,
                       const std::string &from_path,
                       const std::string &to_path) {
  for (std::string &es : s) {
    es = get_relative_path(to_path, get_absolute_path(from_path, es));
  }
  sdb->set_static_value(n, k, s);
}

// Rewrite all relative paths (in String or Strings attributes) in the
// given category, that were relative to from_path, to be relative to to_path
template <class Traits, class SDB>
void rewrite_paths_type(SDB *sdb, Category catb,
                        const std::string &from_path,
                        const std::string &to_path) {
  std::vector<ID<Traits>> keysb = sdb->get_keys(catb, Traits());
  for (auto &k : keysb) {
    if (is_string_key_path(sdb->get_name(k))) {
      for(NodeID n : get_nodes(sdb)) {
        typename Traits::ReturnType s = sdb->get_static_value(n, k);
        if (!Traits::get_is_null_value(s)) {
          rewrite_node_path(sdb, n, k, s, from_path, to_path);
        }
      }
    }
  }
}

// Rewrite all path attributes, that were relative to sba, to be
// relative to sdb
template <class SDA, class SDB>
void rewrite_relative_paths(SDA* sda, SDB* sdb) {
  std::string from_path = sda->get_file_path();
  std::string to_path = sdb->get_file_path();
  for (Category catb : sdb->get_categories()) {
    // Both String and Strings attributes can contain paths
    rewrite_paths_type<StringTraits>(sdb, catb, from_path, to_path);
    rewrite_paths_type<StringsTraits>(sdb, catb, from_path, to_path);
  }
}

template <class SDA, class SDB>
void clone_static_data(SDA* sda, SDB* sdb) {
  for(Category cata : sda->get_categories()) {
    Category catb = sdb->get_category(sda->get_name(cata));
    clone_values_category(sda, cata, sdb, catb, StaticValues());
  }
  // Path attributes are relative to sda, so rewrite (if necessary) to
  // be relative to sdb
  if (!get_is_same_base_path(sda->get_file_path(), sdb->get_file_path())) {
    rewrite_relative_paths(sda, sdb);
  }
}

template <class SDA, class SDB>
void clone_loaded_data(SDA* sda, SDB* sdb) {
  for(Category cata : sda->get_categories()) {
    Category catb = sdb->get_category(sda->get_name(cata));
    clone_values_category(sda, cata, sdb, catb, LoadedValues());
  }
}

#define RMF_CLONE_DATA(Traits, UCName) \
  H::access_data(sdb, Traits()) = H::get_data(sda, Traits());

inline void clone_static_data(const SharedData* sda, SharedData* sdb) {
  typedef StaticValues H;
  RMF_FOREACH_TYPE(RMF_CLONE_DATA);
  sdb->set_static_is_dirty(true);
}

inline void clone_loaded_data(const SharedData* sda, SharedData* sdb) {
  typedef LoadedValues H;
  RMF_FOREACH_TYPE(RMF_CLONE_DATA);
}

}  // namespace internal
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_CLONE_SHARED_DATA_H */
