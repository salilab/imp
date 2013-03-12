/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "HDF5SharedData.h"
#include <RMF/NodeHandle.h>
#include <RMF/Validator.h>
#include <RMF/internal/set.h>
#include <RMF/HDF5/Group.h>
#include <RMF/log.h>
#include <algorithm>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace hdf5_backend {

#define RMF_CLOSE(lcname, Ucname, PassValue, ReturnValue,     \
                  PassValues, ReturnValues)                   \
  lcname##_data_sets_ = DataDataSetCache2D<Ucname##Traits>(); \
  per_frame_##lcname##_data_sets_ = DataDataSetCache3D<Ucname##Traits>()

void HDF5SharedData::close_things() {
  node_names_.reset();
  free_ids_.clear();
  for (unsigned int i = 0; i < 4; ++i) {
    node_data_[i].reset();
    index_cache_[i] = IndexCache();
  }
  key_name_data_sets_ = KeyNameDataSetCache();
  category_names_.reset();
  frame_names_.reset();
  max_cache_.clear();
  RMF_FOREACH_TYPE(RMF_CLOSE);
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
      (file_.add_child_data_set<HDF5::StringTraits, 1>)
        (get_node_name_data_set_name(), props);
    }
    {
      HDF5::DataSetCreationPropertiesD<HDF5::IndexTraits, 2> props;
      props.set_compression(HDF5::GZIP_COMPRESSION);
      props.set_chunk_size(RMF::HDF5::DataSetIndexD<2>(128, 4));
      (file_.add_child_data_set<HDF5::IndexTraits, 2>)
        (get_node_data_data_set_name(),
        props);
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
                    internal::get_error_message("Unsupported rmf version ",
                                      "string found: \"",
                                      version, "\" expected \"",
                                      "rmf 1", "\""));
  }
  node_names_.set(file_, get_node_name_data_set_name());
  node_data_[0].set(file_, get_node_data_data_set_name());
  initialize_categories();
  initialize_free_nodes();
  initialize_keys(0);
  std::string frn = get_frame_name_data_set_name();
  frame_names_.set(file_, frn);
}

#define RMF_LIST_KEYS(lcname, Ucname, PassValue, ReturnValue,           \
                      PassValues, ReturnValues)                         \
  RMF_TRACE(get_logger(), "Checking for " << #lcname << " keys.");      \
  for (int pf = 0; pf < 2; ++pf) {                                      \
    bool per_frame = (pf == 1);                                         \
    HDF5DataSetCacheD<StringTraits, 1>& nameds                          \
        = get_key_list_data_set<Ucname##Traits>(cats[i],                \
                                                per_frame);             \
    HDF5::DataSetIndexD<1> sz = nameds.get_size();                      \
    for (unsigned int j = 0; j < sz[0]; ++j) {                          \
      std::string name = nameds.get_value(HDF5::DataSetIndexD<1>(j));     \
      int id;                                                           \
      NameKeyInnerMap::iterator it = name_key_map_[cats[i]].find(name); \
      if (it == name_key_map_[cats[i]].end()) {                         \
        id = key_data_map_.size();                                      \
        name_key_map_[cats[i]][name] = id;                              \
        key_data_map_[id].name = name;                                  \
        key_data_map_[id].type_index = Ucname##Traits::HDF5Traits::get_index(); \
        key_data_map_[id].per_frame_index = -1;                         \
        key_data_map_[id].static_index = -1;                            \
        key_data_map_[id].category = cats[i];                           \
      } else {                                                          \
        id = it->second;                                                \
      }                                                                 \
      if (per_frame) {                                                  \
        key_data_map_[id].per_frame_index = j;                          \
      } else {                                                          \
        key_data_map_[id].static_index = j;                             \
      }                                                                 \
    }                                                                   \
  }                                                                     \



void HDF5SharedData::initialize_keys(int ) {
  Categories cats = get_categories();
  for (unsigned int i = 0; i < cats.size(); ++i) {
    RMF_FOREACH_TYPE(RMF_LIST_KEYS);
  }
}

