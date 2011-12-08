/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/shared.h>
#include <RMF/NodeHandle.h>
#include <RMF/NodeSetHandle.h>
#include <RMF/Validator.h>

namespace RMF {
  namespace internal {

  void SharedData::initialize_categories(int i, bool create) {
    std::string nm=get_category_name_data_set_name(i+1);
    if (file_.get_has_child(nm)) {
      category_names_[i]= file_.get_child_data_set<StringTraits, 1>(nm);
      for (unsigned int j=0; j< category_names_[i].get_size()[0]; ++j) {
        std::string name
            = category_names_[i].get_value(HDF5DataSetIndex1D(j));
        category_names_cache_[i].push_back(name);
      }
    } else if (i==0 && !create) {
      if (!file_.get_file().get_is_writable()) {
      // backward compatibility
        category_names_cache_[i].push_back("physics");
        category_names_cache_[i].push_back("sequence");
        category_names_cache_[i].push_back("shape");
        category_names_cache_[i].push_back("feature");
      } else {
        add_category(1, "physics");
        add_category(1, "sequence");
        add_category(1, "shape");
        add_category(1, "feature");
      }
    }
  }

  void SharedData::initialize_keys(int i) {
    std::string nm=get_set_data_data_set_name(i+2);
    if (file_.get_has_child(nm)) {
      node_data_[i+1]
          = file_.get_child_data_set<IndexTraits, 2>(nm);
      for (unsigned int j=0; j< node_data_[i+1].get_size()[0]; ++j) {
        if (node_data_[i+1].get_value(HDF5DataSetIndexD<2>(j, 0))==-1) {
          free_ids_[i+1].push_back(j);
        }
      }
    }
  }

  void SharedData::initialize_free_nodes() {
    HDF5DataSetIndexD<2> dim= node_data_[0].get_size();
    for (unsigned int i=0; i< dim[0]; ++i) {
      if (IndexTraits::
          get_is_null_value(node_data_[0].get_value(HDF5DataSetIndexD<2>(i,
                                                                         0)))) {
        free_ids_[0].push_back(i);
      }
    }
  }

  SharedData::SharedData(HDF5Group g, bool create):
      file_(g), frames_hint_(0)
  {
    IMP_RMF_BEGIN_FILE;
    IMP_RMF_BEGIN_OPERATION;
    if (create) {
      IMP_RMF_OPERATION(
          file_.set_attribute<CharTraits>("version", std::string("rmf 1")),
          "adding version string to file.");
      IMP_RMF_OPERATION(
          node_names_
          =(file_.add_child_data_set<StringTraits, 1>)
          (get_node_name_data_set_name());,
          "adding node name data set to file.");
      IMP_RMF_OPERATION(
          node_data_[0]=(file_.add_child_data_set<IndexTraits, 2>)
          (get_node_data_data_set_name());,
          "adding node data data set to file.");
    } else {
      std::string version;
      IMP_RMF_OPERATION(
          version=file_.get_attribute<CharTraits>("version"),
          "reading version string from file.");
      IMP_RMF_USAGE_CHECK(version== "rmf 1",
                          "Unsupported rmf version string found: \""
                          << version << "\" expected \"" << "rmf 1" << "\"");
      IMP_RMF_OPERATION(
          node_names_=(file_.get_child_data_set<StringTraits, 1>)
          (get_node_name_data_set_name());,
          "opening node name data set.");
      IMP_RMF_OPERATION(
          node_data_[0]=(file_.get_child_data_set<IndexTraits, 2>)
          (get_node_data_data_set_name());,
          "opening node child data set.");
    }
      for (unsigned int i=0; i< 4; ++i) {
        initialize_categories(i, create);
        initialize_keys(i);
        // clear caches
        last_node_[i]=-1;
        last_vi_[i]=-1;
      }
      initialize_free_nodes();
      if (create) {
        add_node("root", ROOT);
      } else {
        IMP_RMF_USAGE_CHECK(get_name(0)=="root",
                            "Root node is not so named");
      }
      IMP_RMF_END_OPERATION("initializing");
      IMP_RMF_END_FILE(get_file_name());
    }

    SharedData::~SharedData() {
      // kind of nasty, needed to avoid recursion
      add_ref();
      validate();
      release();
      H5garbage_collect();
    }

