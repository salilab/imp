/**
 *  \file RMF/FileConstHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_FILE_CONST_HANDLE_H
#define IMPLIBRMF_FILE_CONST_HANDLE_H

#include "RMF_config.h"
#include "HDF5Group.h"
#include "HDF5File.h"
#include "internal/shared.h"
#include "Key.h"
#include "NodeHandle.h"
#include "NodeSetHandle.h"
#include <boost/functional/hash.hpp>


#define IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS_INNER(lcname, UCName,      \
                                             arityname, ArityName,      \
                                             PassValue,                 \
                                             ReturnValue,               \
                                             PassValues, ReturnValues,  \
                                             Arity)                     \
    ArityName##UCName##Key get_##lcname##_key(ArityName##Category category_id, \
                                              std::string nm) const {   \
      if (!get_has_##lcname##_key(category_id, nm)) {                    \
        return ArityName##UCName##Key();                                \
      }                                                                 \
      return get_key<UCName##Traits, Arity>(category_id, nm);           \
    }                                                                   \
    bool get_has_##lcname##_key(ArityName##Category category_id,        \
                                std::string nm) const {                 \
      return get_has_key<UCName##Traits, Arity>(category_id, nm);       \
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
      if (k== ArityName##UCName##Key()) return 0;                       \
      return shared_->get_number_of_frames(k);                          \
    }                                                                   \
    bool get_is_per_frame(ArityName##UCName##Key k) const {             \
      return shared_->get_is_per_frame(k);                              \
    }

#ifndef IMP_DOXYGEN
#define IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS(lcname, UCName, PassValue,\
                                             ReturnValue,               \
                                       PassValues, ReturnValues)        \
  IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS_INNER(lcname, UCName, , ,        \
                                         PassValue, ReturnValue,        \
                                         PassValues, ReturnValues, 1)   \
    IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS_INNER(lcname, UCName,          \
                                           pair_, Pair,                 \
                                           PassValue, ReturnValue,      \
                                           PassValues, ReturnValues,    \
                                           2)                           \
    IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS_INNER(lcname, UCName,          \
                                           triplet_, Triplet,           \
                                           PassValue,                   \
                                           ReturnValue,                 \
                                           PassValues, ReturnValues,    \
                                           3)                           \
    IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS_INNER(lcname, UCName,          \
                                           quad_, Quad,                 \
                                           PassValue, ReturnValue,      \
                                           PassValues, ReturnValues,    \
                                           4)
#else
#define IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS(lcname, UCName, PassValue,\
                                             ReturnValue,               \
                                             PassValues, ReturnValues)  \
  IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS_INNER(lcname, UCName, arity_, Arity, \
                                         PassValue, ReturnValue,        \
                                         PassValues, ReturnValues, 1)   \

#endif


#define IMP_HDF5_ROOT_CONST_KEY_SET_METHODS(lcset, UCSet, D)            \
  unsigned int get_number_of_node_##lcset##s() const {                  \
      return get_number_of_node_sets<D>();                              \
    }                                                                   \
  Node##UCSet##ConstHandles get_node_##lcset##s() const {               \
    return get_node_sets<D>();                                          \
    }                                                                   \

#define IMP_RMF_CONST_CATEGORY_METHODS(Arity, prefix, Prefix)           \
    bool get_has_##prefix##category(std::string name) const {           \
      return get_has_category<Arity>(name);                             \
    }                                                                   \
    Prefix##Category get_##prefix##category(std::string name) const {   \
      return get_category<Arity>(name);                                 \
    }                                                                   \
    std::string get_name(Prefix##Category kc) const {                   \
      return get_category_name<Arity>(kc);                              \
    }                                                                   \
    Prefix##Categories get_##prefix##categories() const {               \
      return get_categories<Arity>();                                   \
    }                                                                   \
    std::string get_prefix##category_name(Prefix##Category kc) const {  \
      return get_category_name<Arity>(kc);                              \
    }


namespace RMF {

  class NodeConstHandle;

#ifndef IMP_DOXYGEN
  typedef std::pair<NodeConstHandle, NodeConstHandle> BondPair;
  typedef vector<BondPair> BondPairs;
#endif

  //! A handle for a read-only RMF file
  /** Use this handle to perform operations relevant to the
      whole RMF hierarchy as well as to start traversal of the
      hierarchy.
  */
  class RMFEXPORT FileConstHandle {
    void gather_ids(NodeConstHandle n, Ints &ids,
                    vector<std::string> &paths,
                    std::string path) const;
    friend class NodeConstHandle;
    friend class internal::SharedData;
    boost::intrusive_ptr<internal::SharedData> shared_;
    int compare(const FileConstHandle &o) const {
      if (get_name() < o.get_name()) return -1;
      else if (get_name() > o.get_name()) return 1;
      else return 0;
    }
#if !defined(SWIG) && !defined(IMP_DOXYGEN)
 protected:
    internal::SharedData* get_shared_data() const {return shared_.get();}
    FileConstHandle(internal::SharedData *shared_);
#endif
  public:
    IMP_RMF_COMPARISONS(FileConstHandle);
    IMP_RMF_HASHABLE(FileConstHandle, return boost::hash_value(get_name()););
    IMP_RMF_SHOWABLE(FileConstHandle, get_name());
    //! Empty root handle, no open file.
    FileConstHandle(){}
#ifndef IMP_DOXYGEN
    FileConstHandle(HDF5Group group, bool create);
#endif

    //! Return the root of the hierarchy
    NodeConstHandle get_root_node() const {
      return NodeConstHandle(0, shared_.get());
    }

    std::string get_name() const {
      return shared_->get_file_name();
    }

    /** \name Methods for manipulating keys
        When using C++ it is most convenient to specify types
        when adding and using keys through template arguments. For python
        we provide non-template versions, below.
        @{
    */
    /** Get an existing key that has the given name of the
        given type or Key() if the key is not found.
    */
    template <class TypeT, int Arity>
      Key<TypeT, Arity> get_key(CategoryD<Arity> category_id,
                                std::string name) const {
      if (!get_has_key<TypeT>(category_id, name)) {
        return Key<TypeT, Arity>();
      } else {
        return shared_->get_key<TypeT, Arity>(category_id.get_index(), name);
      }
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

    IMP_RMF_FOREACH_TYPE(IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS);

    /** @} */
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
    NodeConstHandle get_node_from_association(void*d) const;
    NodeConstHandle get_node_from_id(NodeID id) const;
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
      return std::make_pair(get_node_from_id(NodeID(t.get<0>())),
                            get_node_from_id(NodeID(t.get<1>())));
    }
    BondPairs get_bonds() const;
    /** @} */
#endif

    template <int Arity>
      unsigned int get_number_of_node_sets() const {
      return shared_->get_number_of_sets(Arity);
    }
    template <int Arity>
      vector<NodeSetConstHandle<Arity> > get_node_sets() const {
      Indexes ids= shared_->get_set_indexes(Arity);
      vector<NodeSetConstHandle<Arity> > ret(ids.size());
      for (unsigned int i=0; i< ret.size(); ++i) {
        ret[i]=NodeSetConstHandle<Arity>(ids[i], shared_.get());
      }
      return ret;
    }


    IMP_HDF5_ROOT_CONST_KEY_SET_METHODS(pair, Pair, 2);
    IMP_HDF5_ROOT_CONST_KEY_SET_METHODS(triplet, Triplet, 3);
    IMP_HDF5_ROOT_CONST_KEY_SET_METHODS(quad, Quad, 4);


    HDF5Group get_hdf5_group() const {
      return shared_->get_group();
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
    /** @} */


    /** \name Key categories template methods
        Methods for managing the key categories in this RMF.
        @{
    */
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
    IMP_RMF_CONST_CATEGORY_METHODS(1, ,);
    IMP_RMF_CONST_CATEGORY_METHODS(2, pair_, Pair);
    IMP_RMF_CONST_CATEGORY_METHODS(3, triplet_, Triplet);
    IMP_RMF_CONST_CATEGORY_METHODS(4, quad_, Quad);
#else
    IMP_RMF_CONST_CATEGORY_METHODS(1, arity_,Arity);
#endif
    /** @} */

    /** Make sure all data gets written to disk. Once flush is called, it
        should be safe to open the file in another process for reading.
     */
    void flush();
  };

  typedef vector<FileConstHandle> FileConstHandles;


  /** Open an RMF from a file system path.*/
  inline FileConstHandle open_rmf_file_read_only(std::string path) {
    IMP_RMF_FILE_OPERATION(
        return FileConstHandle(open_hdf5_file_read_only(path), false),
        path, "opening read only");
  }

  /** \name Batch data access
      These methods provide batch access to attribute data to try
      to reduce the overhead of repeated function calls.

      The missing_value argument is a placeholder that can fill in
      for values which are not found in the respective node.

      \note These methods are experimental and subject to change.
      @{
  */
  RMFEXPORT Floats get_values(const NodeConstHandles &nodes,
                              FloatKey k,
                              unsigned int frame,
                              Float missing_value
                              =std::numeric_limits<double>::max());
  /** @} */


  template <int D>
  inline FileConstHandle NodeSetConstHandle<D>::get_file() const {
    return get_node(0).get_file();
  }

} /* namespace RMF */

#endif /* IMPLIBRMF_FILE_CONST_HANDLE_H */
