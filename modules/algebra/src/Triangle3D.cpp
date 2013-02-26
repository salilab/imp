/**
 *  \file  Triangle3D.cpp
 *  \brief simple implementation of triangles in 3D
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/algebra/Triangle3D.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/base/log_macros.h>
IMPALGEBRA_BEGIN_NAMESPACE

namespace {
//! Return true if the vectors are paralel
bool get_are_parallel(const Vector3D &v1,
                               const Vector3D &v2) {
  double dot_p = v1*v2;
  double det1 = v1.get_magnitude();
  double det2 = v2.get_magnitude();
  return (std::abs(std::abs(dot_p)-std::abs(det1*det2))<0.0001);
}
}

bool
get_are_colinear(
         const Vector3D &p1,const Vector3D &p2,const Vector3D &p3){
  //check that the three points are not on the same line
  algebra::Vector3D v1=p1-p2;
  algebra::Vector3D v2=p1-p3;
  return get_are_parallel(v1,v2);
}

Triangle3D::Triangle3D(
      const Vector3D &p1,const Vector3D &p2,const Vector3D &p3) {
  p_[0]=p1;
  p_[1]=p2;
  p_[2]=p3;
}
Floats Triangle3D::get_edge_lengths()const {
  Floats lens(3);
  lens[0]=algebra::get_distance(p_[0],p_[1]);
  lens[1]=algebra::get_distance(p_[0],p_[2]);
  lens[2]=algebra::get_distance(p_[1],p_[2]);
  return lens;
}

Triangle3D get_largest_triangle(const Vector3Ds &points) {
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
    max_dist=0;
    for (unsigned int i = 0 ; i < points.size();i++) {
      double dist = algebra::get_distance(seg,points[i]);
      if (dist > max_dist) {
        max_dist = dist;
        triple[2] =points[i];
      }
    }
    return Triangle3D(triple[0],triple[1],triple[2]);
}
namespace {
ReferenceFrame3D
   get_reference_frame_of_triagle(
                                  Triangle3D t){
  Vector3D p0 = t.get_point(0);
  Vector3D p1 = t.get_point(1);
  Vector3D p2 = t.get_point(2);
  Vector3D trans = (p0+p1+p2)/3; //the translation
  Vector3D x = trans - p0;
  x/= get_l2_norm(x);
  Vector3D z = get_vector_product(p2-p0,p2-p1);
  z/= get_l2_norm(z);
  Vector3D y = get_vector_product(z,x);
  Rotation3D rot = get_rotation_from_matrix(x[0],x[1],x[2],
                                            y[0],y[1],y[2],
                                            z[0],z[1],z[2]);
  return ReferenceFrame3D(Transformation3D(rot.get_inverse(),trans));
}
}//close namespace

Transformation3D get_transformation_from_first_triangle_to_second(
     Triangle3D first_tri,Triangle3D second_tri) {
  ReferenceFrame3D first_ref =
    get_reference_frame_of_triagle(first_tri);
  ReferenceFrame3D second_ref =
    get_reference_frame_of_triagle(second_tri);
  return get_transformation_from_first_to_second(first_ref,second_ref);
}

IMPALGEBRA_END_NAMESPACE
