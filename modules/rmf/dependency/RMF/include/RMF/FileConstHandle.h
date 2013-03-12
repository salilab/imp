/**
 *  \file RMF/FileConstHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_FILE_CONST_HANDLE_H
#define RMF_FILE_CONST_HANDLE_H

#include <RMF/config.h>
#include "internal/SharedData.h"
#include "Key.h"
#include "NodeConstHandle.h"
#include "FrameConstHandle.h"
#include <boost/functional/hash.hpp>
#include <boost/intrusive_ptr.hpp>

RMF_ENABLE_WARNINGS

#define RMF_FILE_CATCH(extra_info)                                      \
  catch (Exception &e) {                                                \
    RMF_RETHROW(File(get_path())                                        \
                << Frame(get_current_frame().get_id().get_index())      \
                << Operation(BOOST_CURRENT_FUNCTION)                    \
                extra_info, e);                                         \
  }

#define RMF_HDF5_ROOT_CONST_KEY_TYPE_METHODS(lcname, UCName, \
                                             PassValue,      \
                                             ReturnValue,    \
                                             PassValues,     \
                                             ReturnValues)   \
  UCName##Key                                                \
  get_##lcname##_key(Category category_id,                   \
                     std::string nm) const {                 \
    return get_key<UCName##Traits>(category_id, nm);         \
  }                                                          \
  std::string get_name(UCName##Key k) const {                \
    try {                                                    \
      return shared_->get_name(k);                           \
    } RMF_FILE_CATCH( );                                     \
  }                                                          \
  Category get_category(UCName##Key k) const {               \
    return shared_->get_category(k);                         \
  }                                                          \
  /** This returns all the keys that are used in the current frame.
      Other frames may have different ones.*/     \
  UCName##Key##s                                  \
  get_##lcname##_keys(Category category_id) {     \
    return get_keys<UCName##Traits>(category_id); \
  }


RMF_VECTOR_DECL(FileConstHandle);

