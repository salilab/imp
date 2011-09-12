/**
 * \file HistogramD.h \brief Holds a histogram
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_HISTOGRAM_D_H
#define IMPSTATISTICS_HISTOGRAM_D_H
#include "statistics_config.h"
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/algebra/GridD.h>
#include <IMP/algebra/Grid3D.h>
#include <IMP/base_types.h>
#include <vector>
IMPSTATISTICS_BEGIN_NAMESPACE
//! Histogram
/** Keeps a set of values within a range
    the range is arranged into some number of bins
    specified during construction. */
template <int D>
class HistogramD {
  typedef typename
  algebra::grids::GridD<D,
                        typename algebra::grids::DenseGridStorageD<D,
                                                          int>,
                        int > Grid;
  Grid grid_;
  int count_;
  public:
  HistogramD(): count_(-1){}
  HistogramD(double voxel_size,
             const algebra::BoundingBoxD<D> &bb
             ): grid_(voxel_size, bb, 0),
                                 count_(0){}
   //! Increase the count for the bin that holds a
   // value that is in range for this histogram.
  void add(const algebra::VectorD<D>& x) {
    IMP_USAGE_CHECK(count_ >-1, "Using uninitialized histogram");
    typename Grid::ExtendedIndex ei=grid_.get_nearest_extended_index(x);
    if (grid_.get_has_index(ei)) {
      ++grid_[grid_.get_index(ei)];
    }
    ++count_;
  }
  //! Get the sum of all counts in the histogram.
  unsigned int get_total_count() const {
    return count_;
  }
  typedef typename
  algebra::grids::GridD<D,
                        typename algebra::grids::DenseGridStorageD<D,
                                                          float>,
                        float > FrequencyGrid;
  FrequencyGrid get_frequency_grid() const {
    FrequencyGrid ret(grid_.get_unit_cell()[0],
                      algebra::get_bounding_box(grid_), 0);
    for (typename Grid::AllIndexIterator it= grid_.all_indexes_begin();
         it != grid_.all_indexes_end(); ++it) {
      ret[*it]= static_cast<double>(grid_[*it])/count_;
    }
    return ret;
  }
  algebra::VectorD<D> get_mean() const {
    algebra::VectorD<D> ret;
    std::fill(ret.coordinates_begin(), ret.coordinates_end(), 0.0);
    for (typename Grid::AllIndexIterator it= grid_.all_indexes_begin();
         it != grid_.all_indexes_end(); ++it) {
      ret+= grid_.get_center(*it)*grid_[*it];
    }
    ret/=count_;
    return ret;
  }
  unsigned int get_dimension() const {
    return grid_.get_dimension();
  }
  algebra::VectorD<D>
  get_standard_deviation(const algebra::VectorD<D> &mean) const {
    algebra::VectorD<D> ret;
    std::fill(ret.coordinates_begin(), ret.coordinates_end(), 0.0);
    for (typename Grid::AllIndexIterator it= grid_.all_indexes_begin();
         it != grid_.all_indexes_end(); ++it) {
      algebra::VectorD<D> diff= grid_.get_center(*it)-mean;
      algebra::VectorD<D> diff2;
      for (unsigned int i=0; i<get_dimension(); ++i) {
        diff2[i]= square(diff[i]);
      }
      ret+= grid_[*it]*diff2;
    }
    ret/=count_;
    for (unsigned int i=0; i< get_dimension(); ++i) {
      ret[i] = std::sqrt(ret[i]);
    }
    return ret;
  }

  IMP_SHOWABLE_INLINE(HistogramD,out << "count: " << count_);
};

IMP_OUTPUT_OPERATOR_D(HistogramD);
typedef HistogramD<1> Histogram1D;
IMP_VALUES(Histogram1D, Histogram1Ds);
typedef HistogramD<2> Histogram2D;
IMP_VALUES(Histogram2D, Histogram2Ds);
typedef HistogramD<3> Histogram3D;
IMP_VALUES(Histogram3D, Histogram3Ds);
typedef HistogramD<4> Histogram4D;
IMP_VALUES(Histogram4D, Histogram4Ds);
typedef HistogramD<5> Histogram5D;
IMP_VALUES(Histogram5D, Histogram5Ds);
typedef HistogramD<6> Histogram6D;
IMP_VALUES(Histogram6D, Histogram6Ds);
typedef HistogramD<-1> HistogramKD;
IMP_VALUES(HistogramKD, HistogramKDs);

IMPSTATISTICS_END_NAMESPACE
#endif  /* IMPSTATISTICS_HISTOGRAM_D_H */
