/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF__KEY_CATEGORY_H
#define RMF__KEY_CATEGORY_H

#include <RMF/config.h>
#include "infrastructure_macros.h"
#include <vector>

namespace RMF {

#if !defined(SWIG) && !defined(RMF_DOXYGEN)
namespace internal {
  class SharedData;
}
#endif

/** Data associated with nodes or sets of nodes is divided into
    categories. Each category is identified by a CategoryD identifier
    within the scope of a file as returned by FileHandle::get_category().
    It is undefined behavior to pass a category from one file to another
    file. Typedefs (and python types are provided for single nodes, up to
    quads of nodes, named Category, PairCategory, TripletCategory and
    QuadCategory as well as typedefs for lists of them with names like
    Categories.*/
class Category {
  int i_;
  friend class FileHandle;
  int compare(const Category &o) const {
    if (i_ < o.i_) return -1;
    else if (i_ > o.i_) return 1;
    else return 0;
  }
public:
#ifndef RMF_DOXYGEN
  Category(unsigned int i): i_(i){}
#endif
  Category(): i_(-1){}
  unsigned int get_index() const {
    RMF_USAGE_CHECK(i_ >=0, "Invalid Category used");
    return i_;
  }
  RMF_HASHABLE(Category, return i_);
  RMF_COMPARISONS(Category);
  RMF_SHOWABLE(Category, i_);
};

#ifndef RMF_DOXYGEN
typedef vector<Category > Categories;
#endif


} /* namespace RMF */

#endif /* RMF__KEY_CATEGORY_H */
