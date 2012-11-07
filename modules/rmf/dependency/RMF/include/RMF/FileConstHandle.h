/**
 *  \file RMF/FileConstHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF__FILE_CONST_HANDLE_H
#define RMF__FILE_CONST_HANDLE_H

#include <RMF/config.h>
#include "internal/SharedData.h"
#include "Key.h"
#include "NodeHandle.h"
#include <boost/functional/hash.hpp>
#include <boost/intrusive_ptr.hpp>


#define RMF_HDF5_ROOT_CONST_KEY_TYPE_METHODS(lcname, UCName,            \
                                             PassValue,                 \
                                             ReturnValue,               \
                                             PassValues,                \
                                             ReturnValues)              \
  UCName##Key                                                           \
  get_##lcname##_key(Category category_id,                              \
                     std::string nm,                                    \
                     bool per_frame) const {                            \
    return get_key<UCName##Traits>(category_id, nm, per_frame);         \
  }                                                                     \
  bool get_has_##lcname##_key(Category category_id,                     \
                              std::string nm, bool per_frame) const {   \
    return get_has_key<UCName##Traits>(category_id, nm,                 \
                                       per_frame);                      \
  }                                                                     \
  std::string get_name(UCName##Key k) const {                           \
    return shared_->get_name(k);                                        \
  }                                                                     \
  Category get_category(UCName##Key k) const {                          \
    return k.get_category();                                            \
  }                                                                     \
  UCName##Key##s                                                        \
  get_##lcname##_keys(Category category_id) const {                     \
    return get_keys<UCName##Traits>(category_id);                       \
  }                                                                     \
  bool get_is_per_frame(UCName##Key k) const {                          \
    return shared_->get_is_per_frame(k);                                \
  }




namespace RMF {

  class NodeConstHandle;

#ifndef RMF_DOXYGEN
  typedef std::pair<NodeConstHandle, NodeConstHandle> BondPair;
  typedef vector<BondPair> BondPairs;
#endif

  //! A handle for a read-only RMF file
  /** Use this handle to perform operations relevant to the
      whole RMF hierarchy as well as to start traversal of the
      hierarchy.
      \see open_rmf_file_read_only
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
#if !defined(SWIG) && !defined(RMF_DOXYGEN)
 protected:
    internal::SharedData* get_shared_data() const {return shared_.get();}
    FileConstHandle(internal::SharedData *shared_);
#endif
  public:
    RMF_COMPARISONS(FileConstHandle);
    RMF_HASHABLE(FileConstHandle, return boost::hash_value(get_name()););
    RMF_SHOWABLE(FileConstHandle, get_name());
    //! Empty root handle, no open file.
    FileConstHandle(){}
#ifndef RMF_DOXYGEN
    FileConstHandle(std::string name);
#endif

    //! Return the root of the hierarchy
    NodeConstHandle get_root_node() const {
      return NodeConstHandle(0, shared_.get());
    }

    std::string get_name() const {
      return shared_->get_file_name();
    }

    std::string get_path() const {
      return shared_->get_file_path();
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
    template <class TypeT>
      Key<TypeT> get_key(Category category_id,
                         std::string name, bool per_frame) const {
      if (category_id == Category()) {
        return Key<TypeT>();
      } else {
        unsigned int num
          =internal::ConstGenericSharedData<TypeT>
          ::get_number_of_keys(shared_.get(), category_id.get_index(),
                               per_frame);
        for (unsigned int i=0; i< num; ++i) {
          Key<TypeT> k(category_id, i, per_frame);
          if (shared_->get_name(k) == name) return k;
        }
        return Key<TypeT> ();
      }
    }
    template <class TypeT>
      vector<Key<TypeT> > get_keys(Category category_id,
                                   const Strings& names,
                                   bool per_frame) const {
      vector<Key<TypeT> > ret(names.size());
      for (unsigned int i=0; i< names.size(); ++i) {
        ret[i]= get_key<TypeT>(category_id, names[i], per_frame);
        if (ret[i]==Key<TypeT>()) {
          ret.clear();
          return ret;
        }
      }
      return ret;
    }
    template <class TypeT>
      bool get_has_key(Category category_id,
                       std::string name, bool per_frame) const {
      return get_key<TypeT>(category_id, name, per_frame)
        != Key<TypeT>();
    }
    /** Return true if it has all the passed keys as a block.*/
    template <class TypeT>
        bool get_has_keys(Category category_id,
                       const Strings& names, bool per_frame) const {
      return get_key<TypeT>(category_id, names[0], per_frame)
          != Key<TypeT>();
    }
    /** Get a list of all keys of the given type,
     */
    template <class TypeT>
      vector<Key<TypeT> > get_keys(Category category_id) const {
      if (category_id==Category()) return vector<Key<TypeT> >();
      unsigned int num=internal::ConstGenericSharedData<TypeT>
        ::get_number_of_keys(shared_.get(), category_id.get_index(), false);
      unsigned int numpf=internal::ConstGenericSharedData<TypeT>
        ::get_number_of_keys(shared_.get(), category_id.get_index(), true);
      vector<Key<TypeT> > ret(num+numpf);
      for (unsigned int i=0; i< ret.size(); ++i) {
        bool pf=i >= num;
        unsigned int idx=pf? i-num: i;
        ret[i]= Key<TypeT>(category_id, idx, pf);
        RMF_INTERNAL_CHECK(!get_name(ret[i]).empty(),
                               "No name for key");
      }
      return ret;
    }
    /** @} */

    /** The file always has a single frame that is currently active at any given
        point.

        A value of ALL_FRAMES means one is only dealing with static data.

        @{
    */
    int get_current_frame() const {
      return shared_->get_current_frame();
    }
    void set_current_frame(int frame) {
      shared_->set_current_frame(frame);
    }
    /* @} */

    /** Return the number of frames in the file. Currently, this is the number
        of frames that the x-coordinate has, but it should be made more general.
    */
    unsigned int get_number_of_frames() const {
      return shared_->get_number_of_frames();
    }
    /** Frames can have associated comments which can be used to label
        particular frames of interest. Returns an empty string if the
        frame doesn't have a name.*/
    std::string get_frame_name() const;

    /** \name Non-template versions for python

        Type is one of the \ref rmf_types "standard types".
        @{
    */

   RMF_FOREACH_TYPE(RMF_HDF5_ROOT_CONST_KEY_TYPE_METHODS);

    /** @} */
