/**
 *  \file RMF/FrameHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_FRAME_HANDLE_H
#define RMF_FRAME_HANDLE_H

#include <RMF/config.h>
#include "internal/SharedData.h"
#include "types.h"
#include "FrameID.h"
#include "FrameConstHandle.h"

RMF_ENABLE_WARNINGS

#define RMF_HDF5_FRAME_KEY_TYPE_METHODS(lcname, UCName, PassValue, ReturnValue, \
                                        PassValues, ReturnValues)               \
  /** \brief  set the value of the attribute k for this frame
      If it is a per-frame attribute, frame must be specified.
   */                                                  \
  void set_value(UCName##Key k, PassValue v) {         \
    get_shared_data()->set_value_frame(get_frame_id(), \
                                       k, v);          \
  }

RMF_VECTOR_DECL(FrameHandle);

namespace RMF {

class FileHandle;

//! A handle for a particular frame in the hierarchy.
/** Use these handles to access and modify parts of the
    hierarchy.

    Make sure to check out the base class for the const
    methods.
 */
class RMFEXPORT FrameHandle: public FrameConstHandle {
  friend class FileHandle;
#if !defined(SWIG) && !defined(RMF_DOXYGEN)
public:
  FrameHandle(int frame, internal::SharedData *shared);
#endif

public:
  FrameHandle() {
  }
  /** Create a new frame as a child of this one and make it
      the current frame.
   */
  FrameHandle add_child(std::string name, FrameType t);

  /** Add an existing frame as a child.*/
  void add_child(FrameConstHandle nh);

  FrameHandles get_children() const;

  /** \name Functions to access attributes

      Type is one of the \ref rmf_types "standard types".

      @{
   */
  RMF_FOREACH_TYPE(RMF_HDF5_FRAME_KEY_TYPE_METHODS);
  /** @} */

  FileHandle get_file() const;
};
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_FRAME_HANDLE_H */
