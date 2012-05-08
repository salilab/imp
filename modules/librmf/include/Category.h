/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_KEY_CATEGORY_H
#define IMPLIBRMF_KEY_CATEGORY_H

#include "RMF_config.h"
#include "infrastructure_macros.h"
#include <vector>

namespace RMF {

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
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
template <int Arity>
class CategoryD {
  int i_;
  friend class FileHandle;
  int compare(const CategoryD<Arity> &o) const {
    if (i_ < o.i_) return -1;
    else if (i_ > o.i_) return 1;
    else return 0;
  }
public:
#ifndef IMP_DOXYGEN
  CategoryD(unsigned int i): i_(i){}
#endif
  CategoryD(): i_(-1){}
  unsigned int get_index() const {
    IMP_RMF_USAGE_CHECK(i_ >=0, "Invalid Category used");
    return i_;
  }
  IMP_RMF_HASHABLE(CategoryD, return i_);
  IMP_RMF_COMPARISONS(CategoryD);
  IMP_RMF_SHOWABLE(CategoryD, i_);
};

#ifndef IMP_DOXYGEN
typedef CategoryD<1> Category;
typedef CategoryD<2> PairCategory;
typedef CategoryD<3> TripletCategory;
typedef CategoryD<4> QuadCategory;
typedef vector<CategoryD<1> > Categories;
typedef vector<CategoryD<2> > PairCategories;
typedef vector<CategoryD<3> > TripletCategories;
typedef vector<CategoryD<4> > QuadCategories;
#endif


} /* namespace RMF */

#endif /* IMPLIBRMF_KEY_CATEGORY_H */
