/**
 *  \file KMData.cpp   \brief Holds data points to cluster using k-means
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include <IMP/domino/KMData.h>
#include <IMP/domino/random_generator.h>
#include <iostream>
IMPDOMINO_BEGIN_NAMESPACE
KMData::KMData(int d, int n) : dim_(d) {
  points_ = allocate_points(n,dim_);
}

KMData::~KMData() {
  deallocate_points(points_);
}
KMPoint KMData::sample_center(){
  return *((*points_)[random_int(points_->size())]);
}

void KMData::sample_centers( KMPointArray *sample,int k,
      bool allow_duplicate) {
  std::cout<<"in KMdata::sample_centers"<<std::endl;
   if (!allow_duplicate)
     IMP_assert(((unsigned int)k)<= points_->size(),
                "not enough points to sample from");
   for (int i = 0; i < k; i++) {
     std::cout<<"in KMdata::sampleCtrs i:"<< i << std::endl;
     int ri = random_int(points_->size());
     std::vector<int> sampled_ind;
     if (!allow_duplicate) {
       bool dup_found;
       do {
         dup_found = false;
         // search for duplicates
         for (int j = 0; j < i; j++) {
           if (sampled_ind[j] == ri) {
             dup_found = true;
             ri = random_int(points_->size());
             break;
           }
         }
      } while (dup_found);
     }
     std::cout<<"in KMdata::sample_centers 2"<<std::endl;
     sampled_ind.push_back(ri);
     KMPoint *p = new KMPoint();
     copy_point((*points_)[ri],p);
     sample->push_back(p);
   }
}

void copy_points(KMPointArray *from, KMPointArray *to) {
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
IMPDOMINO_END_NAMESPACE
