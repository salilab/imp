/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/metric_clustering.h>
#include <IMP/statistics/internal/KMData.h>
#include <IMP/statistics/internal/KMTerminationCondition.h>
#include <IMP/statistics/internal/KMLocalSearchLloyd.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/atom/distance.h>
#include <IMP/statistics/internal/centrality_clustering.h>

IMPSTATISTICS_BEGIN_NAMESPACE

Metric::Metric(std::string name): Object(name){}

Metric::~Metric(){}

EuclideanMetric::EuclideanMetric(Embedding *em):
  Metric("Euclidean %1%"), em_(em){}
double EuclideanMetric::get_distance(unsigned int i,
                                       unsigned int j) const {
  return algebra::get_distance(em_->get_point(i), em_->get_point(j));
}

unsigned int EuclideanMetric::get_number_of_items() const {
  return em_->get_number_of_items();
}

void EuclideanMetric::do_show(std::ostream &) const {
}


ConfigurationSetRMSDMetric::ConfigurationSetRMSDMetric(ConfigurationSet *cs,
                                                         SingletonContainer *sc,
                                                         bool align):
  Metric("CS RMS %1%"),
  cs_(cs), sc_(sc), align_(align){

}

namespace {
  algebra::Vector3Ds get_vectors(ConfigurationSet *cs,
                        unsigned int i,
                        SingletonContainer *sc) {
    algebra::Vector3Ds ret(sc->get_number_of_particles());
    cs->load_configuration(i);
    IMP_FOREACH_SINGLETON(sc, {
        ret[_2]= _1->_get_coordinates().get_center();
      });
    return ret;
  }
}

double ConfigurationSetRMSDMetric::get_distance(unsigned int i,
                                                 unsigned int j) const {
  algebra::Vector3Ds vi= get_vectors(cs_, i, sc_);
  algebra::Vector3Ds vj= get_vectors(cs_, j, sc_);
  algebra::Transformation3D tr;
  if (align_) {
    tr= algebra::get_transformation_aligning_first_to_second(vi, vj);
  } else {
    tr=algebra::get_identity_transformation_3d();
  }
  return atom::get_rmsd(vi, vj, tr);
}

unsigned int ConfigurationSetRMSDMetric::get_number_of_items() const {
  return cs_->get_number_of_configurations();
}

void ConfigurationSetRMSDMetric::do_show(std::ostream &) const {
}


PartitionalClustering *create_centrality_clustering(Metric *d,
                                                 double far,
                                                 int k) {
  IMP::internal::OwnerPointer<Metric> dp(d);
  unsigned int n=d->get_number_of_items();
  internal::CentralityGraph g(n);
  boost::property_map<internal::CentralityGraph,
                      boost::edge_weight_t>::type w
    = boost::get(boost::edge_weight, g);

  for (unsigned int i=0; i<n; ++i) {
    for (unsigned int j=0; j< i; ++j) {
      double dist= d->get_distance(i,j);
      if (dist < far) {
        boost::graph_traits<internal::CentralityGraph>::edge_descriptor e
          =add_edge(i,j, g).first;
        w[e]=/*1.0/*/dist;
      }
    }
  }
  return internal::get_centrality_clustering(g, k);
}

IMPSTATISTICS_END_NAMESPACE
