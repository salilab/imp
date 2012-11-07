/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/HDF5SharedData.h>
#include <RMF/NodeHandle.h>
#include <RMF/Validator.h>
#include <RMF/internal/set.h>
#include <algorithm>

namespace RMF {
  namespace internal {

#define RMF_CLOSE(lcname, Ucname, PassValue, ReturnValue,               \
                  PassValues, ReturnValues)                             \
    lcname##_data_sets_=DataDataSetCache2D<Ucname##Traits>();           \
    per_frame_##lcname##_data_sets_=DataDataSetCache3D<Ucname##Traits>()

    void HDF5SharedData::close_things() {
      node_names_.reset();
      for (unsigned int i=0; i< 4; ++i) {
        node_data_[i].reset();
        free_ids_[i].clear();
        index_cache_[i]=IndexCache();
        key_name_data_sets_[i]= KeyNameDataSetCache();
        category_names_[i].reset();
      }
      frame_names_.reset();
      max_cache_.clear();
      RMF_FOREACH_TYPE(RMF_CLOSE);
      flush();
      file_=HDF5Group();
      H5garbage_collect();
    }
    void HDF5SharedData::open_things(bool create, bool read_only) {
      read_only_=read_only;
      if (create) {
        file_=create_hdf5_file(get_file_path());
        RMF_OPERATION(
                      file_.set_attribute<CharTraits>("version", std::string("rmf 1")),
                      "adding version string to file.");
        {
          HDF5DataSetCreationPropertiesD<StringTraits, 1> props;
          props.set_compression(GZIP_COMPRESSION);
          RMF_OPERATION((file_.add_child_data_set<StringTraits, 1>)
                        (get_node_name_data_set_name(), props);,
                        "adding node name data set to file.");
        }
        {
          HDF5DataSetCreationPropertiesD<IndexTraits, 2> props;
          props.set_compression(GZIP_COMPRESSION);
          props.set_chunk_size(RMF::HDF5DataSetIndexD<2>(128, 4));
          RMF_OPERATION(
                        (file_.add_child_data_set<IndexTraits, 2>)
                        (get_node_data_data_set_name(),
                         props);,
                        "adding node data data set to file.");
        }
      } else {
        if (read_only) {
          // walk around type checking
          file_=open_hdf5_file_read_only_returning_nonconst(get_file_path());
        } else {
          file_=open_hdf5_file(get_file_path());
        }
        std::string version;
        RMF_OPERATION(
                      version=file_.get_attribute<CharTraits>("version"),
                      "reading version string from file.");
        RMF_USAGE_CHECK(version== "rmf 1",
                        get_error_message("Unsupported rmf version ",
                                          "string found: \"",
                                          version , "\" expected \"" ,
                                          "rmf 1" , "\""));
      }
      node_names_.set(file_, get_node_name_data_set_name());
      node_data_[0].set(file_, get_node_data_data_set_name());
      for (unsigned int i=0; i< 4; ++i) {
        initialize_categories(i, create);
        initialize_keys(i);
      }
      initialize_free_nodes();
      std::string frn=get_frame_name_data_set_name();
      frame_names_.set(file_, frn);
    }

    void HDF5SharedData::initialize_categories(int i, bool) {
      std::string nm=get_category_name_data_set_name(i+1);
      category_names_[i].set(file_, nm);
    }

    void HDF5SharedData::initialize_keys(int i) {
      std::string nm=get_set_data_data_set_name(i+1);
    }

    void HDF5SharedData::initialize_free_nodes() {
      HDF5DataSetIndexD<2> dim= node_data_[0].get_size();
      for (unsigned int i=0; i< dim[0]; ++i) {
        if (IndexTraits::
            get_is_null_value(node_data_[0].get_value(HDF5DataSetIndexD<2>(i,
                                                                           0)))) {
          free_ids_[0].push_back(i);
        }
      }
    }

    HDF5SharedData::HDF5SharedData(std::string g, bool create, bool read_only):
    SharedData(g), frames_hint_(0), link_category_(-1)
    {
      RMF_BEGIN_FILE;
      RMF_BEGIN_OPERATION;
      open_things(create, read_only);
      if (create) {
        add_node("root", ROOT);
      } else {
        RMF_USAGE_CHECK(get_name(0)=="root",
                        "Root node is not so named");
        for (unsigned int i=0; i< get_number_of_categories(); ++i) {
          if (get_category_name(i)== "link") {
            link_category_=i; break;
          }
        }
        if (link_category_!=-1) {
          link_key_= get_key_impl<NodeIDTraits>(link_category_,
                                                 "linked",
                                                 false);
          RMF_INTERNAL_CHECK(link_key_ != NodeIDKey(),
                             "Bad link key found in init");
        }
      }
      RMF_END_OPERATION("initializing");
      RMF_END_FILE(get_file_name());
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
      for (unsigned int i=0; i< 4; ++i) {
        category_names_[i].flush();
        node_data_[i].flush();
      }
    }