    void SharedData::audit_key_name(std::string name) const {
      if (name.empty()) {
        IMP_RMF_THROW("Empty key name", UsageException);
      }
      static const char *illegal="\\:=()[]{}\"'";
      const char *cur=illegal;
      while (*cur != '\0') {
        if (name.find(*cur) != std::string::npos) {
          IMP_RMF_THROW("Key names can't contain "<< *cur, UsageException);
        }
        ++cur;
      }
      if (name.find("  ") != std::string::npos) {
        IMP_RMF_THROW("Key names can't contain two consecutive spaces",
                      UsageException);
      }
    }

    void SharedData::audit_node_name(std::string name) const {
      if (name.empty()) {
        IMP_RMF_THROW("Empty key name", UsageException);
      }
      static const char *illegal="\"";
      const char *cur=illegal;
      while (*cur != '\0') {
        if (name.find(*cur) != std::string::npos) {
          IMP_RMF_THROW("Node names names can't contain \""<< *cur
                        << "\", but \"" << name << "\" does.",
                        UsageException);
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
      IMP_RMF_BEGIN_FILE;
      int ret;
      IMP_RMF_BEGIN_OPERATION;
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
      IMP_RMF_END_OPERATION("figuring out where to add node " << name);
      audit_node_name(name);
      IMP_RMF_BEGIN_OPERATION
      node_names_.set_value(HDF5DataSetIndexD<1>(ret), name);
      node_data_[0].set_value(HDF5DataSetIndexD<2>(ret, TYPE), type);
      node_data_[0].set_value(HDF5DataSetIndexD<2>(ret, CHILD),
                           IndexTraits::get_null_value());
      node_data_[0].set_value(HDF5DataSetIndexD<2>(ret, SIBLING),
                           IndexTraits::get_null_value());
      return ret;
      IMP_RMF_END_OPERATION("adding node data");
      IMP_RMF_END_FILE(get_file_name());
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
    int ind=add_set(tp, BOND);
    PairIndexKey pik=get_key<IndexTraits, 2>(bond, "type");
    if (pik==PairIndexKey()) {
      pik= add_key<IndexTraits, 2>(bond, "type", false);
    }
    IMP_RMF_IF_CHECK{
      flush();
      for ( int i=0; i< ind; ++i) {
        /*boost::tuple<int,int,int> bd=*/ get_bond(i);
      }
    }
    IMP_RMF_USAGE_CHECK(type != -1, "Invalid type passed: " << type);
    set_value<IndexTraits, 2>(ind, pik, type, -1);
    IMP_RMF_IF_CHECK{
      flush();
      for ( int i=0; i< ind+1; ++i) {
        /*boost::tuple<int,int,int> bd=*/ get_bond(i);
      }
    }
  }

    unsigned int SharedData::get_number_of_bonds() const {
      // not really right
      return get_number_of_sets(2);
    }
    boost::tuple<int,int,int> SharedData::get_bond(unsigned int i) const {
      int bond=get_category(2, "bond");
      int na= get_set_member(2, i, 0);
      int nb= get_set_member(2, i, 1);
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

  void SharedData::check_set(int arity, unsigned int index) const {
    IMP_RMF_USAGE_CHECK(node_data_[arity-1] != HDF5IndexDataSet2D(),
                        "Invalid set arity requested: " << arity);
    IMP_RMF_USAGE_CHECK(node_data_[arity-1]
                        .get_value(HDF5DataSetIndexD<2>(index,
                                                        0))
                        >=0,
                        "Invalid type for set: " << arity << " and "
                        << index);
    for ( int i=0; i< arity; ++i) {
      int cur=node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(index,
                                                                i+1));
      check_node(cur);
    }
  }

  unsigned int SharedData::get_number_of_sets(int arity) const {
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
  RMF::Indexes SharedData::get_set_indexes(int arity) const {
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
  unsigned int SharedData::add_set( RMF::Indexes nis, int t) {
    IMP_RMF_BEGIN_FILE;
    std::sort(nis.begin(), nis.end());
    const int arity=nis.size();
    IMP_RMF_BEGIN_OPERATION;
    if (node_data_[arity-1]==HDF5IndexDataSet2D()) {
      std::string nm=get_set_data_data_set_name(arity);
      node_data_[arity-1]
          = file_.add_child_data_set<IndexTraits, 2>(nm);
    }
    IMP_RMF_END_OPERATION("adding data set to store set");
    int slot;
    if (free_ids_[arity-1].empty()) {
      IMP_RMF_BEGIN_OPERATION
      slot= node_data_[arity-1].get_size()[0];
      int nsz=std::max<int>(arity+1, node_data_[arity-1].get_size()[1]);
      node_data_[arity-1].set_size(HDF5DataSetIndexD<2>(slot+1,
                                                        nsz));
      IMP_RMF_END_OPERATION("allocating new slot for set");
    } else {
      slot= free_ids_[arity-1].back();
      free_ids_[arity-1].pop_back();
    }
    IMP_RMF_BEGIN_OPERATION
    node_data_[arity-1].set_value(HDF5DataSetIndexD<2>(slot, 0), t);
    for ( int i=0; i< arity; ++i) {
      node_data_[arity-1].set_value(HDF5DataSetIndexD<2>(slot, i+1), nis[i]);
    }
    IMP_RMF_END_OPERATION("storing set data");
    check_set(arity, slot);
    return slot;
    IMP_RMF_END_FILE(get_file_name());
  }
  unsigned int SharedData::get_set_member(int arity, unsigned int index,
                                            int member_index) const {
    check_set(arity, index);
    return node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(index,
                                                             member_index+1));
  }
  unsigned int SharedData::get_set_type(int arity, unsigned int index) const {
    check_set(arity, index);
    return node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(index, 0));
  }


  int SharedData::add_category(int Arity, std::string name) {
    IMP_RMF_BEGIN_FILE;
    IMP_RMF_INTERNAL_CHECK((!category_names_[Arity-1]
                            && category_names_cache_[Arity-1].empty())
                           || ( category_names_cache_[Arity-1].size()
                                == category_names_[Arity-1].get_size()[0]),
                           "Cache and data set sizes don't match: "
                           << category_names_cache_[Arity-1].size() << " vs "
                           << category_names_[Arity-1].get_size()[0]);
    IMP_RMF_USAGE_CHECK(get_category(Arity, name)==-1,
                        "File already has category " << name
                        << " with arity " << Arity);
    if (category_names_[Arity-1]
        == HDF5DataSetD<StringTraits, 1>()) {
      IMP_RMF_BEGIN_OPERATION;
      std::string nm=get_category_name_data_set_name(Arity);
      category_names_[Arity-1]
          =file_.add_child_data_set<StringTraits, 1>(nm);
      IMP_RMF_END_OPERATION("add category list data set");
    }
    IMP_RMF_BEGIN_OPERATION
    // fill in later
    int sz= category_names_[Arity-1].get_size()[0];
    category_names_[Arity-1].set_size(HDF5DataSetIndex1D(sz+1));
    category_names_[Arity-1].set_value(HDF5DataSetIndex1D(sz), name);
    category_names_cache_[Arity-1]
      .resize(std::max<int>(sz+1,
                            category_names_cache_[Arity-1].size()));
    category_names_cache_[Arity-1][sz]=name;
    return sz;
    IMP_RMF_END_OPERATION("adding category to list");
    IMP_RMF_END_FILE(get_file_name());
  }
  int SharedData::get_category(int Arity, std::string name) const {
    if (category_names_cache_[Arity-1].empty()) {
      return -1;
    } else {
      for (unsigned int i=0; i< category_names_cache_[Arity-1].size(); ++i) {
        if (category_names_cache_[Arity-1][i]==name) return i;
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
    Ints ret;
    for (unsigned int i=0; i< sz; ++i) {
      if (!category_names_cache_[Arity-1][i].empty()) {
        ret.push_back(i);
      }
    }
    return ret;
  }

  void SharedData::validate() const {
    Creators cs= get_validators();
    for (unsigned int i=0; i< cs.size(); ++i) {
      boost::scoped_ptr<Validator>
          ptr(cs[i]->create(RootHandle(const_cast<SharedData*>(this))));
      ptr->write_errors(std::cerr);
    }
  }

  } // namespace internal
} /* namespace RMF */
