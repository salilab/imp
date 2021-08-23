/**
 *  \file interaction_graph.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/analysis.h>
#include <IMP/core/XYZ.h>
#include <IMP/statistics/Embedding.h>
#include <IMP/statistics/PartitionalClusteringWithCenter.h>
#include <IMP/statistics/point_clustering.h>
#include <IMP/domino/particle_states.h>

IMPDOMINO_BEGIN_NAMESPACE

namespace {

class ParticleStatesEmbedding : public statistics::Embedding {
  Pointer<Particle> p_;
  Pointer<ParticleStates> ps_;
  Ints allowed_;

 public:
  ParticleStatesEmbedding(Particle *p, ParticleStates *ps,
                          const Ints &allowed, std::string name)
      : Embedding(name), p_(p), ps_(ps), allowed_(allowed) {}
  algebra::VectorKD get_point(unsigned int i) const IMP_OVERRIDE;
  algebra::VectorKDs get_points() const IMP_OVERRIDE {
    algebra::VectorKDs ret(get_number_of_items());
    for (unsigned int i = 0; i < ret.size(); ++i) {
      ret[i] = ParticleStatesEmbedding::get_point(i);
    }
    return ret;
  }
  unsigned int get_number_of_items() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ParticleStatesEmbedding);
};

algebra::VectorKD ParticleStatesEmbedding::get_point(unsigned int i) const {
  ps_->load_particle_state(allowed_[i], p_);
  core::XYZ d(p_);
  Floats ret(3, 0.0);
  ret[0] = d.get_coordinate(0);
  ret[1] = d.get_coordinate(1);
  ret[2] = d.get_coordinate(2);
  return algebra::VectorKD(ret.begin(), ret.end());
}
unsigned int ParticleStatesEmbedding::get_number_of_items() const {
  return allowed_.size();
}

Ints get_state_clusters(Particle *p, ParticleStates *ps,
                        const Ints &allowed_states, double resolution) {
  IMP::PointerMember<ParticleStatesEmbedding> pse =
      new ParticleStatesEmbedding(p, ps, allowed_states, "domino embedding");
  IMP::PointerMember<statistics::PartitionalClusteringWithCenter> c =
      create_connectivity_clustering(pse, resolution);
  Ints ret(ps->get_number_of_particle_states(), -1);
  IMP_LOG_TERSE("For particle " << p->get_name() << " there are "
                                << allowed_states.size()
                                << " states which clustered to "
                                << c->get_number_of_clusters() << std::endl);
  for (unsigned int i = 0; i < c->get_number_of_clusters(); ++i) {
    Ints is = c->get_cluster(i);
    int rep = allowed_states[c->get_cluster_representative(i)];
    for (unsigned int j = 0; j < is.size(); ++j) {
      ret[allowed_states[is[j]]] = rep;
    }
  }
  return ret;
}

Assignments filter_states(const Subset &, const Assignments &in,
                          const Vector<Ints> &clustering) {
  Assignments ret;
  for (unsigned int i = 0; i < in.size(); ++i) {
    Ints cur(in[i].size(), -1);
    for (unsigned int j = 0; j < in [i].size(); ++j) {
      cur[j] = clustering[j][in[i][j]];
    }
    Assignment curs(cur.begin(), cur.end());
    ret.push_back(curs);
  }
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  IMP_LOG_TERSE("Filtered from " << in.size() << " to " << ret.size()
                                 << std::endl);
  return ret;
}
}
Assignments get_state_clusters(const Subset &subset, const Assignments &states,
                               ParticleStatesTable *pst, double resolution) {
  Vector<Ints> rotated(subset.size(), Ints(states.size(), -1));
  for (unsigned int i = 0; i < states.size(); ++i) {
    for (unsigned int j = 0; j < states[i].size(); ++j) {
      rotated[j][i] = states[i][j];
    }
  }
  for (unsigned int i = 0; i < rotated.size(); ++i) {
    std::sort(rotated[i].begin(), rotated[i].end());
    rotated[i].erase(std::unique(rotated[i].begin(), rotated[i].end()),
                     rotated[i].end());
  }
  Vector<Ints> clustering(states.size());
  for (unsigned int i = 0; i < subset.size(); ++i) {
    IMP::PointerMember<ParticleStates> ps =
        pst->get_particle_states(subset[i]);
    Ints c = get_state_clusters(subset[i], ps, rotated[i], resolution);
    clustering[i] = c;
  }

  return filter_states(subset, states, clustering);
}

IMPDOMINO_END_NAMESPACE
