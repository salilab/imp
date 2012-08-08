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
#include "internal/SharedData.h"
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
      return internal::GenericSharedData<UCName##Traits, Arity>         \
        ::add_key(get_shared_data(), category_id.get_index(), nm, per_frame); \
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
  class HDF5Group;

  //! A handle for an RMF file
  /** Use this handle to perform operations relevant to the
      whole RMF hierarchy as well as to start traversal of the
      hierarchy.

      Make sure to check out the base class documentation
      for the non-modifying methods.

      \see create_rmf_file
      \see open_rmf_file
  */
  class RMFEXPORT FileHandle: public FileConstHandle {
    friend class NodeHandle;
    friend class internal::SharedData;
    FileHandle(internal::SharedData *shared_);
  public:
    //! Empty file handle, no open file.
    FileHandle(){}
#ifndef IMP_DOXYGEN
    FileHandle(std::string name, bool create);
#endif

    /** \name Methods for manipulating keys
        When using C++ it is most convenient to specify types
        when adding and using keys through template arguments. For python
        we provide non-template versions, below.
        @{
    */
    /** Create a key for a type of data. There must not
        already be a key with the same name of any type.
    */
    template <class TypeT, int Arity>
      Key<TypeT, Arity> add_key(CategoryD<Arity> category_id,
                                std::string name, bool per_frame) const {
      return internal::GenericSharedData<TypeT, Arity>::
        add_key(get_shared_data(),
                category_id.get_index(),
                name, per_frame);
    }
    /** Create new key for a type of data. There must not
        already be keys with the same name of any type. These keys
        can be used with set_values();
    */
    template <class TypeT, int Arity>
        vector<Key<TypeT, Arity> > add_keys(CategoryD<Arity> category_id,
                                            const Strings& names,
                                            bool per_frame) const {
      // later make sure they are consecutive
      vector<Key<TypeT, Arity> > ret(names.size());
      for (unsigned int i=0; i< names.size(); ++i) {
        ret[i]= add_jey(category_id, names[i], per_frame);
      }
      return ret;
    }
    /** @} */

    /** Frames can have associated comments which can be used to label
        particular frames of interest.*/
    void set_frame_name(unsigned int frame,
                        std::string comment);

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
      unsigned int num= get_shared_data()->get_number_of_sets(Arity);
    vector<NodeSetHandle<Arity> > ret(num);
    for (unsigned int i=0; i< num; ++i) {
      ret[i]=NodeSetHandle<Arity>(i, get_shared_data());
      }
      return ret;
    }


    IMP_HDF5_ROOT_KEY_SET_METHODS(pair, Pair, 2);
    IMP_HDF5_ROOT_KEY_SET_METHODS(triplet, Triplet, 3);
    IMP_HDF5_ROOT_KEY_SET_METHODS(quad, Quad, 4);
#ifndef SWIG
    /** Each node in the hierarchy can be associated with some arbitrary bit
        of external data. Nodes can be extracted using these bits of data.
    */
    template <class T>
      NodeHandle get_node_from_association(const T&d) const {
      if (! get_shared_data()->get_has_associated_node(d)) {
        return NodeHandle();
      } else {
        return NodeHandle(get_shared_data()->get_associated_node(d),
                          get_shared_data());
      }
    }
#else
    NodeHandle get_node_from_association(void*d) const ;
#endif
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

    /*HDF5Group get_hdf5_group() const {
      return get_shared_data()->get_group();
      }*/
    /** \name Key categories template methods
        Methods for managing the key categories in this RMF.
        @{
    */
    template <int Arity>
      CategoryD<Arity> add_category(std::string name) {
      IMP_RMF_USAGE_CHECK(!get_has_category<Arity>(name),
                          "Category already exists");
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
  };

  typedef vector<FileHandle> FileHandles;

  /**
   Create an RMF from a file system path.

   \param path the system path to the rmf file
   \exception RMF::IOException couldn't create file, or unsupported file format
   */
  RMFEXPORT FileHandle create_rmf_file(std::string path);

  /**
   Open an RMF from a file system path.

   \param path the system path to the rmf file
   \exception RMF::IOException couldn't create file, or unsupported file format
*/
  RMFEXPORT FileHandle open_rmf_file(std::string path);


  template <int D>
  inline FileHandle NodeSetHandle<D>::get_file() const {
    return get_node(0).get_file();
  }

