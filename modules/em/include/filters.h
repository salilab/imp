/**
 *  \file filters.h
 *  \brief Classes to deal with filters
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPEM_FILTERS_H
#define IMPEM_FILTERS_H

#include "config.h"
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
    _threshold = threshold;
    _mode = mode;
    _value = value;
  }

  void set_parameters(T threshold,int mode) {
    _threshold = threshold;
    _mode = mode;
    _value = threshold;
  }

  //! Function to apply the filter
  void apply(algebra::MultiArray<T,D>& m) {
    std::vector<int> idx(D);
    while (algebra::internal::roll_inds(idx, m.shape(),m.index_bases())) {
      switch (_mode) {
      case 0: // Filter all the values BELOW the threshold
        if(m(idx)<_threshold) {m(idx) = _value;}
        break;
      case 1: // Filter all the values ABOVE the threshold
        if(m(idx)>_threshold) {m(idx) = _value;}
        break;
      }
    }
  }
protected:
  T _threshold;
  int _mode;
  T _value;
};


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

  void apply(DensityMap& m);

protected:
  float _threshold;
  int _mode;
  float _value;
};

IMPEM_END_NAMESPACE

#endif  /* IMPEM_FILTERS_H */
