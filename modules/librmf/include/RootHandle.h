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
#include "NodeSetHandle.h"


#define IMP_HDF5_ROOT_KEY_TYPE_METHODS_INNER(lcname, UCName,            \
                                             arityname, ArityName,      \
                                             PassValue,                 \
                                             ReturnValue,               \
                                             PassValues, ReturnValues,  \
                                             Arity)                     \
    ArityName##UCName##Key get_##lcname##_key(ArityName##Category category_id, \
                                              std::string nm) const {   \
      return get_key<UCName##Traits, Arity>(category_id, nm);           \
    }                                                                   \
    bool get_has_##lcname##_key(ArityName##Category category_id,        \
                                std::string nm) const {                 \
      return get_has_key<UCName##Traits, Arity>(category_id, nm);       \
    }                                                                   \
    ArityName##UCName##Key add_##lcname##_key(ArityName##Category category_id, \
                                              std::string nm,           \
                                              bool per_frame) const {   \
      return add_key<UCName##Traits, Arity>(category_id, nm, per_frame); \
    }                                                                   \
    std::string get_name(ArityName##UCName##Key k) const {              \
      return shared_->get_name(k);                                      \
    }                                                                   \
    ArityName##Category get_category(ArityName##UCName##Key k) const {  \
      return k.get_category();                                          \
    }                                                                   \
    ArityName##UCName##Key##s                                           \
      get_##lcname##_keys(ArityName##Category category_id) const {      \
      return get_keys<UCName##Traits, Arity>(category_id);              \
    }                                                                   \
    unsigned int get_number_of_frames(ArityName##UCName##Key k) const { \
      return shared_->get_number_of_frames(k);                          \
    }                                                                   \
    bool get_is_per_frame(ArityName##UCName##Key k) const {             \
      return shared_->get_is_per_frame(k);                              \
    }

#ifndef IMP_DOXYGEN
#define IMP_HDF5_ROOT_KEY_TYPE_METHODS(lcname, UCName, PassValue, ReturnValue, \
                                       PassValues, ReturnValues)        \
    IMP_HDF5_ROOT_KEY_TYPE_METHODS_INNER(lcname, UCName, , ,            \
                                         PassValue, ReturnValue,        \
                                         PassValues, ReturnValues, 1)   \
      IMP_HDF5_ROOT_KEY_TYPE_METHODS_INNER(lcname, UCName,              \
                                           pair_, Pair,                 \
                                           PassValue, ReturnValue,      \
                                           PassValues, ReturnValues,    \
                                           2)                           \
      IMP_HDF5_ROOT_KEY_TYPE_METHODS_INNER(lcname, UCName,              \
                                           triplet_, Triplet,           \
                                           PassValue,                   \
                                           ReturnValue,                 \
                                           PassValues, ReturnValues,    \
                                           3)                           \
      IMP_HDF5_ROOT_KEY_TYPE_METHODS_INNER(lcname, UCName,              \
                                           quad_, Quad,                 \
                                           PassValue, ReturnValue,      \
                                           PassValues, ReturnValues,    \
                                           4)
#else
#define IMP_HDF5_ROOT_KEY_TYPE_METHODS(lcname, UCName, PassValue, ReturnValue, \
                                       PassValues, ReturnValues)        \
    IMP_HDF5_ROOT_KEY_TYPE_METHODS_INNER(lcname, UCName, arity_, Arity, \
                                         PassValue, ReturnValue,        \
                                         PassValues, ReturnValues, 1)   \

#endif


#define IMP_HDF5_ROOT_KEY_SET_METHODS(lcset, UCSet, D)                  \
    unsigned int get_number_of_node_##lcset##s() const {                \
      return get_number_of_node_sets<D>();                              \
    }                                                                   \
    Node##UCSet##Handles get_node_##lcset##s() const {                  \
      return get_node_sets<D>();                                        \
    }                                                                   \
    Node##UCSet##Handle add_node_##lcset(const NodeHandles &nh,         \
                                         NodeSetType tt) {              \
      return add_node_set<D>(nh, tt);                                   \
    }

#define IMP_RMF_CATEGORY_METHODS(Arity, prefix, Prefix)                 \
    Prefix##Category add_##prefix##category(std::string name) {         \
      return add_category<Arity>(name);                                 \
    }                                                                   \
    bool get_has_##prefix##category(std::string name) const {           \
      return get_has_category<Arity>(name);                             \
    }                                                                   \
    Prefix##Category get_##prefix##category(std::string name) const {   \
      return get_category<Arity>(name);                                 \
    }                                                                   \
    Prefix##Categories get_##prefix##categories() const {               \
      return get_categories<Arity>();                                   \
    }                                                                   \
    std::string get_prefix##category_name(Prefix##Category kc) const {  \
      return get_category_name<Arity>(kc);                              \
    }


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
    //! Lift NodeHandle::get_name() into class scope
    std::string get_name() const {
      return NodeHandle::get_name();
    }

    /** \name Methods for manipulating keys
        When using C++ it is most convenient to specify types
        when adding and using keys through template arguments. For python
        we provide non-template versions, below.
        @{
    */
    /** Get an existing key that has the given name of the
        given type.
    */
    template <class TypeT, int Arity>
      Key<TypeT, Arity> get_key(CategoryD<Arity> category_id,
                                std::string name) const {
      return shared_->get_key<TypeT, Arity>(category_id.get_index(), name);
    }
    /** Create a key for a new type of data. There must not
        already be a key with the same name of any type.
    */
    template <class TypeT, int Arity>
      Key<TypeT, Arity> add_key(CategoryD<Arity> category_id,
                                std::string name, bool per_frame) const {
      return shared_->add_key<TypeT, Arity>(category_id.get_index(),
                                            name, per_frame);
    }
    template <class TypeT, int Arity>
      bool get_has_key(CategoryD<Arity> category_id,
                       std::string name) const {
      return shared_->get_key<TypeT, Arity>(category_id.get_index(), name)
        != Key<TypeT, Arity>();
    }
    /** Get a list of all keys of the given type,
     */
    template <class TypeT, int Arity>
      vector<Key<TypeT, Arity> > get_keys(CategoryD<Arity> category_id) const {
      return shared_->get_keys<TypeT, Arity>(category_id.get_index());
    }
    /** @} */

    /** Return the number of frames in the file. Currently, this is the number
        of frames that the x-coordinate has, but it should be made more general.
    */
    unsigned int get_number_of_frames() const {
      return shared_->get_number_of_frames(get_key<FloatTraits,
                                           1>(CategoryD<1>(0),
                                              "cartesian x"));
    }

    /** \name Non-template versions for python

        Type is one of the \ref rmf_types "standard types"
        an arity is one of the empty string (for single nodes),
        pair, triplet or quad.
        @{
    */

    IMP_RMF_FOREACH_TYPE(IMP_HDF5_ROOT_KEY_TYPE_METHODS);

    /** @} */
