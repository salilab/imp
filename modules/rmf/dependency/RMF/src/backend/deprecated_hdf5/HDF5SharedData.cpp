/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <H5Fpublic.h>
#include <algorithm>
#include <ostream>

#include "HDF5SharedData.h"
#include "RMF/HDF5/DataSetCreationPropertiesD.h"
#include "RMF/HDF5/File.h"
#include "RMF/HDF5/Group.h"
#include "RMF/HDF5/infrastructure_macros.h"
#include "RMF/HDF5/types.h"
#include "RMF/log.h"
#include "HDF5DataSetCache1D.h"
#include "HDF5DataSetCache2D.h"
#include "names.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace hdf5_backend {

#define RMF_CLOSE(lcname, Ucname, PassValue, ReturnValue, PassValues, \
                  ReturnValues)                                       \
  lcname##_data_sets_ = DataDataSetCache2D<Ucname##Traits>();         \
  per_frame_##lcname##_data_sets_ = DataDataSetCache3D<Ucname##Traits>()

void HDF5SharedData::close_things() {
  node_names_.reset();
  free_ids_.clear();
  node_data_.reset();
  index_cache_ = IndexCache();
  key_name_data_sets_ = KeyNameDataSetCache();
  category_names_.reset();
  frame_names_.reset();
  max_cache_.clear();
  RMF_FOREACH_BACKWARDS_TYPE(RMF_CLOSE);
  flush();
  file_ = HDF5::Group();
  H5garbage_collect();
}

void HDF5SharedData::open_things(bool create, bool read_only) {
  read_only_ = read_only;
  if (create) {
    file_ = HDF5::create_file(get_file_path());
    file_.set_attribute<HDF5::CharTraits>("version", std::string("rmf 1"));
    {
      HDF5::DataSetCreationPropertiesD<HDF5::StringTraits, 1> props;
      props.set_compression(HDF5::GZIP_COMPRESSION);
      (file_.add_child_data_set<HDF5::StringTraits,
                                1>)(get_node_name_data_set_name(), props);
    }
    {
      HDF5::DataSetCreationPropertiesD<HDF5::IndexTraits, 2> props;
      props.set_compression(HDF5::GZIP_COMPRESSION);
      props.set_chunk_size(RMF::HDF5::DataSetIndexD<2>(128, 4));
      (file_.add_child_data_set<HDF5::IndexTraits,
                                2>)(get_node_data_data_set_name(), props);
    }
  } else {
    if (read_only) {
      // walk around type checking
      file_ = HDF5::open_file_read_only_returning_nonconst(get_file_path());
    } else {
      file_ = HDF5::open_file(get_file_path());
    }
    std::string version;
    version = file_.get_attribute<HDF5::CharTraits>("version");
    RMF_USAGE_CHECK(version == "rmf 1",
                    internal::get_error_message(
                        "Unsupported rmf version ", "string found: \"", version,
                        "\" expected \"", "rmf 1", "\""));
  }
  node_names_.set(file_, get_node_name_data_set_name());
  node_data_.set(file_, get_node_data_data_set_name());
  initialize_categories();
  initialize_free_nodes();
  initialize_keys(0);
  std::string frn = get_frame_name_data_set_name();
  frame_names_.set(file_, frn);
}

