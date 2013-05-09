/**
 *  \file grid_range_D.h   \brief Simple D vector class.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_INTERNAL_GRID_RANGE_D_H
#define IMPALGEBRA_INTERNAL_GRID_RANGE_D_H

#include "../VectorD.h"
#include <IMP/base/RefCounted.h>
#include <IMP/base/Pointer.h>
#include <boost/range.hpp>

IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

template <int D> struct GridRangeData : public base::RefCounted {
  const BoundingBoxD<D> bb;
  double step;
  GridRangeData(const BoundingBoxD<D> &ibb, double stp) : bb(ibb), step(stp) {}
};

template <int D>
    std::ostream &operator<<(std::ostream &out, const GridRangeData<D> &d) {
  out << d.min << " " << d.max << " " << d.step << std::endl;
  return out;
}

template <int D> class GridIteratorD {
  base::Pointer<GridRangeData<D> > data_;
  VectorD<D> cur_;

 public:
  typedef const VectorD<D> value_type;
  typedef unsigned int difference_type;
  typedef const VectorD<D> &reference;
  typedef const VectorD<D> *pointer;
  typedef std::forward_iterator_tag iterator_category;

  GridIteratorD(base::Pointer<GridRangeData<D> > d, reference cur)
      : data_(d), cur_(cur) {}
  reference operator*() const { return cur_; }
  pointer operator->() const { return &cur_; }
  const GridIteratorD &operator++() {
    for (unsigned int i = 0; i < data_->bb.get_dimension(); ++i) {
      cur_[i] += data_->step;
      if (cur_[i] > data_->bb.get_corner(1)[i]) {
        cur_[i] = data_->bb.get_corner(0)[i];
      } else {
        return *this;
      }
    }
    cur_ = data_->bb.get_corner(1);
    return *this;
  }

  GridIteratorD operator++(int) {
    GridIteratorD ret = *this;
    this->operator++();
    return ret;
  }

  bool operator==(const GridIteratorD &o) const {
    return compare(cur_, o.cur_) == 0;
  }
  bool operator!=(const GridIteratorD &o) const {
    return compare(cur_, o.cur_) != 0;
  }
  bool operator<(const GridIteratorD &o) const {
    return compare(cur_, o.cur_) < 0;
  }
  bool operator>(const GridIteratorD &o) const {
    return compare(cur_, o.cur_) > 0;
  }
};

//! A Boost.Range over the vertices of a grid
/** This range range the VectorD objects whose coordinates
    are
    \f$ \mathbf{v}= \mathbf{\min}+ \sum_i q_i \hat{\mathbf{x}}_i \f$
    such that
    \f$ \mathbf{v}\left[i\right] < \mathbf{\max}\left[i\right]\f$
    where \f$\hat{\mathbf{x}_i}\f$ is the ith basis vector, \f$q_i\f$ is
    an arbitrary integer and \f$i\f$ ranges over the dimension of the vector.
*/
template <int D> class GridRangeD {
 private:
  IMP::base::Pointer<GridRangeData<D> > data_;

 public:
  typedef GridIteratorD<D> iterator;
  typedef iterator const_iterator;
  //! Create a new range on the volume [min, max] with step step
  GridRangeD(const BoundingBoxD<D> &bb, double step)
      : data_(new GridRangeData<D>(bb, step)) {}
#if !defined(SWIG)
  const_iterator begin() const {
    return iterator(data_, data_->bb.get_corner(0));
  }
  const_iterator end() const {
    return iterator(data_, data_->bb.get_corner(1));
  }
#endif
  base::Vector<VectorD<D> > get() const {
    return base::Vector<VectorD<D> >(begin(), end());
  }
};

IMPALGEBRA_END_INTERNAL_NAMESPACE

#endif /* IMPALGEBRA_INTERNAL_GRID_RANGE_D_H */
