/**
 * \file PCAAligner.cpp \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/em/PCAAligner.h>
#include <IMP/algebra/Transformation3D.h>

IMPEM_BEGIN_NAMESPACE

algebra::Transformation3Ds PCAAligner::align(
                                  const algebra::Vector3Ds& points) const{
  algebra::Transformation3Ds transforms;
  IMP::algebra::PrincipalComponentAnalysis points_pc_ =
    IMP::algebra::get_principal_components(points);

  IMP::algebra::Vector3D x = map_pc_.get_principal_component(0);
  IMP::algebra::Vector3D y = map_pc_.get_principal_component(1);
  IMP::algebra::Rotation3D r = get_rotation_matrix(x, y);
  IMP::algebra::Transformation3D map_trans(
                IMP::algebra::Transformation3D(r, -(r*map_pc_.get_centroid())));
  IMP::algebra::Transformation3D inverse_map_trans = map_trans.get_inverse();

  // align the principal components by enumeration 6 xy choices
  for(int i=0; i<3; i++) {
    for(int j=0; j<3; j++) {
      if(i != j) {
        IMP::algebra::Vector3D xi = points_pc_.get_principal_component(i);
        IMP::algebra::Vector3D yi = points_pc_.get_principal_component(j);
        IMP::algebra::Rotation3Ds rotations(4);
        rotations[0] = get_rotation_matrix(xi, yi);
        rotations[1] = get_rotation_matrix(xi, -yi);
        rotations[2] = get_rotation_matrix(-xi, yi);
        rotations[3] = get_rotation_matrix(-xi, -yi);
        for(unsigned int k=0; k<rotations.size(); k++) {
          IMP::algebra::Transformation3D points_trans =
            IMP::algebra::Transformation3D(rotations[k],
                                 -(rotations[k]*points_pc_.get_centroid()));
          IMP::algebra::Transformation3D ps2dens =
            inverse_map_trans * points_trans;
          transforms.push_back(ps2dens);
        }
      }
    }
  }
  return transforms;
}

IMPEM_END_NAMESPACE
