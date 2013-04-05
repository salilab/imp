/**
 *  \file IMP/multifit/DensityDataPoints.h
 *  \brief Handling of data for anchor points segmentation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_DENSITY_DATA_POINTS_H
#define IMPMULTIFIT_DENSITY_DATA_POINTS_H

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/algebra/standard_grids.h>
#include <IMP/statistics/internal/random_generator.h>
#include <IMP/statistics/internal/DataPoints.h>
#include <IMP/em/converters.h>
#include <IMP/em/DensityMap.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/atom/Hierarchy.h>
#include <boost/scoped_ptr.hpp>
#include <IMP/multifit/multifit_config.h>
#include <map>

IMPMULTIFIT_BEGIN_NAMESPACE
typedef IMP::algebra::DenseGrid3D<double> DensGrid;

IMPMULTIFITEXPORT
em::DensityMap *grid2map(const DensGrid &dg,float spacing);


//! Stoers density voxels as a vector of Array1D.
/**
\note This manipulation is needed for matrix operations.
 */
class IMPMULTIFITEXPORT DensityDataPoints
  : public IMP::statistics::internal::XYZDataPoints
{
 public:
  DensityDataPoints(em::DensityMap *dens,
                    float density_threshold);
  DensityDataPoints(const DensGrid &dens,
                    float density_threshold);
  IMP::statistics::internal::Array1DD sample() const;

  //  em::DensityMap* get_density_map() const {return dens_;}

  IMP_OBJECT_INLINE(DensityDataPoints,
                    { out << "DensityDataPoints" << std::endl; }, {});
 protected:
  void populate_data();
  void set_max_min_density_values();
  void set_density(em::DensityMap *d);
  void set_density(const DensGrid &dens);
  //TODO - change back once DensityMap will be Grid3D
  //Pointer<DensGrid> dens_; /// TODO - make the class an object
  boost::scoped_ptr<DensGrid> dens_;
  //  em::DensityMap *dens_;
  double max_value_,min_value_;
  double threshold_;
};
IMP_OBJECTS(DensityDataPoints, DensityDataPointsList);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_DENSITY_DATA_POINTS_H */
