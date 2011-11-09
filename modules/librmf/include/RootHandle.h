/**
 *  \file RMF/RootHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_ROOT_HANDLE_H
#define IMPLIBRMF_ROOT_HANDLE_H

#include "RMF_config.h"
#include "HDF5Group.h"
#include "internal/shared.h"
#include "Key.h"
#include "NodeHandle.h"
#include "NodeTupleHandle.h"

namespace RMF {

class NodeHandle;

#ifndef IMP_DOXYGEN
typedef std::pair<NodeHandle, NodeHandle> BondPair;
typedef vector<BondPair> BondPairs;
#endif

//! A handle for an RMF root
/** Use this handle to perform operations relevant to the
    whole RMF hierarchy as well as to start traversal of the
    hierarchy.
*/
class RMFEXPORT RootHandle: public NodeHandle {
  void gather_ids(NodeHandle n, Ints &ids,
                  vector<std::string> &paths,
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
  template <class TypeT, int Arity>
      Key<TypeT, Arity> get_key(Category category_id,
                                std::string name) const {
    return shared_->get_key<TypeT, Arity>(category_id, name);
  }
  //! Lift NodeHandle::get_name() into class scope
  std::string get_name() const {
    return NodeHandle::get_name();
  }
  /** Create a key for a new type of data. There must not
      already be a key with the same name of any type.
  */
  template <class TypeT, int Arity>
      Key<TypeT, Arity> add_key(Category category_id,
                            std::string name, bool per_frame) const {
    return shared_->add_key<TypeT, Arity>(category_id, name, per_frame);
  }
  template <class TypeT, int Arity>
      bool get_has_key(Category category_id,
                       std::string name) const {
    return shared_->get_key<TypeT, Arity>(category_id, name)
        != Key<TypeT, Arity>();
  }
  /** Get a list of all keys of the given type,
   */
  template <class TypeT, int Arity>
      vector<Key<TypeT, Arity> > get_keys(Category category_id) const {
    return shared_->get_keys<TypeT, Arity>(category_id);
  }

  /** Return the number of frames in the file. Currently, this is the number
      of frames that the x-coordinate has, but it should be made more general.
  */
  unsigned int get_number_of_frames() const {
    return shared_->get_number_of_frames(get_key<FloatTraits, 1>(Physics,
                                                              "cartesian x"));
  }

  /** \name Non-template versions for python
      @{
  */
#define IMP_HDF5_ROOT_KEY_TYPE_METHODS(lcname, UCName, PassValue, ReturnValue, \
                                       PassValues, ReturnValues)        \
  UCName##Key get_##lcname##_key(Category category_id,                  \
                                 std::string nm) const {                \
    return get_key<UCName##Traits, 1>(category_id, nm);                 \
  }                                                                     \
  bool get_has_##lcname##_key(Category category_id,                     \
                              std::string nm) const {                   \
    return get_has_key<UCName##Traits, 1>(category_id, nm);             \
  }                                                                     \
  UCName##Key add_##lcname##_key(Category category_id,                  \
                                 std::string nm,                        \
                                 bool per_frame) const {                \
    return add_key<UCName##Traits, 1>(category_id, nm, per_frame);      \
  }                                                                     \
  std::string get_name(UCName##Key k) const {                           \
    return shared_->get_name(k);                                        \
  }                                                                     \
  Category get_category(UCName##Key k) const {                          \
    return k.get_category();                                            \
  }                                                                     \
  UCName##Key##s                                                        \
      get_##lcname##_keys(Category category_id) const {                 \
    return get_keys<UCName##Traits, 1>(category_id);                    \
  }                                                                     \
  unsigned int get_number_of_frames(UCName##Key k) const {              \
    return shared_->get_number_of_frames(k);                            \
  }                                                                     \
  bool get_is_per_frame(UCName##Key k) const {                          \
    return shared_->get_is_per_frame(k);                                \
  }

  IMP_RMF_FOREACH_TYPE(IMP_HDF5_ROOT_KEY_TYPE_METHODS);

  /** @} */
#ifdef IMP_DOXYGEN
  /** \name Python only
      The following methods are only available in python.
      @{
  */
  //! Return a list with all the keys from that category
  /** If arity>1 then the keys for the appropriate tuples are
      returned.
  */
  PythonList get_keys(Category c, int arity=1) const;
  //! Return all tuples of that arity
  PythonList get_node_tuples(int arity) const;
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
#ifndef IMP_DOXYGEN
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
    return std::make_pair(get_node_handle_from_id(NodeID(t.get<0>())),
                          get_node_handle_from_id(NodeID(t.get<1>())));
  }
  void add_bond(NodeHandle na, NodeHandle nb, unsigned int type) {
    NodeID ida= na.get_id();
    NodeID idb= nb.get_id();
    shared_->add_bond(ida.get_index(), idb.get_index(), type);
  }
  BondPairs get_bonds() const;
  /** @} */