#endif
#ifdef IMP_DOXYGEN
    /** \name Python only
        The following methods are only available in python.
        @{
    */
    //! Return a list with all the keys from that category
    /** If arity>1 then the keys for the appropriate sets are
        returned.
    */
    PythonList get_keys(Category c, int arity=1) const;
    //! Return all sets of that arity
    PythonList get_node_sets(int arity) const;
    /** @} */
#endif
    /** Each node in the hierarchy can be associated with some arbitrary bit
        of external data using a void* pointer. Nodes can be extracted using
        these bits of data.
    */
    NodeHandle get_node_handle_from_association(void*d) const;
    NodeHandle get_node_handle_from_id(NodeID id) const;
    void show(std::ostream &out= std::cout) const {
      using std::operator<<;
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
      unsigned int get_number_of_node_sets() const {
      return shared_->get_number_of_sets(Arity);
    }
    template <int Arity>
      vector<NodeSetHandle<Arity> > get_node_sets() const {
      Indexes ids= shared_->get_set_indexes(Arity);
      vector<NodeSetHandle<Arity> > ret(ids.size());
      for (unsigned int i=0; i< ret.size(); ++i) {
        ret[i]=NodeSetHandle<Arity>(ids[i], shared_.get());
      }
      return ret;
    }
    template <int Arity>
      NodeSetHandle<Arity>  add_node_set(const NodeHandles &nh,
                                             NodeSetType tt) {
      IMP_RMF_USAGE_CHECK(nh.size()==Arity, "Wrong size for handles list");
      Indexes ix(nh.size());
      for (unsigned int i=0; i< nh.size(); ++i) {
        ix[i]=nh[i].get_id().get_index();
      }
      int id=shared_->add_set(ix, tt);
      return NodeSetHandle<Arity>(id, shared_.get());
    }



    IMP_HDF5_ROOT_KEY_SET_METHODS(pair, Pair, 2);
    IMP_HDF5_ROOT_KEY_SET_METHODS(triplet, Triplet, 3);
    IMP_HDF5_ROOT_KEY_SET_METHODS(quad, Quad, 4);


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


    /** \name Key categories template methods
        Methods for managing the key categories in this RMF.
        @{
    */
    template <int Arity>
      CategoryD<Arity> add_category(std::string name) {
      return CategoryD<Arity>(shared_->add_category(Arity, name));
    }
    template <int Arity>
      bool get_has_category(std::string name) const {
      return shared_->get_category(Arity, name) >=0;
    }
    template <int Arity>
      CategoryD<Arity> get_category(std::string name) const {
      int v=(shared_->get_category(Arity, name));
      IMP_RMF_USAGE_CHECK(v>=0, "Could not find category " << name
                          //<< " in list " << shared_->get_category_names(Arity)
                          << " with arity " << Arity);
      return CategoryD<Arity>(v);
    }
    //! Add the category if necessary
    template <int Arity>
      CategoryD<Arity> get_or_add_category(std::string name) {
      if (!get_has_category<Arity>(name)) {
        return add_category<Arity>(name);
      } else {
        return get_category<Arity>(name);
      }
    }
    template <int Arity>
      vector<CategoryD<Arity> > get_categories() const {
      Ints r= shared_->get_categories(Arity);
      vector<CategoryD<Arity> > ret(r.size());
      for (unsigned int i=0; i< r.size(); ++i) {
        ret[i]= CategoryD<Arity>(i);
      }
      return ret;
    }
    template <int Arity>
      std::string get_category_name(CategoryD<Arity> kc) const {
      return shared_->get_category_name(Arity, kc.get_index());
    }
    /** @} */
    /** \name Key categories non-template methods
        We also provide non-template methods for use in \c Python or
        environments where templates are not convenient. Arity is one
        of the empty string, pair, triplet, or quad.
        @{
    */
#ifndef IMP_DOXYGEN
    IMP_RMF_CATEGORY_METHODS(1, ,);
    IMP_RMF_CATEGORY_METHODS(2, pair_, Pair);
    IMP_RMF_CATEGORY_METHODS(3, triplet_, Triplet);
    IMP_RMF_CATEGORY_METHODS(4, quad_, Quad);
#else
    IMP_RMF_CATEGORY_METHODS(1, arity_,Arity);
#endif
    /** @} */

    /** Make sure all data gets written to disk. Once flush is called, it
        should be safe to open the file in another process for reading.
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
  inline RootHandle NodeSetHandle<D>::get_root_handle() const {
    return get_node(0).get_root_handle();
  }

} /* namespace RMF */

#endif /* IMPLIBRMF_ROOT_HANDLE_H */
