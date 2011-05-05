/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/DominoSampler.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/domino/utility.h>
#include <IMP/domino/internal/tree_inference.h>
#include <IMP/domino/optimize_restraints.h>

#include <IMP/internal/graph_utility.h>
#include <IMP/file.h>
#include <boost/graph/connected_components.hpp>
#include <IMP/domino/internal/loopy_inference.h>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif


IMPDOMINO_BEGIN_NAMESPACE


DominoSampler::DominoSampler(Model *m, ParticleStatesTable* pst,
                             std::string name):
  DiscreteSampler(m, pst, name), has_sg_(false), has_mt_(false), csf_(false) {
}

DominoSampler::DominoSampler(Model *m, std::string name):
  DiscreteSampler(m, new ParticleStatesTable(), name), has_sg_(false),
  csf_(false){
}



namespace {

  bool get_is_tree(const SubsetGraph &g) {
    // check connected components too
    if  (boost::num_edges(g)+1 != boost::num_vertices(g)) {
      IMP_LOG(TERSE, "Graph has " << boost::num_edges(g)
              << " and " << boost::num_vertices(g) << " and so is not a tree"
              << std::endl);
      return false;
    } else {
      std::vector<int> comp(boost::num_vertices(g));
      int cc= boost::connected_components(g, &comp[0]);
      IMP_LOG(TERSE, "Graph has " << cc
              << " components"
              << std::endl);
      return cc==1;
    }
  }
}

template <class G>
void check_graph(const G &jt,
                 Subset known_particles) {
 IMP_IF_CHECK(USAGE) {
    IMP::internal::Set<Particle*> used;
    typename boost::property_map< G, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, jt);
    for (unsigned int i=0; i< boost::num_vertices(jt); ++i) {
      Subset s= boost::get(subset_map, i);
      used.insert(s.begin(), s.end());
    }
    IMP_USAGE_CHECK(used.size()==known_particles.size(),
                    "Unexpected number of particles found in graph. Expected "
                    << known_particles.size() << " found " << used.size());
  }
}

Assignments DominoSampler
::do_get_sample_assignments(const Subset &known_particles) const {
  IMP_LOG(TERSE, "Sampling with " << known_particles.size()
          << " particles as " << known_particles << std::endl);
  IMP_USAGE_CHECK(known_particles.size()>0, "No particles to sample");
  Pointer<RestraintSet> rs= get_model()->get_root_restraint_set();
    OptimizeRestraints ro(rs, get_particle_states_table());
  ParticlesTemp pt(known_particles.begin(), known_particles.end());

  SubsetFilterTables sfts= get_subset_filter_tables_to_use(rs,
                                             get_particle_states_table());
  IMP_IF_LOG(TERSE) {
    IMP_LOG(TERSE, "Filtering with ");
    for (unsigned int i=0; i< sfts.size(); ++i) {
      IMP_LOG(TERSE, sfts[i]->get_name() << " ");
    }
    IMP_LOG(TERSE, std::endl);
  }
  IMP::internal::OwnerPointer<AssignmentsTable> sst
    = DiscreteSampler::get_assignments_table_to_use(sfts);

  Assignments final_solutions;
  if (has_sg_) {
    IMP_LOG(TERSE,"DOMINOO running loopy"<<std::endl);
    check_graph(sg_, known_particles);
    final_solutions
      = internal::loopy_get_best_conformations(sg_, known_particles,
                                               sfts, sst,
                                    get_maximum_number_of_assignments());
    IMP_LOG(TERSE,"DOMINOO end running loopy"<<std::endl);
  } else {
    MergeTree mt;
    if (has_mt_) {
      IMP_LOG(TERSE,"DOMINOO has merge tree"<<std::endl);
      mt= mt_;
      check_graph(mt_, known_particles);
      IMP_USAGE_CHECK(get_is_merge_tree(mt, known_particles, true),
                      "Not a merge tree");
      IMP_LOG(TERSE,"DOMINOO END merge tree"<<std::endl);
    } else {
      IMP_LOG(TERSE,"DOMINOO has junction tree"<<std::endl);
      SubsetGraph jt= get_junction_tree(get_interaction_graph(rs,
                                               get_particle_states_table()));
      MergeTree mt= get_merge_tree(jt);
      ListSubsetFilterTable* lsft=NULL;
      if (csf_) {
        lsft= new ListSubsetFilterTable(get_particle_states_table());
        sfts.push_back(lsft);
      }
      IMP_LOG(TERSE,"domino::DominoSampler entering InferenceStatistics\n");
      stats_=internal::InferenceStatistics();
      IMP_LOG(TERSE,"domino::DominoSampler entering get_best_conformations\n");
      final_solutions
        = internal::get_best_conformations(mt, boost::num_vertices(mt)-1,
                                           known_particles,
                                           sfts, sst, lsft, stats_,
                                    get_maximum_number_of_assignments());
      IMP_LOG(TERSE,"domino::DominoSampler end get_best_conformations\n");
      if (lsft) {
        IMP_LOG(TERSE, lsft->get_ok_rate()
                << " were ok with the cross set filtering"
                << std::endl);
      }
      IMP_LOG(TERSE,"DOMINOO FINISH junction tree"<<std::endl);
    }
  }
  return final_solutions;
}

void DominoSampler::set_subset_graph(const SubsetGraph &sg) {
  IMP_IF_CHECK(USAGE) {
    std::vector<int> comp(boost::num_vertices(sg));
    IMP_CHECK_CODE(int cc= boost::connected_components(sg, &comp[0]));
    IMP_USAGE_CHECK(cc==1, "Graph must have exactly one connected component."
                    << " It has " << cc);
  }
  if (get_is_tree(sg_)) {
    IMP_WARN("A tree has been specified for the inference graph. Now if you "
             << "want to do"
             << " non-loopy inference and specify things manually, please "
             << "specify the "
             << "merge tree instead. Just call "
             << "sample.set_merge_tree(get_merge_tree(junction_tree))");
  }
  sg_=sg;
  has_sg_=true;
}


void DominoSampler::set_merge_tree(const MergeTree &sg) {
  mt_=sg;
  has_mt_=true;
}

void DominoSampler::do_show(std::ostream &out) const {
  out << "pst: " << *get_particle_states_table() << std::endl;
}


unsigned int
DominoSampler::get_number_of_assignments_for_vertex(unsigned int tree_vertex)
  const {
  IMP_USAGE_CHECK(has_mt_, "Can only query statistics of the merge tree"
                  << " if you set one.");
  boost::property_map< MergeTree, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, mt_);
  return stats_.get_number_of_assignments(subset_map[tree_vertex]);
}
/*
Assignments
DominoSampler::get_sample_assignments_for_vertex(unsigned int tree_vertex)
  const {
  IMP_USAGE_CHECK(has_mt_, "Can only query statistics of the merge tree"
                  << " if you set one.");
  boost::property_map< MergeTree, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, mt_);
  return stats_.get_sample_assignments(subset_map[tree_vertex]);

  }*/


IMPDOMINO_END_NAMESPACE
