/**
 *  \file DistanceRMSDMetric.h
 *  \brief Distance RMSD Metric
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPMEMBRANE_DISTANCE_RMSD_METRIC_H
#define IMPMEMBRANE_DISTANCE_RMSD_METRIC_H

#include "membrane_config.h"
#include <IMP/statistics.h>
#include <IMP/algebra.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>

IMPMEMBRANE_BEGIN_NAMESPACE

/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPMEMBRANEEXPORT DistanceRMSDMetric: public statistics::Metric {

  Particles ps_;
  algebra::Transformation3Ds tr_;
  Pointer<Particle> px_;
  Pointer<Particle> py_;
  Pointer<Particle> pz_;
  Floats weight_;
  std::vector<Floats> matrices_;
  std::vector< std::vector<unsigned> > matrixmap_;

  void initialize(Ints align);
  Floats get_distance_matrix(algebra::Vector3Ds coords) const;
  Float  get_distance(algebra::Vector3D v0, algebra::Vector3D v1) const;
  double get_drmsd(const Floats &m0, const Floats &m1) const;
  double get_drmsd_min(const Floats &dist0, const Floats &dist1) const;
  algebra::Vector3D get_vector(algebra::Vector3D center) const;
  algebra::Transformation3D
   get_transformation(algebra::Transformation3D trans) const;
 public:
  DistanceRMSDMetric(Particles ps, Ints align, algebra::Transformation3Ds tr,
                     Particle *px, Particle *py, Particle *pz);
  void add_configuration(double weight=1.0);
  Float get_weight(unsigned i);

  IMP_METRIC(DistanceRMSDMetric);
};

 IMPMEMBRANEEXPORT statistics::PartitionalClustering* create_gromos_clustering
 (statistics::Metric *d, double cutoff);


IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_DISTANCE_RMSD_METRIC_H */
