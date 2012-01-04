/**
 *  \file output_helpers.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_GRID_3D_H
#define IMPALGEBRA_INTERNAL_GRID_3D_H

#include "../algebra_config.h"


IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE


template <class A, class B>
inline bool get_is_non_empty(const A &a, const B &b) {
  IMP_USAGE_CHECK(a.get_dimension() == b.get_dimension(),
                  "Dimensions don't match");
  for (unsigned int i=0; i< a.get_dimension(); ++i) {
    if (a[i] >= b[i]) return false;
  }
  return true;
}

// this is an actual compare function unlike std::lexicographical_compare
// which does less
template <class It>
inline int  lexicographical_compare(It b0, It e0, It b1, It e1) {
  IMP_USAGE_CHECK(std::distance(b0, e0)
                  == std::distance(b1, e1), "Lengths don't match");
  while (b0 != e0) {
    if (*b0 < *b1) return -1;
    else if (*b0 > *b1) return 1;
    ++b0;
    ++b1;
  }
  return 0;
}


template <class BI, class IsVI>
class GridIndexIterator
{
  typedef typename IsVI::ReturnType VI;
  void advance() {
    IMP_INTERNAL_CHECK(*this != GridIndexIterator(),
                       "Incrementing invalid iterator");
    IMP_INTERNAL_CHECK(cur_ >= lb_, "cur out of range");
    IMP_INTERNAL_CHECK(cur_ < ub_, "cur out of range");
    for (unsigned int i=0; i< cur_.get_dimension(); ++i) {
      //for (int i=cur_.get_dimension()-1; i>=0; --i) {
      ++cur_.access_data().get_data()[i];
      if ( cur_.access_data().get_data()[i] == ub_[i]) {
        cur_.access_data().get_data()[i]= lb_[i];
      } else {
        IMP_INTERNAL_CHECK(cur_ > lb_, "Problems advancing");
        IMP_INTERNAL_CHECK(get_is_non_empty(cur_, ub_), "Problems advancing");
        return;
      }
    }
    cur_=BI();
    return;
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
    IMP_INTERNAL_CHECK(get_is_non_empty(lb_, ub_),
               "Invalid range in GridIndexIterator");
    while (cur_ != BI() && !isvi_.get_is_good(cur_)) {
      advance();
    }
  }
  typedef const VI reference;
  typedef const VI* pointer;
  typedef VI value_type;
  typedef std::forward_iterator_tag iterator_category;
  typedef unsigned int difference_type;

  GridIndexIterator(){}

  IMP_COMPARISONS_1(GridIndexIterator, cur_);

  This operator++() {
    do {
      advance();
    } while (cur_ != BI() && !isvi_.get_is_good(cur_));
    return *this;
  }
  This operator++(int) {
    IMP_INTERNAL_CHECK(*this != VI(), "Incrementing invalid iterator");
    This o= *this;
    operator++();
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

template <class ED>
inline int snap(unsigned int dim, int v, ED d) {
  IMP_INTERNAL_CHECK(dim <d.get_dimension(), "Invalid dim");
  if (v < 0) return 0;
  else if (v > d[dim]) {
    return d[dim];
  }
  else return v;
}

template <class EI, class ED>
inline EI snap(const EI &v, ED d) {
  Ints is(v.begin(), v.end());
  for (unsigned int i=0; i< is.size(); ++i) {
    is[i]= snap(i, is[i], d);
  }
  return EI(is.begin(), is.end());
}
template <class EI>
inline std::pair<EI, EI> empty_range() {
  return std::make_pair(EI(0,0,0), EI(0,0,0));
}


template <class EI, class ED>
inline std::pair<EI, EI> intersect(EI l,
                                   EI u,
                                   ED d) {
  for (unsigned int i=0; i< l.get_dimension(); ++i) {
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
    R ret(v.begin(), v.end());
    return ret;
  }
};


template <class Functor, class Grid, int D>
struct GridApplier {
  static void apply(const Grid &g,
                    typename Grid::ExtendedIndex &lb,
                    typename Grid::ExtendedIndex &ub,
                    const typename Grid::Vector &corner,
                    const typename Grid::Vector &cell,
                    typename Grid::Index &index,
                    typename Grid::Vector &center,
                    Functor &f) {
    int *data=index.access_data().get_data();
    for (data[D]=lb[D];
         data[D]<ub[D];
         ++data[D]) {
      center[D]=corner[D]+cell[D]*(data[D]+.5);
      GridApplier<Functor, Grid, D-1>::apply(g, lb, ub,
                                             corner, cell, index, center, f);
    }
  }
};

template <class Functor, class Grid>
struct GridApplier<Functor, Grid, 0> {
  static void apply(const Grid &g,
                    typename Grid::ExtendedIndex &lb,
                    typename Grid::ExtendedIndex &ub,
                    const typename Grid::Vector &corner,
                    const typename Grid::Vector &cell,
                    typename Grid::Index &index,
                    typename Grid::Vector &center,
                    Functor &f) {
    const int D=0;
    for (index.access_data().get_data()[D]=lb[0];
         index[0]<ub[D]; ++index.access_data().get_data()[D]) {
      center[D]=corner[D]+cell[D]*(index[D]+.5);
      f(g, index, center);
    }
  }
};

// silly and do it on D-1
template <class Functor, class Grid>
struct GridApplier<Functor, Grid, -2> {
  static void apply_recursive(const Grid &g,
                              typename Grid::ExtendedIndex &lb,
                              typename Grid::ExtendedIndex &ub,
                              const typename Grid::Vector &corner,
                              unsigned int D,
                              const typename Grid::Vector &cell,
                              typename Grid::Index &index,
                              typename Grid::Vector &center,
                              Functor &f) {
    int *data=index.access_data().get_data();
    for (data[D]=lb[D]; data[D]<ub[D]; ++data[D]) {
      center[D]=corner[D]+cell[D]*data[D];
      if (D==0) {
        f(g, index, center);
      } else {
        apply_recursive(g, lb, ub,
                        corner, D-1,
                        cell, index,
                        center, f);
      }
    }
  }
  static void apply(const Grid &g,
                    typename Grid::ExtendedIndex &lb,
                    typename Grid::ExtendedIndex &ub,
                    const typename Grid::Vector &corner,
                    const typename Grid::Vector &cell,
                    typename Grid::Index &index,
                    typename Grid::Vector &center,
                    Functor &f) {
    apply_recursive(g, lb, ub, corner, g.get_dimension()-1,
                    cell, index, center, f);
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_GRID_3D_H */
