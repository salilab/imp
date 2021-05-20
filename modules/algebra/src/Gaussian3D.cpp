/**
 *  \file  Gaussian3D.cpp
 *  \brief Gaussian shape
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */
#include "IMP/algebra/Gaussian3D.h"

#include <Eigen/Geometry>
#include <Eigen/LU>
#include <Eigen/Eigenvalues>

#include <IMP/algebra/grid_utility.h>

namespace {
  // convenience typedef
  typedef IMP::algebra::DenseGrid3D<double> DensityGrid;
}

IMPALGEBRA_BEGIN_NAMESPACE

Eigen::Matrix3d get_covariance(const Gaussian3D &g) {
  Transformation3D trans = g.get_reference_frame().get_transformation_to();
  Vector3D center = trans.get_translation();
  Vector4D iq = trans.get_rotation().get_quaternion();
  Eigen::Quaterniond q(iq[0], iq[1], iq[2], iq[3]);
  Eigen::Matrix3d rot = q.toRotationMatrix();
  Vector3D variances = g.get_variances();
  Eigen::Matrix3d rad = Eigen::Vector3d(variances[0], variances[1],
                                                variances[2]).asDiagonal();
  Eigen::Matrix3d covar = rot * (rad * rot.transpose());
  return covar;
}

Gaussian3D get_gaussian_from_covariance(const Eigen::Matrix3d &covar,
                                        const Vector3D &center) {
  Rotation3D rot;
  Vector3D radii;

  // get eigen decomposition and sort by eigen vector
  Eigen::EigenSolver<Eigen::Matrix3d> es(covar);
  Eigen::Matrix3d evecs = es.eigenvectors().real();
  Eigen::Vector3d evals = es.eigenvalues().real();

  // fill in sorted stuff
  for (int i = 0; i < 3; i++) {
    radii[i] = evals[i];
  }

  // reflect if necessary
  double det = evecs.determinant();
  // std::cout<<"Determinant is "<<det<<std::endl;
  if (det < 0) {
    Eigen::Matrix3d reflect = Eigen::Vector3d(1, 1, -1).asDiagonal();
    evecs = evecs * reflect;
  }

  // create rotation matrix and return
  Eigen::Quaterniond eq(evecs);
  rot = Rotation3D(eq.w(), eq.x(), eq.y(), eq.z());
  return Gaussian3D(ReferenceFrame3D(Transformation3D(rot, center)), radii);
}
namespace {
double get_gaussian_eval_prefactor(double determinant) {
  return 1.0 / pow(2 * algebra::PI, 2.0 / 3.0) / std::sqrt(determinant);
}
Eigen::Vector3d get_vec_from_center(GridIndex3D i, DensityGrid const &g,
                                    Eigen::Vector3d const &center) {
  Vector3D aloc = g.get_center(i);
  Eigen::Vector3d loc(aloc[0], aloc[1], aloc[2]);
  Eigen::Vector3d r(loc - center);
  return r;
}
void update_value(DensityGrid *g, DensityGrid::Index i, Eigen::Vector3d r,
                  Eigen::Matrix3d inverse, double pre, Float weight) {
  double d(r.transpose() * (inverse * r));
  double score(pre * weight * std::exp(-0.5 * (d)));
  (*g)[i] += score;
}
}

DensityGrid get_rasterized(const Gaussian3Ds &gmm, const Floats &weights,
                           double cell_width, const BoundingBox3D &bb) {
  DensityGrid ret(cell_width, bb, 0);
  for (unsigned int ng = 0; ng < gmm.size(); ng++) {
    Eigen::Matrix3d covar = get_covariance(gmm[ng]);
    Eigen::Matrix3d inverse = Eigen::Matrix3d::Zero(3, 3);

    double determinant;
    bool invertible;
    covar.computeInverseAndDetWithCheck(inverse, determinant, invertible);
    IMP_INTERNAL_CHECK((invertible && determinant > 0),
                       "Tried to invert Gaussian, but it's not proper matrix");
    double pre(get_gaussian_eval_prefactor(determinant));
    Eigen::Vector3d center(gmm[ng].get_center().get_data());
    IMP_INTERNAL_CHECK(invertible, "matrix wasn't invertible! uh oh!");
    IMP_FOREACH(const DensityGrid::Index & i, ret.get_all_indexes()) {
      Eigen::Vector3d r(get_vec_from_center(i, ret, center));
      update_value(&ret, i, r, inverse, pre, weights[ng]);
    }
  }
  return ret;
}

DensityGrid get_rasterized_fast(const Gaussian3Ds &gmm, const Floats &weights,
                                double cell_width, const BoundingBox3D &bb, double factor) {
  DensityGrid ret(cell_width, bb, 0);
  for (unsigned int ng = 0; ng < gmm.size(); ng++) {
    Eigen::Matrix3d covar = get_covariance(gmm[ng]);
    Eigen::Matrix3d inverse = Eigen::Matrix3d::Zero(3, 3);

    double determinant;
    bool invertible;
    covar.computeInverseAndDetWithCheck(inverse, determinant, invertible);
    IMP_INTERNAL_CHECK((invertible && determinant > 0),
                       "Tried to invert Gaussian, but it's not proper matrix");
    double pre(get_gaussian_eval_prefactor(determinant));
    Eigen::Vector3d evals = covar.eigenvalues().real();
    double maxeval = sqrt(evals.maxCoeff());
    double cutoff = factor * maxeval;
    double cutoff2 = cutoff * cutoff;
    Vector3D c = gmm[ng].get_center();
    Vector3D lower = c - Vector3D(cutoff, cutoff, cutoff);
    Vector3D upper = c + Vector3D(cutoff, cutoff, cutoff);
    GridIndex3D lowerindex = ret.get_nearest_index(lower);
    GridIndex3D upperindex = ret.get_nearest_index(upper);
    Eigen::Vector3d center(c.get_data());
    IMP_INTERNAL_CHECK(invertible, "matrix wasn't invertible! uh oh!");
    IMP_GRID3D_FOREACH_SMALLER_EXTENDED_INDEX_RANGE(ret, upperindex, lowerindex,
                                                    upperindex, {
      GridIndex3D i(voxel_index[0], voxel_index[1], voxel_index[2]);
      Eigen::Vector3d r(get_vec_from_center(i, ret, center));
      if (r.squaredNorm() < cutoff2) {
        update_value(&ret, i, r, inverse, pre, weights[ng]);
      }
    })
  }
  return ret;
}

IMPALGEBRA_END_NAMESPACE
