/**
 *  \file  Triangle3D.cpp
 *  \brief simple implementation of triangles in 3D
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */
#include <IMP/algebra/Triangle3D.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/log.h>
IMPALGEBRA_BEGIN_NAMESPACE

//! Return true if the vectors are paralel
double get_are_parallel(const VectorD<3> &v1,
                               const VectorD<3> &v2) {
  float dot_p = v1*v2;
  float det1 = v1.get_magnitude();
  float det2 = v2.get_magnitude();
  return (std::abs(std::abs(dot_p)-std::abs(det1*det2))<0.0001);
}


Triangle3D::Triangle3D(
      const VectorD<3> &p1,const VectorD<3> &p2,const VectorD<3> &p3) {
  p_[0]=p1;
  p_[1]=p2;
  p_[2]=p3;
  //check that the three points are not on the same line
  algebra::Vector3D v1=p_[0]-p_[1];
  algebra::Vector3D v2=p_[0]-p_[2];
  IMP_USAGE_CHECK(!get_are_parallel(v1,v2),
                  "Three co-linear points can not form a triangle\n");
}
Floats Triangle3D::get_edge_lengths()const {
  Floats lens(3);
  lens[0]=algebra::get_distance(p_[0],p_[1]);
  lens[1]=algebra::get_distance(p_[0],p_[2]);
  lens[2]=algebra::get_distance(p_[1],p_[2]);
  return lens;
}

Triangle3D get_bounding_triangle(const Vector3Ds &points) {
   double max_dist = 0;
   Vector3Ds triple(3);
   for (unsigned int i = 0 ; i < points.size() ; i++) {
     for (unsigned int j = 0 ; j < points.size() ;j++) {
       double dist = algebra::get_squared_distance(points[i],points[j]);
       if (dist > max_dist) {
         max_dist = dist;
         triple[0] = points[i];
         triple[1] = points[j];
       }
     }
   }
    algebra::Segment3D seg(triple[0],triple[1]);

    for (unsigned int i = 0 ; i < points.size();i++) {
      double dist = algebra::get_distance(seg,points[i]);
      if (dist > max_dist) {
        max_dist = dist;
        triple[2] =points[i];
      }
    }
    return Triangle3D(triple[0],triple[1],triple[2]);
}

IMPALGEBRA_END_NAMESPACE
