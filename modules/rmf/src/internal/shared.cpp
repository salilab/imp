/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/shared.h>
#include <IMP/rmf/NodeHandle.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

SharedData::SharedData(HDF5Group g, bool create):
  file_(g),
  names_(file_.get_child_data_set<StringTraits>(get_node_name_data_set_name(),
                                                1)),
node_data_(file_.get_child_data_set<IndexTraits>(get_node_data_data_set_name(),
                                                   2)),
bond_data_(file_.get_child_data_set<IndexTraits>(get_bond_data_data_set_name(),
                                                   2)),
  frames_hint_(0),
  last_node_(-1), last_vi_(-1)
{
  if (create) {
    file_.set_attribute<CharTraits>("version", std::string("rmf 1"));
  } else {
    std::string version=file_.get_attribute<CharTraits>("version");
    IMP_USAGE_CHECK(version== "rmf 1",
                    "Unsupported rmf version string found: \""
                    << version << "\" expected \"" << "rmf 1" << "\"");
  }
  Ints dim= node_data_.get_size();
  for ( int i=0; i< dim[0]; ++i) {
    if (IndexTraits::get_is_null_value(node_data_.get_value(make_index(i,
                                                                       0)))) {
      free_ids_.push_back(i);
    }
  }

  Ints dimb= bond_data_.get_size();
  for ( int i=0; i< dimb[0]; ++i) {
    if (IndexTraits::get_is_null_value(node_data_.get_value(make_index(i,
                                                                       0)))) {
      free_bonds_.push_back(i);
    }
  }
  if (create) {
    IMP_USAGE_CHECK(!file_.get_has_child("root"),
                    "Already has a root Group");
    add_node("root", ROOT);
  } else {
    IMP_USAGE_CHECK(get_name(0)=="root",
                    "Root node is not so named");
  }
}

SharedData::~SharedData() {
  H5garbage_collect();
}

void SharedData::audit_key_name(std::string name) const {
  if (name.empty()) {
    IMP_RMF_THROW("Empty key name", ValueException);
  }
  static const char *illegal="\\:=()[]{}\"'";
  const char *cur=illegal;
  while (*cur != '\0') {
    if (name.find(*cur) != std::string::npos) {
      IMP_RMF_THROW("Key names can't contain "<< *cur, ValueException);
    }
    ++cur;
  }
  if (name.find("  ") != std::string::npos) {
    IMP_RMF_THROW("Key names can't contain two consecutive spaces",
              ValueException);
  }
}

void SharedData::audit_node_name(std::string name) const {
  if (name.empty()) {
    IMP_RMF_THROW("Empty key name", ValueException);
  }
  static const char *illegal="\"";
  const char *cur=illegal;
  while (*cur != '\0') {
    if (name.find(*cur) != std::string::npos) {
      IMP_RMF_THROW("Node names names can't contain \""<< *cur
                << "\", but \"" << name << "\" does.", ValueException);
    }
    ++cur;
  }
}

void SharedData::check_node(unsigned int node) const {
  IMP_USAGE_CHECK(names_.get_size()[0] > int(node), "Invalid node specified: "
                  << node);
}
int SharedData::add_node(std::string name, unsigned int type) {
  int ret;
  if (free_ids_.empty()) {
    Ints nsz= names_.get_size();
    ret= nsz[0];
    ++nsz[0];
    names_.set_size(nsz);
    Ints dsz= node_data_.get_size();
    dsz[0]= ret+1;
    dsz[1]= std::max(3, dsz[1]);
    node_data_.set_size(dsz);
  } else {
    ret= free_ids_.back();
    free_ids_.pop_back();
  }
  audit_node_name(name);
  names_.set_value(make_index(ret), name);
  node_data_.set_value(make_index(ret, TYPE), type);
  node_data_.set_value(make_index(ret, CHILD), IndexTraits::get_null_value());
  node_data_.set_value(make_index(ret, SIBLING), IndexTraits::get_null_value());
  return ret;
}
void SharedData::set_name(unsigned int node, std::string name) {
  audit_node_name(name);
  names_.set_value(make_index(node), name);
}
int SharedData::get_first_child(unsigned int node) const {
  check_node(node);
  return node_data_.get_value(make_index(node, CHILD));
}
int SharedData::get_sibling(unsigned int node) const {
  check_node(node);
  return node_data_.get_value(make_index(node, SIBLING));
}
void SharedData::set_first_child(unsigned int node, int c) {
  check_node(node);
  return node_data_.set_value(make_index(node, CHILD), c);
}
void SharedData::set_sibling(unsigned int node, int c) {
  check_node(node);
  return node_data_.set_value(make_index(node, SIBLING), c);
}
std::string SharedData::get_name(unsigned int node) const {
  check_node(node);
  return names_.get_value(make_index(node));
}
unsigned int SharedData::get_type(unsigned int node) const {
  check_node(node);
  return node_data_.get_value(make_index(node, TYPE));
}


void SharedData::add_bond( int ida,  int idb,  int type) {
  IMP_USAGE_CHECK(ida>=0 && idb>=0 && type>=0,
                  "Invalid bond " << ida << " " << idb << " " << type);
  int ret;
  if (free_bonds_.empty()) {
    Ints nsz= bond_data_.get_size();
    nsz[1]=std::max(3, nsz[1]);
    ret= nsz[0];
    ++nsz[0];
    bond_data_.set_size(nsz);
  } else {
    ret= free_bonds_.back();
    free_bonds_.pop_back();
  }
  bond_data_.set_value(make_index(ret, 0), ida);
  bond_data_.set_value(make_index(ret, 1), idb);
  bond_data_.set_value(make_index(ret, 2), type);
}

unsigned int SharedData::get_number_of_bonds() const {
  return bond_data_.get_size()[0];
}
boost::tuple<int,int,int> SharedData::get_bond(unsigned int i) const {
  int na= bond_data_.get_value(make_index(i, 0));
  int nb= bond_data_.get_value(make_index(i, 1));
  int t= bond_data_.get_value(make_index(i, 2));
  return boost::tuple<int,int,int>(na, nb, t);
}


int SharedData::add_child(int node, std::string name, int t) {
  int old_child=get_first_child(node);
  int nn= add_node(name, t);
  set_first_child(node, nn);
  set_sibling(nn, old_child);
  return nn;
}

Ints SharedData::get_children(int node) const {
  int cur= get_first_child(node);
  Ints ret;
  while (!IndexTraits::get_is_null_value(cur)) {
    ret.push_back(cur);
    cur= get_sibling(cur);
  }
  return ret;
}



KeyCategories SharedData::get_categories() const {
  KeyCategories ret;
  for (unsigned int i=0; i< file_.get_number_of_children(); ++i) {
    std::string name= file_.get_child_name(i);
    if (name.rfind("list")== name.size()-4) {
      size_t first= name.find('_');
      if (first == std::string::npos) continue;
      std::string trunc(name, first+1);
      std::string catname(trunc, 0, trunc.find('_'));
      bool found=false;
      for (unsigned int j=0; j< ret.size(); ++j) {
        if (ret[j].get_name() == catname) {
          found=true;
          break;
        }
      }
      if (!found) {
        ret.push_back(Category::get_category(catname));
      }
    }
  }
  return ret;
}
IMPRMF_END_INTERNAL_NAMESPACE
