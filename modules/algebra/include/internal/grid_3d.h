/**
 *  \file output_helpers.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_GRID_3D_H
#define IMPALGEBRA_INTERNAL_GRID_3D_H

#include "../algebra_config.h"


IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE


template <class BI, class IsVI>
class GridIndexIterator
{
  typedef typename IsVI::ReturnType VI;
  void advance() {
    IMP_INTERNAL_CHECK(*this != GridIndexIterator(),
                       "Incrementing invalid iterator");
    IMP_INTERNAL_CHECK(cur_ >= lb_, "cur out of range");
    IMP_INTERNAL_CHECK(cur_ < ub_, "cur out of range");
    int r[3];
    unsigned int carry=1;
    for (int i=2; i>=0; --i) {
      r[i]= cur_[i]+carry;
      if ( r[i] == ub_[i]) {
        r[i]= lb_[i];
        carry=1;
      } else {
        carry=0;
      }
    }
    if (carry==1) {
      cur_= BI();
    } else {
      BI nc= BI(r[0], r[1], r[2]);
      IMP_INTERNAL_CHECK(nc > cur_, "Nonfunctional increment");
      IMP_INTERNAL_CHECK(nc > lb_, "Problems advancing");
      IMP_INTERNAL_CHECK(ub_.strictly_larger_than(nc), "Problems advancing");
      cur_= nc;
    }
  }
  void check_and_advance() {
    while (cur_ != BI() && !isvi_.get_is_good(cur_)) {
      advance();
    }
  }
public:
  BI lb_;
  BI ub_;
  BI cur_;
  IsVI isvi_;
  typedef GridIndexIterator This;
  GridIndexIterator(BI lb,
                    BI ub,
                    IsVI isvi=IsVI()): lb_(lb),
                                ub_(ub), cur_(lb),
                                isvi_(isvi) {
    IMP_INTERNAL_CHECK(ub_.strictly_larger_than(lb_),
               "Invalid range in GridIndexIterator");
    check_and_advance();
  }
  typedef const VI reference;
  typedef const VI* pointer;
  typedef VI value_type;
  typedef std::forward_iterator_tag iterator_category;
  typedef unsigned int difference_type;

  GridIndexIterator(){}

  IMP_COMPARISONS_1(cur_);

  This operator++() {
    do {
      advance();
    } while (cur_ != BI() && !isvi_.get_is_good(cur_));
    return *this;
  }
  This operator++(int) {
    IMP_INTERNAL_CHECK(*this != VI(), "Incrementing invalid iterator");
    This o= *this;
    operator++;
    return o;
  }
  reference operator*() const {
    IMP_INTERNAL_CHECK(*this != GridIndexIterator(),
                       "Dereferencing invalid iterator");
    return isvi_.get_return(cur_);
  }
  pointer operator->() const {
    IMP_INTERNAL_CHECK(*this != GridIndexIterator(),
                       "Dereferencing invalid iterator");
    static VI ret;
    ret= isvi_.get_return(cur_);
    return &ret;
  }
};


inline int snap(unsigned int dim, int v, const int d[]) {
  IMP_INTERNAL_CHECK(dim <3, "Invalid dim");
  if (v < 0) return 0;
  else if (v > d[dim]) {
    return d[dim];
  }
  else return v;
}

template <class EI>
EI snap(const EI &v, const int d[]) {
  return EI(snap(0, v[0], d),
            snap(1, v[1], d),
            snap(2, v[2], d));
}
template <class EI>
std::pair<EI, EI> empty_range() {
  return std::make_pair(EI(0,0,0), EI(0,0,0));
}


template <class EI>
std::pair<EI, EI> intersect(EI l,
                            EI u,
                            const int d[]) {
  EI rlb;
  EI rub;
  for (unsigned int i=0; i< 3; ++i) {
    if (u[i] <= 0) return empty_range<EI>();
    if (l[i] >= d[i])
      return empty_range<EI>();
  }
  return std::make_pair(snap<EI>(l, d), snap<EI>(u, d));
}

template <class E, class R>
struct AllItHelp {
  typedef R ReturnType;
  bool get_is_good(E) const {
    return true;
  }
  R get_return(const E&v) const {
    return R(v[0], v[1], v[2]);
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_GRID_3D_H */
