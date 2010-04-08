/**
 *  \file Histogram.cpp  \brief simple histogram class
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#include <IMP/statistics/Histogram.h>
#include <numeric>
IMPSTATISTICS_BEGIN_NAMESPACE


Histogram::Histogram(double start, double end,
                     unsigned int num_bins) {
  start_=start;
  end_=end;
  interval_size_ = (end-start)/num_bins;
  freq_.insert(freq_.end(),num_bins,0);
}
void Histogram::add(double x) {
  IMP_INTERNAL_CHECK(!(x<start_ || x>end_),"the value being added to the "<<
                     "histogram "<<x<<" is out of range"<<std::endl);
  const unsigned int i =
    (static_cast<unsigned int>((x-start_)/interval_size_));
  freq_[i]++;
}
unsigned int Histogram::get_total_count() const {
  return std::accumulate(freq_.begin(),freq_.end(),0);
}
void Histogram::show(std::ostream& out) const {
  out<<"histogram for range:["<<start_<<","<<end_<<"] with step size:"
     <<interval_size_<<std::endl;
  for(unsigned int i=0;i<freq_.size();i++) {
    out<<start_+interval_size_*i<<" | "<<freq_[i]<<std::endl;
  }
}
double Histogram::get_top(double percentage) const {
  IMP_INTERNAL_CHECK(!((percentage<0.)||(percentage>1.)),
                     "The input number is not a percentage\n");
  float stop_count = get_total_count()*percentage;
  int partial_count=0;
  for(unsigned int i=0;i<freq_.size();i++) {
    partial_count += freq_[i];
    if (partial_count > stop_count) {
      return start_+i*interval_size_;
    }
  }
  return end_;
}
IMPSTATISTICS_END_NAMESPACE