#ifdef RMF_DOXYGEN
    /** \name Python only
        The following methods are only available in python.
        @{
    */
    //! Return a list with all the keys from that category
    PythonList get_keys(Category c) const;
    /** @} */
#endif
#ifndef SWIG
    /** Each node in the hierarchy can be associated with some arbitrary bit
        of external data. Nodes can be extracted using
        these bits of data.
    */
    template <class T>
      NodeConstHandle get_node_from_association(const T &d) const {
      if (! shared_->get_has_associated_node(d)) {
        return NodeConstHandle();
      } else {
        return NodeConstHandle(shared_->get_associated_node(d), shared_.get());
      }
    }
#else
    NodeConstHandle get_node_from_association(void* v) const;
#endif
    NodeConstHandle get_node_from_id(NodeID id) const;


    /** Along with the associations for nodes, arbitrary data can
        be associated with the file in memory to aid in processing.
    */
    template <class T>
      void add_associated_data(int index, const T &t) {
      shared_->set_user_data(index, t);
    }
    /** To get back the ith user data.*/
    template <class T>
      T get_associated_data(int index) {
      return shared_->get_user_data<T>(index);
    }

   /** To get back the ith user data.*/
    bool get_has_associated_data(int index) {
      return shared_->get_has_user_data(index);
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


    /** \name Key categories methods
        Methods for managing the key categories in this RMF.
        @{
    */
    bool get_has_category(std::string name) const {
      return get_category(name) != Category();
    }
    Category get_category(std::string name) const {
      for (unsigned int i=0; i< shared_->get_number_of_categories();++i) {
        if (shared_->get_category_name(i)==name) {
          return Category(i);
        }
      }
      return Category();
    }
    Categories get_categories() const {
      unsigned int r= shared_->get_number_of_categories();
      vector<Category > ret(r);
      for (unsigned int i=0; i< r; ++i) {
        ret[i]= Category(i);
      }
      return ret;
    }
  std::string get_name(Category kc) const {
    return shared_->get_category_name(kc.get_index());
  }
    /** @} */

    /** Make sure all data gets written to disk. Once flush is called, it
        should be safe to open the file in another process for reading.
     */
    void flush();

    /** Some backends support locking to allow simulataneous reading from
        and writing to the file from different processes.
        @{
    */
    bool get_supports_locking() const;
    /** Try to lock/unlock the file and return whether you have the lock.
        That is if you try to lock the file and false is returned, you did
        not succeed and should retry.

        You probably should use FileLock instead of calling this directly. */
    bool set_is_locked(bool tf);
    /** @} */

    /** Run the various validators that attempt to check that the RMF file
        is correct. Print messages to the provided stream if errors are
        encounted.*/
    void validate(std::ostream &out) const;

    /** Reread the file.
        \note This may invalidate various thing (eg the number of nodes may
        vary). Be careful.
    */
    void reload();
  };

  typedef vector<FileConstHandle> FileConstHandles;


  /**
     Open an RMF from a file system path in read-only mode.

     \param path the system path to the rmf file
     \exception RMF::IOException couldn't open file, or unsupported file format
  */
  RMFEXPORT FileConstHandle open_rmf_file_read_only(std::string path);

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
                              Float missing_value
                              =std::numeric_limits<double>::max());
  /** @} */


} /* namespace RMF */

#endif /* RMF__FILE_CONST_HANDLE_H */
