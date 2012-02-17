/**
 * \file PCAAligner \brief
 *
 * Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMP_PCA_ALIGNER_H
#define IMP_PCA_ALIGNER_H

#include <IMP/em/converters.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/atom/pdb.h>

class PCAAligner {
public:
  PCAAligner(const IMP::em::DensityMap& map, float density_threshold) :
    map_(map), density_threshold_(density_threshold) {
    IMP::algebra::Vector3Ds density_points =
      IMP::em::density2vectors((IMP::em::DensityMap*)&map_, density_threshold_);
    map_pc_ =IMP::algebra::get_principal_components(density_points);
  }

  void align(const IMP::algebra::Vector3Ds& points,
             std::vector<IMP::algebra::Transformation3D>& transforms);

  IMP::algebra::Rotation3D get_rotation_matrix(const IMP::algebra::Vector3D& x,
                                            const IMP::algebra::Vector3D& y) {
    IMP::algebra::Vector3D z = get_vector_product(x, y);
    return IMP::algebra::get_rotation_from_matrix(x[0], x[1], x[2],
                                                  y[0], y[1], y[2],
                                                  z[0], z[1], z[2]);
  }

private:
  const IMP::em::DensityMap& map_;
  float density_threshold_;
  IMP::algebra::PrincipalComponentAnalysis map_pc_;
};

#endif /* IMP_PCA_ALIGNER_H */
