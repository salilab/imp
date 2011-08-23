/**
 *  \file IMP/rmf/RootHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ROOT_HANDLE_H
#define IMPRMF_ROOT_HANDLE_H

#include "rmf_config.h"
#include "hdf5_wrapper.h"
#include "internal/shared.h"
#include "Key.h"
#include "NodeHandle.h"

IMPRMF_BEGIN_NAMESPACE
class NodeHandle;

typedef std::pair<NodeHandle, NodeHandle> BondPair;
typedef std::vector<BondPair> BondPairs;

/** A handle for an hdf5 root */
class IMPRMFEXPORT RootHandle: public NodeHandle {
  void gather_ids(NodeHandle n, Ints &ids,
                  std::vector<std::string> &paths,
                  std::string path) const;
  friend class NodeHandle;
  RootHandle(internal::SharedData *shared_);
 public:
  //! Empty root handle, no open file.
  RootHandle(){}

#ifndef IMP_DOXYGEN
  RootHandle(HDF5Group group, bool create);
#endif

  /** Get an existing key that has the given name of the
      given type.
  */
  template <class TypeT>
    Key<TypeT> get_key(Category category_id, std::string name) const {
    return shared_->get_key<TypeT>(category_id, name);
  }
  //! Lift NodeHandle::get_name() into class scope
  std::string get_name() const {
    return NodeHandle::get_name();
  }
  /** Create a key for a new type of data. There must not
      already be a key with the same name of any type.
  */
  template <class TypeT>
    Key<TypeT> add_key(Category category_id,
                       std::string name, bool per_frame) const {
    return shared_->add_key<TypeT>(category_id, name, per_frame);
  }
  template <class TypeT>
    bool get_has_key(Category category_id,
                     std::string name) const {
    return shared_->get_key<TypeT>(category_id, name) != Key<TypeT>();
  }
  /** Get a list of all keys of the given type,
   */
  template <class TypeT>
  std::vector<Key<TypeT> > get_keys(Category category_id) const {
    return shared_->get_keys<TypeT>(category_id);
  }
  /** \name Non-template versions for python
      @{
  */
#define IMP_HDF5_ROOT_KEY_TYPE_METHODS(lcname, UCName)                  \
  UCName##Key get_##lcname##_key(Category category_id,               \
                                 std::string nm) const {                \
    return get_key<UCName##Traits>(category_id, nm);                    \
  }                                                                     \
  bool get_has_##lcname##_key(Category category_id,                  \
                                     std::string nm) const {            \
    return get_has_key<UCName##Traits>(category_id, nm);                \
  }                                                                     \
  UCName##Key add_##lcname##_key(Category category_id,               \
                                 std::string nm,                        \
                                 bool per_frame) const {                \
    return add_key<UCName##Traits>(category_id, nm, per_frame);         \
  }                                                                     \
  std::string get_name(UCName##Key k) const {                           \
    return shared_->get_name(k);                                        \
  }                                                                     \
  UCName##Key##s                                                        \
    get_##lcname##_keys(Category category_id) const {                \
    return get_keys<UCName##Traits>(category_id);                       \
  }                                                                     \
  unsigned int get_number_of_frames(UCName##Key k) const {              \
    return shared_->get_number_of_frames(k);                            \
  }                                                                     \
  bool get_is_per_frame(UCName##Key k) const {                          \
    return shared_->get_is_per_frame(k);                                \
  }


  IMP_HDF5_ROOT_KEY_TYPE_METHODS(int, Int);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(float, Float);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(string, String);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(index, Index);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(node_id, NodeID);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(data_set, DataSet);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(node_ids, NodeIDs);
  /** @} */
#ifdef IMP_DOXYGEN
  /** \name Python only
      The following methods are only available in python.
      @{
  */
  //! Return a list with all the keys from that category
  PythonList get_keys(Category c) const;
  /** @} */
#endif
  /** Each node in the hierarchy can be associated with some arbitrary bit
      of external data using a void* pointer. Nodes can be extracted using
      these bits of data.
  */
  NodeHandle get_node_handle_from_association(void*d) const;
  NodeHandle get_node_handle_from_id(NodeID id) const;
  void show(std::ostream &out= std::cout) const {
    out << "RootHandle";
  }
  /** \name Bonds
      The hierarchy also contains information about bonds connecting
      arbitrary nodes in the hierarchy.
      @{
  */
  unsigned int get_number_of_bonds() const {
    return shared_->get_number_of_bonds();
  }
  BondPair get_bond(unsigned int i) const {
    boost::tuple<int,int,int> t= shared_->get_bond(i);
    return std::make_pair(get_node_handle_from_id(t.get<0>()),
                          get_node_handle_from_id(t.get<1>()));
  }
  void add_bond(NodeHandle na, NodeHandle nb, unsigned int type) {
    NodeID ida= na.get_id();
    NodeID idb= nb.get_id();
    shared_->add_bond(ida.get_index(), idb.get_index(), type);
  }
  BondPairs get_bonds() const;
  /** @} */
  HDF5Group get_hdf5_group() const {
    return shared_->get_group();
  }
  /** Suggest how many frames the file is likely to have. This can
      make writing more efficient as space will be preallocated.
  */
  void set_number_of_frames_hint(unsigned int i) {
    shared_->save_frames_hint(i);
  }
  /** \name Descriptions
      Each RMF structure has an associated description. This should
      consist of unstructured text describing the contents of the RMF
      data. Conventionally. this description can consist of multiple
      paragraphs, each separated by a newline character and should end
      in a newline.
      @{
  */
  std::string get_description() const;
  void set_description(std::string descr);
  /** @} */

  /** \name Key categories
      Methods for managing the key categories in this RMF.
      @{
  */
  Category add_category(std::string name);
  bool get_has_category(std::string name) const;
  Categories get_categories() const;
  /** @} */

  /** Make sure all data gets written to disk.
   */
  void flush();
};

typedef std::vector<RootHandle> RootHandles;

/** Create an RMF from a file system path.*/
inline RootHandle create_rmf_file(std::string path) {
  return RootHandle(create_hdf5_file(path), true);
}

/** Open an RMF from a file system path.*/
inline RootHandle open_rmf_file(std::string path) {
  return RootHandle(open_hdf5_file(path), false);
}

/** Open an RMF from a file system path.*/
inline RootHandle open_rmf_file_read_only(std::string path) {
  return RootHandle(open_hdf5_file_read_only(path), false);
}

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ROOT_HANDLE_H */
