/**
 *  \file RMF/FileHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF__FILE_HANDLE_H
#define RMF__FILE_HANDLE_H

#include <RMF/config.h>
#include "internal/SharedData.h"
#include "Key.h"
#include "FileConstHandle.h"


#define RMF_HDF5_ROOT_KEY_TYPE_METHODS(lcname, UCName,                  \
                                             PassValue,                 \
                                             ReturnValue,               \
                                             PassValues, ReturnValues)  \
  UCName##Key add_##lcname##_key(Category category_id,                  \
                                 std::string nm,                        \
                                 bool per_frame) const {                \
    return internal::GenericSharedData<UCName##Traits>                  \
      ::add_key(get_shared_data(), category_id.get_index(), nm, per_frame); \
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
#ifndef RMF_DOXYGEN
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
    template <class TypeT>
      Key<TypeT> add_key(Category category_id,
                         std::string name, bool per_frame) const {
      return internal::GenericSharedData<TypeT>::
        add_key(get_shared_data(),
                category_id.get_index(),
                name, per_frame);
    }
    /** Create new key for a type of data. There must not
        already be keys with the same name of any type. These keys
        can be used with set_values();
    */
    template <class TypeT>
      vector<Key<TypeT> > add_keys(Category category_id,
                                   const Strings& names,
                                   bool per_frame) const {
      // later make sure they are consecutive
      vector<Key<TypeT> > ret(names.size());
      for (unsigned int i=0; i< names.size(); ++i) {
        ret[i]= add_key<TypeT>(category_id, names[i], per_frame);
      }
      return ret;
    }
    /** @} */

    /** Frames can have associated comments which can be used to label
        particular frames of interest.*/
    void set_frame_name(std::string comment);

    /** \name Non-template versions for python

        Type is one of the \ref rmf_types "standard types".
        @{
    */
    RMF_FOREACH_TYPE(RMF_HDF5_ROOT_KEY_TYPE_METHODS);
    /** @} */

    NodeHandle get_root_node() const {
      return NodeHandle(0, get_shared_data());
    }

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
    Category add_category(std::string name) {
      RMF_USAGE_CHECK(!get_has_category(name),
                          "Category already exists");
      return Category(get_shared_data()->add_category(name));
    }
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


/** \name Helper functions
    These functions help make working with keys and categories easier,
    by allowing you to assume that the key/category is always
    there.
    @{
*/
inline Category get_category_always(FileHandle fh, std::string name) {
  if (fh.get_has_category(name)) {
    return fh.get_category(name);
  } else {
    return fh.add_category(name);
  }
}


template <class TypeT>
Key<TypeT> get_key_always(FileHandle fh, Category cat,
                             std::string name, bool per_frame=false) {
  if (fh.get_has_key<TypeT>(cat, name, per_frame)) {
    RMF_USAGE_CHECK(per_frame
                    == fh.get_is_per_frame(fh.get_key<TypeT>(cat,
                                                             name,
                                                             per_frame)),
                    internal::get_error_message("A per_frame value for the key requested didn't match",
                                                " the actual per_frame value."));
    return fh.get_key<TypeT>(cat, name, per_frame);
  } else {
    return fh.add_key<TypeT>(cat, name, per_frame);
  }
}

template <class TypeT>
vector<Key<TypeT> > get_keys_always(FileHandle fh, Category cat,
                                    const Strings& names,
                                    bool per_frame=false) {
  vector<Key<TypeT> > ret(names.size());
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= get_key_always<TypeT>(fh, cat, names[i], per_frame);
  }
  return ret;
}


#define RMF_GET_KEY_ALWAYS(lcname, UCName, PassValue,                   \
                                    ReturnValue,                        \
                                    PassValues, ReturnValues)           \
  inline Key<UCName##Traits> get_##lcname##_key_always(FileHandle rh,   \
                                                       Category cat,    \
                                                       std::string name, \
                                                       bool per_frame   \
                                                       = false) {       \
    return get_key_always<UCName##Traits>(rh, cat, name, per_frame);    \
  }

RMF_FOREACH_TYPE(RMF_GET_KEY_ALWAYS);
/** @} */

} /* namespace RMF */

#endif /* RMF__FILE_HANDLE_H */
