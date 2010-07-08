/**
 *  \file RestraintGraph.cpp
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/internal/inference.h>
#include <IMP/domino2/subset_states.h>
#include <IMP/domino2/subset_evaluators.h>
#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>


IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE

ParticleIndex get_index(const Subset &s) {
  ParticleIndex ret;
  for (unsigned int i=0; i< s.size(); ++i) {
    ret[s[i]]=i;
  }
  return ret;
}


Subset get_intersection(const Subset &a, const Subset &b) {
  ParticlesTemp rs;
  std::set_intersection(a.begin(), a.end(),
                        b.begin(), b.end(),
                        std::back_inserter(rs));
  Subset ret(rs, true);
  return ret;
}

Subset get_union(const Subset &a, const Subset &b) {
  ParticlesTemp rs;
  std::set_union(a.begin(), a.end(),
                 b.begin(), b.end(),
                 std::back_inserter(rs));
  Subset ret(rs, true);
  return ret;
}

NodeData get_node_data(const Subset &s,
                       const SubsetEvaluatorTable *eval,
                       const SubsetStatesTable *states,
                       double max_score) {
  IMP_LOG(VERBOSE, "Generating data for node " << s << std::endl);
  NodeData ret;
  IMP::internal::OwnerPointer<SubsetStates> ss
    = states->get_subset_states(s);
  IMP::internal::OwnerPointer<SubsetEvaluator> se
    = eval->get_subset_evaluator(s);
  for (unsigned int i=0; i< ss->get_number_of_states(); ++i) {
    SubsetState state(ss->get_state(i));
    double score= se->get_score(state);
    IMP_USAGE_CHECK(score >=0, "Domino needs non-negative scores.");
    if (score < max_score) {
      ret.set_score(state, score);
    } else {
      //IMP_LOG(VERBOSE, "State " << state <<
      // " rejected with score " << score << std::endl);
    }
  }
  IMP_LOG(VERBOSE, "Found " << ret.get_number_of_scores()
          << " states" << std::endl);
  return ret;
}

// indexes[i] is the index in is of the ith component
SubsetState get_subset_state(const IncompleteStates &is,
                             const Subset &s,
                             const ParticleIndex &index) {
  Ints iret(s.size(), -1);
  for (unsigned int i=0; i< iret.size(); ++i) {
    Particle *p= s[i];
    iret[i]= is[index.find(p)->second];
  }
  for (unsigned int i=0; i< iret.size(); ++i) {
    IMP_USAGE_CHECK(iret[i] >=0, "not initialized at " << i);
  }
  return SubsetState(iret);
}


PropagatedData get_merged(const Subset& subset,
                          const SubsetStates *states,
                          const Subset &sa,
                          const Subset &sb,
                          const PropagatedData &da,
                          const PropagatedData &db,
                          const ParticleIndex &all_index,
                          const EdgeData &ed,
                          const SubsetFilterTables &filters,
                          double max_score) {
  IMP_LOG(VERBOSE, "Generating merged data for " << sa
          << " and " << sb << std::endl);
  PropagatedData ret;
  SubsetFilters state_filters;
  Subsets exclusions;
  exclusions.push_back(sa);
  exclusions.push_back(sb);
  for (unsigned int i=0; i< filters.size(); ++i) {
    state_filters.push_back(filters[i]->get_subset_filter(subset, exclusions));
    state_filters.back()->set_was_used(true);
  }
  for (PropagatedData::ScoresIterator ita = da.scores_begin();
       ita != da.scores_end(); ++ita) {
    SubsetState edge_state_a= get_subset_state(ita->first, ed.get_subset(),
                                               all_index);
    double edge_score=ed.get_score(edge_state_a);
    for (PropagatedData::ScoresIterator itb = db.scores_begin();
         itb != db.scores_end(); ++itb) {
      /*IMP_LOG(VERBOSE, "Lists are " << ita->first << "(" << ita->second << ")"
        << " " << itb->first << " (" << itb->second << ")");*/
      SubsetState edge_state_b= get_subset_state(itb->first, ed.get_subset(),
                                                 all_index);
      if (edge_state_b != edge_state_a) {
        //IMP_LOG(VERBOSE, " Rejected" << std::endl);
      } else {
        /*IMP_LOG(VERBOSE, " edge " << edge_state_a
          << " ("<< edge_score << ")");*/
        double nscore= ita->second+itb->second-edge_score;
        if (nscore < max_score) {
          IncompleteStates merged= get_merged(ita->first, itb->first);
          SubsetState union_state= get_subset_state(merged, subset, all_index);
          bool ok=true;
          for (unsigned int i=0; i< state_filters.size(); ++i) {
            if (!state_filters[i]->get_is_ok(union_state)) {
              ok=false;
              break;
            }
          }
          if (ok) {
          /*IMP_LOG(VERBOSE, " ok " << merged << " with score "
            << nscore <<std::endl);*/
            ret.set_score(merged, nscore);
          }
        } else {
          //IMP_LOG(VERBOSE, " Rejected" << std::endl);
        }
      }
    }
  }
  IMP_LOG(VERBOSE, "Found " << ret.get_number_of_scores()
          << " states" << std::endl);
  return ret;
}

