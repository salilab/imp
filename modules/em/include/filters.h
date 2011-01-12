/**
 *  \file filters.h
 *  \brief Classes to deal with filters
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPEM_FILTERS_H
#define IMPEM_FILTERS_H

#include "em_config.h"
#include "DensityMap.h"
#include "IMP/algebra/MultiArray.h"

IMPEM_BEGIN_NAMESPACE

//! Class defining a filter that filters by value directly, just getting
//! the values above or below a given threshold
/**
  \note It can be applied to 2D, 3D or arbitrary dimension
**/
template<typename T,int D>
class FilterByThreshold
{
public:
  FilterByThreshold() {}

  FilterByThreshold(double threshold,int mode,double value) {
    set_parameters(threshold,mode,value);
  }

  //! Function to set the paramters of of the filter
  /**
    \param[in] threshold
    \param[in] mode: 0 - Filter all the values BELOW the threshold
                     1 - Filter all the values ABOVE the threshold
    \param[in] value Value to given to the filtered entries. By default
     the given value is the threshold
  **/
  void set_parameters(T threshold,int mode,T value) {
    threshold_ = threshold;
    mode_ = mode;
    value_ = value;
  }

  void set_parameters(T threshold,int mode) {
    threshold_ = threshold;
    mode_ = mode;
    value_ = threshold;
  }

  //! Function to apply the filter
  void apply(algebra::MultiArray<T,D>& m) {
    std::vector<int> idx(D);
    while (algebra::internal::roll_inds(idx, m.shape(),m.index_bases())) {
      switch (mode_) {
      case 0: // Filter all the values BELOW the threshold
        if(m(idx)<threshold_) {m(idx) = value_;}
        break;
      case 1: // Filter all the values ABOVE the threshold
        if(m(idx)>threshold_) {m(idx) = value_;}
        break;
      }
    }
  }

  IMP_SHOWABLE_INLINE(FilterByThreshold,
                      {out << "threshold: " << threshold_ << "; mode: "
                           << mode_ << "; value: " << value_;});

protected:
  T threshold_;
  int mode_;
  T value_;
};

typedef FilterByThreshold<double,2> FilterByThreshold2D;
typedef std::vector<FilterByThreshold<double,2> > FilterByThreshold2Ds;
typedef FilterByThreshold<double,3> FilterByThreshold3D;
typedef std::vector<FilterByThreshold<double,3> > FilterByThreshold3Ds;

//! Class to filter by threshold (only DensityMap)
class IMPEMEXPORT MapFilterByThreshold
{
public:

  MapFilterByThreshold() {}

  MapFilterByThreshold(double threshold,int mode,double value) {
    set_parameters(threshold,mode,value);
  }

  //! Function to set the paramters of of the filter
  /**
    \param[in] threshold
    \param[in] mode: 0 - Filter all the values BELOW the threshold
                     1 - Filter all the values ABOVE the threshold
    \param[in] value Value to given to the filtered entries. By default
     the given value is the threshold
  **/
  void set_parameters(float threshold,int mode,float value);

  void set_parameters(float threshold,int mode);

  void apply(DensityMap* m);

  IMP_SHOWABLE_INLINE(MapFilterByThreshold,
                      {out << "threshold: " << threshold_ << "; mode: "
                           << mode_ << "; value: " << value_;});

protected:
  float threshold_;
  int mode_;
  float value_;
};
IMP_VALUES(MapFilterByThreshold, MapFilterByThresholds);

IMPEM_END_NAMESPACE

#endif  /* IMPEM_FILTERS_H */
