/**
 *  \file anchor_graph.cpp
 *  \brief anchor graph utilities
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/anchor_graph.h>
#include <IMP/algebra/VectorD.h>

IMPMULTIFIT_BEGIN_NAMESPACE

ProbabilisticAnchorGraph::ProbabilisticAnchorGraph(
                                   algebra::Vector3Ds anchor_positions){
  GVertex u;
  for(int i=0;i<anchor_positions.size();i++) {
    u = boost::add_vertex(g_);
    id2node_.push_back(u);
  }
  positions_.insert(positions_.end(),anchor_positions.begin(),
                    anchor_positions.end());
}

void ProbabilisticAnchorGraph::set_component_probabilities_on_anchors(
                           int comp_ind,
                           algebra::Vector3D comp_center,
                           FittingSolutionRecords sols) {
  Floats probs;
  Ints anchor_counters;
  anchor_counters.insert(anchor_counters.end(),positions_.size(),0);
  //use KNN once it is working
  for (unsigned int i=0;i<sols.size();i++) {
    //get_closets anchor
    float max_len=INT_MAX;
    int closest_anchor=0;
    algebra::Vector3D loc=
      sols[i].get_transformation().get_transformed(comp_center);
    for (unsigned int j=0;j<positions_.size();j++) {
      if (algebra::get_squared_distance(positions_[j],loc)<max_len) {
        closest_anchor=j;
      }
    }
    anchor_counters[closest_anchor]++;
  }
  for (unsigned int i=0;i<anchor_counters.size();i++) {
   anchor_to_comp_probabilities_[i][comp_ind]=1.*anchor_counters[i]/sols.size();
  }
}

void ProbabilisticAnchorGraph::show(std::ostream& out) const {
  out<<"( nodes:"<<boost::num_vertices(g_)<<", edges:"<<
    boost::num_edges(g_)<<std::endl;
}


// ConfigurationSet align_anchor_graph_to_proteomics_graph(
//                ProbabilisticAnchorGraph *anchor_g,
//                domino2::RestraintGraph *rest_g) {
//   //setup a domino optimization
//   IMP_NEW(Model,mdl,());

// }
IntRanges ProbabilisticAnchorGraph::get_edge_list() const {
    IntRanges edge_list;
    GTraits::edge_iterator e, e_end;
    for (tie(e, e_end) = boost::edges(g_); e != e_end; ++e) {
      edge_list.push_back(IntRange(
                    boost::source(*e, g_),
                    boost::target(*e, g_)));
    }
    return edge_list;
  }


IMPMULTIFIT_END_NAMESPACE
