/**
 *  \file RMF/NodeConstHandle.h
 *  \brief Declaration of NodeConstHandlke.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_NODE_CONST_HANDLE_H
#define RMF_NODE_CONST_HANDLE_H

#include <boost/any.hpp>
#include <boost/current_function.hpp>
#include <memory>
#include <iostream>
#include <string>
#include <vector>

#include "RMF/ID.h"
#include "RMF/Nullable.h"
#include "RMF/config.h"
#include "RMF/exceptions.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/internal/SharedData.h"
#include "RMF/internal/errors.h"
#include "RMF/constants.h"
#include "RMF/enums.h"
#include "RMF/internal/SharedData.h"
#include "RMF/keys.h"
#include "RMF/traits.h"
#include "RMF/types.h"

namespace RMF {
class NodeConstHandle;
}  // namespace RMF

RMF_ENABLE_WARNINGS

#define RMF_NODE_CATCH(extra_info)                                             \
  catch (Exception& e) {                                                       \
    RMF_RETHROW(                                                               \
        File(get_file_name()) << Node(get_id())                                \
                              << Frame(get_current_frame_id())                 \
                              << Operation(BOOST_CURRENT_FUNCTION) extra_info, \
        e);                                                                    \
  }

#define RMF_NODE_CATCH_KEY(k, extra_info) \
  RMF_NODE_CATCH(<< Key(get_name(k))      \
                 << Category(get_category_name(k)) extra_info)

#ifndef SWIG
#define RMF_HDF5_NODE_CONST_KEY_TYPE_METHODS_DECL(Traits, UCName)
#else
#define RMF_HDF5_NODE_CONST_KEY_TYPE_METHODS_DECL(Traits, UCName) \
 public:                                                          \
  Nullable<UCName> get_value(UCName##Key k) const;                \
  bool get_has_value(UCName##Key k) const;                        \
  Nullable<UCName> get_frame_value(UCName##Key k) const;          \
  Nullable<UCName> get_static_value(UCName##Key k) const;
#endif
namespace RMF {

class FileConstHandle;
class NodeConstHandle;
class RootConstHandle;

//! Pass a list of them
typedef std::vector<NodeConstHandle> NodeConstHandles;

//! A handle for a particular node in a read-only hierarchy.
/** Use these handles to access parts of the
    hierarchy.

    The get_value_always() methods return the appropriate
    value or TypeTraits::get_null_value() if the node does
    not have that attribute.

    See the NodeHandle for modifying the contents.
 */
class RMFEXPORT NodeConstHandle
#ifdef SWIG
    // get conversions right
    : public NodeID
#endif
      {
  friend class FileHandle;
  int compare(const NodeConstHandle& o) const {
    if (node_ < o.node_)
      return -1;
    else if (node_ > o.node_)
      return 1;
    else if (shared_.get() < o.shared_.get())
      return -1;
    else if (shared_.get() > o.shared_.get())
      return 1;
    else
      return 0;
  }

  // hopefully get_value will be inlined...
  template <class Tag>
  Nullable<typename Tag::Type> get_value_impl(ID<Tag> k) const {
    if (shared_->get_loaded_frame() != FrameID()) {
      Nullable<typename Tag::Type> ret = get_frame_value(k);
      if (!ret.get_is_null()) return ret;
    }
    return get_static_value(k);
  }

 protected:
  NodeID node_;
  std::shared_ptr<internal::SharedData> shared_;
  // for error messages
  std::string get_file_name() const;
  // for error messages
  FrameID get_current_frame_id() const;

#if !defined(SWIG) && !defined(RMF_DOXYGEN)
 public:
  NodeConstHandle(NodeID node, std::shared_ptr<internal::SharedData> shared);
#endif

 public:
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  std::shared_ptr<internal::SharedData> get_shared_data() const {
    return shared_;
  }
#endif
#if !defined(SWIG)
  operator NodeID() const { return node_; }
#endif

  /** Get the NodeID for this handle. */
  NodeID get_id() const { return node_; }
  RMF_COMPARISONS(NodeConstHandle);
  RMF_HASHABLE(NodeConstHandle, return node_.get_index());
  NodeConstHandle() {}

  /** The name of the node is not necessarily unique. */
  std::string get_name() const { return shared_->get_name(node_); }
  NodeConstHandles get_children() const;

 /** \name Functions to access attributes

      @{
   */
  template <class Tag>
  Nullable<typename Tag::Type> get_value(ID<Tag> k) const {
    return get_value_impl(k);
  }
  template <class Tag>
  bool get_has_value(ID<Tag> k) const {
    return !get_value(k).get_is_null();
  }
  template <class Tag>
  Nullable<typename Tag::Type> get_frame_value(ID<Tag> k) const {
    RMF_USAGE_CHECK(shared_->get_loaded_frame() != FrameID(),
                    "Need to set a current frame before getting frame values.");
    return shared_->get_loaded_value(node_, k);
  }
  template <class Tag>
  Nullable<typename Tag::Type> get_static_value(ID<Tag> k) const {
    return shared_->get_static_value(node_, k);
  }
/** @} */

#ifndef SWIG
  /** Each node can be associated at runtime with an
      arbitrary piece of data to aid in maintaining the
      relationship with application data structures. This
      data type must be convertible to a void* pointer.
      If direct casting does not work, you can implement
      a function called \c get_void_pointer() that takes
      the passed type and returns a \c void* pointer in
      the namespace where the type is declared (so it is
      found via Koenig lookup). Support has already been
      added for std::shared_ptr.

      Either the association must not have been set before
      or overwrite must be true. If overwrite is true,
      the type must be the same as the old type.
   */
  template <class T>
  void set_association(const T& v, bool overwrite = false) const {
    shared_->set_association(node_, v, overwrite);
  }
#else
  void set_association(void* v, bool overwrite = false) const;
#endif
  //! Return the associated pointer for this node
  /** An exception will be thrown if it doesn't have one.*/
  template <class T>
  T get_association() const {
    return boost::any_cast<T>(shared_->get_association(node_));
  }

  bool get_has_association() const {
    return shared_->get_has_association(node_);
  }

  //! get the type of this node
  NodeType get_type() const { return NodeType(shared_->get_type(node_)); }

  RMF_FOREACH_TYPE(RMF_HDF5_NODE_CONST_KEY_TYPE_METHODS_DECL);

  RMF_SHOWABLE(NodeConstHandle, get_name() << "(" << get_type() << ", " << node_
                                           << ")");

  FileConstHandle get_file() const;
};

//! Produce hash values for boost hash tables.
inline std::size_t hash_value(const NodeConstHandle& t) {
  return t.__hash__();
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_NODE_CONST_HANDLE_H */
