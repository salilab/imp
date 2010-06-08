/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/DominoSampler.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/domino2/utility.h>
#include <IMP/domino2/internal/JunctionTree.h>

#include <IMP/domino2/internal/RestraintEvaluator.h>
#include <IMP/domino2/internal/DominoOptimizer.h>
#include <IMP/file.h>

IMPDOMINO2_BEGIN_NAMESPACE


DominoSampler::DominoSampler(Model *m):
  Sampler(m, "Domino Sampler %1"),
  enumerators_(new ParticleStatesTable()),
  node_enumerators_(new DefaultSubsetStatesTable(enumerators_)),
  evaluators_(new ModelSubsetEvaluatorTable(m, enumerators_)){
}

struct IntsLess {
  bool operator()(const Ints &a, const Ints &b) const {
    IMP_USAGE_CHECK(a.size() == b.size(), "Sizes don't match "
                    << a.size() << " " << b.size());
    for (unsigned int i=0; i< a.size(); ++i ){
      if (a[i] < b[i]) return true;
      else if (a[i] > b[i]) return false;
    }
    return false;
  }
};

namespace {
  void index_particles(container::ListSingletonContainer *known_particles,
                       std::map<Particle*, unsigned int>&index) {
    StringKey k= internal::node_name_key();
    for (unsigned int i=0; i< known_particles->get_number_of_particles(); ++i) {
      std::ostringstream oss;
      oss << i;
      Particle *p= known_particles->get_particle(i);
      index[p]=i;
      if (p->has_attribute(k)) {
        p->set_value(k, oss.str());
      } else {
        p->add_attribute(k, oss.str());
      }
    }
  }
  void create_junction_tree(const InteractionGraph &ig,
                            const std::map<Particle*, unsigned int>&index,
                            internal::JunctionTree &jt) {
    std::string graphname;
    {
      TextOutput dgraph= IMP::create_temporary_file();
      graphname=dgraph.get_name();
      typedef boost::property_map< InteractionGraph,
        boost::vertex_name_t>::const_type ParticleConstMap;

      ParticleConstMap pm= boost::get(boost::vertex_name, ig);
      typedef boost::graph_traits<InteractionGraph>::edge_iterator EIT;
      typedef boost::graph_traits<InteractionGraph>::vertex_descriptor V;
      EIT b,e;
      boost::tie(b,e)= boost::edges(ig);
      for (; b!= e; ++b) {
        V f=boost::source(*b, ig);
        V t=boost::target(*b, ig);
        std::cout << "Writing edge " << boost::get(pm, f)->get_name()
                  << " " << boost::get(pm, t)->get_name() << std::endl;
        dgraph.get_stream() << index.find(boost::get(pm, f))->second << " "
                            << index.find(boost::get(pm, t))->second
                            << "\n";
      }
    }
    std::string jtreename;
    {
      TextOutput jtree= IMP::create_temporary_file();
      jtreename= jtree.get_name();
    }
    std::string scriptname;
    {
      TextOutput script= IMP::create_temporary_file();
      scriptname = script.get_name();
      script.get_stream() << "import IMP.domino2\n";
      script.get_stream() << "IMP.domino2._compute_junction_tree_from_file(\""
                          << graphname << "\", \""
                          << jtreename << "\")\n";
    }
    std::ostringstream oss;
    oss << "python " << scriptname;
    IMP_LOG(TERSE, "Calling external script for jtree " << std::endl);
    int ev=system(oss.str().c_str());
    if (ev != 0) {
      IMP_THROW("Error running junction tree script", IOException);
    }
    internal::read_junction_tree(jtreename,&jt);
  }

