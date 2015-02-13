/**
 *  \file IMP/multi_state/stat_helpers.cpp
 *  \brief basic stat functions
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
*/

#include <IMP/multi_state/stat_helpers.h>

#include <cmath>
#include <numeric>

IMPMULTISTATE_BEGIN_NAMESPACE

double get_average(const std::vector<double>& v) {
  return std::accumulate(v.begin(), v.end(), 0.0)/v.size();
}

std::pair<double, double> get_average_and_stdev(const std::vector<double>& v) {
  if(v.size() == 1) return std::make_pair(v[0], 1.0); //??
  double average = 0.0;
  double stdev = 0.0;
  for(unsigned int i=0; i<v.size(); i++) {
    average += v[i];
    stdev += (v[i]*v[i]);
  }
  average /= v.size();
  stdev /= v.size();
  stdev -= (average*average);
  stdev = sqrt(stdev);
  return std::make_pair(average, stdev);
}

IMPMULTISTATE_END_NAMESPACE
