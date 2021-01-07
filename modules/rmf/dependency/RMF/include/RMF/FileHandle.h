/**
 *  \file RMF/FileHandle.h
 *  \brief Declaration for RMF::FileHandle.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_FILE_HANDLE_H
#define RMF_FILE_HANDLE_H

#include <boost/shared_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "FileConstHandle.h"
#include "ID.h"
#include "NodeHandle.h"
#include "RMF/config.h"
#include "RMF/internal/SharedData.h"
#include "compiler_macros.h"
#include "enums.h"
#include "internal/SharedData.h"
#include "types.h"

namespace RMF {
namespace internal {
class SharedData;
}  // namespace internal
}  // namespace RMF

RMF_ENABLE_WARNINGS

namespace RMF {

class FileHandle;

//! Pass a list of them
typedef std::vector<FileHandle> FileHandles;

class BufferHandle;

//! A handle for an RMF file
/** Use this handle to perform operations relevant to the
    whole RMF hierarchy as well as to start traversal of the
    hierarchy.

    Make sure to check out the base class documentation
    for the non-modifying methods.

    \see create_rmf_file
    \see create_rmf_buffer
 */
class RMFEXPORT FileHandle : public FileConstHandle {
  friend class NodeHandle;
  friend class boost::shared_ptr<internal::SharedData>;

 public:
  //! Empty file handle, no open file.
  FileHandle() {}
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  FileHandle(boost::shared_ptr<internal::SharedData> shared_);
#endif

  //! Return the root of the hierarchy stored in the file.
  NodeHandle get_root_node() const { return NodeHandle(NodeID(0), shared_); }

  //! Add a frame and make it the current frame.
  FrameID add_frame(std::string name, FrameType t = FRAME) const;

  //! Add a frame and make it the current frame.
  /** It will be the child of the passed frame. */
  FrameID add_frame(std::string name, FrameID parent,
                    FrameType t = FRAME) const;
#ifndef SWIG
  /** Each node in the hierarchy can be associated with some arbitrary bit
      of external data. Nodes can be extracted using these bits of data.
   */
  template <class T>
  NodeHandle get_node_from_association(const T& d) const {
    if (!shared_->get_has_associated_node(d)) {
      return NodeHandle();
    } else {
      return NodeHandle(shared_->get_associated_node(d), shared_);
    }
  }
#else
  NodeHandle get_node_from_association(void* d) const;
#endif
  /* @} */

  //! Return a NodeHandle from a NodeID.
  /** The NodeID must refer to a valid NodeHandle. */
  NodeHandle get_node(NodeID id) const;

  //! Add a node with no parents.
  /** This node will not be accessible in the
      hierarchy unless you add it as a child of something. */
  NodeHandle add_node(std::string name, NodeType t) const;

  /** Each RMF structure has an associated description. This should
      consist of unstructured text describing the contents of the RMF
      data. Conventionally. this description can consist of multiple
      paragraphs, each separated by a newline character and should end
      in a newline.
   */
  void set_description(std::string descr) const;

  /** Each RMF structure has an associated field that the code that
      produced the file can use to describe itself.
   */
  void set_producer(std::string) const;

  //! Make sure all data gets written to disk.
  /** Once flush is called, it should be safe to open the file in
      another process for reading.
   */
  void flush() const;
};

//! Create an RMF from a file system path.
/** \param path the system path to the rmf file
    \exception RMF::IOException couldn't create file, or unsupported file format
 */
RMFEXPORT FileHandle create_rmf_file(std::string path);

//! Create an RMF in a buffer.
/** \param buffer The buffer to place the contents in.
 */
RMFEXPORT FileHandle create_rmf_buffer(BufferHandle buffer);

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_FILE_HANDLE_H */
