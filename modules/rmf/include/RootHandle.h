/**
 *  \file IMP/RootHandle.h
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

/** A handle for an hdf5 root */
class IMPRMFEXPORT RootHandle: public NodeHandle {
  void gather_ids(NodeHandle n, std::vector<int> &ids,
                  std::vector<std::string> &paths,
                  std::string path) const;
  friend class NodeHandle;
  RootHandle(internal::SharedData *shared_);
 public:
  /** \param[in] name The name of the file to open.
      \param[in] clear If clear is true and the file exists, the current
      contents are cleared before the file is opened.
  */
  RootHandle(std::string name, bool clear);
  /** Get an existing key that has the given name of the
      given type.
  */
  template <class TypeT>
    Key<TypeT> get_key(KeyCategory category_id, std::string name) const {
    return shared_->get_key<TypeT>(category_id, name);
  }
  /** Create a key for a new type of data. There must not
      already be a key with the same name of any type.
  */
  template <class TypeT>
    Key<TypeT> add_key(KeyCategory category_id,
                       std::string name, bool per_frame) const {
    return shared_->add_key<TypeT>(category_id, name, per_frame);
  }
  template <class TypeT>
    bool get_has_key(KeyCategory category_id,
                     std::string name) const {
    return shared_->get_key<TypeT>(category_id, name) != Key<TypeT>();
  }
  /** Get the string name for the given key.
   */
  template <class TypeT>
    std::string get_name(Key<TypeT> k) const {
    return shared_->get_name(k);
  }
  /** Get a list of all keys of the given type,
   */
  template <class TypeT>
  std::vector<Key<TypeT> > get_keys(KeyCategory category_id) const {
    return shared_->get_keys<TypeT>(category_id);
  }

  /** Get a list of all keys of the given type,
   */
  template <class TypeT>
    bool get_is_per_frame(Key<TypeT> k) const {
    return shared_->get_is_per_frame(k);
  }
  template <class TypeT>
    unsigned int get_number_of_frames(Key<TypeT> k) const {
    return shared_->get_number_of_frames(k);
  }
  /** \name Non-template versions for python
      @{
  */
#define IMP_HDF5_ROOT_KEY_TYPE_METHODS(lcname, UCName)                  \
  Key<UCName##Traits> get_##lcname##_key(KeyCategory category_id,       \
                                         std::string nm) const {        \
    return get_key<UCName##Traits>(category_id, nm);                    \
  }                                                                     \
  Key<UCName##Traits> add_##lcname##_key(KeyCategory category_id,       \
                                         std::string nm,                \
                                         bool per_frame) const {        \
    return add_key<UCName##Traits>(category_id, nm, per_frame);         \
  }                                                                     \
  std::string get_name(Key<UCName##Traits> k) const {                   \
    return get_name<UCName##Traits>(k);                                 \
  }                                                                     \
  std::vector<Key<UCName##Traits> >                                     \
    get_##lcname##_keys(KeyCategory category_id) const {                \
    return get_keys<UCName##Traits>(category_id);                       \
  }                                                                     \
  unsigned int get_number_of_frames(Key<UCName##Traits> k) const {      \
    return shared_->get_number_of_frames(k);                            \
  }

  IMP_HDF5_ROOT_KEY_TYPE_METHODS(int, Int);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(float, Float);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(string, String);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(index, Index);
  IMP_HDF5_ROOT_KEY_TYPE_METHODS(node_id, NodeID);

  //IMP_HDF5_ROOT_TYPE(string, 0, std::string);
  /** @} */

  NodeHandle get_node_handle_from_association(void*d) const;
  NodeHandle get_node_handle_from_id(NodeID id) const;
  void show(std::ostream &out= std::cout) const {
    out << "RootHandle";
  }
  unsigned int get_number_of_bonds() const {
    return shared_->get_number_of_bonds();
  }
  std::pair<NodeHandle, NodeHandle> get_bond(unsigned int i) const {
    boost::tuple<int,int,int> t= shared_->get_bond(i);
    return std::make_pair(get_node_handle_from_id(t.get<0>()),
                          get_node_handle_from_id(t.get<1>()));
  }
  void add_bond(NodeHandle na, NodeHandle nb, unsigned int type) {
    NodeID ida= na.get_id();
    NodeID idb= nb.get_id();
    shared_->add_bond(ida.get_index(), idb.get_index(), type);
  }

  HDF5Group get_hdf5_group() const {
    return shared_->get_group();
  }
  void set_number_of_frames_hint(unsigned int i) {
    shared_->save_frames_hint(i);
  }
};
IMP_VALUES(RootHandle, RootHandles);

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ROOT_HANDLE_H */