/** \name Helper functions
    These functions help make working with keys and categories easier,
    by allowing you to assume that the key/category is always
    there.
    @{
*/
template <int D>
CategoryD<D> get_category_always(FileHandle fh, std::string name) {
  if (fh.get_has_category<D>(name)) {
    return fh.get_category<D>(name);
  } else {
    return fh.add_category<D>(name);
  }
}


inline CategoryD<1> get_singleton_category_always(FileHandle rh,
                                                 std::string name) {
  return get_category_always<1>(rh, name);
}
inline CategoryD<2> get_pair_category_always(FileHandle rh,
                                            std::string name) {
  return get_category_always<2>(rh, name);
}
inline CategoryD<3> get_triplet_category_always(FileHandle rh,
                                               std::string name) {
  return get_category_always<3>(rh, name);
}
inline CategoryD<4> get_quad_category_always(FileHandle rh,
                                            std::string name) {
  return get_category_always<4>(rh, name);
}



template <class TypeT, int D>
Key<TypeT, D> get_key_always(FileHandle fh, CategoryD<D> cat,
                             std::string name, bool per_frame=false) {
  if (fh.get_has_key<TypeT, D>(cat, name, per_frame)) {
    IMP_RMF_USAGE_CHECK(per_frame
                        == fh.get_is_per_frame(fh.get_key<TypeT, D>(cat,
                                                                    name,
                                                                    per_frame)),
                        internal::get_error_message(
                       "A per_frame value for the key requested didn't match",
                       " the actual per_frame value."));
    return fh.get_key<TypeT, D>(cat, name, per_frame);
  } else {
    return fh.add_key<TypeT, D>(cat, name, per_frame);
  }
}

template <class TypeT, int D>
vector<Key<TypeT, D> > get_keys_always(FileHandle fh, CategoryD<D> cat,
                                       const Strings& names,
                                       bool per_frame=false) {
  vector<Key<TypeT, D> > ret(names.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= get_key_always<TypeT>(fh, cat, names[i], per_frame);
  }
  return ret;
}


#define IMP_RMF_GET_KEY_ALWAYS_CATEGORY(lcname, UCName, PassValue,      \
                                        ReturnValue,                    \
                                        PassValues, ReturnValues, D)    \
  inline Key<UCName##Traits, D> get_##lcname##_key_always(FileHandle rh, \
                                                          CategoryD<D> cat, \
                                                          std::string name, \
                                                          bool per_frame \
                                                          = false) {    \
    return get_key_always<UCName##Traits, D>(rh, cat, name, per_frame); \
  }
#define IMP_RMF_GET_KEY_ALWAYS(lcname, UCName, PassValue, ReturnValue,  \
                               PassValues, ReturnValues)                \
  IMP_RMF_GET_KEY_ALWAYS_CATEGORY(lcname, UCName, PassValue, ReturnValue, \
                                  PassValues, ReturnValues, 1);         \
  IMP_RMF_GET_KEY_ALWAYS_CATEGORY(lcname, UCName, PassValue, ReturnValue, \
                                  PassValues, ReturnValues, 2);         \
  IMP_RMF_GET_KEY_ALWAYS_CATEGORY(lcname, UCName, PassValue, ReturnValue, \
                                  PassValues, ReturnValues, 3);         \
  IMP_RMF_GET_KEY_ALWAYS_CATEGORY(lcname, UCName, PassValue, ReturnValue, \
                                  PassValues, ReturnValues, 4)
IMP_RMF_FOREACH_TYPE(IMP_RMF_GET_KEY_ALWAYS);
/** @} */

} /* namespace RMF */

#endif /* IMPLIBRMF_FILE_HANDLE_H */
