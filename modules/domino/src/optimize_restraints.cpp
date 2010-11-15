/**
 *  \file interaction_graph.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/optimize_restraints.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/ScoreState.h>
#include <IMP/internal/map.h>
#include <boost/graph/graphviz.hpp>
#include <IMP/internal/graph_utility.h>
#include <IMP/domino/internal/restraint_evaluator.h>
#include <IMP/RestraintSet.h>
#include <IMP/core/internal/CoreClosePairContainer.h>
#include <IMP/domino/particle_states.h>


IMP_BEGIN_NAMESPACE
bool
get_is_static_container(Container *c,
                        const DependencyGraph &dg,
                        const ParticlesTemp &pst);
IMP_END_NAMESPACE
IMPDOMINO_BEGIN_NAMESPACE


namespace {
 typedef boost::graph_traits<DependencyGraph> DGTraits;
  typedef DGTraits::vertex_descriptor DGVertex;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::type DGVertexMap;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::const_type
  DGConstVertexMap;
}

namespace {
  void optimize_restraint(Restraint *r, RestraintSet *p,
                          const DependencyGraph &dg,
                          const IMP::internal::Map<Object*,
                          unsigned int> &index,
                          const ParticlesTemp &pst,
                          boost::ptr_vector<ScopedRemoveRestraint> &removed,
                          boost::ptr_vector<ScopedRestraint> &added) {
    // containers can be particles, skip them if they are input particles
    ContainersTemp ic= r->get_input_containers();
    for (unsigned int i=0; i< ic.size(); ++i) {
      bool found=false;
      for (unsigned int j=0; j< pst.size(); ++j){
        if (pst[j]==ic[i]) {
          found=true;
          break;
        }
      }
      if (!found) {
        int start=index.find(ic[i])->second;
        if (IMP::internal::get_has_ancestor(dg, start, pst)) {
          IMP_LOG(TERSE, "Container " << ic[i]->get_name()
                  << " has an optimized anscestor" << std::endl);
          return;
        }
      }
    }
    Restraints rs= r->get_decomposition();
    if (rs.size()==0 || (rs.size() >=1 && rs[0] != r)) {
      IMP_LOG(TERSE, "Restraint \"" << r->get_name()
              << "\" is being decompsed into " << rs.size() << " restraints"
              << std::endl);
      IMP_NEW(RestraintSet, rss, (r->get_name()));
      rss->add_restraints(rs);
      double max= r->get_model()->get_maximum_score(r);
      if (max < std::numeric_limits<double>::max()) {
        for (unsigned int i=0; i< rs.size(); ++i) {
          r->get_model()->set_maximum_score(rs[i], max);
        }
      }
      removed.push_back(new ScopedRemoveRestraint(r, p));
      added.push_back(new ScopedRestraint(rss, p));
    } else {
    }
  }

  void optimize_restraint_parent(RestraintSet *p,
                                 const DependencyGraph &dg,
                                 const IMP::internal::Map<Object*,
                                 unsigned int> &index,
                                 const ParticlesTemp &pst,
                           boost::ptr_vector<ScopedRemoveRestraint> &removed,
                                 boost::ptr_vector<ScopedRestraint> &added) {
    Restraints all(p->restraints_begin(), p->restraints_end());
    for (unsigned int i=0; i < all.size(); ++i) {
      Restraint *r=all[i];
      RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
      if (rs) {
        optimize_restraint_parent(rs, dg, index, pst,
                                  removed,
                                  added);
      } else {
        optimize_restraint(all[i], p, dg, index, pst,
                           removed, added);
      }
    }
  }

  void make_static_cpc(core::internal::CoreClosePairContainer *cpc,
                       const ParticlesTemp &optimized,
                       const ParticleStatesTable *pst,
                       core::internal::CoreClosePairContainers &staticed) {
    IMP_LOG(TERSE, "Making container \"" << cpc->get_name()
            << "\" static " << std::endl);
    ParticleStatesList psl(optimized.size());
    unsigned int max=0;
    for (unsigned int i=0; i< optimized.size(); ++i) {
      psl[i]= pst->get_particle_states(optimized[i]);
      max= std::max(psl[i]->get_number_of_particle_states(), max);
    }
    ParticlesTemp inputs
      = cpc->get_singleton_container()->get_contained_particles();
    algebra::BoundingBox3Ds bbs(inputs.size());
    for (unsigned int i=0; i< max; ++i) {
      for (unsigned int j=0; j< optimized.size(); ++j) {
        psl[j]->load_particle_state(std::min(i,
                    psl[j]->get_number_of_particle_states()-1),
                           optimized[j]);
      }
      // make sure invariants are updated
      cpc->get_model()->update();
      for (unsigned int j=0; j< inputs.size(); ++j) {
        core::XYZ d(inputs[j]);
        bbs[j]+= d.get_coordinates();
      }
    }
    for (unsigned int j=0; j< inputs.size(); ++j) {
      core::XYZR d(inputs[j]);
      bbs[j]+= d.get_radius();
    }
    cpc->set_is_static(true, bbs);
    staticed.push_back(cpc);
    IMP_LOG(VERBOSE, cpc->get_number_of_particle_pairs()
            << "pairs" << std::endl);
  }

  void optimize_container_parent(RestraintSet *p,
                                 const DependencyGraph &dg,
                                 const ParticlesTemp &optimized,
                                 const ParticleStatesTable *pst,
                   core::internal::CoreClosePairContainers &staticed) {
    DGConstVertexMap pm=boost::get(boost::vertex_name, dg);
    ParticlesTemp particles=pst->get_particles();
    for (std::pair< DGTraits::vertex_iterator,
            DGTraits::vertex_iterator> be
           = boost::vertices(dg); be.first != be.second; ++be.first) {
      core::internal::CoreClosePairContainer *cpc
        = dynamic_cast<core::internal::CoreClosePairContainer*>(pm[*be.first]);
      if (cpc && IMP::internal::get_has_ancestor(dg, *be.first, particles)) {
        make_static_cpc(cpc, optimized,pst, staticed);
      }
    }
  }
}


void OptimizeRestraints::optimize_model(RestraintSet *m,
                                        const ParticlesTemp &particles) {
  IMP::internal::Map<Object*, unsigned int> index;
  //std::cout << "new gra[j is \n";
  //IMP::internal::show_as_graphviz(m->get_dependency_graph(), std::cout);
  const DependencyGraph dg
    =get_dependency_graph(get_restraints(m));
  DGConstVertexMap vm= boost::get(boost::vertex_name,dg);
  unsigned int nv=boost::num_vertices(dg);
  for (unsigned int i=0; i< nv; ++i) {
    index[vm[i]]= i;
  }
  optimize_restraint_parent(m,
                            dg, index,
                            particles, removed_,  added_);
}



void OptimizeContainers::optimize_model(RestraintSet *m,
                                        const ParticleStatesTable *pst) {
  optimize_container_parent(m,
                            get_dependency_graph(get_restraints(m)),
                            pst->get_particles(), pst, staticed_);
}

void OptimizeContainers::unoptimize_model() {
  for (unsigned int i=0; i< staticed_.size(); ++i) {
    staticed_[i]->set_is_static(false, algebra::BoundingBox3Ds());
  }
}




IMPDOMINO_END_NAMESPACE
