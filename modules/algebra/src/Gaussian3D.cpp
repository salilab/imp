/**
 *  \file  Gaussian3D.cpp
 *  \brief gaussian shape
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include "IMP/algebra/Gaussian3D.h"

#include <IMP/algebra/eigen3/Eigen/Geometry>

#include <IMP/algebra/eigen3/Eigen/LU>
#include <IMP/algebra/eigen3/Eigen/Eigenvalues>

IMPALGEBRA_BEGIN_NAMESPACE

IMP_Eigen::Matrix3d get_covariance(const Gaussian3D &g) {
  Transformation3D trans =
      g.get_reference_frame().get_transformation_to();
  Vector3D center = trans.get_translation();
  Vector4D iq = trans.get_rotation().get_quaternion();
  IMP_Eigen::Quaterniond q(iq[0], iq[1], iq[2], iq[3]);
  IMP_Eigen::Matrix3d rot = q.toRotationMatrix();
  Vector3D variances = g.get_variances();
  IMP_Eigen::Matrix3d rad =
      IMP_Eigen::Vector3d(variances[0], variances[1],
                          variances[2]).asDiagonal();
  IMP_Eigen::Matrix3d covar = rot * (rad * rot.transpose());
  return covar;
}

Gaussian3D get_gaussian_from_covariance(const IMP_Eigen::Matrix3d &covar,
                                        const Vector3D &center) {
  Rotation3D rot;
  Vector3D radii;

  // get eigen decomposition and sort by eigen vector
  IMP_Eigen::EigenSolver<IMP_Eigen::Matrix3d> es(covar);
  IMP_Eigen::Matrix3d evecs=es.eigenvectors().real();
  IMP_Eigen::Vector3d evals=es.eigenvalues().real();

  // fill in sorted stuff
  for (int i=0;i<3;i++) {
    radii[i]=evals[i];
  }

  // reflect if necessary
  double det = evecs.determinant();
  //std::cout<<"Determinant is "<<det<<std::endl;
  if (det<0){
    IMP_Eigen::Matrix3d reflect=IMP_Eigen::Vector3d(1,1,-1).asDiagonal();
    evecs=evecs*reflect;
  }

  // create rotation matrix and return
  IMP_Eigen::Quaterniond eq(evecs);
  rot=Rotation3D(eq.w(),eq.x(),eq.y(),eq.z());
  return Gaussian3D(ReferenceFrame3D(Transformation3D(rot, center)), radii);
}

DenseGrid3D<float> get_rasterized(const Gaussian3Ds &gmm, const Floats &weights,
                                  double cell_width, const BoundingBox3D &bb ) {
  DenseGrid3D<float> ret(cell_width, bb, 0);
  for (unsigned int ng=0;ng<gmm.size();ng++){
    IMP_Eigen::Matrix3d covar = get_covariance(gmm[ng]);
    IMP_Eigen::Matrix3d inverse;
    double determinant;
    bool invertible;
    covar.computeInverseAndDetWithCheck(inverse, determinant, invertible);
    double pre = 1.0 / pow(2 * algebra::PI, 2.0 / 3.0) / std::sqrt(determinant);
    if (!invertible || determinant < 0) {
      std::cout << "\n\n\n->>>>not proper matrix!!\n\n\n" << std::endl;
    }
    IMP_Eigen::Vector3d center(gmm[ng].get_center().get_data());
    IMP_INTERNAL_CHECK(invertible, "matrix wasn't invertible! uh oh!");
    IMP_FOREACH(const DenseGrid3D<float>::Index &i, ret.get_all_indexes()) {
      Vector3D aloc = ret.get_center(i);
      IMP_Eigen::Vector3d loc(aloc[0], aloc[1], aloc[2]);
      IMP_Eigen::Vector3d r = loc - center;
      double d = r.transpose() * (inverse * r);
      double score = pre * weights[ng] * std::exp(-0.5 * (d));
      if (score > 1e10) {
        score = 100;
      }
      if (score > 0) {
        ret[i]+=score;
      }
    }
  }
  return ret;
}

IMPALGEBRA_END_NAMESPACE
