/**
 *  \file RMF/FileHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_FILE_HANDLE_H
#define IMPLIBRMF_FILE_HANDLE_H

#include "RMF_config.h"
#include "HDF5Group.h"
#include "HDF5File.h"
#include "internal/shared.h"
#include "Key.h"
#include "FileConstHandle.h"


#define IMP_HDF5_ROOT_KEY_TYPE_METHODS_INNER(lcname, UCName,            \
                                             arityname, ArityName,      \
                                             PassValue,                 \
                                             ReturnValue,               \
                                             PassValues, ReturnValues,  \
                                             Arity)                     \
    ArityName##UCName##Key add_##lcname##_key(ArityName##Category category_id, \
                                              std::string nm,           \
                                              bool per_frame) const {   \
      return add_key<UCName##Traits, Arity>(category_id, nm, per_frame); \
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
  Node##UCSet##Handles get_node_##lcset##s() const {                    \
    return get_node_sets<D>();                                          \
    }                                                                   \
    Node##UCSet##Handle add_node_##lcset(const NodeHandles &nh,         \
                                         NodeSetType tt) {              \
      return add_node_set<D>(nh, tt);                                   \
    }

#define IMP_RMF_CATEGORY_METHODS(Arity, prefix, Prefix)                 \
    Prefix##Category add_##prefix##category(std::string name) {         \
      return add_category<Arity>(name);                                 \
    }


namespace RMF {

  //! A handle for an RMF file
  /** Use this handle to perform operations relevant to the
      whole RMF hierarchy as well as to start traversal of the
      hierarchy.

      Make sure to check out the base class for the non-modifying
      methods.
  */
  class RMFEXPORT FileHandle: public FileConstHandle {
    friend class NodeHandle;
    friend class internal::SharedData;
    FileHandle(internal::SharedData *shared_);
  public:
    //! Empty file handle, no open file.
    FileHandle(){}
#ifndef IMP_DOXYGEN
    FileHandle(HDF5Group group, bool create);
#endif

    /** \name Methods for manipulating keys
        When using C++ it is most convenient to specify types
        when adding and using keys through template arguments. For python
        we provide non-template versions, below.
        @{
    */
    /** Create a key for a new type of data. There must not
        already be a key with the same name of any type.
    */
    template <class TypeT, int Arity>
      Key<TypeT, Arity> add_key(CategoryD<Arity> category_id,
                                std::string name, bool per_frame) const {
      return get_shared_data()->add_key<TypeT, Arity>(category_id.get_index(),
                                            name, per_frame);
    }
    /** @} */

    /** \name Non-template versions for python

        Type is one of the \ref rmf_types "standard types"
        an arity is one of the empty string (for single nodes),
        pair, triplet or quad.
        @{
    */
    IMP_RMF_FOREACH_TYPE(IMP_HDF5_ROOT_KEY_TYPE_METHODS);
    /** @} */
#ifdef IMP_DOXYGEN
    /** \name Python only
        The following methods are only available in python.
        @{
    */
#endif
    template <int Arity>
      NodeSetHandle<Arity>  add_node_set(const NodeHandles &nh,
                                             NodeSetType tt) {
      IMP_RMF_USAGE_CHECK(nh.size()==Arity, "Wrong size for handles list");
      Indexes ix(nh.size());
      for (unsigned int i=0; i< nh.size(); ++i) {
        ix[i]=nh[i].get_id().get_index();
      }
      int id=get_shared_data()->add_set(ix, tt);
      return NodeSetHandle<Arity>(id, get_shared_data());
    }


    NodeHandle get_root_node() const {
      return NodeHandle(0, get_shared_data());
    }

    template <int Arity>
      vector<NodeSetHandle<Arity> > get_node_sets() const {
      Indexes ids= get_shared_data()->get_set_indexes(Arity);
      vector<NodeSetHandle<Arity> > ret(ids.size());
      for (unsigned int i=0; i< ret.size(); ++i) {
        ret[i]=NodeSetHandle<Arity>(ids[i], get_shared_data());
      }
      return ret;
    }


    IMP_HDF5_ROOT_KEY_SET_METHODS(pair, Pair, 2);
    IMP_HDF5_ROOT_KEY_SET_METHODS(triplet, Triplet, 3);
    IMP_HDF5_ROOT_KEY_SET_METHODS(quad, Quad, 4);

    /** Each node in the hierarchy can be associated with some arbitrary bit
        of external data using a void* pointer. Nodes can be extracted using
        these bits of data.
    */
    NodeHandle get_node_from_association(void*d) const;
    NodeHandle get_node_from_id(NodeID id) const;
    /** Suggest how many frames the file is likely to have. This can
        make writing more efficient as space will be preallocated.
    */
    void set_number_of_frames_hint(unsigned int i) {
      get_shared_data()->save_frames_hint(i);
    }
    /** \name Descriptions
        Each RMF structure has an associated description. This should
        consist of unstructured text describing the contents of the RMF
        data. Conventionally. this description can consist of multiple
        paragraphs, each separated by a newline character and should end
        in a newline.
        @{
    */
    void set_description(std::string descr);
    /** @} */


    /** \name Key categories template methods
        Methods for managing the key categories in this RMF.
        @{
    */
    template <int Arity>
      CategoryD<Arity> add_category(std::string name) {
      return CategoryD<Arity>(get_shared_data()->add_category(Arity, name));
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

    /** Check invariants that should old in the file but are not checked
        on the fly. New invariants can be added with the IMP_RMF_VALIDATOR()
        macro.*/
    void validate() const {get_shared_data()->validate();}
  };

  typedef vector<FileHandle> FileHandles;

  /** Create an RMF from a file system path.*/
  inline FileHandle create_rmf_file(std::string path) {
    IMP_RMF_FILE_OPERATION(
        return FileHandle(create_hdf5_file(path), true),
        path, "creating");
  }

  /** Open an RMF from a file system path.*/
  inline FileHandle open_rmf_file(std::string path) {
    IMP_RMF_FILE_OPERATION(
        return FileHandle(open_hdf5_file(path), false),
        path, "opening");
  }


  template <int D>
  inline FileHandle NodeSetHandle<D>::get_file() const {
    return get_node(0).get_file();
  }

} /* namespace RMF */

#endif /* IMPLIBRMF_FILE_HANDLE_H */