  std::vector<Ints> get_solutions(const internal::JunctionTree &jt,
                     container::ListSingletonContainer *known_particles,
                                  Model *model,
                                  ParticleStatesTable *pst,
                                  SubsetStatesTable *sst,
                                  double max_score) {
    internal::RestraintEvaluator re(model, pst);
    IMP_NEW(internal::DominoOptimizer, opt, (known_particles, jt,
                                             model, &re));
    internal::DiscreteSampler ds(pst, sst);
    opt->set_sampling_space(&ds);
    unsigned int numsol=5;
    do {
      // search for right number of solutions
      opt->set_number_of_solutions(numsol);
      opt->optimize(numsol);
      // check that last energy is greater than cutoff
      double score=-std::numeric_limits<double>::max();
      for (unsigned int i=0; i< numsol; ++i) {
        const internal::CombState *cs= opt->get_graph()->get_opt_combination(i);
        score= std::max(score,
                        static_cast<double>(opt->get_graph()
                                            ->move_to_configuration(*cs)));
      }
      if (score > max_score) {
        break;
      } else {
        numsol*=2;
      }
    } while (true);
    /*container::ListSingletonContainers subsets;
      typedef std::map<Ints, double, IntsLess> Table;
      std::map<Subset*, Table> tables;
      for (unsigned int i=0; i< subsets.size(); ++i) {
      Pointer<SubsetStates> e= node_enumerators_->get_subset_states(subsets[i]);
      Pointer<SubsetEvaluator> eval
      =evaluators_->get_subset_evaluator(subsets[i]);
      unsigned int nstates=e->get_number_of_states();
      for (unsigned int j=0; j< nstates; ++j) {
      Ints state= e->get_state(j);
      double score= eval->get_score(state);
      tables[subsets[i]][state]=score;
      }
      }*/
    std::vector<Ints> final_solutions;
    for (unsigned int i=0; i< numsol; ++i) {
      const internal::CombState *cs= opt->get_graph()->get_opt_combination(i);
      internal::CombData cd= *cs->get_data();
      Ints sol(known_particles->get_number_of_particles());
      for (unsigned int i=0; i< sol.size(); ++i) {
        sol[i]= cd[known_particles->get_particle(i)];
      }
      final_solutions.push_back(sol);
    }
    return final_solutions;
  }
}

ConfigurationSet *DominoSampler::do_sample() const {
  IMP_OBJECT_LOG;

  Pointer<ConfigurationSet> ret= new ConfigurationSet(get_model());
  set_was_used(true);
  IMP_NEW(container::ListSingletonContainer, known_particles,
          (enumerators_->get_particles()));
  StringKey k=internal::node_name_key();
  std::map<Particle*, unsigned int> index;
  index_particles(known_particles, index);
  IMP_LOG(TERSE, "Sampling with " << known_particles->get_number_of_particles()
          << " particles" << std::endl);
  InteractionGraph ig= get_interaction_graph(get_model(),
                                             known_particles->get_particles());
  internal::JunctionTree jt;
  create_junction_tree(ig, index, jt);
  std::vector<Ints> final_solutions= get_solutions(jt, known_particles,
                                                   get_model(),
                                                   get_particle_states_table(),
                                                   get_subset_states_table(),
                                                   get_maximum_score());

  for (unsigned int i=0; i< final_solutions.size(); ++i) {
    IMP_INTERNAL_CHECK(final_solutions[i].size()
                       == known_particles->get_number_of_particles(),
                       "Number of particles doesn't match");
    ret->load_configuration(-1);
    for (unsigned int j=0; j< known_particles->get_number_of_particles(); ++j) {
      Particle *p=known_particles->get_particle(j);
      Pointer<ParticleStates> ps=enumerators_->get_particle_states(p);
      ps->load_state(final_solutions[i][j], p);
    }
    if (get_is_good_configuration()) {
      ret->save_configuration();
    }
  }
  return ret.release();
}

void DominoSampler::set_particle_states(Particle *p, ParticleStates *se) {
  std::cout << "Setting enumerator for " << p->get_name() << std::endl;
  enumerators_->set_particle_states(p, se);
  std::cout << "particles are " << std::endl;
  ParticlesTemp pt=enumerators_->get_particles();
  for (unsigned int i=0; i< pt.size(); ++i) {
    std::cout << pt[i]->get_name() << std::endl;
  }
  std::cout << "done" << std::endl;
}
void DominoSampler::set_subset_evaluator_table(SubsetEvaluatorTable *eval) {
  evaluators_= eval;
}
void DominoSampler::set_subset_states_table(SubsetStatesTable *cse) {
  node_enumerators_= cse;
}

void DominoSampler::do_show(std::ostream &out) const {
}

IMPDOMINO2_END_NAMESPACE