#define RMF_LIST_KEYS(lcname, Ucname, PassValue, ReturnValue, PassValues, \
                      ReturnValues)                                       \
  initialize_keys(cat, #lcname, Ucname##Traits());

void HDF5SharedData::initialize_keys(int) {
  Categories cats = get_categories();
  RMF_FOREACH(Category cat, cats) { RMF_FOREACH_BACKWARDS_TYPE(RMF_LIST_KEYS); }
  initialize_keys(get_category("link"), "nodeid", NodeIDTraits());
}

void HDF5SharedData::initialize_free_nodes() {
  HDF5::DataSetIndexD<2> dim = node_data_.get_size();
  for (unsigned int i = 0; i < dim[0]; ++i) {
    if (IndexTraits::get_is_null_value(
            node_data_.get_value(HDF5::DataSetIndexD<2>(i, 0)))) {
      free_ids_.push_back(i);
    }
  }
}

void HDF5SharedData::initialize_categories() {
  std::string nm = get_category_name_data_set_name();
  category_names_.set(file_, nm);
  HDF5::DataSetIndex1D sz = category_names_.get_size();
  for (unsigned int i = 0; i < sz[0]; ++i) {
    std::string name = category_names_.get_value(HDF5::DataSetIndex1D(i));
    Category cat(i);
    name_category_map_[name] = cat;
    category_data_map_[cat].name = name;
    category_data_map_[cat].index = i;
  }
}

HDF5SharedData::HDF5SharedData(std::string g, bool create, bool read_only)
    : BackwardsIOBase(g), frames_hint_(0) {
  open_things(create, read_only);
  link_category_ = get_category("link");
  link_key_ = get_key(link_category_, "linked", NodeIDTraits());
  if (create) {
    add_node("root", ROOT);
  } else {
    RMF_INFO("Found " << node_names_.get_size() << " nodes");
    RMF_USAGE_CHECK(
        get_name(NodeID(0)) == "root",
        std::string("Root node is not so named ") + get_name(NodeID(0)));
  }
}

HDF5SharedData::~HDF5SharedData() { close_things(); }

void HDF5SharedData::flush() {
  RMF_HDF5_CALL(H5Fflush(file_.get_handle(), H5F_SCOPE_GLOBAL));
  // SharedData::validate();
  node_names_.flush();
  frame_names_.flush();
  category_names_.flush();
  node_data_.flush();
}

void HDF5SharedData::check_node(NodeID node) const {
  RMF_USAGE_CHECK(
      node_names_.get_size()[0] > node.get_index(),
      internal::get_error_message("Invalid node specified: ", node));
}

NodeID HDF5SharedData::add_node(std::string name, NodeType type) {
  NodeID ret;
  if (free_ids_.empty()) {
    HDF5::DataSetIndexD<1> nsz = node_names_.get_size();
    ret = NodeID(nsz[0]);
    ++nsz[0];
    node_names_.set_size(nsz);
    HDF5::DataSetIndexD<2> dsz = node_data_.get_size();
    dsz[0] = ret.get_index() + 1;
    dsz[1] = std::max<hsize_t>(3, dsz[1]);
    node_data_.set_size(dsz);
  } else {
    ret = NodeID(free_ids_.back());
    free_ids_.pop_back();
  }
  node_names_.set_value(HDF5::DataSetIndexD<1>(ret.get_index()), name);
  node_data_.set_value(HDF5::DataSetIndexD<2>(ret.get_index(), TYPE), type);
  node_data_.set_value(HDF5::DataSetIndexD<2>(ret.get_index(), CHILD),
                       IndexTraits::get_null_value());
  node_data_.set_value(HDF5::DataSetIndexD<2>(ret.get_index(), SIBLING),
                       IndexTraits::get_null_value());
  return ret;
}
NodeID HDF5SharedData::get_first_child(NodeID node) const {
  check_node(node);
  int child =
      node_data_.get_value(HDF5::DataSetIndexD<2>(node.get_index(), CHILD));
  if (child == -1)
    return NodeID();
  else
    return NodeID(child);
}
NodeID HDF5SharedData::get_sibling(NodeID node) const {
  check_node(node);
  int sib =
      node_data_.get_value(HDF5::DataSetIndexD<2>(node.get_index(), SIBLING));
  if (sib == -1) {
    return NodeID();
  } else {
    return NodeID(sib);
  }
}
void HDF5SharedData::set_first_child(NodeID node, NodeID c) {
  check_node(node);
  return node_data_.set_value(HDF5::DataSetIndexD<2>(node.get_index(), CHILD),
                              c.get_index());
}
void HDF5SharedData::set_sibling(NodeID node, NodeID c) {
  check_node(node);
  if (c == NodeID()) {
    node_data_.set_value(HDF5::DataSetIndexD<2>(node.get_index(), SIBLING), -1);
  } else {
    node_data_.set_value(HDF5::DataSetIndexD<2>(node.get_index(), SIBLING),
                         c.get_index());
  }
}
std::string HDF5SharedData::get_name(NodeID node) const {
  if (static_cast<unsigned int>(node.get_index()) < get_number_of_nodes()) {
    check_node(node);
    return node_names_.get_value(HDF5::DataSetIndexD<1>(node.get_index()));
  } else {
    return "bond";
  }
}
NodeType HDF5SharedData::get_type(NodeID index) const {
  if (static_cast<unsigned int>(index.get_index()) < get_number_of_nodes()) {
    check_node(index);
    return NodeType(
        node_data_.get_value(HDF5::DataSetIndexD<2>(index.get_index(), TYPE)));
  } else {
    return BOND;
  }
}

NodeID HDF5SharedData::add_child(NodeID node, std::string name, NodeType t) {
  NodeID old_child = get_first_child(node);
  NodeID nn = add_node(name, t);
  set_first_child(node, nn);
  set_sibling(nn, old_child);
  return nn;
}

void HDF5SharedData::add_child(NodeID node, NodeID child_node) {
  RMF_INTERNAL_CHECK(NodeID() != child_node, "Bad child being added");
  NodeID link = add_child(node, "link", LINK);
  get_category_index_create(link_category_);
  set_static_value(link, link_key_, child_node);
  RMF_INTERNAL_CHECK(get_linked(link) == child_node, "Return does not match");
}

NodeID HDF5SharedData::get_linked(NodeID node) const {
  NodeID ret = get_static_value(node, link_key_);
  RMF_INTERNAL_CHECK(ret != NodeID(), "Bad link value found");
  return ret;
}

NodeIDs HDF5SharedData::get_children(NodeID node) const {
  NodeID cur = get_first_child(node);
  NodeIDs ret;
  while (!NodeIDTraits::get_is_null_value(cur)) {
    if (get_type(cur) != LINK) {
      ret.push_back(cur);
      cur = get_sibling(cur);
    } else {
      ret.push_back(get_linked(cur));
      cur = get_sibling(cur);
    }
  }
  std::reverse(ret.begin(), ret.end());

  return ret;
}

unsigned int HDF5SharedData::add_category_impl(std::string name) {
  // fill in later
  int sz = category_names_.get_size()[0];
  category_names_.set_size(HDF5::DataSetIndex1D(sz + 1));
  category_names_.set_value(HDF5::DataSetIndex1D(sz), name);
  return sz;
}

Categories HDF5SharedData::get_categories() const {
  Categories ret;
  for (CategoryDataMap::const_iterator it = category_data_map_.begin();
       it != category_data_map_.end(); ++it) {
    if (it->second.name == "link") continue;
    ret.push_back(it->first);
  }
  return ret;
}

Category HDF5SharedData::get_category(std::string name) {
  NameCategoryMap::const_iterator it = name_category_map_.find(name);
  if (it == name_category_map_.end()) {
    Category cat(name_category_map_.size());
    name_category_map_[name] = cat;
    category_data_map_[cat].index = -1;
    category_data_map_[cat].name = name;
    return cat;
  } else {
    return it->second;
  }
}

#define RMF_SEARCH_KEYS(lcname, Ucname, PassValue, ReturnValue, PassValues,    \
                        ReturnValues)                                          \
  {                                                                            \
    int category_index = get_category_index(cats[i]);                          \
    if (category_index == -1) continue;                                        \
    ret = std::max<int>(ret,                                                   \
                        get_number_of_frames<Ucname##Traits>(category_index)); \
  }

unsigned int HDF5SharedData::get_number_of_frames() const {
  Categories cats = get_categories();
  int ret = 0;
  for (unsigned int i = 0; i < cats.size(); ++i) {
    RMF_FOREACH_BACKWARDS_TYPE(RMF_SEARCH_KEYS);
  }
  return std::max<int>(frame_names_.get_size()[0], ret);
}

std::string HDF5SharedData::get_description() const {
  if (!get_group().get_has_attribute("description")) {
    return std::string();
  } else
    return get_group().get_char_attribute("description");
}
void HDF5SharedData::set_description(std::string str) {
  RMF_USAGE_CHECK(str.empty() || str[str.size() - 1] == '\n',
                  "Description should end in a newline.");
  get_group().set_char_attribute("description", str);
}

std::string HDF5SharedData::get_producer() const {
  if (!get_group().get_has_attribute("producer")) {
    return std::string();
  } else
    return get_group().get_char_attribute("producer");
}
void HDF5SharedData::set_producer(std::string str) {
  get_group().set_char_attribute("producer", str);
}

void HDF5SharedData::set_name(FrameID i, std::string str) {
  RMF_USAGE_CHECK(i != ALL_FRAMES,
                  "Cannot set the name frame name for static data");
  if (frame_names_.get_size()[0] <= i.get_index()) {
    frame_names_.set_size(HDF5::DataSetIndexD<1>(i.get_index() + 1));
  }
  frame_names_.set_value(HDF5::DataSetIndexD<1>(i.get_index()), str);
}
std::string HDF5SharedData::get_loaded_frame_name() const {
  FrameID i = get_loaded_frame();
  if (frame_names_.get_size()[0] > i.get_index()) {
    return frame_names_.get_value(HDF5::DataSetIndexD<1>(i.get_index()));
  } else {
    return std::string();
  }
}

void HDF5SharedData::reload() {
  close_things();
  open_things(false, read_only_);
}

#define RMF_HDF5_SET_FRAME(lcname, Ucname, PassValue, ReturnValue, PassValues, \
                           ReturnValues)                                       \
  per_frame_##lcname##_data_sets_.set_current_frame(frame);

void HDF5SharedData::set_loaded_frame(FrameID frame) {
  RMF_TRACE("Loading frame " << frame);
  BackwardsIOBase::set_loaded_frame(frame);
  RMF_FOREACH_BACKWARDS_TYPE(RMF_HDF5_SET_FRAME);
}
}  // namespace hdf5_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS
