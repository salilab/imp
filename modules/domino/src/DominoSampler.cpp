/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/DominoSampler.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/domino/utility.h>
#include <IMP/domino/internal/tree_inference.h>
#include <IMP/base/warning_macros.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/file.h>
#include <boost/scoped_ptr.hpp>
#include <boost/graph/connected_components.hpp>
#include <IMP/domino/internal/loopy_inference.h>


IMPDOMINO_BEGIN_NAMESPACE


DominoSampler::DominoSampler(Model *m, ParticleStatesTable* pst,
                             std::string name):
  DiscreteSampler(m, pst, name), has_sg_(false), has_mt_(false), csf_(false){
}

DominoSampler::DominoSampler(Model *m, std::string name):
  DiscreteSampler(m, new ParticleStatesTable(), name), has_sg_(false),
  csf_(false){
}



namespace {

  bool get_is_tree(const SubsetGraph &g) {
    // check connected components too
    if  (boost::num_edges(g)+1 != boost::num_vertices(g)) {
      IMP_LOG_TERSE( "Graph has " << boost::num_edges(g)
              << " and " << boost::num_vertices(g) << " and so is not a tree"
              << std::endl);
      return false;
    } else {
      Ints comp(boost::num_vertices(g));
      int cc= boost::connected_components(g, &comp[0]);
      IMP_LOG_TERSE( "Graph has " << cc
              << " components"
              << std::endl);
      return cc==1;
    }
  }
}

template <class G>
void check_graph(const G &jt,
                 Subset known_particles) {
  IMP_CHECK_VARIABLE(known_particles);
  IMP_CHECK_VARIABLE(jt);
 IMP_IF_CHECK(USAGE) {
    IMP::base::set<Particle*> used;
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
  IMP_LOG_TERSE( "Sampling with " << known_particles.size()
          << " particles as " << known_particles << std::endl);
  IMP_USAGE_CHECK(known_particles.size()>0, "No particles to sample");
  Pointer<RestraintSet> rs= get_model()->get_root_restraint_set();
  ParticlesTemp pt(known_particles.begin(), known_particles.end());

  SubsetFilterTables sfts
    = get_subset_filter_tables_to_use(RestraintsTemp(1, rs),
                                      get_particle_states_table());
  IMP_IF_LOG(TERSE) {
    IMP_LOG_TERSE( "Filtering with ");
    for (unsigned int i=0; i< sfts.size(); ++i) {
      IMP_LOG_TERSE( sfts[i]->get_name() << " ");
    }
    IMP_LOG_TERSE( std::endl);
  }
  IMP::OwnerPointer<AssignmentsTable> sst
    = DiscreteSampler::get_assignments_table_to_use(sfts);

  Assignments final_solutions;
  if (has_sg_) {
    IMP_LOG_TERSE("DOMINO running loopy"<<std::endl);
    check_graph(sg_, known_particles);
    /*final_solutions
      = internal::loopy_get_best_conformations(sg_, known_particles,
                                               sfts, sst,
                                get_maximum_number_of_assignments());*/
    IMP_FAILURE("DOMINO does not support loopy at the moment.");
    IMP_LOG_TERSE("DOMINO end running loopy"<<std::endl);
  } else {
    MergeTree mt;
    if (has_mt_) {
      IMP_LOG_TERSE("DOMINOO has merge tree"<<std::endl);
      mt= mt_;
      check_graph(mt_, known_particles);
      IMP_USAGE_CHECK(get_is_merge_tree(mt, known_particles, true),
                      "Not a merge tree");
      IMP_LOG_TERSE("DOMINOO END merge tree"<<std::endl);
    } else {
      IMP_LOG_TERSE("DOMINOO has junction tree"<<std::endl);
      SubsetGraph jt
        = get_junction_tree(get_interaction_graph(RestraintsTemp(1, rs),
                                         get_particle_states_table()));
      mt= get_merge_tree(jt);
    }
    ListSubsetFilterTable* lsft=nullptr;
    if (csf_) {
      lsft= new ListSubsetFilterTable(get_particle_states_table());
      sfts.push_back(lsft);
    }
    IMP_LOG_TERSE("domino::DominoSampler entering InferenceStatistics\n");
    {
      // sillyness for mac os with boost 1.48 bug
      internal::InferenceStatistics tmp;
      stats_=tmp;
    }
    IMP_LOG_TERSE("domino::DominoSampler entering get_best_conformations\n");
    IMP_NEW(PackedAssignmentContainer, as, ());
    as->set_was_used(true);
    internal::load_best_conformations(mt, boost::num_vertices(mt)-1,
                                         known_particles,
                                         sfts, sst, lsft, &stats_,
                                          get_maximum_number_of_assignments(),
                                          as);
    final_solutions= as->get_assignments(IntRange(0,
                                         as->get_number_of_assignments()));
    IMP_LOG_TERSE("domino::DominoSampler end get_best_conformations\n");
    if (lsft) {
      IMP_LOG_TERSE( lsft->get_ok_rate()
              << " were ok with the cross set filtering"
              << std::endl);
    }
    IMP_LOG_TERSE("DOMINOO FINISH junction tree"<<std::endl);
  }
  return final_solutions;
}

void DominoSampler::set_subset_graph(const SubsetGraph &sg) {
  IMP_IF_CHECK(USAGE) {
    Ints comp(boost::num_vertices(sg));
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

Assignments
DominoSampler::get_sample_assignments_for_vertex(unsigned int tree_vertex)
  const {
  IMP_USAGE_CHECK(has_mt_, "Can only query statistics of the merge tree"
                  << " if you set one.");
  boost::property_map< MergeTree, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, mt_);
  return stats_.get_sample_assignments(subset_map[tree_vertex]);
}

Assignments DominoSampler::get_vertex_assignments(unsigned int node_index,
                                             unsigned int max_states) const {
  IMP_NEW(PackedAssignmentContainer, ret,());
  ret->set_was_used(true);
  load_vertex_assignments(node_index, ret, max_states);
  return ret->get_assignments(IntRange(0, ret->get_number_of_assignments()));
}
Assignments DominoSampler::get_vertex_assignments(unsigned int node_index,
                                                  const Assignments &first,
                                                  const Assignments &second,
                                            unsigned int max_states) const {
  IMP_NEW(PackedAssignmentContainer, ret, ());
  IMP_NEW(PackedAssignmentContainer, firstc, ());
  firstc->add_assignments(first);
  IMP_NEW(PackedAssignmentContainer, secondc, ());
  secondc->add_assignments(second);
  ret->set_was_used(true);
  load_vertex_assignments(node_index, firstc, secondc, ret, max_states);
  return ret->get_assignments(IntRange(0, ret->get_number_of_assignments()));
}


void DominoSampler::load_vertex_assignments(unsigned int node_index,
                                            AssignmentContainer *ac,
                                            unsigned int max_states) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(has_mt_,
                  "Must set merge tree before using interactive functions.");
  typedef boost::property_map< MergeTree, boost::vertex_name_t>::const_type
      SubsetMap;
  typedef boost::graph_traits<MergeTree>::adjacency_iterator
    NeighborIterator;
  SubsetMap subset_map= boost::get(boost::vertex_name, mt_);

  Pointer<RestraintSet> rs= get_model()->get_root_restraint_set();
  //ParticlesTemp known_particles= get_particle_states_table()->get_particles();
  SubsetFilterTables sfts= get_subset_filter_tables_to_use(RestraintsTemp(1,rs),
                                         get_particle_states_table());
  IMP::OwnerPointer<AssignmentsTable> sst
    = DiscreteSampler::get_assignments_table_to_use(sfts, max_states);
  ListSubsetFilterTable* lsft=nullptr;
  if (csf_) {
    lsft= new ListSubsetFilterTable(get_particle_states_table());
    sfts.push_back(lsft);
  }
  std::pair<NeighborIterator, NeighborIterator> be
      = boost::adjacent_vertices(node_index, mt_);
  IMP_CHECK_VARIABLE(be);
  IMP_USAGE_CHECK(std::distance(be.first, be.second)==0,
                  "Not a binary tree leaf");
  Subset curs=boost::get(subset_map, node_index);
  internal::load_leaf_assignments(curs,
                                  sst, lsft, nullptr,
                                  ac);
}


void DominoSampler::load_vertex_assignments(unsigned int node_index,
                                            AssignmentContainer* first,
                                            AssignmentContainer* second,
                                            AssignmentContainer* ret,
                                            unsigned int max_states) const {
  set_was_used(true);
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(has_mt_,
                  "Must set merge tree before using interactive functions.");
  typedef boost::property_map< MergeTree, boost::vertex_name_t>::const_type
      SubsetMap;
  typedef boost::graph_traits<MergeTree>::adjacency_iterator
    NeighborIterator;
  SubsetMap subset_map= boost::get(boost::vertex_name, mt_);

  Pointer<RestraintSet> rs= get_model()->get_root_restraint_set();
  //ParticlesTemp known_particles= get_particle_states_table()->get_particles();
  //ParticlesTemp pt(known_particles.begin(), known_particles.end())
  SubsetFilterTables sfts= get_subset_filter_tables_to_use(RestraintsTemp(1,rs),
                                         get_particle_states_table());
  ListSubsetFilterTable* lsft=nullptr;
  if (csf_) {
    lsft= new ListSubsetFilterTable(get_particle_states_table());
    sfts.push_back(lsft);
  }
  std::pair<NeighborIterator, NeighborIterator> be
      = boost::adjacent_vertices(node_index, mt_);
  IMP_USAGE_CHECK(std::distance(be.first, be.second)==2,
                  "Not a binary tree node");
  int firsti= *be.first;
  int secondi= *(++be.first);
  if (firsti > secondi) {
    std::swap(firsti, secondi);
  }
  Subset firsts=boost::get(subset_map, firsti);
  Subset seconds=boost::get(subset_map, secondi);
  IMP_IF_CHECK(USAGE) {
    if (first->get_number_of_assignments() > 0) {
      IMP_USAGE_CHECK(first->get_assignment(0).size() == firsts.size(),
                      "The size of an assignment from the first set is not "
                      << "as expected, are you sure the order is right?");
    }
    if (second->get_number_of_assignments() > 0) {
      IMP_USAGE_CHECK(second->get_assignment(0).size() == seconds.size(),
                      "The size of an assignment from the first set is not "
                      << "as expected, are you sure the order is right?");
    }
  }
  internal::load_merged_assignments(firsts, first,
                                    seconds, second,
                                    sfts, lsft, nullptr,
                                    max_states, ret);
}




IMPDOMINO_END_NAMESPACE
