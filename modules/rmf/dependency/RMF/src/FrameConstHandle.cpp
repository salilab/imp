/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/FrameConstHandle.h>
#include <boost/tuple/tuple.hpp>
#include <RMF/Category.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>

RMF_ENABLE_WARNINGS

RMF_VECTOR_DEF(FrameConstHandle);

namespace RMF {

FrameConstHandle::FrameConstHandle(int frame, internal::SharedData *shared):
  frame_(frame), shared_(shared) {
}

FileConstHandle FrameConstHandle::get_file() const {
  return FileConstHandle(shared_.get());
}

FrameConstHandles FrameConstHandle::get_children() const {
  Ints children = shared_->get_children_frame(frame_);
  FrameConstHandles ret(children.size());
  for (unsigned int i = 0; i < ret.size(); ++i) {
    ret[i] = FrameConstHandle(children[i], shared_.get());
  }
  return ret;
}

void FrameConstHandle::set_as_current_frame() {
  get_file().set_current_frame(frame_);
}

std::string get_frame_type_name(FrameType t) {
  switch (t) {
  case STATIC:
    return "static";
  case FRAME:
    return "frame";
  case MODEL:
    return "model";
  case CENTER:
    return "center";
  case FRAME_ALIAS:
    return "alias";
  default:
    return "unknown";
  }
}

std::ostream &operator<<(std::ostream &out,
                         FrameType    t) {
  out << get_frame_type_name(t);
  return out;
}
std::istream &operator>>(std::istream &in,
                         FrameType    &t) {
  std::string token;
  in >> token;
  for (FrameType i = STATIC; i <= FRAME_ALIAS; i = FrameType(i + 1)) {
    if (token == get_frame_type_name(i)) {
      t = i;
      return in;
    }
  }
  t = FRAME;
  return in;
}
} /* namespace RMF */

RMF_DISABLE_WARNINGS

