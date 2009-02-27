/**
 *  \file grid_range_D.h   \brief Simple D vector class.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_RANGE_D_H
#define IMPALGEBRA_GRID_RANGE_D_H

#include "VectorD.h"
#include <IMP/RefCountedObject.h>
#include <IMP/Pointer.h>
#include <boost/range.hpp>

IMPALGEBRA_BEGIN_NAMESPACE

namespace {
  template <unsigned int D>
  struct GridRangeData: public RefCountedObject {
    VectorD<D> min,max;
    double step;
    GridRangeData(const VectorD<D> &mn,
         const VectorD<D> &mx,
         double stp): min(mn), max(mx), step(stp){}
  };

  template <unsigned int D>
  std::ostream &operator<<(std::ostream &out, const GridRangeData<D> &d) {
    out << d.min << " " << d.max << " " << d.step << std::endl;
    return out;
  }
}

template <unsigned int D>
class GridIteratorD
{
  Pointer<GridRangeData<D> > data_;
  VectorD<D> cur_;
public:
  typedef GridIteratorD<D> This;
  typedef const VectorD<D>  value_type;
  typedef unsigned int difference_type;
  typedef const VectorD<D>& reference;
  typedef const VectorD<D>* pointer;
  typedef std::forward_iterator_tag iterator_category;

  GridIteratorD(Pointer<GridRangeData<D> > d, reference cur):
    data_(d), cur_(cur) {
  }
  reference operator*() const {
    return cur_;
  }
  pointer operator->() const {
    return &cur_;
  }
  const This& operator++() {
    for (unsigned int i=0; i< D; ++i) {
      cur_[i]+= data_->step;
      if (cur_[i] > data_->max[i]) {
        cur_[i]= data_->min[i];
      } else {
        return *this;
      }
    }
    cur_= data_->max;
    return *this;
  }

  This operator++(int) {
    This ret= *this;
    this->operator++();
    return ret;
  }

  bool operator==(const This &o) const {
    return compare(cur_, o.cur_) ==0;
  }
  bool operator!=(const This &o) const {
    return compare(cur_, o.cur_) !=0;
  }
  bool operator<(const This &o) const {
    return compare(cur_, o.cur_) <0;
  }
  bool operator>(const This &o) const {
    return compare(cur_, o.cur_) >0;
  }
};

template <unsigned int D>
std::ostream &operator<<(std::ostream &out, const GridIteratorD<D> &v) {
  v.show(out);
  return out;
}

//! A Boost.Range over the vertices of a grid
/** This range range the VectorD objects whose coordinates
    are
    \f$ \mathbf{v}= \mathbf{\min}+ \sum_i q_i \hat{\mathbf{x}}_i \f$
    such that
    \f$ \mathbf{v}\left[i\right] < \mathbf{\max}\left[i\right]\f$
    where \f$\hat{\mathbf{x}_i}\f$ is the ith basis vector, \f$q_i\f$ is
    an arbitrary integer and \f$i\f$ ranges over the dimension of the vector.
*/
template <int D>
class GridRangeD {
private:
  Pointer<GridRangeData<D> > data_;
public:
  typedef GridIteratorD<D> iterator;
  typedef iterator const_iterator;
  //! Create a new range on the volume [min, max] with step step
  GridRangeD(const VectorD<D>& min, const VectorD<D>& max, double step):
    data_(new GridRangeData<D>(min, max, step)){}
  const_iterator begin() const {
    return iterator(data_, data_->min);
  }
  const_iterator end() const {
    return iterator(data_, data_->max);
  }
};



IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_GRID_RANGE_D_H */
