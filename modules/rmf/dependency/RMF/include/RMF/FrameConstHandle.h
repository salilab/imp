/**
 *  \file RMF/FrameConstHandle.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_FRAME_CONST_HANDLE_H
#define RMF_FRAME_CONST_HANDLE_H

#include <RMF/config.h>
#include "internal/SharedData.h"
#include "types.h"
#include "FrameID.h"
#include "constants.h"
#include <boost/intrusive_ptr.hpp>

RMF_ENABLE_WARNINGS

#define RMF_HDF5_FRAME_CONST_KEY_TYPE_METHODS(lcname, UCName, PassValue, \
                                              ReturnValue,               \
                                              PassValues, ReturnValues)  \
  /** \brief get the value of the attribute k from this frame
      The frame must have the attribute and if it is a per-frame
      attribute, and frame is not specified then frame 0 is
      used.
   */                                                                              \
  ReturnValue get_value(UCName##Key k) const {                                     \
    RMF_USAGE_CHECK(get_has_value(k),                                              \
                    internal::get_error_message("Frame ", get_name(),              \
                                                " does not have a value for key ", \
                                                shared_->get_name(k)));            \
    return get_value_always(k);                                                    \
  }                                                                                \
  /** Return the attribute value or TypeTraits::get_null_value() if the
      frame does not have the attribute. In python the method a value equal to
      eg RMF.NullFloat if the attribute is not there.*/               \
  ReturnValue get_value_always(UCName##Key k) const {                 \
    if (k == UCName##Key()) return UCName##Traits::get_null_value();  \
    return shared_->get_value_frame(frame_, k);                       \
  }                                                                   \
  /** If the default key is passed, false is returned.*/              \
  bool get_has_value(UCName##Key k) const {                           \
    if (k == UCName##Key()) return false;                             \
    return !UCName##Traits::get_is_null_value(get_value_always(k));   \
  }

RMF_VECTOR_DECL(FrameConstHandle);

namespace RMF {

class FileConstHandle;

//! The types of the frames.
enum FrameType {
  //! The root frame
  STATIC,
  //! A frame in a sequence of frames
  FRAME,
  //! An independent model
  MODEL,
  //! A cluster center
  CENTER,
  //! An alias for another frame
  FRAME_ALIAS
};

/** Return a string version of the type name.*/
RMFEXPORT
std::string get_frame_type_name(FrameType t);



#if !defined(RMF_DOXYGEN) && !defined(SWIG)
RMFEXPORT std::ostream &operator<<(std::ostream &out,
                                   FrameType    t);
RMFEXPORT std::istream &operator>>(std::istream &in,
                                   FrameType    &t);
#endif


class RootConstHandle;

//! A handle for a particular frame in a read-only hierarchy.
/** Use these handles to access parts of the
    frame hierarchy.

    The get_value_always() methods return the appropriate
    value or TypeTraits::get_null_value() if the frame does
    not have that attribute.

    See the FrameHandle for modifying the contents.
 */
class RMFEXPORT FrameConstHandle {
  int frame_;
  friend class FileHandle;
  boost::intrusive_ptr<internal::SharedData> shared_;
  int compare(const FrameConstHandle &o) const {
    if (frame_ < o.frame_) return -1;
    else if (frame_ > o.frame_) return 1;
    else if (shared_.get() < o.shared_.get()) return -1;
    else if (shared_.get() > o.shared_.get()) return 1;
    else return 0;
  }
#if !defined(SWIG) && !defined(RMF_DOXYGEN)
protected:
  internal::SharedData* get_shared_data() const {
    return shared_.get();
  }
public:
  int get_frame_id() const {
    return frame_;
  }
  FrameConstHandle(int frame, internal::SharedData *shared);
#endif

public:
  RMF_COMPARISONS(FrameConstHandle);
  RMF_HASHABLE(FrameConstHandle, return frame_);
  FrameConstHandle(): frame_(-2) {
  }

  //! Return the name of the frame
  std::string get_name() const {
    return shared_->get_frame_name(frame_);
  }
  FrameConstHandles get_children() const;

  //! get the type of this frame
  FrameType get_type() const {
    return FrameType(shared_->get_type(frame_));
  }
  //! get a unique id for this frame
  FrameID get_id() const {
    return FrameID(frame_);
  }

  //! Set this to be the current frame in the file
  void set_as_current_frame();

  /** \name Functions to access attributes

      Type is one of the \ref rmf_types "standard types".

      @{
   */
  RMF_FOREACH_TYPE(RMF_HDF5_FRAME_CONST_KEY_TYPE_METHODS);
  /** @} */
  RMF_SHOWABLE(FrameConstHandle,
               get_name() << "(" << get_type() << ", " << frame_ << ")");

  FileConstHandle get_file() const;
};

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_FRAME_CONST_HANDLE_H */
