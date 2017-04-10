/**
 *  \file RMF/FileConstHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_FILE_CONST_HANDLE_H
#define RMF_FILE_CONST_HANDLE_H

#include <boost/current_function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <iosfwd>
#include <limits>
#include <string>
#include <vector>

#include "BufferHandle.h"
#include "NodeConstHandle.h"
#include "RMF/ID.h"
#include "RMF/config.h"
#include "RMF/constants.h"
#include "RMF/enums.h"
#include "RMF/exceptions.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/internal/SharedData.h"
#include "RMF/internal/errors.h"
#include "RMF/internal/shared_data_ranges.h"
#include "internal/SharedData.h"
#include "internal/shared_data_ranges.h"
#include "types.h"

namespace RMF {
class BufferConstHandle;
class FileConstHandle;
}  // namespace RMF

RMF_ENABLE_WARNINGS

#define RMF_FILE_CATCH(extra_info)                                        \
  catch (Exception& e) {                                                  \
    RMF_RETHROW(                                                          \
        File(get_path()) << Frame(get_current_frame())                    \
                         << Operation(BOOST_CURRENT_FUNCTION) extra_info, \
        e);                                                               \
  }

#ifndef SWIG
#define RMF_HDF5_ROOT_CONST_KEY_TYPE_METHODS(Traits, UCName)
#else
#define RMF_HDF5_ROOT_CONST_KEY_TYPE_METHODS(Traits, UCName)        \
  UCName##Key get_key(Category category_id, std::string nm,         \
                      UCName##Tag) const;                           \
  std::string get_name(UCName##Key k) const;                        \
  Category get_category(UCName##Key k) const;                       \
  /** This returns all the keys that are used in the current frame. \
      Other frames may have different ones.*/                       \
  UCName##Key##s get_keys(Category category_id, UCName##Tag);
#endif

namespace RMF {

class FileConstHandle;
class FileHandle;
class NodeConstHandle;

//! Pass a list of them
typedef std::vector<FileConstHandle> FileConstHandles;

//! A handle for a read-only RMF file
/** Use this handle to perform operations relevant to the
    whole RMF hierarchy as well as to start traversal of the
    hierarchy.
    \see open_rmf_file_read_only
 */
class RMFEXPORT FileConstHandle {
  void gather_ids(NodeConstHandle n, Ints& ids, std::vector<std::string>& paths,
                  std::string path) const;
#ifndef SWIG
  friend class RMFEXPORT NodeConstHandle;
  friend RMFEXPORT void clone_file_info(FileConstHandle, FileHandle);
  friend RMFEXPORT void clone_hierarchy(FileConstHandle, FileHandle);
  friend RMFEXPORT void clone_static_frame(FileConstHandle, FileHandle);
  friend RMFEXPORT void clone_loaded_frame(FileConstHandle, FileHandle);
  friend RMFEXPORT bool get_equal_current_values(FileConstHandle,
                                                 FileConstHandle);
  friend RMFEXPORT bool get_equal_static_values(FileConstHandle,
                                                FileConstHandle);
#endif
  int compare(const FileConstHandle& o) const {
    if (get_name() < o.get_name())
      return -1;
    else if (get_name() > o.get_name())
      return 1;
    else
      return 0;
  }

 protected:
  boost::shared_ptr<internal::SharedData> shared_;

 public:
  RMF_COMPARISONS(FileConstHandle);
  RMF_HASHABLE(FileConstHandle, return boost::hash_value(get_name()););
  RMF_SHOWABLE(FileConstHandle, get_name());
  //! Empty root handle, no open file.
  FileConstHandle() {}
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  FileConstHandle(boost::shared_ptr<internal::SharedData> shared);
#endif

  //! Return the root of the hierarchy
  NodeConstHandle get_root_node() const {
    return NodeConstHandle(NodeID(0), shared_);
  }

  std::string get_name() const { return shared_->get_file_name(); }

  std::string get_path() const { return shared_->get_file_path(); }

  /** \name Methods for manipulating keys
      When using C++ it is most convenient to specify types
      when adding and using keys through template arguments. For python
      we provide non-template versions, below.
      @{
   */
  /** Get an existing key that has the given name of the
      given type or Key() if the key is not found.
   */
  template <class Tag>
  ID<Tag> get_key(Category category, std::string name) const {
    try {
      return shared_->get_key(category, name, Tag());
    }
    RMF_FILE_CATCH(<< Category(get_name(category)) << Key(name));
  }

  template <class Tag>
  std::vector<ID<Tag> > get_keys(Category category_id,
                                 const Strings& names) const {
    try {
      std::vector<ID<Tag> > ret(names.size());
      for (unsigned int i = 0; i < names.size(); ++i) {
        ret[i] = get_key<Tag>(category_id, names[i]);
        if (ret[i] == ID<Tag>()) {
          ret.clear();
          return ret;
        }
      }
      return ret;
    }
    RMF_FILE_CATCH(<< Category(get_name(category_id)));
  }

  /** Get a list of all keys of the given type,
   */
  template <class Tag>
  std::vector<ID<Tag> > get_keys(Category category) const {
    try {
      if (category == Category()) return std::vector<ID<Tag> >();
      return shared_->get_keys(category, Tag());
    }
    RMF_FILE_CATCH(<< Category(get_name(category)));
  }

