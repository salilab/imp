/**
 *  \file RMF/DataSetIndexD.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_HDF_5DATA_SET_INDEX_D_H
#define RMF_HDF_5DATA_SET_INDEX_D_H

#include <RMF/config.h>
#include "infrastructure_macros.h"
#include <H5public.h>
#include "types.h"
#include <boost/functional/hash.hpp>
#include <stdexcept>

RMF_ENABLE_WARNINGS

namespace RMF {
namespace HDF5 {
/** Store an index into a data set. Typedefs are provides
    for 1,2 and 3 dimension indexes, name like
    DataSetIndex2D.
 */
template <int D>
class DataSetIndexD
{
  hsize_t d_[D];
  int compare(const DataSetIndexD<D> &o) const {
    for (unsigned int i = 0; i < D; ++i) {
      if (d_[i] < o.d_[i]) return -1;
      else if (d_[i] > o.d_[i]) return 1;
    }
    return 0;
  }
public:
  DataSetIndexD(const Ints &o) {
    RMF_USAGE_CHECK(o.size() == D, "Wrong number of values");
    std::copy(o.begin(), o.end(), d_);
  }
  DataSetIndexD() {
    std::fill(d_, d_ + D, -1);
  }
  DataSetIndexD(unsigned int i) {
    RMF_USAGE_CHECK(D == 1, "Constructor does not match dimension.");
    d_[0] = i;
  }
  DataSetIndexD(unsigned int i, unsigned int j) {
    RMF_USAGE_CHECK(D == 2, "Constructor does not match dimension.");
    d_[0] = i;
    if (D > 1) d_[1] = j;
  }
  DataSetIndexD(unsigned int i, unsigned int j, unsigned int k) {
    RMF_USAGE_CHECK(D == 3, "Constructor does not match dimension.");
    d_[0] = i;
    // for clang
    if (D > 1) d_[1] = j;
    if (D > 2) d_[2] = k;
  }
#ifndef SWIG
  hsize_t& operator[](unsigned int i) {
    RMF_INDEX_CHECK(i, D);
    return d_[i];
  }
  hsize_t operator[](unsigned int i) const {
    RMF_INDEX_CHECK(i, D);
    return d_[i];
  }
  typedef const hsize_t * const_iterator;
  const_iterator begin() const {
    return d_;
  }
  const_iterator end() const {
    return d_ + D;
  }
  typedef hsize_t * iterator;
  iterator begin() {
    return d_;
  }
  iterator end() {
    return d_ + D;
  }
  hsize_t* get() const {
    return const_cast<hsize_t*>(d_);
  }
#endif
  int __getitem__(unsigned int i) const {
    if (i >= D) {
      // special for swig/python
      throw std::runtime_error("out of range");
    }
    return operator[](i);
  }

  unsigned int get_dimension() const {
    return D;
  }
  RMF_SHOWABLE(DataSetIndexD, Ints(d_, d_ + D));
  RMF_COMPARISONS(DataSetIndexD);
  RMF_HASHABLE(DataSetIndexD,
               size_t ret = 0;
               for (unsigned int i = 0; i < D; ++i) {
                 boost::hash_combine(ret, static_cast<size_t>(d_[i]));
               }
               return ret; );
};

#ifndef RMF_DOXYGEN
typedef DataSetIndexD<1> DataSetIndex1D;
typedef std::vector<DataSetIndex1D> DataSetIndex1Ds;
typedef DataSetIndexD<2> DataSetIndex2D;
typedef std::vector<DataSetIndex2D> DataSetIndex2Ds;
typedef DataSetIndexD<3> DataSetIndex3D;
typedef std::vector<DataSetIndex3D> DataSetIndex3Ds;
#endif

} /* namespace HDF5 */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_HDF_5DATA_SET_INDEX_D_H */
