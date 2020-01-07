/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/Enum.h"

RMF_ENABLE_WARNINGS

namespace RMF {
RMF_SMALL_UNORDERED_MAP<std::string, int>& FrameTypeTag::get_from() {
  static RMF_SMALL_UNORDERED_MAP<std::string, int> from_name;
  return from_name;
}
RMF_SMALL_UNORDERED_MAP<int, std::string>& FrameTypeTag::get_to() {
  static RMF_SMALL_UNORDERED_MAP<int, std::string> to_name;
  return to_name;
}

RMF_SMALL_UNORDERED_MAP<std::string, int>& NodeTypeTag::get_from() {
  static RMF_SMALL_UNORDERED_MAP<std::string, int> from_name;
  return from_name;
}
RMF_SMALL_UNORDERED_MAP<int, std::string>& NodeTypeTag::get_to() {
  static RMF_SMALL_UNORDERED_MAP<int, std::string> to_name;
  return to_name;
}

RMF_SMALL_UNORDERED_MAP<std::string, int>& RepresentationTypeTag::get_from() {
  static RMF_SMALL_UNORDERED_MAP<std::string, int> from_name;
  return from_name;
}
RMF_SMALL_UNORDERED_MAP<int, std::string>& RepresentationTypeTag::get_to() {
  static RMF_SMALL_UNORDERED_MAP<int, std::string> to_name;
  return to_name;
}
} /* namespace RMF */

RMF_DISABLE_WARNINGS
