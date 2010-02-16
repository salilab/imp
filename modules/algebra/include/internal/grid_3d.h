/**
 *  \file output_helpers.h
 *  \brief manipulation of text, and Interconversion between text and numbers
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_GRID_3D_H
#define IMPALGEBRA_INTERNAL_GRID_3D_H

#include "../config.h"
#include "../Vector3D.h"

IMPALGEBRA_BEGIN_NAMESPACE
template <class V>
class Grid3D;
IMPALGEBRA_END_NAMESPACE

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

// Represent an index into an infinite grid
/* The index entries can be positive or negative and do not need to correspond
   directly to cells in the grid. They get mapped on to actual grid cells
   by various functions.
 */
class VirtualGridIndex
{
  typedef VirtualGridIndex This;
  int d_[3];
public:
  //! Create a grid cell from three arbitrary indexes
  VirtualGridIndex(int x, int y, int z) {
    d_[0]=x;
    d_[1]=y;
    d_[2]=z;
  }
  VirtualGridIndex() {
    d_[0]=d_[1]=d_[2]=std::numeric_limits<int>::max();
  }
  IMP_COMPARISONS_3(d_[0], d_[1], d_[2]);
  //! Get the ith component (i=0,1,2)
  int operator[](unsigned int i) const {
    IMP_INTERNAL_CHECK(i <3, "Bad i");
    return d_[i];
  }
  void show(std::ostream &out=std::cout) const {
    out << "Cell(" << d_[0] << ", " << d_[1] << ", " << d_[2] << ")";
  }
  bool strictly_larger_than(const VirtualGridIndex &o) const {
    return d_[0] > o.d_[0] && d_[1] > o.d_[1] && d_[2] > o.d_[2];
  }
};

IMP_OUTPUT_OPERATOR(VirtualGridIndex);

template <class GI>
class GridIndexIterator
{
  template <class V>
  friend class Grid3D;
  VirtualGridIndex lb_;
  VirtualGridIndex ub_;
  GI cur_;
  typedef GridIndexIterator This;
  GridIndexIterator(VirtualGridIndex lb,
                    VirtualGridIndex ub): lb_(lb),
                                          ub_(ub), cur_(lb[0], lb[1], lb[2]) {
    IMP_INTERNAL_CHECK(ub_.strictly_larger_than(lb_),
               "Invalid range in GridIndexIterator");
  }
public:
  typedef const GI& reference_type;
  typedef const GI* pointer_type;
  typedef GI value_type;
  typedef std::forward_iterator_tag iterator_category;
  typedef unsigned int difference_type;

  GridIndexIterator(){}

  IMP_COMPARISONS_1(cur_);

  This operator++() {
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
      cur_= GI();
    } else {
      GI nc= GI(r[0], r[1], r[2]);
      IMP_INTERNAL_CHECK(nc > cur_, "Nonfunctional increment");
      IMP_INTERNAL_CHECK(nc > lb_, "Problems advancing");
      IMP_INTERNAL_CHECK(ub_.strictly_larger_than(nc), "Problems advancing");
      cur_= nc;
    }
    return *this;
  }
  This operator++(int) {
    IMP_INTERNAL_CHECK(*this != GI(), "Incrementing invalid iterator");
    This o= *this;
    operator++;
    return o;
  }
  reference_type operator*() const {
    IMP_INTERNAL_CHECK(*this != GridIndexIterator(),
                       "Dereferencing invalid iterator");
    return cur_;
  }
  pointer_type operator->() const {
    IMP_INTERNAL_CHECK(*this != GridIndexIterator(),
                       "Dereferencing invalid iterator");
    return &cur_;
  }
};

//! Represent a real cell in a grid
/* These indexes represent an actual cell in the grid with no mapping.
   They can only be constructed by the grid.
 */
class GridIndex: public VirtualGridIndex
{
public:
  GridIndex(): VirtualGridIndex() {
  }
private:
  template <class V>
  friend class Grid3D;
  template <class G>
  friend class GridIndexIterator;
  GridIndex(int x, int y, int z): VirtualGridIndex(x,y,z) {
    IMP_USAGE_CHECK(x>=0 && y>=0 && z>=0, "Bad indexes in grid index",
              IndexException);
  }
};


IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif  /* IMPALGEBRA_INTERNAL_GRID_3D_H */
