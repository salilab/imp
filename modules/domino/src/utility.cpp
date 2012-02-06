/**
 *  \file interaction_graph.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/utility.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/ScoreState.h>
#include <IMP/domino/assignment_containers.h>
#include <IMP/compatibility/map.h>
#include <boost/graph/graphviz.hpp>
#include <IMP/internal/graph_utility.h>
#include <IMP/domino/internal/restraint_evaluator.h>
#include <IMP/domino/internal/tree_inference.h>
#include <IMP/RestraintSet.h>
#include <IMP/domino/particle_states.h>
#include <boost/graph/depth_first_search.hpp>
#include <IMP/core/GridClosePairsFinder.h>
#include <boost/graph/reverse_graph.hpp>
#include <IMP/compatibility/vector_property_map.h>


IMPDOMINO_BEGIN_NAMESPACE


void load_particle_states(const Subset &s,
                          const Assignment &ss,
                          const ParticleStatesTable *pst) {
  internal::load_particle_states(s.begin(), s.end(), ss, pst);
  if (s.size()!=0) {
    s[0]->get_model()->update();
  }
}
namespace {
RestraintsAndWeights get_restraints_and_weights(const Subset &s,
                              const ParticleStatesTable *pst,
                              const DependencyGraph &dg,
                              RestraintSet *rs) {
  RestraintsAndWeights rw= get_restraints_and_weights(RestraintsTemp(1, rs));
  Subset other=pst->get_subset();
  ParticlesTemp oms;
  std::set_difference(other.begin(), other.end(),
                      s.begin(), s.end(),
                      std::back_inserter(oms));
  IMP::compatibility::map<Restraint*, int> index
    = IMP::base::internal::get_graph_index<Restraint>(dg);
  Ints to_remove;
  for (unsigned int i=0; i< rw.first.size(); ++i) {
    if (IMP::internal::get_has_ancestor(dg, index[rw.first[i]], oms)) {
      to_remove.push_back(i);
    }
  }
  for (int i=to_remove.size()-1; i >=0; --i) {
    rw.first.erase(rw.first.begin()+to_remove[i]);
    rw.second.erase(rw.second.begin()+to_remove[i]);
  }
  return rw;
}
}

RestraintsTemp get_restraints(const Subset &s,
                              const ParticleStatesTable *pst,
                              const DependencyGraph &dg,
                              RestraintSet *rs) {
  return get_restraints_and_weights(s, pst, dg, rs).first;
}



Ints get_partial_index(const ParticlesTemp &particles,
               const Subset &subset, const Subsets &excluded) {
  for (unsigned int i=0; i< excluded.size(); ++i) {
    bool all=true;
    for (unsigned int j=0; j< particles.size(); ++j) {
      if (!std::binary_search(excluded[i].begin(), excluded[i].end(),
                              particles[j])) {
        all=false;
        break;
      }
    }
    if (all) {
      return Ints();
    }
  }
  Ints ret(particles.size(), -1);
  for (unsigned int i=0; i< particles.size(); ++i) {
    Subset::const_iterator it= std::lower_bound(subset.begin(),
                                                subset.end(), particles[i]);
    if (it!= subset.end() && *it == particles[i]) {
      ret[i]= it-subset.begin();
    }
  }
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "Returning ");
    for (unsigned int i=0; i< ret.size(); ++i) {
      IMP_LOG(VERBOSE, ret[i] << " ");
    }
    IMP_LOG(VERBOSE, "for ");
     for (unsigned int i=0; i< particles.size(); ++i) {
       IMP_LOG(VERBOSE, particles[i]->get_name() << " ");
     }
     IMP_LOG(VERBOSE, " subset " << subset << std::endl);
  }
  return ret;
}

Ints get_index(const ParticlesTemp &particles,
               const Subset &subset, const Subsets &excluded) {
  Ints pi= get_partial_index(particles, subset, excluded);
  if (std::find(pi.begin(), pi.end(), -1) != pi.end()) return Ints();
  else return pi;
}



  Ints get_order(const Subset &s,
                 const ParticlesTemp &all_particles) {
    Ints ret(s.size(), -1);
    int cur=0;
    for (unsigned int i=0; i< all_particles.size(); ++i) {
      for (unsigned int j=0; j< s.size(); ++j) {
        if (all_particles[i]==s[j]) {
          ret[j]=cur;
          ++cur;
        }
      }
    }
    return ret;
  }

#ifdef IMP_DOMINO_USE_IMP_RMF


void save_assignments(AssignmentContainer *assignments,
                      const Subset &s,
                      const ParticlesTemp &all_particles,
                      RMF::HDF5IndexDataSet2D dataset
                     ) {
  IMP_FUNCTION_LOG;
  IMP::OwnerPointer<AssignmentContainer> op(assignments);
  Ints order= get_order(s, all_particles);
  unsigned int ns=assignments->get_number_of_assignments();
  dataset.set_size(RMF::HDF5DataSetIndex2D(ns,
                                           s.size()));
  int cur;
  for (unsigned int i=0; i< ns; ++i) {
    cur=i;
    Assignment cas= assignments->get_assignment(i);
    Ints as= get_output(cas, order);
    dataset.set_row(RMF::HDF5DataSetIndexD<1>(cur), as);
  }
}
AssignmentContainer*
create_assignments_container(RMF::HDF5IndexDataSet2D dataset,
                             const Subset &s,
                             const ParticlesTemp &all_particles) {
  return new ReadHDF5AssignmentContainer(dataset, s, all_particles,
                                     "Assignments from file %1%");
}
#endif



ParticlePairsTemp get_possible_interactions(const ParticlesTemp &ps,
                                            double max_distance,
                                            ParticleStatesTable *pst) {
  if (ps.empty()) return ParticlePairsTemp();
  ParticleStatesList psl;
  ParticlesTemp all= pst->get_particles();
  unsigned int max=0;
  for (unsigned int i=0; i< all.size(); ++i) {
    psl.push_back( pst->get_particle_states(all[i]));
    max= std::max(psl[i]->get_number_of_particle_states(), max);
  }
  algebra::BoundingBox3Ds bbs(ps.size());
  for (unsigned int i=0; i< max; ++i) {
    for (unsigned int j=0; j< all.size(); ++j) {
      psl[j]->load_particle_state(std::min(i,
                 psl[j]->get_number_of_particle_states()-1),
                                      all[j]);
    }
    ps[0]->get_model()->update();
    for (unsigned int j=0; j< ps.size(); ++j) {
      core::XYZ d(ps[j]);
      bbs[j]+= d.get_coordinates();
    }
  }
  for (unsigned int j=0; j< ps.size(); ++j) {
    core::XYZR d(ps[j]);
    bbs[j]+= d.get_radius() + max_distance;
  }
  IMP_NEW(core::GridClosePairsFinder, gcpf, ());
  gcpf->set_distance(max_distance);
  IntPairs ips= gcpf->get_close_pairs(bbs);
  ParticlePairsTemp ret(ips.size());
  for (unsigned int i=0; i< ips.size(); ++i) {
    ret[i]= ParticlePair(ps[ips[i].first], ps[ips[i].second]);
  }
  return ret;
}




void load_leaf_assignments(const Subset& subset,
                             AssignmentsTable *at,
                             AssignmentContainer *ac) {
  IMP_FUNCTION_LOG;
  internal::InferenceStatistics stats;
  internal::load_leaf_assignments(subset,
                                  at, nullptr, stats,
                                  ac);
}


void load_merged_assignments(const Subset &first_subset,
                             AssignmentContainer* first,
                             const Subset &second_subset,
                             AssignmentContainer* second,
                             const SubsetFilterTablesTemp &filters,
                             AssignmentContainer* ret,
                             double max_error,
                             ParticleStatesTable *pst,
                             const statistics::Metrics &metrics,
                             unsigned int max_states) {
  IMP_FUNCTION_LOG;
  internal::InferenceStatistics stats;
  SubsetFilterTables ts(filters.begin(), filters.end());
  internal::load_merged_assignments(first_subset, first,
                                    second_subset, second,
                                    ts, nullptr, stats,
                                    max_error, pst, metrics,
                                    max_states, ret);
}

 algebra::VectorKD get_embedding(const Subset &s,
                                 const Assignment &a,
                                 ParticleStatesTable *pst){
   Floats embed;
  for (unsigned int i=0; i< s.size(); ++i) {
    algebra::VectorKD cur
        = pst->get_particle_states(s[i])->get_embedding(a[i]);
    embed.insert(embed.end(), cur.coordinates_begin(),
                 cur.coordinates_end());
  }
  return embed;
 }


Assignment
get_nearest_assignment(const Subset &s,
                       const algebra::VectorKD &embedding,
                       ParticleStatesTable *pst) {

  Ints ret(s.size());
  unsigned int cur=0;
  // kind of a hack to get size
  for (unsigned int i=0; i< s.size(); ++i) {
    unsigned int sz
        = pst->get_particle_states(s[i])->get_embedding(0).get_dimension();
    algebra::VectorKD cpt(embedding.coordinates_begin()+cur,
                          embedding.coordinates_begin()+cur+sz);
    cur+=sz;
    ret[i]= pst->get_particle_states(s[i])->get_nearest_state(cpt);
  }
  return Assignment(ret);
}

IMPDOMINO_END_NAMESPACE