    void HDF5SharedData::check_node(unsigned int node) const {
      RMF_USAGE_CHECK(node_names_.get_size()[0] > node,
                      get_error_message("Invalid node specified: ",
                                        node));
    }

    int HDF5SharedData::add_node(std::string name, unsigned int type) {
      RMF_BEGIN_FILE;
      int ret;
      RMF_BEGIN_OPERATION;
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
      RMF_END_OPERATION("figuring out where to add node " << name);
      audit_node_name(name);
      RMF_BEGIN_OPERATION
        node_names_.set_value(HDF5DataSetIndexD<1>(ret), name);
      node_data_[0].set_value(HDF5DataSetIndexD<2>(ret, TYPE), type);
      node_data_[0].set_value(HDF5DataSetIndexD<2>(ret, CHILD),
                              IndexTraits::get_null_value());
      node_data_[0].set_value(HDF5DataSetIndexD<2>(ret, SIBLING),
                              IndexTraits::get_null_value());
      return ret;
      RMF_END_OPERATION("adding node data");
      RMF_END_FILE(get_file_name());
    }
    int HDF5SharedData::get_first_child(unsigned int node) const {
      check_node(node);
      return node_data_[0].get_value(HDF5DataSetIndexD<2>(node, CHILD));
    }
    int HDF5SharedData::get_sibling(unsigned int node) const {
      check_node(node);
      return node_data_[0].get_value(HDF5DataSetIndexD<2>(node, SIBLING));
    }
    void HDF5SharedData::set_first_child(unsigned int node, int c) {
      check_node(node);
      return node_data_[0].set_value(HDF5DataSetIndexD<2>(node, CHILD), c);
    }
    void HDF5SharedData::set_sibling(unsigned int node, int c) {
      check_node(node);
      return node_data_[0].set_value(HDF5DataSetIndexD<2>(node, SIBLING), c);
    }
    std::string HDF5SharedData::get_name(unsigned int node) const {
      if (node < get_number_of_real_nodes()) {
        check_node(node);
        return node_names_.get_value(HDF5DataSetIndexD<1>(node));
      } else {
        return "bond";
      }
    }
    unsigned int HDF5SharedData::get_type(unsigned int index) const {
      if (index < get_number_of_real_nodes()) {
        check_node(index);
        return node_data_[0].get_value(HDF5DataSetIndexD<2>(index, TYPE));
      } else {
        return BOND;
      }
    }


    int HDF5SharedData::add_child(int node, std::string name, int t) {
      int old_child=get_first_child(node);
      int nn= add_node(name, t);
      set_first_child(node, nn);
      set_sibling(nn, old_child);
      return nn;
    }

    void HDF5SharedData::add_child(int node, int child_node) {
      RMF_INTERNAL_CHECK(-1 != child_node,
                         "Bad child being added");
      init_link();
      int link= add_child(node, "link", LINK);
      set_value_impl(link, link_key_, NodeID(child_node), -1);
      RMF_INTERNAL_CHECK(get_linked(link)== child_node,
                         "Return does not match");
    }

    void HDF5SharedData::init_link() {
      if (link_category_ !=-1) {
        RMF_INTERNAL_CHECK(link_key_ != NodeIDKey(),
                           "Invalid link key");
        return;
      }
      link_category_=add_category("link");
      link_key_= add_key_impl<NodeIDTraits>(link_category_,
                                             "linked", false);
      RMF_INTERNAL_CHECK(link_key_ != NodeIDKey(),
                         "Invalid link key after add");
    }

    int HDF5SharedData::get_linked(int node) const {
      RMF_INTERNAL_CHECK(link_category_==-1 || link_key_ != NodeIDKey(),
                         "Invalid link key but valid category");
      int ret= get_value(node, link_key_).get_index();
      RMF_INTERNAL_CHECK(ret >= 0, "Bad link value found");
      return ret;
    }