  template <class Tag>
  ID<Tag> get_key(Category category_id, std::string nm, Tag) const {
    return get_key<Tag>(category_id, nm);
  }
  template <class Tag>
  std::string get_name(ID<Tag> k) const {
    try {
      return shared_->get_name(k);
    }
    RMF_FILE_CATCH();
  }
  template <class Tag>
  Category get_category(ID<Tag> k) const {
    return shared_->get_category(k);
  }
  /** This returns all the keys that are used in the current frame. \
      Other frames may have different ones.*/
  template <class Tag>
  std::vector<ID<Tag> > get_keys(Category category_id, Tag) {
    return get_keys<Tag>(category_id);
  }

  /** @} */

  /** The file always has a single frame that is currently active at any given
      point.
      @{
   */
  FrameID get_current_frame() const { return shared_->get_loaded_frame(); }
  FrameType get_type(FrameID fr) const {
    return shared_->get_frame_data(fr).type;
  }
  std::string get_name(FrameID fr) const {
    return shared_->get_frame_data(fr).name;
  }
  FrameIDs get_children(FrameID id) const {
    const internal::FrameData& fd = shared_->get_frame_data(id);
    return FrameIDs(fd.children.begin(), fd.children.end());
  }
  FrameIDs get_parents(FrameID id) const {
    const internal::FrameData& fd = shared_->get_frame_data(id);
    return FrameIDs(fd.parents.begin(), fd.parents.end());
  }
  void set_current_frame(FrameID frame) const {
    RMF_USAGE_CHECK(frame != FrameID(), "Invalid frame passed.");
    RMF_USAGE_CHECK(frame != ALL_FRAMES,
                    "Use set_static_value() and get_static_value() to "
                    "manipulate the static frame.");
    try {
      shared_->set_loaded_frame(frame);
    }
    RMF_FILE_CATCH(<< Frame(frame));
  }

  /** Return the number of frames in the file.
   */
  unsigned int get_number_of_frames() const {
    try {
      return shared_->get_number_of_frames();
    }
    RMF_FILE_CATCH();
  }

  /** Return the number of nodes in the file.
   */
  unsigned int get_number_of_nodes() const {
    try {
      return shared_->get_number_of_nodes();
    }
    RMF_FILE_CATCH();
  }

  /** Return a string identifying the file type.
  */
  std::string get_file_type() const { return shared_->get_file_type(); }

  /** Get all the frames that are roots (aren't subframes). */
  FrameIDs get_root_frames() const;

#ifndef SWIG
  boost::iterator_range<internal::integer_iterator<FrameID> > get_frames()
      const {
    return internal::get_frames(shared_.get());
  }

  boost::iterator_range<internal::integer_iterator<NodeID> > get_node_ids()
      const {
    return internal::get_nodes(shared_.get());
  }
#endif

  /** \name Non-template versions for python

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
  NodeConstHandle get_node_from_association(const T& d) const {
    if (!shared_->get_has_associated_node(d)) {
      return NodeConstHandle();
    } else {
      return NodeConstHandle(shared_->get_associated_node(d), shared_);
    }
  }
#else
  NodeConstHandle get_node_from_association(void* v) const;
#endif
  NodeConstHandle get_node(NodeID id) const;

  /** Along with the associations for nodes, arbitrary data can
      be associated with the file in memory to aid in processing.
   */
  template <class T>
  void add_associated_data(int index, const T& t) {
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
    }
    RMF_FILE_CATCH(<< Category(name));
  }
  /** This returns all the categories that are used in the current frame.
      Other frames may have different ones.*/
  Categories get_categories() const {
    try {
      return shared_->get_categories();
    }
    RMF_FILE_CATCH();
  }
  std::string get_name(Category kc) const {
    try {
      return shared_->get_name(kc);
    }
    RMF_FILE_CATCH();
  }
  /** @} */

  //! Reread the file.
  /** \note This may invalidate various things (e.g. the number of nodes may
      vary). Be careful.
   */
  void reload();
};

//! Produce hash values for boost hash tables.
inline std::size_t hash_value(const FileConstHandle& t) {
  return t.__hash__();
}

/**
   Open an RMF from a file system path in read-only mode.

   \param path the system path to the rmf file
   \exception RMF::IOException couldn't open file, or unsupported file format
 */
RMFEXPORT FileConstHandle open_rmf_file_read_only(std::string path);

/**
   Open an RMF from a buffer in read-only mode.

   \exception RMF::IOException couldn't open file, or unsupported file format
 */
RMFEXPORT FileConstHandle open_rmf_buffer_read_only(BufferConstHandle buffer);

/** \name Batch data access
    These methods provide batch access to attribute data to try
    to reduce the overhead of repeated function calls.

    The missing_value argument is a placeholder that can fill in
    for values which are not found in the respective node.

    \note These methods are experimental and subject to change.
    @{
 */
RMFEXPORT Floats
    get_values(const NodeConstHandles& nodes, FloatKey k,
               Float missing_value = std::numeric_limits<float>::max());
/** @} */

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_FILE_CONST_HANDLE_H */
