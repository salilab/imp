/**
 *  \file DataPoints.h
 *  \brief Handling of data for anchor points segmentation
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_DATA_POINTS_H
#define IMPMULTIFIT_DATA_POINTS_H

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/algebra/Grid3D.h>
#include <IMP/statistics/internal/random_generator.h>
#include <IMP/em/converters.h>
#include <IMP/em/DensityMap.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/atom/Hierarchy.h>
#include <boost/scoped_ptr.hpp>
#include "multifit_config.h"
#include <map>

IMPMULTIFIT_BEGIN_NAMESPACE
//TODO all of that should be internal
typedef algebra::internal::TNT::Array2D<double> Array2DD;
typedef algebra::internal::TNT::Array1D<double> Array1DD;
typedef std::vector<Array1DD> Array1DD_VEC;
typedef IMP::algebra::DenseGrid3D<double> DensGrid;

IMPMULTIFITEXPORT
em::DensityMap *grid2map(const DensGrid &dg,float spacing);

//density grid (to remove once DensityMap is grid3d)
//! Holds the data points to be used in the clustering procedure
class IMPMULTIFITEXPORT DataPoints : public Object {
 public:
  DataPoints(): Object("DataPoints%1%"){
    atts_ = core::XYZ::get_xyz_keys();
    data_=Array1DD_VEC();
  }
  //TODO - change to get_random_point
  virtual Array1DD sample() const {
    return  data_[statistics::internal::random_int(data_.size())];
  }
  const Array1DD_VEC *get_data() const {return &data_;}
  int get_number_of_data_points()const {return data_.size();}
  IMP_OBJECT_INLINE(DataPoints, { out << "DataPoints" << std::endl; }, {});
protected:
  FloatKeys atts_;
  Array1DD_VEC data_;
};
IMP_OBJECTS(DataPoints, DataPointsList);
class IMPMULTIFITEXPORT XYZDataPoints : public DataPoints {
public:
  XYZDataPoints():DataPoints(){}
  XYZDataPoints(const algebra::Vector3Ds &vecs):DataPoints(){
    populate_data_points(vecs);
  }
  void populate_data_points(const algebra::Vector3Ds &vecs);
  //  ~XYZDataPoints(){}
  inline algebra::Vector3D get_vector(int i) const {return vecs_[i];}

  IMP_OBJECT_INLINE(XYZDataPoints, { out << "XYZDataPoints" << std::endl; },
                    {});
protected:
  algebra::Vector3Ds vecs_;
};
IMP_OBJECTS(XYZDataPoints, XYZDataPointsList);

class IMPMULTIFITEXPORT ParticlesDataPoints : public XYZDataPoints {
public:
  ParticlesDataPoints() : XYZDataPoints() {
  }

  ParticlesDataPoints(Particles ps) : XYZDataPoints() {
    populate_data_points(ps);
  }
  void populate_data_points(Particles ps);
  Particle* get_particle(int i) const { return ps_[i];}
  const Particles  &get_particles() const { return ps_;}

  IMP_OBJECT_INLINE(ParticlesDataPoints,
                    { out << "ParticlesDataPoints" << std::endl; }, {});
protected:
  Particles ps_;
};
IMP_OBJECTS(ParticlesDataPoints, ParticlesDataPointsList);

//! Stoers density voxels as a vector of Array1D.
/**
\note This manipulation is needed for matrix operations.
 */
class IMPMULTIFITEXPORT DensityDataPoints: public XYZDataPoints {
public:
  DensityDataPoints(em::DensityMap *dens,
                    float density_threshold);
  DensityDataPoints(const DensGrid &dens,
                    float density_threshold);
  Array1DD sample() const;

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

#endif /* IMPMULTIFIT_DATA_POINTS_H */
