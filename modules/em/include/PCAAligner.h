/**
 * \file PCAAligner.h \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM_PCA_ALIGNER_H
#define IMPEM_PCA_ALIGNER_H

#include <IMP/em/em_config.h>
#include <IMP/em/converters.h>
#include <IMP/algebra/eigen_analysis.h>
#include <IMP/base/Object.h>

IMPEM_BEGIN_NAMESPACE

/** class that supports fast alignment of a set of points to the density map
    using principal components of the map and the points.

    This time of alignment is good for low-resolution maps.

    Note, that principal components are not well-defined for globular
    objects, so the method may fail in case of symmetric structures
 */
class IMPEMEXPORT PCAAligner : public IMP::base::Object {
public:
  /** init the distance transform
      \param map input density map
      \param density_threshold a threshold that detemines
                              which voxels belong to the object
  */
  PCAAligner(IMP::em::DensityMap* map, float density_threshold) :
    base::Object("EM_PCA_Aligner") {
    IMP::algebra::Vector3Ds density_points =
      IMP::em::density2vectors(map, density_threshold);
    map_pc_ = IMP::algebra::get_principal_components(density_points);
  }

  /**
     align the principal components of the points to the principal
     components of the density map.
     \param points The points for PCA alignment to the map
     \return 24 transformations to the density map
  */
  algebra::Transformation3Ds align(const algebra::Vector3Ds& points) const;

  // methods required by Object
  IMP::base::VersionInfo get_version_info() const {
    return IMP::base::VersionInfo(get_module_name(), get_module_version());
  }

  ~PCAAligner() {}

 private:
  algebra::Rotation3D get_rotation_matrix(const algebra::Vector3D& x,
                                          const algebra::Vector3D& y) const {
    algebra::Vector3D z = get_vector_product(x, y);
    return algebra::get_rotation_from_matrix(x[0], x[1], x[2],
                                             y[0], y[1], y[2],
                                             z[0], z[1], z[2]);
  }

private:
  IMP::algebra::PrincipalComponentAnalysis map_pc_;
};

IMPEM_END_NAMESPACE

#endif /* IMPEM_PCA_ALIGNER_H */