#endif

  template <int Arity>
      unsigned int get_number_of_node_tuples() const {
    return shared_->get_number_of_tuples(Arity);
  }
  template <int Arity>
      vector<NodeTupleHandle<Arity> > get_node_tuples() const {
    Indexes ids= shared_->get_tuple_indexes(Arity);
    vector<NodeTupleHandle<Arity> > ret(ids.size());
    for (unsigned int i=0; i< ret.size(); ++i) {
      ret[i]=NodeTupleHandle<Arity>(ids[i], shared_.get());
    }
    return ret;
  }
  template <int Arity>
  NodeTupleHandle<Arity>  add_node_tuple(const NodeHandles &nh,
                                            NodeTupleType tt) {
    IMP_RMF_USAGE_CHECK(nh.size()==Arity, "Wrong size for handles list");
    Indexes ix(nh.size());
    for (unsigned int i=0; i< nh.size(); ++i) {
      ix[i]=nh[i].get_id().get_index();
    }
    int id=shared_->add_tuple(ix, tt);
    return NodeTupleHandle<Arity>(id, shared_.get());
  }

#define IMP_HDF5_ROOT_KEY_TUPLE_TYPE_METHODS_INNER(lctupe, UCTuple, D,  \
                                                   lcname, UCName, PassValue, \
                                                   ReturnValue,         \
                                                   PassValues, ReturnValues) \
  UCTuple##UCName##Key get_##lctupe##_##lcname##_key(Category category_id, \
                                                   std::string nm) const { \
    return get_key<UCName##Traits, D>(category_id, nm);                 \
  }                                                                     \
  bool get_has_##lctupe##_##lcname##_key(Category category_id,             \
                                      std::string nm) const {           \
    return get_has_key<UCName##Traits, D>(category_id, nm);             \
  }                                                                     \
  UCTuple##UCName##Key add_##lctupe##_##lcname##_key(Category category_id, \
                                                  std::string nm,       \
                                                  bool per_frame) const { \
    return add_key<UCName##Traits, D>(category_id, nm, per_frame);      \
  }                                                                     \
  std::string get_name(UCTuple##UCName##Key k) const {                  \
    return shared_->get_name(k);                                        \
  }                                                                     \
  Category get_category(UCTuple##UCName##Key k) const {                 \
    return k.get_category();                                            \
  }                                                                     \
  UCTuple##UCName##Key##s                                               \
      get_##lctupe##_##lcname##_keys(Category category_id) const {      \
    return get_keys<UCName##Traits, D>(category_id);                    \
  }                                                                     \
  unsigned int get_number_of_frames(UCTuple##UCName##Key k) const {     \
    return shared_->get_number_of_frames(k);                            \
  }                                                                     \
  bool get_is_per_frame(UCTuple##UCName##Key k) const {                 \
    return shared_->get_is_per_frame(k);                                \
  }

#define IMP_HDF5_ROOT_KEY_TUPLE_TYPE_METHODS(lcname, UCName, PassValue, \
                                             ReturnValue,               \
                                             PassValues, ReturnValues)  \
  IMP_HDF5_ROOT_KEY_TUPLE_TYPE_METHODS_INNER(pair, Pair, 2,             \
                                             lcname, UCName, PassValue, \
                                             ReturnValue,               \
                                             PassValues, ReturnValues); \
  IMP_HDF5_ROOT_KEY_TUPLE_TYPE_METHODS_INNER(triplet, Triplet, 3,       \
                                             lcname, UCName, PassValue, \
                                             ReturnValue,               \
                                             PassValues, ReturnValues); \
  IMP_HDF5_ROOT_KEY_TUPLE_TYPE_METHODS_INNER(quad, Quad, 4,             \
                                             lcname, UCName, PassValue, \
                                             ReturnValue,               \
                                             PassValues, ReturnValues)

#define IMP_HDF5_ROOT_KEY_TUPLE_METHODS(lctuple, UCTuple, D)            \
  unsigned int get_number_of_node_##lctuple##s() const {                \
    return get_number_of_node_tuples<D>();                              \
  }                                                                     \
  Node##UCTuple##Handles get_node_##lctuple##s() const {                \
    return get_node_tuples<D>();                                        \
  }                                                                     \
  Node##UCTuple##Handle add_node_##lctuple(const NodeHandles &nh,       \
                                          NodeTupleType tt) {           \
    return add_node_tuple<D>(nh, tt);                                   \
  }

  IMP_HDF5_ROOT_KEY_TUPLE_METHODS(pair, Pair, 2);
  IMP_HDF5_ROOT_KEY_TUPLE_METHODS(triplet, Triplet, 3);
  IMP_HDF5_ROOT_KEY_TUPLE_METHODS(quad, Quad, 4);
  IMP_RMF_FOREACH_TYPE(IMP_HDF5_ROOT_KEY_TUPLE_TYPE_METHODS);



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

typedef vector<RootHandle> RootHandles;

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

/** \name Batch data access
    These methods provide batch access to attribute data to try
    to reduce the overhead of repeated function calls.

    The missing_value argument is a placeholder that can fill in
    for values which are not found in the respective node.

    \note These methods are experimental and subject to change.
    @{
*/
RMFEXPORT Floats get_values(const NodeHandles &nodes,
                            FloatKey k,
                            unsigned int frame,
                            Float missing_value
                            =std::numeric_limits<double>::max());
/** @} */


template <int D>
inline RootHandle NodeTupleHandle<D>::get_root_handle() const {
  return get_node(0).get_root_handle();
}

} /* namespace RMF */

#endif /* IMPLIBRMF_ROOT_HANDLE_H */
