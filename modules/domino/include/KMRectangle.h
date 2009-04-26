/**
 *  \file KMRectangle.h   \brief Orthogonal (axis aligned) rectangle
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPDOMINO_KM_RECTANGLE_H
#define IMPDOMINO_KM_RECTANGLE_H

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <climits>
#include <ctime>
#include <string>
#include <vector>
#include "config.h"
#include "KMData.h"
#include <IMP/base_types.h>
IMPDOMINO_BEGIN_NAMESPACE

//! Returns the squared distances betweentwo points
double km_distance2(const KMPoint &p,const KMPoint &q);
//! Returns true if p and q are the same points.
bool km_is_equal(const KMPoint &p,const KMPoint &q);

//!  Orthogonal (axis aligned) rectangle
class KMRectangle {
public:
  KMRectangle(){}
//! Constructor
/**
/param[in] dim dimension
 */
  KMRectangle(int dim) {
    lo_.clear();hi_.clear();
    lo_.insert(lo_.end(),dim,0);
    hi_.insert(hi_.end(),dim,0);
  }
  KMRectangle(const KMPoint &l,const KMPoint &h):lo_(l),hi_(h){}
  //! Checks if the point is inside the rectangle
  /** Returns true if a point lies inside the (closed) rectangle and false
      otherwise.
  */
  bool is_inside(const KMPoint &p);
  //! Expand by factor x. The center stays in place.
  KMRectangle expand(double x);
  int get_dim(){return lo_.size();}
  KMPoint *get_point(int i) {
    IMP_assert(i==0 || i==1,"wrong index");
    if (i==0) return &lo_;
    return &hi_;
  }
  const KMPoint *get_point(int i) const {
    IMP_assert(i==0 || i==1,"wrong index");
    if (i==0) return &lo_;
    return &hi_;
  }
  //! sample point inside the rectangle uniformly
  KMPoint sample();
  double max_length() const {
    double max_length = hi_[0]-lo_[0];
    for (unsigned int i=1;i<lo_.size(); i++) {
      double length = hi_[i]-lo_[i];
      if (length  > max_length) {
        max_length = length;
      }
    }
    return max_length;
  }
  KMPoint find_closest_vertex(const KMPoint &p){
    KMPoint closest_vertex;
    for(int d=0;d<get_dim();d++) {
      if (p[d]>hi_[d]) {
        closest_vertex.push_back(hi_[d]);
      }
      else if (p[d]<lo_[d]) {
        closest_vertex.push_back(lo_[d]);
      }
      else { //p[d] is in the box, find the closest vertex
        if ((p[d]-lo_[d]) < (hi_[d]-p[d])) {
          closest_vertex.push_back(lo_[d]);
        }
        else {
          closest_vertex.push_back(hi_[d]);
        }
      }
    }
    return closest_vertex;
  }
protected:
  KMPoint lo_;
  KMPoint hi_;
};

IMPDOMINO_END_NAMESPACE
#endif /* IMPDOMINO_KM_RECTANGLE_H */
