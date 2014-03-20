/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/metrics.h>
#include <IMP/singleton_macros.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/geometric_alignment.h>
IMPSTATISTICS_BEGIN_NAMESPACE

EuclideanMetric::EuclideanMetric(Embedding *em)
    : Metric("Euclidean %1%"), em_(em) {}
double EuclideanMetric::get_distance(unsigned int i, unsigned int j) const {
  return algebra::get_distance(em_->get_point(i), em_->get_point(j));
}

unsigned int EuclideanMetric::get_number_of_items() const {
  return em_->get_number_of_items();
}

ConfigurationSetRMSDMetric::ConfigurationSetRMSDMetric(ConfigurationSet *cs,
                                                       SingletonContainer *sc,
                                                       bool align)
    : Metric("CS RMS %1%"), cs_(cs), sc_(sc), align_(align) {}

namespace {
algebra::Vector3Ds get_vectors(ConfigurationSet *cs, unsigned int i,
                               SingletonContainer *sc) {
  algebra::Vector3Ds ret;
  cs->load_configuration(i);
  IMP_CONTAINER_FOREACH(SingletonContainer, sc,
  { ret.push_back(cs->get_model()->get_sphere(_1).get_center()); });
  return ret;
}

template <class Vector3DsOrXYZs0, class Vector3DsOrXYZs1>
inline double get_rmsd(const Vector3DsOrXYZs0 &m1, const Vector3DsOrXYZs1 &m2,
                       const IMP::algebra::Transformation3D &tr_for_second =
                           IMP::algebra::get_identity_transformation_3d()) {
  IMP_USAGE_CHECK(std::distance(m1.begin(), m1.end()) ==
                      std::distance(m2.begin(), m2.end()),
                  "The input sets of XYZ points "
                      << "should be of the same size");
  float rmsd = 0.0;
  typename Vector3DsOrXYZs0::const_iterator it0 = m1.begin();
  typename Vector3DsOrXYZs1::const_iterator it1 = m2.begin();
  for (; it0 != m1.end(); ++it0, ++it1) {
    algebra::Vector3D tred =
        tr_for_second.get_transformed(get_vector_geometry(*it1));
    rmsd += algebra::get_squared_distance(get_vector_geometry(*it0), tred);
  }
  return std::sqrt(rmsd / m1.size());
}
}

double ConfigurationSetRMSDMetric::get_distance(unsigned int i,
                                                unsigned int j) const {
  algebra::Vector3Ds vi = get_vectors(cs_, i, sc_);
  algebra::Vector3Ds vj = get_vectors(cs_, j, sc_);
  algebra::Transformation3D tr;
  if (align_) {
    tr = algebra::get_transformation_aligning_first_to_second(vi, vj);
  } else {
    tr = algebra::get_identity_transformation_3d();
  }
  return get_rmsd(vj, vi, tr);
}

unsigned int ConfigurationSetRMSDMetric::get_number_of_items() const {
  return cs_->get_number_of_configurations();
}

IMPSTATISTICS_END_NAMESPACE