namespace RMF {

class NodeConstHandle;

//! A handle for a read-only RMF file
/** Use this handle to perform operations relevant to the
    whole RMF hierarchy as well as to start traversal of the
    hierarchy.
    \see open_rmf_file_read_only
 */
class RMFEXPORT FileConstHandle {
  void gather_ids(NodeConstHandle n, Ints &ids,
                  std::vector<std::string> &paths,
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
  internal::SharedData* get_shared_data() const {
    return shared_.get();
  }
#endif
public:
  RMF_COMPARISONS(FileConstHandle);
  RMF_HASHABLE(FileConstHandle, return boost::hash_value(get_name()); );
  RMF_SHOWABLE(FileConstHandle, get_name());
  //! Empty root handle, no open file.
  FileConstHandle() {
  }
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  FileConstHandle(internal::SharedData *shared_);
  FileConstHandle(std::string name);
#endif

  //! Return the root of the hierarchy
  NodeConstHandle get_root_node() const {
    return NodeConstHandle(0, shared_.get());
  }

  //! Return the root of the hierarchy
  FrameConstHandle get_root_frame() const {
    return FrameConstHandle(-1, shared_.get());
  }

  //! Return the ith frame
  FrameConstHandle get_frame(unsigned int i) const {
    try {
      RMF_INDEX_CHECK(i, get_number_of_frames());
      return FrameConstHandle(i, shared_.get());
    } RMF_FILE_CATCH(<< Frame(i));
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
  Key<TypeT> get_key(Category    category,
                     std::string name) const {
    try {
      return internal::GenericSharedData<TypeT>
             ::get_key(shared_.get(), category,
                       name);
    } RMF_FILE_CATCH(<< Category(get_name(category))
                     << Key(name));
  }
  template <class TypeT>
      std::vector<Key<TypeT> > get_keys(Category      category_id,
                               const Strings & names) const {
    try {
      std::vector<Key<TypeT> > ret(names.size());
      for (unsigned int i = 0; i < names.size(); ++i) {
        ret[i] = get_key<TypeT>(category_id, names[i]);
        if (ret[i] == Key<TypeT>()) {
          ret.clear();
          return ret;
        }
      }
      return ret;
    } RMF_FILE_CATCH(<< Category(get_name(category_id)));
  }
  /** Get a list of all keys of the given type,
   */
  template <class TypeT>
      std::vector<Key<TypeT> > get_keys(Category category) {
    try {
      if (category == Category()) return std::vector<Key<TypeT> >();
      return internal::GenericSharedData<TypeT>
             ::get_keys(shared_.get(), category);
    } RMF_FILE_CATCH(<< Category(get_name(category)));
  }
  /** @} */

  /** The file always has a single frame that is currently active at any given
      point.

      A value of ALL_FRAMES means one is only dealing with static data.

      @{
   */
  FrameConstHandle get_current_frame() const {
    return FrameConstHandle(shared_->get_current_frame(), shared_.get());
  }
#ifndef IMP_DOXYGEN
  void set_current_frame(int frame) {
    try {
      shared_->set_current_frame(frame);
    } RMF_FILE_CATCH(<< Frame(frame));
  }
#endif
  /* @} */

  /** Return the number of frames in the file. Currently, this is the number
      of frames that the x-coordinate has, but it should be made more general.
   */
  unsigned int get_number_of_frames() const {
    try {
      return shared_->get_number_of_frames();
    } RMF_FILE_CATCH( );
  }

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
    if (!shared_->get_has_associated_node(d)) {
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

  /** Each RMF structure has an associated description. This should
      consist of unstructured text describing the contents of the RMF
      data. Conventionally. this description can consist of multiple
      paragraphs, each separated by a newline character and should end
      in a newline.
   */
  std::string get_description() const;

  /** Each RMF structure has an associated field that the code that
      produced the file can use to describe itself.
   */
  std::string get_producer() const;

  /** \name Key categories methods
      Methods for managing the key categories in this RMF.
      @{
   */
  Category get_category(std::string name) {
    try {
      return shared_->get_category(name);
    } RMF_FILE_CATCH(<< Category(name));
  }
  /** This returns all the categories that are used in the current frame.
      Other frames may have different ones.*/
  Categories get_categories() const {
    try {
      return shared_->get_categories();
    } RMF_FILE_CATCH( );
  }
  std::string get_name(Category kc) const {
    try {
      return shared_->get_category_name(kc);
    } RMF_FILE_CATCH( );
  }
  /** @} */

  /** Run the various validators that attempt to check that the RMF file
      is correct. Print messages to the provided stream if errors are
      encounted.*/
  void validate(std::ostream &out);

  /** Like validate(std::ostream&) except it returns a sting describing
      the errors.*/
  std::string validate();

  /** Reread the file.
      \note This may invalidate various thing (eg the number of nodes may
      vary). Be careful.
   */
  void reload();
};


/**
   Open an RMF from a file system path in read-only mode.

   \param path the system path to the rmf file
   \exception RMF::IOException couldn't open file, or unsupported file format
 */
RMFEXPORT FileConstHandle open_rmf_file_read_only(std::string path);

/**
   Open an RMF from a buffer in read-only mode.

   \param buffer a buffer containing an RMF
   \exception RMF::IOException couldn't parse the buffer,
   or unsupported file format
 */
RMFEXPORT FileConstHandle open_rmf_buffer_read_only(const std::string& buffer);


/** \name Batch data access
    These methods provide batch access to attribute data to try
    to reduce the overhead of repeated function calls.

    The missing_value argument is a placeholder that can fill in
    for values which are not found in the respective node.

    \note These methods are experimental and subject to change.
    @{
 */
RMFEXPORT Floats get_values(const NodeConstHandles            &nodes,
                            FloatKey                          k,
                            Float                             missing_value
                              = std::numeric_limits<double>:: max());
/** @} */


} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_FILE_CONST_HANDLE_H */
