/**
 *  \file IMP/statistics/DataPoints.h
 *  \brief Handling of data for anchor points segmentation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_DATA_POINTS_H
#define IMPSTATISTICS_INTERNAL_DATA_POINTS_H

#include <IMP/statistics/statistics_config.h>
#include <IMP/algebra/standard_grids.h>
#include <IMP/statistics/internal/random_generator.h>
//#include <IMP/em/converters.h>
//#include <IMP/em/DensityMap.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/internal/tnt_array2d.h>
#include <IMP/algebra/internal/tnt_array2d_utils.h>
#include <boost/scoped_ptr.hpp>
#include <IMP/statistics/statistics_config.h>
#include <map>

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
// TODO all of that should be internal
// TODO convert all to standard IMP base types?
typedef algebra::internal::TNT::Array2D<double> Array2DD;
typedef algebra::internal::TNT::Array1D<double> Array1DD;
typedef std::vector<Array1DD> Array1DD_VEC;


//density grid (to remove once DensityMap is grid3d)
//! Holds the data points to be used in the clustering procedure
/** \unstable{DataPoints} */
class IMPSTATISTICSEXPORT DataPoints : public IMP::base::Object {
 public:
  DataPoints(): Object("DataPoints%1%"){
    data_=Array1DD_VEC();
  }
  //TODO - change to get_random_point
  virtual Array1DD sample() const {
    return  data_[statistics::internal::random_int(data_.size())];
  }
  const Array1DD_VEC *get_data() const {return &data_;}
  int get_number_of_data_points()const {return data_.size();}
  IMP_OBJECT_METHODS(DataPoints);
protected:
  Array1DD_VEC data_;
};
IMP_OBJECTS(DataPoints, DataPointsList);
/** \unstable{XYZDataPoint}) */
class IMPSTATISTICSEXPORT XYZDataPoints : public DataPoints {
public:
  XYZDataPoints():DataPoints(){}
  XYZDataPoints(const algebra::Vector3Ds &vecs):DataPoints(){
    populate_data_points(vecs);
  }
  void populate_data_points(const algebra::Vector3Ds &vecs);
  //  ~XYZDataPoints(){}
  inline algebra::Vector3D get_vector(int i) const {return vecs_[i];}

  IMP_OBJECT_METHODS(XYZDataPoints);
protected:
  algebra::Vector3Ds vecs_;
};
IMP_OBJECTS(XYZDataPoints, XYZDataPointsList);

/** \unstable{ParticleDataPoints} */
class IMPSTATISTICSEXPORT ParticlesDataPoints : public XYZDataPoints {
public:
  ParticlesDataPoints() : XYZDataPoints() {
  }

  ParticlesDataPoints(ParticlesTemp ps) : XYZDataPoints() {
    populate_data_points(ps);
  }
  void populate_data_points(ParticlesTemp ps);
  Particle* get_particle(int i) const { return ps_[i];}
  const Particles  &get_particles() const { return ps_;}

  IMP_OBJECT_INLINE(ParticlesDataPoints,
                    { out << "ParticlesDataPoints" << std::endl; }, {});
protected:
  Particles ps_;
};
IMP_OBJECTS(ParticlesDataPoints, ParticlesDataPointsList);


IMPSTATISTICS_END_INTERNAL_NAMESPACE

#endif /* IMPSTATISTICS_INTERNAL_DATA_POINTS_H */
