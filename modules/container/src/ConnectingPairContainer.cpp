/**
 *  \file ConnectingPairContainer.cpp   \brief A list of ParticlePairs.
 *
 *  Copyright 2007-2021 IMP Inventors. Connecting rights reserved.
 *
 */

#include "IMP/container/ConnectingPairContainer.h"
#include <IMP/container/ListPairContainer.h>
#include <IMP/core/internal/close_pairs_helpers.h>
#include <IMP/internal/ContainerScoreState.h>
#include <IMP/PairModifier.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/vector_property_map.h>
#include <boost/pending/disjoint_sets.hpp>
#include <IMP/container_macros.h>
#include <algorithm>

// Work around Boost bug with adjacency_matrix in 1.60:
// https://svn.boost.org/trac/boost/ticket/11880
#include <boost/version.hpp>
#if BOOST_VERSION == 106000
# include <boost/type_traits/ice.hpp>
#endif

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>

IMPCONTAINER_BEGIN_NAMESPACE

namespace {
typedef boost::vector_property_map<unsigned int> LIndex;
typedef LIndex Parent;
typedef boost::disjoint_sets<LIndex, Parent> UF;

/*typedef boost::adjacency_list<boost::vecS, boost::vecS,
                      boost::undirectedS, boost::no_property,
                      boost::property<boost::edge_weight_t, double> > Graph;*/
typedef boost::adjacency_matrix<boost::undirectedS, boost::no_property,
                                boost::property<boost::edge_weight_t, double> >
    Graph;
typedef boost::graph_traits<Graph>::edge_descriptor Edge;
typedef Graph::edge_property_type Weight;
typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

void compute_mst(Model *m, const ParticleIndexes &imp_indexes,
                 ParticleIndexPairs &out) {
  static unsigned int nnn = 10;
  algebra::Vector3Ds vs(imp_indexes.size());
  for (unsigned int i = 0; i < vs.size(); ++i) {
    vs[i] = core::XYZ(m, imp_indexes[i]).get_coordinates();
  }
  IMP_NEW(algebra::NearestNeighborD<3>, nn, (vs));
  /// unsigned int nnn=static_cast<unsigned int>(std::sqrt(vs.size())+1);
  Graph g(vs.size());
  for (unsigned int i = 0; i < vs.size(); ++i) {
    core::XYZR di(m, imp_indexes[i]);
    Ints ni = nn->get_nearest_neighbors(i, nnn);
    for (unsigned int j = 0; j < ni.size(); ++j) {
      core::XYZR dj(m, imp_indexes[ni[j]]);
      double d = algebra::get_distance(di.get_sphere(), dj.get_sphere());
      boost::add_edge(i, ni[j], Weight(d), g);
    }
  }
  // count sets as we go along
  std::vector<Edge> mst(vs.size() - 1);
  boost::kruskal_minimum_spanning_tree(g, mst.begin());

  for (unsigned int index = 0; index < mst.size(); ++index) {
    int i = boost::target(mst[index], g);
    int j = boost::source(mst[index], g);
    out.push_back(ParticleIndexPair(imp_indexes[i], imp_indexes[j]));
  }
}

void compute_mst(const SingletonContainer *sc,
                 ParticleIndexPairs &out) {
  IMP_CONTAINER_ACCESS(SingletonContainer, sc,
  { compute_mst(sc->get_model(), imp_indexes, out); });
}
}

ConnectingPairContainer::ConnectingPairContainer(SingletonContainer *c,
                                                 double error)
    : IMP::internal::ListLikeContainer<PairContainer>(
          c->get_model(), "ConnectingPairContainer"),
      error_bound_(error),
      mst_(true) {
  initialize(c);
}

void ConnectingPairContainer::initialize(SingletonContainer *sc) {
  sc_ = sc;
  ParticleIndexPairs new_list;
  compute_mst(sc_, new_list);
  swap(new_list);
  mv_ = new core::internal::XYZRMovedSingletonContainer(sc, error_bound_);
  score_state_ =
      new IMP::internal::ContainerScoreState<ConnectingPairContainer>(this);
}

ModelObjectsTemp ConnectingPairContainer::do_get_inputs() const {
  ModelObjectsTemp ret;
  ret.push_back(sc_);
  ret.push_back(mv_);
  ret.push_back(score_state_);
  return ret;
}

ModelObjectsTemp ConnectingPairContainer::get_score_state_inputs() const {
  ModelObjectsTemp ret = get_particles(get_model(), sc_->get_indexes());
  ret.push_back(sc_);
  ret.push_back(mv_);
  IMP_FOREACH(ModelObject * mo, ret) {
    IMP_UNUSED(mo);
    IMP_INTERNAL_CHECK(mo, "Null object found in " << ret);
  }
  return ret;
}

ParticleIndexPairs ConnectingPairContainer::get_range_indexes() const {
  ParticleIndexes ia = sc_->get_range_indexes();
  ParticleIndexPairs ret;
  ret.reserve(ia.size() * (ia.size() - 1) / 2);
  for (unsigned int i = 0; i < ia.size(); ++i) {
    for (unsigned int j = 0; j < i; ++j) {
      ret.push_back(ParticleIndexPair(ia[i], ia[j]));
    }
  }
  return ret;
}

ParticleIndexes ConnectingPairContainer::get_all_possible_indexes() const {
  return sc_->get_all_possible_indexes();
}

void ConnectingPairContainer::do_score_state_before_evaluate() {
  if (mv_->get_access().size()) {
    ParticleIndexPairs new_list;
    compute_mst(sc_, new_list);
    swap(new_list);
    mv_->reset();
  }
}

IMPCONTAINER_END_NAMESPACE
