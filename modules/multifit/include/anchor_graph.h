/**
 *  \file IMP/multifit/anchor_graph.h
 *  \brief anchor graph utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_ANCHOR_GRAPH_H
#define IMPMULTIFIT_ANCHOR_GRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include <IMP/algebra/Vector3D.h>
#include <IMP/multifit/multifit_config.h>
#include "FittingSolutionRecord.h"
#include <IMP/base/Object.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Probabilistic anchor graph.
class IMPMULTIFITEXPORT ProbabilisticAnchorGraph : public IMP::base::Object {
  // Property types
  typedef boost::property<boost::edge_weight_t, float> EdgeWeightProperty;
  typedef boost::property<boost::vertex_index_t, int> VertexIndexProperty;
  // Graph type
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                                VertexIndexProperty,
                                EdgeWeightProperty> AnchorGraph;
  typedef boost::graph_traits<AnchorGraph> GTraits;
  typedef boost::graph_traits<AnchorGraph const> Const_GTraits;
  typedef GTraits::vertex_descriptor GVertex;
  typedef GTraits::edge_descriptor GEdge;

 public:
  ProbabilisticAnchorGraph(algebra::Vector3Ds anchor_positions);

  void add_edge(int i, int j) { boost::add_edge(id2node_[i], id2node_[j], g_); }
  //! Set the probability of a component to be located at each anchor position
  /**
     \param[in] p
     \param[in] sols the fitting solutions of the component
   */
  void set_particle_probabilities_on_anchors(
      kernel::Particle *p, multifit::FittingSolutionRecords sols);
  void show(std::ostream &out = std::cout) const;
  unsigned int get_number_of_anchors() const { return boost::num_vertices(g_); }
  unsigned int get_number_of_edges() const { return boost::num_edges(g_); }
  IntRanges get_edge_list() const;
  algebra::Vector3Ds get_anchors() const { return positions_; }
  algebra::Vector3Ds get_particle_anchors(kernel::Particle *p,
                                          float min_prob = 0) const;
  bool get_are_probabilities_for_particle_set(kernel::Particle *p) const {
    return particle_to_anchor_probabilities_.find(p) !=
           particle_to_anchor_probabilities_.end();
  }
  Floats get_particle_probabilities(kernel::Particle *p) const;
  IMP_OBJECT_METHODS(ProbabilisticAnchorGraph);

 private:
  AnchorGraph g_;
  std::map<kernel::Particle *, Floats> particle_to_anchor_probabilities_;
  algebra::Vector3Ds positions_;
  std::vector<GVertex> id2node_;
};

IMP_OBJECTS(ProbabilisticAnchorGraph, ProbabilisticAnchorGraphs);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_ANCHOR_GRAPH_H */