void HDF5SharedData::initialize_free_nodes() {
  HDF5::DataSetIndexD<2> dim = node_data_[0].get_size();
  for (unsigned int i = 0; i < dim[0]; ++i) {
    if (IndexTraits::
        get_is_null_value(node_data_[0].get_value(HDF5::DataSetIndexD<2>(i,
                                                                       0)))) {
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

HDF5SharedData::HDF5SharedData(std::string g, bool create, bool read_only):
  SharedData(g), frames_hint_(0), link_category_(-1)
{
  open_things(create, read_only);
  link_category_ = get_category("link");
  link_key_ = get_node_id_key(link_category_,
                              "linked");
  if (create) {
    add_node("root", ROOT);
  } else {
    RMF_USAGE_CHECK(get_name(0) == "root",
                    "Root node is not so named");
  }
}

HDF5SharedData::~HDF5SharedData() {
  add_ref();
  close_things();
  release();
}

void HDF5SharedData::flush() {
  RMF_HDF5_CALL(H5Fflush(file_.get_handle(), H5F_SCOPE_GLOBAL));
  //SharedData::validate();
  node_names_.flush();
  frame_names_.flush();
  category_names_.flush();
  for (unsigned int i = 0; i < 4; ++i) {
    node_data_[i].flush();
  }
}

void HDF5SharedData::check_node(unsigned int node) const {
  RMF_USAGE_CHECK(node_names_.get_size()[0] > node,
                  internal::get_error_message("Invalid node specified: ",
                                    node));
}

int HDF5SharedData::add_node(std::string name, unsigned int type) {
  int ret;
  if (free_ids_.empty()) {
    HDF5::DataSetIndexD<1> nsz = node_names_.get_size();
    ret = nsz[0];
    ++nsz[0];
    node_names_.set_size(nsz);
    HDF5::DataSetIndexD<2> dsz = node_data_[0].get_size();
    dsz[0] = ret + 1;
    dsz[1] = std::max<hsize_t>(3, dsz[1]);
    node_data_[0].set_size(dsz);
  } else {
    ret = free_ids_.back();
    free_ids_.pop_back();
  }
  audit_node_name(name);
  node_names_.set_value(HDF5::DataSetIndexD<1>(ret), name);
  node_data_[0].set_value(HDF5::DataSetIndexD<2>(ret, TYPE), type);
  node_data_[0].set_value(HDF5::DataSetIndexD<2>(ret, CHILD),
                          IndexTraits::get_null_value());
  node_data_[0].set_value(HDF5::DataSetIndexD<2>(ret, SIBLING),
                          IndexTraits::get_null_value());
  return ret;
}
int HDF5SharedData::get_first_child(unsigned int node) const {
    check_node(node);
  return node_data_[0].get_value(HDF5::DataSetIndexD<2>(node, CHILD));
}
int HDF5SharedData::get_sibling(unsigned int node) const {
    check_node(node);
  return node_data_[0].get_value(HDF5::DataSetIndexD<2>(node, SIBLING));
}
void HDF5SharedData::set_first_child(unsigned int node, int c) {
    check_node(node);
  return node_data_[0].set_value(HDF5::DataSetIndexD<2>(node, CHILD), c);
}
void HDF5SharedData::set_sibling(unsigned int node, int c) {
    check_node(node);
  return node_data_[0].set_value(HDF5::DataSetIndexD<2>(node, SIBLING), c);
}
std::string HDF5SharedData::get_name(unsigned int node) const {
  if (node < get_number_of_real_nodes()) {
    check_node(node);
    return node_names_.get_value(HDF5::DataSetIndexD<1>(node));
  } else {
    return "bond";
  }
}
unsigned int HDF5SharedData::get_type(unsigned int index) const {
  if (index < get_number_of_real_nodes()) {
    check_node(index);
    return node_data_[0].get_value(HDF5::DataSetIndexD<2>(index, TYPE));
  } else {
    return BOND;
  }
}


int HDF5SharedData::add_child(int node, std::string name, int t) {
  int old_child = get_first_child(node);
  int nn = add_node(name, t);
  set_first_child(node, nn);
  set_sibling(nn, old_child);
  return nn;
}

void HDF5SharedData::add_child(int node, int child_node) {
  RMF_INTERNAL_CHECK(-1 != child_node,
                     "Bad child being added");
  int link = add_child(node, "link", LINK);
  get_category_index_create(link_category_);
  set_value(link, link_key_,
            NodeID(child_node));
  RMF_INTERNAL_CHECK(get_linked(link) == child_node,
                     "Return does not match");
}

int HDF5SharedData::get_linked(int node) const {
  int ret = get_value(node, link_key_).get_index();
  RMF_INTERNAL_CHECK(ret >= 0, "Bad link value found");
  return ret;
}

Ints HDF5SharedData::get_children(int node) const {
  if (node < static_cast<int>(get_number_of_real_nodes())) {
    int cur = get_first_child(node);
    Ints ret;
    while (!IndexTraits::get_is_null_value(cur)) {
      if (get_type(cur) != LINK) {
        ret.push_back(cur);
        cur = get_sibling(cur);
      } else {
        ret.push_back(get_linked(cur));
        cur = get_sibling(cur);
      }
    }
    std::reverse(ret.begin(), ret.end());

    if (node == 0) {
      for (unsigned int i = 0; i < get_number_of_sets(2); ++i) {
        ret.push_back(get_number_of_real_nodes() + i);
      }
    }
    return ret;
  } else {
    Ints ret(2);
    ret[0] = get_set_member(2, node - get_number_of_real_nodes(), 0);
    ret[1] = get_set_member(2, node - get_number_of_real_nodes(), 1);
    return ret;
  }
}
unsigned int HDF5SharedData::get_number_of_sets(int arity) const {
  HDF5::DataSetIndexD<2> sz = node_data_[arity - 1].get_size();
  unsigned int ct = 0;
  for (unsigned int i = 0; i < sz[0]; ++i) {
    if (node_data_[arity - 1].get_value(HDF5::DataSetIndexD<2>(i, 0)) >= 0) {
      ++ct;
    }
  }
  return ct;
}
unsigned int HDF5SharedData::get_set_member(int arity, unsigned int index,
                                            int member_index) const {
  return node_data_[arity - 1].get_value(HDF5::DataSetIndexD<2>(index,
                                                              member_index + 1));
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


#define RMF_SEARCH_KEYS(lcname, Ucname, PassValue, ReturnValue,                \
                        PassValues, ReturnValues)                              \
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
    RMF_FOREACH_TYPE(RMF_SEARCH_KEYS);
  }
  return std::max<int>(frame_names_.get_size()[0], ret);
}


std::string HDF5SharedData::get_description() const {
  if (!get_group().get_has_attribute("description")) {
    return std::string();
  } else return get_group().get_char_attribute("description");
}
void HDF5SharedData::set_description(std::string str) {
  RMF_USAGE_CHECK(str.empty()
                  || str[str.size() - 1] == '\n',
                  "Description should end in a newline.");
  get_group().set_char_attribute("description", str);
}

std::string HDF5SharedData::get_producer() const {
  if (!get_group().get_has_attribute("producer")) {
    return std::string();
  } else return get_group().get_char_attribute("producer");
}
void HDF5SharedData::set_producer(std::string str) {
  RMF_USAGE_CHECK(str.empty()
                  || str[str.size() - 1] == '\n',
                  "Producer should end in a newline.");
  get_group().set_char_attribute("producer", str);
}

void HDF5SharedData::set_frame_name(int i, std::string str) {
  RMF_USAGE_CHECK(i != ALL_FRAMES,
                  "Cannot set the name frame name for static data");
  if (static_cast<int>(frame_names_.get_size()[0]) <= i) {
    frame_names_.set_size(HDF5::DataSetIndexD<1>(i + 1));
  }
  frame_names_.set_value(HDF5::DataSetIndexD<1>(i), str);
}
std::string HDF5SharedData::get_frame_name(int i) const {
  RMF_USAGE_CHECK(i != ALL_FRAMES,
                  "The static data frame does not have a name");
  if (static_cast<int>(frame_names_.get_size()[0]) > i) {
    return frame_names_.get_value(HDF5::DataSetIndexD<1>(i));
  } else {
    return std::string();
  }
}



void HDF5SharedData::reload() {
  close_things();
  open_things(false, read_only_);
}

#define RMF_HDF5_SET_FRAME(lcname, Ucname, PassValue, ReturnValue, \
                           PassValues, ReturnValues)               \
  per_frame_##lcname##_data_sets_.set_current_frame(frame);


void HDF5SharedData::set_current_frame(int frame) {
  RMF_TRACE(get_logger(), "Loading frame " << frame);
  SharedData::set_current_frame(frame);
  if (frame >= 0) {
    RMF_FOREACH_TYPE(RMF_HDF5_SET_FRAME);
  }
}
}   // namespace hdf5_backend
} /* namespace RMF */

RMF_DISABLE_WARNINGS

