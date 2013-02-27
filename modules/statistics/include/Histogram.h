/**
 * \file IMP/statistics/Histogram.h \brief Holds a histogram
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_HISTOGRAM_H
#define IMPSTATISTICS_HISTOGRAM_H
#include <IMP/statistics/statistics_config.h>
#include <IMP/base_types.h>
#include <vector>
IMPSTATISTICS_BEGIN_NAMESPACE
#if IMP_HAS_DEPRECATED
//! Histogram
/** Keeps a set of values within a range
    the range is arranged into some number of bins
    specified during construction.
    \deprecated{Use HistogramD instead.}
*/
class IMPSTATISTICSEXPORT Histogram {
  public:
  //! Constructor
  /**
  \param[in] start the start value of the range
  \param[in] end the end value of the range
  \param[in] num_bins the number of bins [start,end] is divided to
   */
  Histogram(double start, double end,
            unsigned int num_bins);
   //! Increase the count for the bin that holds a
   // value that is in range for this histogram.
  void add(double x);
  //! Get the sum of all counts in the histogram.
  unsigned int get_total_count() const;
  //! Get the lowest value for which X% of the histogram data is lower from
  /**
     \param percentage the percenrage (between 0 to 1)
     The name of this function will change to be more accurate.
     \unstable{get_top}
   */
  double get_top(double percentage)const;
  IMP_SHOWABLE(Histogram);
 private:
  double start_,end_,interval_size_;
  Ints freq_;
};

IMP_VALUES(Histogram, Histograms);
#endif

IMPSTATISTICS_END_NAMESPACE
#endif  /* IMPSTATISTICS_HISTOGRAM_H */
