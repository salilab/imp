/**
 *  \file Histogram.cpp  \brief simple histogram class
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#include <IMP/statistics/Histogram.h>
#include <IMP/base/deprecation_macros.h>
#include <numeric>
#include <iostream>
#include <iomanip>
IMPSTATISTICS_BEGIN_NAMESPACE

#if IMP_HAS_DEPRECATED

Histogram::Histogram(double start, double end,
                     unsigned int num_bins) {
  start_=start;
  end_=end;
  interval_size_ = (end-start)/num_bins;
  freq_.insert(freq_.end(),num_bins,0);
  IMP_DEPRECATED_CLASS(Histogram, HistogramD);
}
void Histogram::add(double x) {
  IMP_INTERNAL_CHECK(!(x<start_ || x>end_),
                     "the value being added to the "<<
                     "histogram "<<x<<" is out of range ["<<start_<<
                     ","<<end_<<"]"<<std::endl);
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
  out.setf(std::ios::fixed, std::ios::floatfield);
  out.precision(2);
  out << "|" << std::setw(13) << std::setfill(' ') << "Bin" << "|";
  out<<std::setw(13)<<std::setfill(' ') << "Num voxels"<< "|";
  out<<std::setw(15)<<std::setfill(' ')<<"Accumolate Freq "<<std::endl;

  int all_num=accumulate(freq_.begin(),freq_.end(),0);
  int counter=0;
  for(unsigned int i=0;i<freq_.size();i++) {
    counter += freq_[i];
    out << "|" << std::setw(13) << std::setfill(' ') <<
      start_+interval_size_*i << "|";
    out<<std::setw(13)<<std::setfill(' ') << freq_[i]<< "|";
    out<<std::setw(15)<<std::setfill(' ')<<1.*counter/all_num<<std::endl;
  }
}
double Histogram::get_top(double percentage) const {
  IMP_INTERNAL_CHECK(!((percentage<0.)||(percentage>1.)),
                     "The input number is not a percentage\n");
  double stop_count = get_total_count()*percentage;
  int partial_count=0;
  for(unsigned int i=0;i<freq_.size();i++) {
    partial_count += freq_[i];
    if (partial_count > stop_count) {
      return start_+i*interval_size_;
    }
  }
  return end_;
}
#endif

IMPSTATISTICS_END_NAMESPACE
