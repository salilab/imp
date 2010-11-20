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
Triangle3D::Triangle3D(
      const VectorD<3> &p1,const VectorD<3> &p2,const VectorD<3> &p3) {
  p_[0]=p1;
  p_[1]=p2;
  p_[2]=p3;
  //check that the three points are not on the same line
  algebra::Vector3D v1=p_[0]-p_[1];
  algebra::Vector3D v2=p_[0]-p_[2];
  IMP_USAGE_CHECK(!algebra::get_are_parallel(v1,v2),
                  "Three co-linear points can not form a triangle\n");
}
algebra::Vector3D Triangle3D::get_edge_lenghts()const {
  Floats lens(3);
  lens[0]=algebra::get_distance(p_[0],p_[1]);
  lens[1]=algebra::get_distance(p_[0],p_[2]);
  lens[2]=algebra::get_distance(p_[1],p_[2]);
  return algebra::Vector3D(lens[0],lens[1],lens[2]);
}

Triangle3D get_bounding_triangle(const Vector3Ds &points) {
   float max_dist = 0;
   float dist;
   Vector3Ds triple(3);
   for (unsigned int i = 0 ; i < points.size() ; i++) {
     for (unsigned int j = 0 ; j < points.size() ;j++) {
       dist = algebra::get_squared_distance(points[i],points[j]);
       if (dist > max_dist) {
         max_dist = dist;
         triple[0] = points[i];
         triple[1] = points[j];
       }
     }
   }

    float maxDist = 0;
    algebra::Segment3D seg(triple[0],triple[1]);

    for (unsigned int i = 0 ; i < points.size();i++) {
       dist = algebra::get_distance(seg,points[i]);
      if (dist > max_dist) {
        max_dist = dist;
        triple[2] =points[i];
      }
    }
    return Triangle3D(triple[0],triple[1],triple[2]);
}

IMPALGEBRA_END_NAMESPACE
