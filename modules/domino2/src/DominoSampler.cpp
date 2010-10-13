/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/DominoSampler.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/domino2/utility.h>
#include <IMP/domino2/internal/tree_inference.h>
#include <IMP/domino2/optimize_restraints.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/file.h>
#include <boost/graph/connected_components.hpp>
#include <IMP/domino2/internal/loopy_inference.h>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif


IMPDOMINO2_BEGIN_NAMESPACE


DominoSampler::DominoSampler(Model *m, ParticleStatesTable* pst,
                             std::string name):
  DiscreteSampler(m, pst, name), has_sg_(false) {
}

DominoSampler::DominoSampler(Model *m, std::string name):
  DiscreteSampler(m, new ParticleStatesTable(), name), has_sg_(false){
}



namespace {

  bool get_is_tree(const SubsetGraph &g) {
    // check connected components too
    if  (boost::num_edges(g)+1 != boost::num_vertices(g)) return false;
    else {
      std::vector<int> comp(boost::num_vertices(g));
      int cc= boost::connected_components(g, &comp[0]);
      return cc==1;
    }
  }
}

SubsetStates DominoSampler
::do_get_sample_states(const Subset &known_particles) const {
  IMP_LOG(TERSE, "Sampling with " << known_particles.size()
          << " particles as " << known_particles << std::endl);
  Pointer<RestraintSet> rs= get_model()->get_root_restraint_set();
  OptimizeContainers co(rs, get_particle_states_table());
  OptimizeRestraints ro(rs, get_particle_states_table());
  ParticlesTemp pt(known_particles.begin(), known_particles.end());
  SubsetGraph jt;
  if (has_sg_) {
    jt= sg_;
  } else {
    ParticlesTemp kppt(known_particles.begin(),
                       known_particles.end());
    jt= get_junction_tree(get_interaction_graph(rs,
                                                get_particle_states_table()));
  }
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "Subset graph is ");
    //std::ostringstream oss;
    IMP::internal::show_as_graphviz(jt, std::cout);
    //oss << std::endl;
    //IMP_LOG(TERSE, oss.str() << std::endl);
  }
  IMP::internal::OwnerPointer<SubsetEvaluatorTable> set
    = get_subset_evaluator_table_to_use();
  SubsetFilterTables sfts= get_subset_filter_tables_to_use(set);
  IMP::internal::OwnerPointer<SubsetStatesTable> sst
    = DiscreteSampler::get_subset_states_table_to_use(sfts);

  SubsetStates final_solutions;
  if (get_is_tree(jt)) {
    IMP::internal::OwnerPointer<ListSubsetFilterTable>
      lsft(new ListSubsetFilterTable(get_particle_states_table()));
    sfts.push_back(lsft);
    final_solutions
      = internal::get_best_conformations(jt, 0,
                                         known_particles,
                                         sfts, sst, lsft);
    IMP_LOG(TERSE, lsft->get_ok_rate()
            << " were ok with the cross set filtering"
            << std::endl);
  } else {
    final_solutions
      = internal::loopy_get_best_conformations(jt, known_particles,
                                               sfts, sst);
  }
  return final_solutions;
}

void DominoSampler::set_subset_graph(const SubsetGraph &sg) {
  sg_=sg;
  has_sg_=true;
}

void DominoSampler::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
