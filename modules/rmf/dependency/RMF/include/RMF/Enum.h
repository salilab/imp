/**
 *  \file RMF/Enum.h
 *  \brief Declaration of RMF::Enum.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_ENUM_H
#define RMF_ENUM_H

#include "RMF/config.h"
#include "infrastructure_macros.h"
#include "exceptions.h"
#include "internal/small_set_map.h"

RMF_ENABLE_WARNINGS

namespace RMF {

struct RMFEXPORT FrameTypeTag {
// to make linking easy to get right
#ifndef IMP_DOXYGEN
  static RMF_SMALL_UNORDERED_MAP<std::string, int>& get_from();
  static RMF_SMALL_UNORDERED_MAP<int, std::string>& get_to();
#endif
};
struct RMFEXPORT NodeTypeTag {
#ifndef IMP_DOXYGEN
  static RMF_SMALL_UNORDERED_MAP<std::string, int>& get_from();
  static RMF_SMALL_UNORDERED_MAP<int, std::string>& get_to();
#endif
};

struct RMFEXPORT RepresentationTypeTag {
#ifndef IMP_DOXYGEN
  static RMF_SMALL_UNORDERED_MAP<std::string, int>& get_from();
  static RMF_SMALL_UNORDERED_MAP<int, std::string>& get_to();
#endif
};

//! A strong enum with an associated string name for each value
/** A general purpose Enum that has associated names and is type checked in
 * python.*/
template <class TagT>
class Enum {
  int i_;
  int compare(const Enum<TagT>& o) const {
    if (i_ < o.i_)
      return -1;
    else if (i_ > o.i_)
      return 1;
    else
      return 0;
  }
  std::string get_string() const { return TagT::get_to().find(i_)->second; }

 public:
  typedef TagT Tag;
  Enum() : i_(-1) {}
  explicit Enum(int i) : i_(i) {
    RMF_USAGE_CHECK(TagT::get_to().find(i) != TagT::get_to().end(),
                    "Enum value not defined");
  }
  Enum(std::string name) {
    RMF_USAGE_CHECK(TagT::get_from().find(name) != TagT::get_from().end(),
                    "Enum name not defined");
    i_ = TagT::get_from().find(name)->second;
  }
  Enum(int i, std::string name) : i_(i) {
    TagT::get_to()[i] = name;
    TagT::get_from()[name] = i;
  }
  RMF_COMPARISONS(Enum);
  RMF_HASHABLE(Enum, return i_);
  RMF_SHOWABLE(Enum, get_string());
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  operator int() const { return i_; }
#endif
};

#if !defined(SWIG) && !defined(RMF_DOXYGEN)
template <class Traits>
inline std::ostream& operator<<(std::ostream& out, Enum<Traits> v) {
  v.show(out);
  return out;
}
template <class Traits>
inline std::istream& operator>>(std::istream& in, Enum<Traits>& v) {
  std::string val;
  in >> val;
  v = Enum<Traits>(val);
  return in;
}
#endif

/**  The type for frames.

    See \ref frametypes "Frame Types" for a complete list of the possible
   values.
*/
typedef Enum<FrameTypeTag> FrameType;
/**  The type for nodes.

   See \ref nodetypes "Node Types" for a complete list of the possible values.
 */
typedef Enum<NodeTypeTag> NodeType;
/**  The type for representations used in decorator::Alternatives.

     See \ref representationtypes "Representaiton Types" for a complete list of
     the possible values.
*/
typedef Enum<RepresentationTypeTag> RepresentationType;

#ifndef RMF_DOXYGEN
typedef std::vector<FrameType> FrameTypes;
typedef std::vector<NodeType> NodeTypes;
typedef std::vector<RepresentationType> RepresentationTypes;
#endif

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_ENUM_H */
