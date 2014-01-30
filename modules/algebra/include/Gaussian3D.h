/**
 *  \file IMP/algebra/Gaussian3D.h     \brief Add a name to a particle.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GAUSSIAN3D_H
#define IMPALGEBRA_GAUSSIAN3D_H

#include <IMP/algebra/algebra_config.h>
#include "Transformation3D.h"
#include "ReferenceFrame3D.h"
#include <IMP/algebra/VectorD.h>
#include <IMP/algebra/standard_grids.h>
#include <IMP/algebra/eigen3/Eigen/Dense>

IMPALGEBRA_BEGIN_NAMESPACE

/** A gaussian distribution in 3D.

   The variances are along the axis of the reference frame.
*/
class Gaussian3D : public GeometricPrimitiveD<3> {
  ReferenceFrame3D tr_;
  Vector3D variances_;

 public:
  Gaussian3D() {}
  Gaussian3D(const ReferenceFrame3D &tr, const Vector3D &variances)
      : tr_(tr), variances_(variances) {}

  const ReferenceFrame3D &get_reference_frame() const { return tr_; }
  const Vector3D &get_variances() const { return variances_; }
  inline const Vector3D& get_center() const{
    return get_reference_frame().get_transformation_to().get_translation();
  }
  IMP_SHOWABLE_INLINE(Gaussian3D, out << tr_ << ": " << variances_);
};
IMP_VALUES(Gaussian3D, Gaussian3Ds);

/** Return the covariance matrix from a given set of standard deviations in
    the passed reference frame. */
IMPALGEBRAEXPORT IMP_Eigen::Matrix3d get_covariance(const Gaussian3D &g);

/** Return a Gaussian centered at the origin from a covariance matrix. */
IMPALGEBRAEXPORT Gaussian3D
    get_gaussian_from_covariance(const IMP_Eigen::Matrix3d &covariance,
                                 const Vector3D &center);

/** Rasterize the gaussians to a grid.*/
IMPALGEBRAEXPORT DenseGrid3D<float> get_rasterized(const Gaussian3Ds &gmm,
                                                   const Floats &weights,
                                                   double cell_width,
                                                   const BoundingBox3D &bb);

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_GAUSSIAN3D_H */
