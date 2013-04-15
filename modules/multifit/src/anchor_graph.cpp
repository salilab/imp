/**
 *  \file anchor_graph.cpp
 *  \brief anchor graph utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/anchor_graph.h>
#include <IMP/algebra/VectorD.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/vector_search.h>

IMPMULTIFIT_BEGIN_NAMESPACE

ProbabilisticAnchorGraph::ProbabilisticAnchorGraph(
                                   algebra::Vector3Ds anchor_positions)
  :Object("ProbabilisticAnchorGraph%1%"){
  GVertex u;
  for(unsigned int i=0;i<anchor_positions.size();i++) {
    u = boost::add_vertex(g_);
    id2node_.push_back(u);
  }
  positions_.insert(positions_.end(),anchor_positions.begin(),
                    anchor_positions.end());
}

void ProbabilisticAnchorGraph::set_particle_probabilities_on_anchors(
                         Particle *p,
                         FittingSolutionRecords sols) {
  IMP_USAGE_CHECK(sols.size()>0,
                  "no solutions provided\n");
  IMP_NEW(algebra::NearestNeighborD<3>, nn, (positions_));
  Ints anchor_counters;
  anchor_counters.insert(anchor_counters.end(),positions_.size(),0);
  for (unsigned int i=0;i<sols.size();i++) {
    algebra::Vector3D loc=
      sols[i].get_fit_transformation().get_transformed(
                               core::XYZ(p).get_coordinates());
    anchor_counters[nn->get_nearest_neighbor(loc)]++;
  }
  Floats probs;
  for (unsigned int i=0;i<anchor_counters.size();i++) {
    probs.push_back(1.*anchor_counters[i]/sols.size());
  }
  particle_to_anchor_probabilities_[p]=probs;
}
algebra::Vector3Ds ProbabilisticAnchorGraph::get_particle_anchors(
                                         Particle *p,float min_prob) const {
  Floats probs=get_particle_probabilities(p);
  algebra::Vector3Ds anchors;
  for(unsigned int i=0;i<probs.size();i++) {
    if (probs[i]>=min_prob) {
      anchors.push_back(positions_[i]);
    }
  }
  return anchors;
}
Floats
  ProbabilisticAnchorGraph::get_particle_probabilities(Particle *p) const {
  IMP_USAGE_CHECK(particle_to_anchor_probabilities_.find(p) !=
                  particle_to_anchor_probabilities_.end(),
                    "Particle:"<<p->get_name()<<" is not found\n");
  return particle_to_anchor_probabilities_.find(p)->second;
}

void ProbabilisticAnchorGraph::show(std::ostream& out) const {
  out<<"( nodes:"<<boost::num_vertices(g_)<<", edges:"<<
    boost::num_edges(g_)<<std::endl;
  out<<"Probabilities:"<<std::endl;
  for(std::map<Particle *,Floats>::const_iterator
        it = particle_to_anchor_probabilities_.begin();
      it != particle_to_anchor_probabilities_.end(); it++) {
    out<<it->first->get_name();//<<" : "<<it->second<<std::endl;
  }
}


// ConfigurationSet align_anchor_graph_to_proteomics_graph(
//                ProbabilisticAnchorGraph *anchor_g,
//                domino1::RestraintGraph *rest_g) {
//   //setup a domino optimization
//   IMP_NEW(Model,mdl,());

// }
IntRanges ProbabilisticAnchorGraph::get_edge_list() const {
    IntRanges edge_list;
    GTraits::edge_iterator e, e_end;
    for (boost::tie(e, e_end) = boost::edges(g_); e != e_end; ++e) {
      edge_list.push_back(IntRange(
                    boost::source(*e, g_),
                    boost::target(*e, g_)));
    }
    return edge_list;
  }


IMPMULTIFIT_END_NAMESPACE