    Ints HDF5SharedData::get_children(int node) const {
      if (node < get_number_of_real_nodes()) {
        int cur= get_first_child(node);
        Ints ret;
        while (!IndexTraits::get_is_null_value(cur)) {
          if (get_type(cur) != LINK) {
            ret.push_back(cur);
            cur= get_sibling(cur);
          } else {
            ret.push_back(get_linked(cur));
            cur= get_sibling(cur);
          }
        }
        std::reverse(ret.begin(), ret.end());

        if (node==0) {
          for (unsigned int i=0; i< get_number_of_sets(2); ++i) {
            ret.push_back(get_number_of_real_nodes()+i);
          }
        }
        return ret;
      } else {
        Ints ret(2);
        ret[0]= get_set_member(2, node-get_number_of_real_nodes(), 0);
        ret[1]= get_set_member(2, node-get_number_of_real_nodes(), 1);
        return ret;
      }
    }
    unsigned int HDF5SharedData::get_number_of_sets(int arity) const {
      HDF5DataSetIndexD<2> sz= node_data_[arity-1].get_size();
      unsigned int ct=0;
      for (unsigned int i=0; i< sz[0]; ++i) {
        if (node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(i, 0)) >=0) {
          ++ct;
        }
      }
      return ct;
    }
    unsigned int HDF5SharedData::get_set_member(int arity, unsigned int index,
                                                int member_index) const {
      return node_data_[arity-1].get_value(HDF5DataSetIndexD<2>(index,
                                                                member_index+1));
    }

    int HDF5SharedData::add_category(std::string name) {
      RMF_BEGIN_FILE;
      RMF_BEGIN_OPERATION
        // fill in later
        int sz= category_names_[1-1].get_size()[0];
      category_names_[1-1].set_size(HDF5DataSetIndex1D(sz+1));
      category_names_[1-1].set_value(HDF5DataSetIndex1D(sz), name);
      return sz;
      RMF_END_OPERATION("adding category to list");
      RMF_END_FILE(get_file_name());
    }
    unsigned int HDF5SharedData::get_number_of_categories() const {
      unsigned int sz= category_names_[1-1].get_size()[0];
      return sz;
    }

#define RMF_SEARCH_KEYS(lcname, Ucname, PassValue, ReturnValue, \
                        PassValues, ReturnValues)               \
    {                                                           \
      unsigned int keys                                         \
        = get_number_of_keys_impl<Ucname##Traits>(i, true);     \
      for (unsigned int j=0; j< keys; ++j) {                    \
        Key<Ucname##Traits> k(cat, j, true);                    \
        ret=std::max<int>(ret, get_number_of_frames(k));        \
      }                                                         \
    }

    unsigned int HDF5SharedData::get_number_of_frames() const {
      unsigned int cats= get_number_of_categories();
      int ret=0;
      for (unsigned int i=0; i< cats; ++i) {
        Category cat(i);
        RMF_FOREACH_TYPE(RMF_SEARCH_KEYS);
      }
      return ret;
    }


    std::string HDF5SharedData::get_description() const {
      if (!get_group().get_has_attribute("description")) {
        return std::string();
      } else return get_group().get_char_attribute("description");
    }
    void HDF5SharedData::set_description(std::string str) {
      RMF_USAGE_CHECK(str.empty()
                      || str[str.size()-1]=='\n',
                      "Description should end in a newline.");
      get_group().set_char_attribute("description", str);
    }

    void HDF5SharedData::set_frame_name(std::string str) {
      if (frame_names_.get_size()[0] <= get_current_frame()) {
        frame_names_.set_size(HDF5DataSetIndexD<1>(get_current_frame()+1));
      }
      frame_names_.set_value(HDF5DataSetIndexD<1>(get_current_frame()), str);
    }
    std::string HDF5SharedData::get_frame_name() const {
      if (frame_names_.get_size()[0] > get_current_frame()) {
        return frame_names_.get_value(HDF5DataSetIndexD<1>(get_current_frame()));
      } else {
        return std::string();
      }
    }



    void HDF5SharedData::reload() {
      close_things();
      open_things(false, read_only_);
    }

#define RMF_HDF5_SET_FRAME(lcname, Ucname, PassValue, ReturnValue, \
                                 PassValues, ReturnValues)\
    per_frame_##lcname##_data_sets_.set_current_frame(frame);


    void HDF5SharedData::set_current_frame(int frame) {
      SharedData::set_current_frame(frame);
      if (frame >=0) {
        RMF_FOREACH_TYPE(RMF_HDF5_SET_FRAME);
      }
    }
  } // namespace internal
} /* namespace RMF */
