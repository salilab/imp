/**
 *  \file example/complex_assembly.h
 *  \brief A simple unary function.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */


#ifndef IMPEXAMPLE_COMPLEX_ASSEMBLY_H
#define IMPEXAMPLE_COMPLEX_ASSEMBLY_H

#include "example_config.h"
#include "optimizing.h"
#include <IMP/container/generic.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/SerialMover.h>
#include <IMP/core/BallMover.h>
#include <IMP/core/HarmonicUpperBound.h>
#include <IMP/core/BoundingBox3DSingletonScore.h>
#include <IMP/container/ClosePairContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/generic.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/domino/subset_graphs.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <IMP/scoped.h>


IMPEXAMPLE_BEGIN_NAMESPACE

#ifndef SWIG
class AssemblyData {
  ParticlesTemp ps_;
  compatibility::map<Particle*, int> index_;
  Restraints rs_;
  ParticlesTemps particles_;
  domino::InteractionGraph interactions_;

  int get_degree(unsigned int i, const ParticlesTemp &ps) const {
    int ret=0;
    domino::InteractionGraphConstVertexName vm= boost::get(boost::vertex_name,
                                                      interactions_);
    std::pair<domino::InteractionGraphTraits::adjacency_iterator,
      domino::InteractionGraphTraits::adjacency_iterator> be
      = boost::adjacent_vertices(i, interactions_);
    for (; be.first != be.second; ++be.first) {
      if (std::binary_search(ps.begin(), ps.end(), vm[*be.first])) {
        ++ret;
      }
    }
    return ret;
  }
 public:
  AssemblyData(ParticlesTemp ps,
               const RestraintsTemp &rs): ps_(ps), rs_(rs.begin(), rs.end()) {
    std::sort(ps_.begin(), ps_.end());
    interactions_= domino::get_interaction_graph(rs, ps);
    for (unsigned int i=0; i< rs.size(); ++i) {
      ParticlesTemp cur=rs[i]->get_input_particles();
      std::sort(cur.begin(), cur.end());
      cur.erase(std::unique(cur.begin(), cur.end()), cur.end());
      ParticlesTemp used;
      std::set_intersection(cur.begin(), cur.end(), ps_.begin(), ps_.end(),
                            std::back_inserter(used));
      particles_.push_back(used);
    }
  }
  RestraintsTemp get_restraints( ParticlesTemp ps) const {
    std::sort(ps.begin(), ps.end());
    RestraintsTemp ret;
    for (unsigned int i=0; i< rs_.size(); ++i) {
      ParticlesTemp used;
      std::set_intersection(particles_[i].begin(), particles_[i].end(),
                            ps.begin(), ps.end(),
                            std::back_inserter(used));
      if (used.size() == particles_[i].size()) {
        ret.push_back(rs_[i]);
      }
    }
    return ret;
  }
  Particle* get_highest_degree_unused_particle( ParticlesTemp ps) const {
    std::sort(ps.begin(), ps.end());
    int md=0;
    int mi=-1;
    domino::InteractionGraphConstVertexName vm= boost::get(boost::vertex_name,
                                                           interactions_);
    for (unsigned int i=0; i< boost::num_vertices(interactions_); ++i) {
      if (std::binary_search(ps.begin(), ps.end(), vm[i])) continue;
      int degree= get_degree(i, ps);
      if (degree >= md) {
        md=degree;
        mi= i;
      }
    }
    return vm[mi];
  }
};
#endif


inline void optimize_assembly(Model *m, const ParticlesTemp &components,
                       const RestraintsTemp &interactions,
                       const RestraintsTemp &other_restraints,
                       const algebra::BoundingBox3D &bb,
                       PairScore *ev,
                       double cutoff,
                       const PairFilters &excluded=PairFilters()) {
  IMP_NEW(core::ConjugateGradients, cg, (m));
  IMP_NEW(core::MonteCarlo, mc, (m));
  mc->set_log_level(IMP::SILENT);
  mc->set_use_incremental_evaluate(true);
  AssemblyData ad(components, interactions);
  ParticlesTemp cur;
  IMP_NEW(container::ListSingletonContainer, active, (m));
  // fix distance
  IMP_NEW(container::ClosePairContainer, cpc, (active, 0, 4));
  cpc->set_pair_filters(excluded);
  IMP_NEW(core::SoftSpherePairScore, ssps, (10));
  Pointer<Restraint> evr= container::create_restraint(ssps.get(), cpc.get());
  evr->set_model(m);
  IMP_NEW(core::HarmonicUpperBound, hub, (0,10));
  IMP_NEW(core::BoundingBox3DSingletonScore, bbss, (hub, bb));
  Pointer<Restraint> bbr= container::create_restraint(bbss.get(), active.get());
  bbr->set_model(m);
  do {
    Particle *add= ad.get_highest_degree_unused_particle(cur);
    cur.push_back(add);
    core::XYZ(add).set_coordinates(algebra::get_random_vector_in(bb));
    mc->clear_movers();
    mc->add_mover(create_serial_mover(cur));
    mc->set_close_pair_score(ev, 0,cur, excluded);
    RestraintsTemp rs= other_restraints+ad.get_restraints(cur);
    IMP_LOG(TERSE, "Current restraints are " << rs
            << " and particles " << cur << std::endl);
    mc->set_restraints(rs);
    cg->set_restraints(rs+RestraintsTemp(1, evr.get())
                       +RestraintsTemp(1, bbr.get()));
    active->set_particles(cur);
    double e;
    for (int j=0; j< 5; ++j) {
      mc->set_kt(100.0/(3*j+1));
      mc->optimize(cur.size()*(j+1)*1000);
      e=cg->optimize(10*cur.size());
      IMP_LOG(PROGRESS, "Energy is " << e << std::endl);
    }
    if (e > cutoff) {
      IMP_THROW("Failure to converge", ModelException);
    }
  } while (cur.size() != components.size());

}

IMPEXAMPLE_END_NAMESPACE

#endif  /* IMPEXAMPLE_COMPLEX_ASSEMBLY_H */
