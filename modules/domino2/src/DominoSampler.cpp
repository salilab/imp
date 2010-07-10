/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/DominoSampler.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/domino2/utility.h>

#include <IMP/domino2/internal/inference.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/file.h>

IMPDOMINO2_BEGIN_NAMESPACE


DominoSampler::DominoSampler(Model *m, ParticleStatesTable* pst):
  DiscreteSampler(m, pst, "Domino Sampler %1"){
}

DominoSampler::DominoSampler(Model *m):
  DiscreteSampler(m, new ParticleStatesTable(), "Domino Sampler %1"){
}



namespace {
  std::vector<SubsetState> get_solutions(const SubsetGraph &jt,
                                  const Subset &known_particles,
                                  ParticleStatesTable *pst,
                                  SubsetStatesTable *sst,
                                  SubsetEvaluatorTable *eval,
                                  const SubsetFilterTables &sfts,
                                  double max_score) {
    const internal::PropagatedData pd
      = internal::get_best_conformations(jt, 0,
                                         known_particles,
                                         eval, sfts, sst,
                                         max_score);
    std::vector<SubsetState> final_solutions;
    for (internal::PropagatedData::ScoresIterator it= pd.scores_begin();
         it != pd.scores_end(); ++it) {
      final_solutions.push_back(it->first);
    }
    return final_solutions;
  }
}

SubsetStatesList DominoSampler
::do_get_sample_states(const Subset &known_particles) const {
  IMP_LOG(TERSE, "Sampling with " << known_particles.size()
          << " particles as " << known_particles << std::endl);
  ParticlesTemp pt(known_particles.begin(), known_particles.end());
  InteractionGraph ig= get_interaction_graph(get_model(),
                                             pt);
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "Interaction graph is " << boost::num_vertices(ig));
    // for some reason output to the ostringstream doesn't work
    //std::ostringstream oss;
    IMP::internal::show_as_graphviz(ig, std::cout);
    //oss << std::endl;
    //IMP_LOG(TERSE, oss.str() << std::endl);
  }
  SubsetGraph jt=get_junction_tree(ig);
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "Junction tree is ");
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

  SubsetStatesList final_solutions
    = get_solutions(jt, known_particles,
                    get_particle_states_table(),
                    sst,
                    set,
                    sfts,
                    get_maximum_score());
  return final_solutions;
}

void DominoSampler::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
