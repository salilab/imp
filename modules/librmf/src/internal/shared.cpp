/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/shared.h>
#include <RMF/NodeHandle.h>
#include <RMF/NodeTupleHandle.h>

namespace RMF {
  namespace internal {

    template <class Traits, unsigned int D>
    HDF5DataSetD<Traits, D> get_data_set_always(HDF5Group parent,
                                               std::string name) {
      if (parent.get_has_child(name)) {
        return parent.get_child_data_set<Traits, D>(name);
      } else {
        return parent.add_child_data_set<Traits, D>(name);
      }
    }

    SharedData::SharedData(HDF5Group g, bool create):
      file_(g),
      node_names_(get_data_set_always<StringTraits, 1>
      (file_, get_node_name_data_set_name())),
      frames_hint_(0)
    {
      for (unsigned int i=0; i< 4; ++i) {
        std::string nm=get_category_name_data_set_name(i+1);
        if (file_.get_has_child(nm)) {
          category_names_[i]= file_.get_child_data_set<StringTraits, 1>(nm);
          for (unsigned int j=0; j< category_names_[i].get_size()[0]; ++j) {
            std::string name
              = category_names_[i].get_value(HDF5DataSetIndex1D(j));
            category_names_cache_[i].push_back(name);
          }
        } else if (!create && i==0) {
          add_category(1, "physics");
          add_category(1, "sequence");
          add_category(1, "shape");
          add_category(1, "feature");
        }
      }
      node_data_[0]=get_data_set_always<IndexTraits, 2>
        (file_, get_node_data_data_set_name());
      for (unsigned int i=0; i< 4; ++i) {
        last_node_[i]=-1;
        last_vi_[i]=-1;
      }
      for (unsigned int i=0; i < 3; ++i) {
        std::string nm=get_tuple_data_data_set_name(i+2);
        if (file_.get_has_child(nm)) {
          node_data_[i+1]
              = get_data_set_always<IndexTraits,
              2>(file_, nm);
          for (unsigned int j=0; j< node_data_[i+1].get_size()[0]; ++j) {
            if (node_data_[i+1].get_value(HDF5DataSetIndexD<2>(j, 0))==-1) {
              free_ids_[i+1].push_back(j);
            }
          }
        }
      }
      if (create) {
        file_.set_attribute<CharTraits>("version", std::string("rmf 1"));
      } else {
        std::string version=file_.get_attribute<CharTraits>("version");
        IMP_RMF_USAGE_CHECK(version== "rmf 1",
                            "Unsupported rmf version string found: \""
                            << version << "\" expected \"" << "rmf 1" << "\"");
      }
      HDF5DataSetIndexD<2> dim= node_data_[0].get_size();
      for (unsigned int i=0; i< dim[0]; ++i) {
        if (IndexTraits::
            get_is_null_value(node_data_[0].get_value(HDF5DataSetIndexD<2>(i,
                                                                        0)))) {
          free_ids_[0].push_back(i);
        }
      }
      if (create) {
        IMP_RMF_USAGE_CHECK(!file_.get_has_child("root"),
                            "Already has a root Group");
        add_node("root", ROOT);
      } else {
        IMP_RMF_USAGE_CHECK(get_name(0)=="root",
                            "Root node is not so named");
      }
    }

    SharedData::~SharedData() {
      H5garbage_collect();
    }

    void SharedData::audit_key_name(std::string name) const {
      if (name.empty()) {
        IMP_RMF_THROW("Empty key name", std::runtime_error);
      }
      static const char *illegal="\\:=()[]{}\"'";
      const char *cur=illegal;
      while (*cur != '\0') {
        if (name.find(*cur) != std::string::npos) {
          IMP_RMF_THROW("Key names can't contain "<< *cur, std::runtime_error);
        }
        ++cur;
      }
      if (name.find("  ") != std::string::npos) {
        IMP_RMF_THROW("Key names can't contain two consecutive spaces",
                      std::runtime_error);
      }
    }

    void SharedData::audit_node_name(std::string name) const {
      if (name.empty()) {
        IMP_RMF_THROW("Empty key name", std::runtime_error);
      }
      static const char *illegal="\"";
      const char *cur=illegal;
      while (*cur != '\0') {
        if (name.find(*cur) != std::string::npos) {
          IMP_RMF_THROW("Node names names can't contain \""<< *cur
                        << "\", but \"" << name << "\" does.",
                        std::runtime_error);
        }
        ++cur;
      }
    }

