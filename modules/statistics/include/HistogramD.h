/**
 * \file IMP/statistics/HistogramD.h \brief Holds a histogram
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_HISTOGRAM_D_H
#define IMPSTATISTICS_HISTOGRAM_D_H
#include <IMP/statistics/statistics_config.h>
#include "internal/histogram.h"
#include <IMP/algebra/GeometricPrimitiveD.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/BoundingBoxD.h>
#include <IMP/algebra/GridD.h>
#include <IMP/algebra/standard_grids.h>
#include <IMP/base_types.h>
#include <limits>
#include <vector>
IMPSTATISTICS_BEGIN_NAMESPACE
/** Dynamically build a histogram embedded in D-dimensional space. */
template <int D, class Grid = algebra::GridD<
                     D, typename algebra::DenseGridStorageD<D, double>, double,
                     typename algebra::DefaultEmbeddingD<D> > >
class HistogramD : public algebra::GeometricPrimitiveD<D> {
 public:
  typedef Grid CountGrid;
  HistogramD() : count_(std::numeric_limits<double>::max()) {}
  HistogramD(double voxel_size, const algebra::BoundingBoxD<D> &bb)
      : grid_(voxel_size, bb, 0), count_(0) {}
  /** Increase the count for the bin that holds a
      value that is in range for this histogram.*/
  void add(const algebra::VectorInputD<D> &x, double weight = 1) {
    IMP_USAGE_CHECK(count_ != std::numeric_limits<double>::max(),
                    "Using uninitialized histogram");
    typename CountGrid::ExtendedIndex ei = grid_.get_nearest_extended_index(x);
    if (grid_.get_has_index(ei)) {
      grid_[grid_.get_index(ei)] += weight;
    }
    count_ += weight;
  }
  //! Get the sum of all counts in the histogram.
  double get_total_count() const { return count_; }
  HistogramD<D> get_frequencies() const {
    CountGrid grid(grid_.get_unit_cell()[0], algebra::get_bounding_box(grid_),
                   0);
    grid_.apply(internal::Frequency<D, Grid>(grid, 1.0 / count_));
    return HistogramD<D>(grid);
  }
  /** Get a histogram that has the pdf value as the value for the bin.*/
  HistogramD<D> get_probability_distribution_function() const {
    CountGrid grid(grid_.get_unit_cell()[0], algebra::get_bounding_box(grid_),
                   0);
    double volume =
        algebra::get_volume(grid_.get_bounding_box(*grid_.all_indexes_begin()));
    grid_.apply(internal::Frequency<D, Grid>(grid, 1.0 / (count_ * volume)));
    return HistogramD<D>(grid);
  }
  const CountGrid &get_counts() const { return grid_; }
  algebra::VectorD<D> get_mean() const {
    algebra::VectorD<D> zeros(grid_.get_bounding_box().get_corner(0));
    std::fill(zeros.coordinates_begin(), zeros.coordinates_end(), 0.0);
    return grid_.apply(internal::Mean<D>(zeros)).mn / count_;
  }
  unsigned int get_dimension() const { return grid_.get_dimension(); }
  algebra::VectorD<D> get_standard_deviation(
      const algebra::VectorInputD<D> &mean) const {
    algebra::VectorD<D> zeros(grid_.get_bounding_box().get_corner(0));
    std::fill(zeros.coordinates_begin(), zeros.coordinates_end(), 0.0);
    algebra::VectorD<D> s2 =
        grid_.apply(internal::Sigma2<D>(mean, zeros)).sigma2;
    s2 /= count_;
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      s2[i] = std::sqrt(s2[i]);
    }
    return s2;
  }
  algebra::BoundingBoxD<D> get_bounding_box() const {
    return IMP::algebra::get_bounding_box(grid_);
  }
  FloatPair get_minimum_and_maximum() const {
    return grid_.apply(internal::MinMax<D>()).minmax;
  }

  IMP_SHOWABLE_INLINE(HistogramD, out << "count: " << count_);

 private:
  HistogramD(const CountGrid &g) : grid_(g), count_(1) {}
  CountGrid grid_;
  double count_;
};

#ifndef IMP_DOXYGEN
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
#endif

/** Return the midpoint of the bin that best approximates the
    specified quantile (passed as a fraction). That is,
passing .5 returns the median. And passing .9*/
IMPSTATISTICSEXPORT double get_quantile(const Histogram1D &h, double fraction);

IMPSTATISTICS_END_NAMESPACE
#endif /* IMPSTATISTICS_HISTOGRAM_D_H */