PropagatedData get_propagated_data(const ParticleIndex& all_particles,
                                   const Subset& subset,
                                   const NodeData &nd) {
  PropagatedData ret;
  ret.reserve(nd.get_number_of_scores());
  for (NodeData::ScoresIterator it= nd.scores_begin();
       it != nd.scores_end(); ++it) {
    IncompleteStates is= get_incomplete_states(all_particles,
                                               subset, it->first);
    ret.set_score(is, it->second);
  }
  return ret;
}



namespace {

EdgeData get_edge_data(const ParticleIndex &all,
                       const SubsetEvaluatorTable *eval,
                       const Subset &a,
                       const Subset &b,
                       const NodeData &nda) {
  EdgeData ret;
  ret.set_subset(get_intersection(a,b));
  IMP::internal::OwnerPointer<SubsetEvaluator> edge_eval
    = eval->get_subset_evaluator(ret.get_subset());
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "Edge from node \"" << a.get_name()
            << "\" to \"" << b.get_name()
            << "\" is \"" << ret.get_subset().get_name()
            << "\"" << std::endl);
  }
  ParticleIndex a_index= get_index(a);
  // could be done better
  for (NodeData::ScoresIterator it = nda.scores_begin();
       it != nda.scores_end(); ++it) {
    Ints es(ret.get_subset().size(), -1);
    for (unsigned int i=0; i< es.size(); ++i) {
      Particle *p=ret.get_subset()[i];
      int v= it->first[a_index[p]];
      es[i]=v;
    }
    if (!ret.get_has_score(es)) {
      //std::cout << "Found state " << es << " with score std::endl;
      // compute score and store it
      double score= edge_eval->get_score(es);
      IMP_USAGE_CHECK(score >=0, "Domino needs non-negative scores.");
      ret.set_score(es, score);
    }
  }
  IMP_LOG(VERBOSE,"Data is:\n" << ret << std::endl);
  return ret;
}


  PropagatedData
  get_best_conformations_internal(const SubsetGraph &jt,
                                  unsigned int root,
                                  unsigned int parent,
                                  const ParticleIndex &all_index,
                                  const SubsetEvaluatorTable *eval,
                                  const SubsetFilterTables &filters,
                                  const SubsetStatesTable *states,
                                  double max_score) {
    boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, jt);
    Subset s= boost::get(subset_map, root);
    const NodeData nd= get_node_data(s, eval, states,
                                     max_score);
    IMP_LOG(VERBOSE, "For node " << root
            << " local data is:\n" << nd << std::endl);
    /*double local_min_score=0;
    for (NodeData::ScoresIterator it= nd.scores.begin();
         it != nd.scores.end(); ++it) {
      local_max_score= std::max(it->second, local_max_score);
      }*/
    PropagatedData pd= get_propagated_data(all_index, subset_map[root], nd);
    if (root==parent) {
      IMP_LOG(VERBOSE, "For root " <<root
              << " data is:\n" << pd << std::endl);
    }
    ParticleIndex root_index= get_index(subset_map[root]);
    typedef boost::graph_traits<SubsetGraph>::adjacency_iterator
      NeighborIterator;
    for (std::pair<NeighborIterator, NeighborIterator> be
           = boost::adjacent_vertices(root, jt);
         be.first != be.second; ++be.first) {
      if (*be.first == parent) continue;
      Subset cs=boost::get(subset_map, *be.first);
      EdgeData ed= get_edge_data(all_index, eval, s,
                                 cs, nd);
      Subset edge_union
        = get_union(boost::get(subset_map, root),
                    boost::get(subset_map, *be.first));
      IMP::internal::OwnerPointer<SubsetStates> edge_states
        = states->get_subset_states(edge_union);
      // compute intersection set and index map in one direction
      // for each pattern of that in me, compute subset score
      // subtract the min of mine (assume scores positive)
      // for merged score, subtract off edge value
      const PropagatedData cpd
        = get_best_conformations_internal(jt, *be.first, root,
                                          all_index, eval, filters,
                                          states, max_score);
      IMP_LOG(VERBOSE, "For child " << *be.first
              << " returned data is:\n" << cpd << std::endl);
      pd= get_merged(edge_union, edge_states, s, cs, pd, cpd,
                     all_index, ed, filters, max_score);
      IMP_LOG(VERBOSE, "For child " << *be.first
              << " merged data is:\n" << pd << std::endl);
    }
    return pd;
  }
}

PropagatedData get_best_conformations(const SubsetGraph &jt,
                                      int root,
                                      const Subset& all_particles,
                                      const SubsetEvaluatorTable *eval,
                                      const SubsetFilterTables &filters,
                                      const SubsetStatesTable *states,
                                      double max_score) {
  ParticleIndex all_index=get_index(all_particles);
  const PropagatedData pd= get_best_conformations_internal(jt, root, root,
                                                           all_index, eval,
                                                           filters,
                                                           states,
                                                           max_score);
  // check if is tree
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    for (PropagatedData::ScoresIterator it =pd.scores_begin();
         it != pd.scores_end(); ++it) {
      IncompleteStates is= it->first;
      for (unsigned int i=0; i< is.size(); ++i) {
        IMP_INTERNAL_CHECK(is[i] >= 0, "Entry " << i << " is still unknown");
      }
    }
  }
  return pd;
}


IMPDOMINO2_END_INTERNAL_NAMESPACE