    void SharedData::check_node(unsigned int node) const {
      IMP_RMF_USAGE_CHECK(node_names_.get_size()[0] > node,
                          "Invalid node specified: "
                          << node);
    }
    int SharedData::add_node(std::string name, unsigned int type) {
      int ret;
      if (free_ids_[0].empty()) {
        HDF5DataSetIndexD<1> nsz= node_names_.get_size();
        ret= nsz[0];
        ++nsz[0];
        node_names_.set_size(nsz);
        HDF5DataSetIndexD<2> dsz= node_data_[0].get_size();
        dsz[0]= ret+1;
        dsz[1]= std::max<hsize_t>(3, dsz[1]);
        node_data_[0].set_size(dsz);
      } else {
        ret= free_ids_[0].back();
        free_ids_[0].pop_back();
      }
      audit_node_name(name);
      node_names_.set_value(HDF5DataSetIndexD<1>(ret), name);
      node_data_[0].set_value(HDF5DataSetIndexD<2>(ret, TYPE), type);
      node_data_[0].set_value(HDF5DataSetIndexD<2>(ret, CHILD),
                           IndexTraits::get_null_value());
      node_data_[0].set_value(HDF5DataSetIndexD<2>(ret, SIBLING),
                           IndexTraits::get_null_value());
      return ret;
    }
    void SharedData::set_name(unsigned int node, std::string name) {
      audit_node_name(name);
      node_names_.set_value(HDF5DataSetIndexD<1>(node), name);
    }
    int SharedData::get_first_child(unsigned int node) const {
      check_node(node);
      return node_data_[0].get_value(HDF5DataSetIndexD<2>(node, CHILD));
    }
    int SharedData::get_sibling(unsigned int node) const {
      check_node(node);
      return node_data_[0].get_value(HDF5DataSetIndexD<2>(node, SIBLING));
    }
    void SharedData::set_first_child(unsigned int node, int c) {
      check_node(node);
      return node_data_[0].set_value(HDF5DataSetIndexD<2>(node, CHILD), c);
    }
    void SharedData::set_sibling(unsigned int node, int c) {
      check_node(node);
      return node_data_[0].set_value(HDF5DataSetIndexD<2>(node, SIBLING), c);
    }
    std::string SharedData::get_name(unsigned int node) const {
      check_node(node);
      return node_names_.get_value(HDF5DataSetIndexD<1>(node));
    }
    unsigned int SharedData::get_type(unsigned int node) const {
      check_node(node);
      return node_data_[0].get_value(HDF5DataSetIndexD<2>(node, TYPE));
    }


  void SharedData::add_bond( int ida,  int idb, int type) {
    int bond=get_category(2, "bond");
    if (bond==-1) {
      bond=add_category(2, "bond");
    }
    IMP_RMF_USAGE_CHECK(ida>=0 && idb>=0,
                        "Invalid bond " << ida << " " << idb);
    RMF::Indexes tp(2);
    tp[0]=ida;
    tp[1]=idb;
    int ind=add_tuple(tp, BOND);
    PairIndexKey pik=get_key<IndexTraits, 2>(bond, "type");
    if (pik==PairIndexKey()) {
      pik= add_key<IndexTraits, 2>(bond, "type", false);
    }
    IMP_RMF_IF_CHECK{
      flush();
      for ( int i=0; i< ind; ++i) {
        boost::tuple<int,int,int> bd= get_bond(i);
      }
    }
    IMP_RMF_USAGE_CHECK(type != -1, "Invalid type passed: " << type);
    set_value<IndexTraits, 2>(ind, pik, type, -1);
    IMP_RMF_IF_CHECK{
      flush();
      for ( int i=0; i< ind+1; ++i) {
        boost::tuple<int,int,int> bd= get_bond(i);
      }
    }
  }

    unsigned int SharedData::get_number_of_bonds() const {
      // not really right
      return get_number_of_tuples(2);
    }
    boost::tuple<int,int,int> SharedData::get_bond(unsigned int i) const {
      int bond=get_category(2, "bond");
      int na= get_tuple_member(2, i, 0);
      int nb= get_tuple_member(2, i, 1);
      PairIndexKey pik=get_key<IndexTraits, 2>(bond, "type");
      int t= get_value<IndexTraits, 2>(i, pik, -1);
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

  void SharedData::check_tuple(int arity, unsigned int index) const {
    IMP_RMF_USAGE_CHECK(node_data_[arity-1] != HDF5IndexDataSet2D(),
                        "Invalid tuple arity requested: " << arity);
    IMP_RMF_USAGE_CHECK(node_data_[arity-1]
                        .get_value(HDF5DataSetIndexD<2>(index,
                                                        0))
                        >=0,
                        "Invalid type for tuple: " << arity << " and "
                        << index);
    for ( int i=0; i< arity; ++i) {
      int cur=node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(index,
                                                                i+1));
      check_node(cur);
    }
  }

