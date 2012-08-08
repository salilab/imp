/**
 *  \file RMF/FileConstHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_FILE_CONST_HANDLE_H
#define IMPLIBRMF_FILE_CONST_HANDLE_H

#include "RMF_config.h"
#include "internal/SharedData.h"
#include "Key.h"
#include "NodeHandle.h"
#include "NodeSetHandle.h"
#include <boost/functional/hash.hpp>


#define IMP_HDF5_ROOT_CONST_KEY_TYPE_METHODS_INNER(lcname, UCName,      \
                                             arityname, ArityName,      \
                                             PassValue,                 \
                                             ReturnValue,               \
                                             PassValues, ReturnValues,  \
                                                   Arity)               \
  ArityName##UCName##Key                                                \
  get_##lcname##_key(ArityName##Category category_id,                   \
                     std::string nm,                                    \
                     bool per_frame) const {                            \
    return get_key<UCName##Traits, Arity>(category_id, nm, per_frame);  \
  }                                                                     \
  bool get_has_##lcname##_key(ArityName##Category category_id,          \
                              std::string nm, bool per_frame) const {   \
    return get_has_key<UCName##Traits, Arity>(category_id, nm,          \
                                              per_frame);               \
  }                                                                     \
  std::string get_name(ArityName##UCName##Key k) const {                \
    return shared_->get_name(k);                                        \
  }                                                                     \
  ArityName##Category get_category(ArityName##UCName##Key k) const {    \
    return k.get_category();                                            \
  }                                                                     \
  ArityName##UCName##Key##s                                             \
  get_##lcname##_keys(ArityName##Category category_id) const {          \
    return get_keys<UCName##Traits, Arity>(category_id);                \
  }                                                                     \
  bool get_is_per_frame(ArityName##UCName##Key k) const {               \
    return shared_->get_is_per_frame(k);                                \
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
    FileConstHandle(std::string name);
    BondPairs get_bonds()const;
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
                                std::string name, bool per_frame) const {
      if (category_id == CategoryD<Arity>()) {
        return Key<TypeT, Arity>();
      } else {
        unsigned int num
          =internal::ConstGenericSharedData<TypeT, Arity>
          ::get_number_of_keys(shared_.get(), category_id.get_index(),
                               per_frame);
        for (unsigned int i=0; i< num; ++i) {
          Key<TypeT, Arity> k(category_id, i, per_frame);
          if (shared_->get_name(k) == name) return k;
        }
        return Key<TypeT, Arity> ();
      }
    }
    template <class TypeT, int Arity>
        vector<Key<TypeT, Arity> > get_keys(CategoryD<Arity> category_id,
                                            const Strings& names,
                                            bool per_frame) const {
      vector<Key<TypeT, Arity> > ret(names.size());
      for (unsigned int i=0; i< names.size(); ++i) {
        ret[i]= get_key<TypeT>(category_id, names[i], per_frame);
        if (ret[i]==Key<TypeT, Arity>()) {
          ret.clear();
          return ret;
        }
      }
      return ret;
    }
    template <class TypeT, int Arity>
      bool get_has_key(CategoryD<Arity> category_id,
                       std::string name, bool per_frame) const {
      return get_key<TypeT, Arity>(category_id, name, per_frame)
        != Key<TypeT, Arity>();
    }
    /** Return true if it has all the passed keys as a block.*/
    template <class TypeT, int Arity>
        bool get_has_keys(CategoryD<Arity> category_id,
                       const Strings& names, bool per_frame) const {
      return get_key<TypeT, Arity>(category_id, names[0], per_frame)
          != Key<TypeT, Arity>();
    }
    /** Get a list of all keys of the given type,
     */
    template <class TypeT, int Arity>
      vector<Key<TypeT, Arity> > get_keys(CategoryD<Arity> category_id) const {
      if (category_id==CategoryD<Arity>()) return vector<Key<TypeT, Arity> >();
      unsigned int num=internal::ConstGenericSharedData<TypeT, Arity>
        ::get_number_of_keys(shared_.get(), category_id.get_index(), false);
      unsigned int numpf=internal::ConstGenericSharedData<TypeT, Arity>
        ::get_number_of_keys(shared_.get(), category_id.get_index(), true);
      vector<Key<TypeT, Arity> > ret(num+numpf);
      for (unsigned int i=0; i< ret.size(); ++i) {
        bool pf=i >= num;
        unsigned int idx=pf? i-num: i;
        ret[i]= Key<TypeT, Arity>(category_id, idx, pf);
        IMP_RMF_INTERNAL_CHECK(!get_name(ret[i]).empty(),
                               "No name for key");
      }
      return ret;
    }
    /** @} */

    /** Return the number of frames in the file. Currently, this is the number
        of frames that the x-coordinate has, but it should be made more general.
    */
    unsigned int get_number_of_frames() const {
      return shared_->get_number_of_frames();
    }
   /** Frames can have associated comments which can be used to label
        particular frames of interest.*/
    std::string get_frame_name(unsigned int frame);

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

    template <int Arity>
      unsigned int get_number_of_node_sets() const {
      return shared_->get_number_of_sets(Arity);
    }
    template <int Arity>
      vector<NodeSetConstHandle<Arity> > get_node_sets() const {
      unsigned int num= shared_->get_number_of_sets(Arity);
      vector<NodeSetConstHandle<Arity> > ret(num);
      for (unsigned int i=0; i< num; ++i) {
        ret[i]=NodeSetConstHandle<Arity>(i, shared_.get());
      }
      return ret;
    }


    IMP_HDF5_ROOT_CONST_KEY_SET_METHODS(pair, Pair, 2);
    IMP_HDF5_ROOT_CONST_KEY_SET_METHODS(triplet, Triplet, 3);
    IMP_HDF5_ROOT_CONST_KEY_SET_METHODS(quad, Quad, 4);

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
      return get_category<Arity>(name) != CategoryD<Arity>();
    }
    template <int Arity>
      CategoryD<Arity> get_category(std::string name) const {
      for (unsigned int i=0; i< shared_->get_number_of_categories(Arity);++i) {
        if (shared_->get_category_name(Arity, i)==name) {
          return CategoryD<Arity>(i);
        }
      }
      return CategoryD<Arity>();
    }
    template <int Arity>
      vector<CategoryD<Arity> > get_categories() const {
      unsigned int r= shared_->get_number_of_categories(Arity);
      vector<CategoryD<Arity> > ret(r);
      for (unsigned int i=0; i< r; ++i) {
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
