/**
 *  \file KMData.cpp   \brief Holds data points to cluster using k-means
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/internal/KMData.h>
#include <IMP/statistics/internal/random_generator.h>
#include <IMP/log.h>
#include <iostream>
IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
KMData::KMData(int d, int n) : dim_(d) {
  points_ = allocate_points(n,dim_);
}

KMData::~KMData() {
  deallocate_points(points_);
}
KMPoint KMData::sample_center(double offset){
  KMPoint *sampled_p = (*points_)[internal::random_int(points_->size())];
  if (offset == 0.) {
    return *sampled_p;
  }
  KMPoint p;
  for(int i=0;i<dim_;i++) {
    p.push_back((*sampled_p)[i]+internal::random_uniform(-1.,1)*offset);
  }
  return p;
}

void KMData::sample_centers( KMPointArray *sample,int k,
  double offset, bool allow_duplicate) {
    clear_points(sample);
  IMP_LOG_VERBOSE("KMData::sample_centers size: "<<sample->size()<<std::endl);
  if (!allow_duplicate) {
     IMP_INTERNAL_CHECK(((unsigned int)k)<= points_->size(),
                "not enough points to sample from");
  }
   Ints sampled_ind;
   for (int i = 0; i < k; i++) {
     int ri = internal::random_int(points_->size());
     if (!allow_duplicate) {
       bool dup_found;
       do {
         dup_found = false;
         // search for duplicates
         for (int j = 0; j < i; j++) {
           if (sampled_ind[j] == ri) {
             dup_found = true;
             ri = internal::random_int(points_->size());
             break;
           }
         }
      } while (dup_found);
     }
     sampled_ind.push_back(ri);
     KMPoint *p = new KMPoint();
     KMPoint *copied_p = (*points_)[ri];
     for(int j=0;j<dim_;j++) {
       p->push_back((*copied_p)[j]+internal::random_uniform(-1.,1)*offset);
     }
     sample->push_back(p);
   }
   IMP_LOG_VERBOSE("KMData::sampled centers  : " <<std::endl);
   for (int i = 0; i < k; i++) {
     IMP_LOG_WRITE(VERBOSE,print_point(*((*sample)[i])));
   }
   IMP_LOG_VERBOSE("\nKMData::sample_centers end size : "
           << sample->size()<<std::endl);
}

void copy_points(KMPointArray *from, KMPointArray *to) {
  if (from == nullptr) {
    return;
  }
  for (unsigned int i=0;i<from->size();i++) {
    KMPoint *p = new KMPoint();
    copy_point((*from)[i],p);
    to->push_back(p);
  }
}
void print_point(const KMPoint &p,std::ostream &out)
{
  out << "[ ";
  for (unsigned int i = 0; i < p.size(); i++) {
    out << std::setw(8) << p[i] <<" ";
  }
  out << " ]";
}

void print_points(const std::string &title,const KMPointArray &ps,
 std::ostream &out)
{
    out << "  (" << title << ":\n";
    for (unsigned int i = 0; i < ps.size(); i++) {
      out << "    " << i << "\t";
      print_point(*(ps[i]), out);
      out << "\n";
    }
    out << "  )" << std::endl;
}
IMPSTATISTICS_END_INTERNAL_NAMESPACE