  unsigned int SharedData::get_number_of_tuples(int arity) const {
    if (node_data_[arity-1]==HDF5IndexDataSet2D()) {
      return 0;
    }
    HDF5DataSetIndexD<2> sz= node_data_[arity-1].get_size();
    unsigned int ct=0;
    for (unsigned int i=0; i< sz[0]; ++i) {
      if (node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(i, 0)) >=0) {
        ++ct;
      }
    }
    return ct;
  }
  RMF::Indexes SharedData::get_tuple_indexes(int arity) const {
    if (node_data_[arity-1]==HDF5IndexDataSet2D()) {
      return RMF::Indexes();
    }
    HDF5DataSetIndexD<2> sz= node_data_[arity-1].get_size();
    RMF::Indexes ret;
    for (unsigned int i=0; i< sz[0]; ++i) {
      if (node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(i, 0)) >=0) {
        ret.push_back(i);
      }
    }
    return ret;
  }
  unsigned int SharedData::add_tuple(const RMF::Indexes &nis, int t) {
    const int arity=nis.size();
    if (node_data_[arity-1]==HDF5IndexDataSet2D()) {
      std::string nm=get_tuple_data_data_set_name(arity);
      node_data_[arity-1]
          = file_.add_child_data_set<IndexTraits, 2>(nm);
    }
    int slot;
    if (free_ids_[arity-1].empty()) {
      slot= node_data_[arity-1].get_size()[0];
      int nsz=std::max<int>(arity+1, node_data_[arity-1].get_size()[1]);
      node_data_[arity-1].set_size(HDF5DataSetIndexD<2>(slot+1,
                                                        nsz));
    } else {
      slot= free_ids_[arity-1].back();
      free_ids_[arity-1].pop_back();
    }
    node_data_[arity-1].set_value(HDF5DataSetIndexD<2>(slot, 0), t);
    for ( int i=0; i< arity; ++i) {
      node_data_[arity-1].set_value(HDF5DataSetIndexD<2>(slot, i+1), nis[i]);
    }
    check_tuple(arity, slot);
    return slot;
  }
  unsigned int SharedData::get_tuple_member(int arity, unsigned int index,
                                            int member_index) const {
    check_tuple(arity, index);
    return node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(index,
                                                             member_index+1));
  }
  unsigned int SharedData::get_tuple_type(int arity, unsigned int index) const {
    check_tuple(arity, index);
    return node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(index, 0));
  }


  int SharedData::add_category(int Arity, std::string name) {
    IMP_RMF_USAGE_CHECK(get_category(Arity, name)==-1,
                        "File already has category " << name
                        << " with arity " << Arity);
    if (category_names_[Arity-1]
        == HDF5DataSetD<StringTraits, 1>()) {
      std::string nm=get_category_name_data_set_name(Arity);
      category_names_[Arity-1]
          =file_.add_child_data_set<StringTraits, 1>(nm);
    }
    // fill in later
    int sz= category_names_[Arity-1].get_size()[0];
    category_names_[Arity-1].set_size(HDF5DataSetIndex1D(sz+1));
    category_names_[Arity-1].set_value(HDF5DataSetIndex1D(sz), name);
    category_names_cache_[Arity-1]
      .resize(std::max<int>(sz+1,
                            category_names_cache_[Arity-1].size()));
    category_names_cache_[Arity-1][sz]=name;
    return sz;
  }
  int SharedData::get_category(int Arity, std::string name) const {
    if (category_names_cache_[Arity-1].empty()) {
      return -1;
    } else {
      Ints cs= get_categories(Arity);
      for (unsigned int i=0; i< cs.size(); ++i) {
        if (get_category_name(Arity, cs[i]) == name) {
          return cs[i];
        }
      }
    }
    return -1;
  }
    Strings SharedData::get_category_names(int Arity) const {
      Ints cats= get_categories(Arity);
      Strings ret(cats.size());
      for (unsigned int i=0; i< ret.size(); ++i) {
        ret[i]= get_category_name(Arity, cats[i]);
      }
      return ret;
    }
  Ints SharedData::get_categories(int Arity) const {
    unsigned int sz= category_names_cache_[Arity-1].size();
    IMP_RMF_INTERNAL_CHECK((!category_names_[Arity-1] && sz==0)
                           || (sz == category_names_[Arity-1].get_size()[0]),
                           "Cache and data set sizes don't match: "
                           << sz << " vs "
                           << category_names_[Arity-1].get_size()[0]);
    Ints ret;
    for (unsigned int i=0; i< sz; ++i) {
      if (!category_names_cache_[Arity-1].empty()) {
        ret.push_back(i);
      }
    }
    return ret;
  }

  } // namespace internal
} /* namespace RMF */
