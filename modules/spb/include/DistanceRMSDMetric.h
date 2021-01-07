/**
 *  \file IMP/spb/DistanceRMSDMetric.h
 *  \brief Distance RMSD Metric
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 */

#ifndef IMPSPB_DISTANCE_RMSD_METRIC_H
#define IMPSPB_DISTANCE_RMSD_METRIC_H

#include <IMP/algebra.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/statistics.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

/** Compute the RMSD between two sets of particles in two configurations.
 */
class IMPSPBEXPORT DistanceRMSDMetric : public statistics::Metric {
  Particles ps_;
  algebra::Transformation3Ds tr_;
  IMP::PointerMember<IMP::Particle> px_;
  IMP::PointerMember<IMP::Particle> py_;
  IMP::PointerMember<IMP::Particle> pz_;
  Floats weight_;
  std::vector<Floats> matrices_;
  std::vector<std::vector<unsigned> > matrixmap_;

  void initialize(Ints align);
  Floats get_distance_matrix(algebra::Vector3Ds coords) const;
  Float get_distance(algebra::Vector3D v0, algebra::Vector3D v1) const;
  double get_drmsd(const Floats &m0, const Floats &m1) const;
  algebra::Vector3D get_vector(algebra::Vector3D center) const;
  algebra::Transformation3D get_transformation(
      algebra::Transformation3D trans) const;

 public:
  DistanceRMSDMetric(Particles ps, Ints align, algebra::Transformation3Ds tr,
                     Particle *px, Particle *py, Particle *pz);
  void add_configuration(double weight = 1.0);
  Float get_weight(unsigned i);

  // IMP_METRIC(DistanceRMSDMetric);
  double get_distance(unsigned int i, unsigned int j) const IMP_OVERRIDE;
  unsigned int get_number_of_items() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(DistanceRMSDMetric);
};

IMPSPBEXPORT statistics::PartitionalClustering *create_gromos_clustering(
    statistics::Metric *d, double cutoff);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_DISTANCE_RMSD_METRIC_H */
