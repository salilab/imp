/**
 *  \file DataPoints.h
 *  \brief Handling of data for anchor points segmentation
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_DATA_POINTS_H
#define IMPMULTIFIT_DATA_POINTS_H

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/statistics/internal/random_generator.h>
#include <IMP/em/converters.h>
#include <IMP/em/DensityMap.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <IMP/atom/Hierarchy.h>
#include "multifit_config.h"
#include <map>

IMPMULTIFIT_BEGIN_NAMESPACE
//TODO all of that should be internal
typedef algebra::internal::TNT::Array2D<double> Array2DD;
typedef algebra::internal::TNT::Array1D<double> Array1DD;
typedef std::vector<Array1DD> Array1DD_VEC;
//! Holds the data points to be used in the clustering procedure
class DataPoints{
 public:
  DataPoints(){
    atts_ = core::XYZ::get_xyz_keys();
  }
  virtual ~DataPoints(){
    data_.clear();
  }
  //TODO - change to get_random_point
  virtual Array1DD sample() const {
    return  data_[statistics::internal::random_int(data_.size())];
  }
  const Array1DD_VEC *get_data() const {return &data_;}
  int get_number_of_data_points()const {return data_.size();}
protected:
  FloatKeys atts_;
  Array1DD_VEC data_;
};

class XYZDataPoints : public DataPoints {
public:
  XYZDataPoints():DataPoints(){}
  XYZDataPoints(const algebra::Vector3Ds &vecs):DataPoints(){
    populate_data_points(vecs);
  }
  void populate_data_points(const algebra::Vector3Ds &vecs);
  ~XYZDataPoints(){}
  inline algebra::Vector3D get_vector(int i) const {return vecs_[i];}
protected:
  algebra::Vector3Ds vecs_;
};

class IMPMULTIFITEXPORT ParticlesDataPoints : public XYZDataPoints {
public:
  ParticlesDataPoints() : XYZDataPoints() {
  }

  ParticlesDataPoints(Particles ps) : XYZDataPoints() {
    populate_data_points(ps);
  }
  void populate_data_points(Particles ps);
  ~ParticlesDataPoints(){}
  Particle* get_particle(int i) const { return ps_[i];}
  const Particles  &get_particles() const { return ps_;}
protected:
  Particles ps_;
};

//! Stoers density voxels as a vector of Array1D.
/**
\note This manipulation is needed for matrix operations.
 */
class IMPMULTIFITEXPORT DensityDataPoints: public XYZDataPoints {
public:
  DensityDataPoints(em::DensityMap *dens,
                    float density_threshold);
  Array1DD sample() const;

  em::DensityMap* get_density_map() const {return dens_;}
protected:
  em::DensityMap *dens_;
  Model *m_;
  double max_value_,min_value_;
  double threshold_;
};

IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_DATA_POINTS_H */
