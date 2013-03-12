/**
 *  \file RMF/FileHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_FILE_HANDLE_H
#define RMF_FILE_HANDLE_H

#include <RMF/config.h>
#include "internal/SharedData.h"
#include "Key.h"
#include "FileConstHandle.h"
#include "NodeHandle.h"
#include "FrameHandle.h"

RMF_ENABLE_WARNINGS

RMF_VECTOR_DECL(FileHandle);

namespace RMF {

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
public:
  //! Empty file handle, no open file.
  FileHandle() {
  }
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  FileHandle(internal::SharedData *shared_);
  FileHandle(std::string name, bool create);
#endif

  /** Return the root of the hierarchy stored in the file.
   */
  NodeHandle get_root_node() const {
    return NodeHandle(0, get_shared_data());
  }

  //! Return the root of the frame hierarchy
  FrameHandle get_root_frame() const {
    return FrameHandle(-1, get_shared_data());
  }

  //! Return the ith frame
  FrameHandle get_frame(unsigned int i) const {
    RMF_USAGE_CHECK(i < get_number_of_frames(),
                    "Out of range frame");
    return FrameHandle(i, get_shared_data());
  }

  FrameHandle get_current_frame() const {
    return FrameHandle(get_shared_data()->get_current_frame(),
                        get_shared_data());
  }

#ifndef SWIG
  /** Each node in the hierarchy can be associated with some arbitrary bit
      of external data. Nodes can be extracted using these bits of data.
   */
  template <class T>
  NodeHandle get_node_from_association(const T&d) const {
    if (!get_shared_data()->get_has_associated_node(d)) {
      return NodeHandle();
    } else {
      return NodeHandle(get_shared_data()->get_associated_node(d),
                        get_shared_data());
    }
  }
#else
  NodeHandle get_node_from_association(void*d) const;
#endif
  /** Return a NodeHandle from a NodeID. The NodeID must refer
      to a valid NodeHandle.*/
  NodeHandle get_node_from_id(NodeID id) const;
  /** Suggest how many frames the file is likely to have. This can
      make writing more efficient as space will be preallocated.
   */
  void set_number_of_frames_hint(unsigned int i) {
                        get_shared_data()->save_frames_hint(i);
  }
  /** Each RMF structure has an associated description. This should
      consist of unstructured text describing the contents of the RMF
      data. Conventionally. this description can consist of multiple
      paragraphs, each separated by a newline character and should end
      in a newline.
   */
  void set_description(std::string descr);

  /** Each RMF structure has an associated field that the code that
      produced the file can use to describe itself.
   */
  void set_producer(std::string);

  /** Make sure all data gets written to disk. Once flush is called, it
       should be safe to open the file in another process for reading.
   */
  void flush();
};

/**
   Create an RMF from a file system path.

   \param path the system path to the rmf file
   \exception RMF::IOException couldn't create file, or unsupported file format
 */
RMFEXPORT FileHandle create_rmf_file(std::string path);

#ifndef SWIG
/**
   Create an RMF in a buffer.

   \param buffer The buffer to place the contents in.
 */
RMFEXPORT FileHandle create_rmf_buffer(std::string &buffer);
#endif

/**
   Open an RMF from a file system path.

   \param path the system path to the rmf file
   \exception RMF::IOException couldn't create file, or unsupported file format
 */
RMFEXPORT FileHandle open_rmf_file(std::string path);


} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_FILE_HANDLE_H */
