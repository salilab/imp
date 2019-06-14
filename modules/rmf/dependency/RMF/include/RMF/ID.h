/**
 *  \file RMF/ID.h
 *  \brief Declaration of RMF::ID.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_ID_H
#define RMF_ID_H

#include "RMF/config.h"
#include "infrastructure_macros.h"
#include "exceptions.h"
#include <limits>

RMF_ENABLE_WARNINGS

namespace RMF {

struct FrameTag {
  static std::string get_tag() { return "f"; }
};
struct NodeTag {
  static std::string get_tag() { return "n"; }
};
struct CategoryTag {
  static std::string get_tag() { return "c"; }
};

//! A general purpose ID in RMF used, with different tags, to identify things.
template <class TagT>
class ID {
  int i_;
  int compare(const ID<TagT>& o) const {
    if (i_ < o.i_)
      return -1;
    else if (i_ > o.i_)
      return 1;
    else
      return 0;
  }
  std::string get_string() const {
    if (i_ == -1)
      return Tag::get_tag() + "NULL";
    else if (i_ == std::numeric_limits<int>::min())
      return Tag::get_tag() + "INV";
    else {
      std::ostringstream oss;
      oss << Tag::get_tag() << i_;
      return oss.str();
    }
  }

 public:
  typedef TagT Tag;
#if !defined(RMF_DOXGYGEN) && !defined(SWIG)
  class SpecialTag {};
  ID(int i, SpecialTag) : i_(i) {}
  int get_index_always() const {
    /*RMF_USAGE_CHECK(i_ != std::numeric_limits<int>::min(),
      "get_index called on uninitialized ID");*/
    return i_;
  }
#endif
// filling a std::vector of IDs with ints doesn't work in some VC++ versions
// making this implicit fixes it, but I don't want it to be so in general.
#ifndef _MSC_VER
  explicit
#endif
      ID(unsigned int i)
      : i_(i) {
    RMF_USAGE_CHECK(static_cast<int>(i_) >= 0,
                    Tag::get_tag() + ": Bad index passed on initialize");
  }
  ID() : i_(std::numeric_limits<int>::min()) {}
  unsigned int get_index() const {
    /*RMF_USAGE_CHECK(i_ != std::numeric_limits<int>::min(),
                    "get_index called on uninitialized ID");
                    RMF_USAGE_CHECK(i_ >= 0, "get_index called on special
       ID.");*/
    return i_;
  }
  RMF_COMPARISONS(ID);
  RMF_HASHABLE(ID, return i_);
  RMF_SHOWABLE(ID, get_string());
#if !defined(RMF_DOXYGEN) && !defined(SWIG)
  ID operator++() {
    ++i_;
    return *this;
  }
  // Needed to use std::distance or boost::distance
  int operator-(const ID& other) const {
    return i_ - other.i_;
  }
#endif
};

//! Produce hash values for boost hash tables.
template <class TagT>
inline std::size_t hash_value(const ID<TagT>& t) {
  return t.__hash__();
}

/** Identify a node within a file. */
typedef ID<NodeTag> NodeID;
/** Identify a frame within a file. */
typedef ID<FrameTag> FrameID;
/** Identify a category within a file. */
typedef ID<CategoryTag> Category;

/** List of node ids. */
typedef std::vector<NodeID> NodeIDs;
/** List of frame ids. */
typedef std::vector<FrameID> FrameIDs;
/** List of categories. */
typedef std::vector<Category> Categories;

#if !defined(SWIG) && !defined(RMF_DOXYGEN)
template <class Traits>
inline std::ostream& operator<<(std::ostream& out, ID<Traits> null) {
  null.show(out);
  return out;
}
#endif

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_ID_H */
