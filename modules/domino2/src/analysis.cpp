/**
 *  \file interaction_graph.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino2/analysis.h>
#include <IMP/statistics/point_clustering.h>
#include <IMP/domino2/particle_states.h>

IMPDOMINO2_BEGIN_NAMESPACE

namespace {

  class ParticleStatesEmbedding: public statistics::Embedding {
    IMP::Pointer<Particle> p_;
    IMP::Pointer<ParticleStates> ps_;
    Ints allowed_;
  public:
    ParticleStatesEmbedding(Particle *p,
                            ParticleStates *ps,
                            const Ints &allowed,
                            std::string name): Embedding(name),
                                               p_(p), ps_(ps),
                                               allowed_(allowed){}
    IMP_EMBEDDING(ParticleStatesEmbedding);
  };
  void ParticleStatesEmbedding::do_show(std::ostream &out) const {

  }
  Floats ParticleStatesEmbedding::get_point(unsigned int i) const {
    ps_->load_particle_state(allowed_[i], p_);
    core::XYZ d(p_);
    Floats ret(3,0.0);
    ret[0]= d.get_coordinate(0);
    ret[1]= d.get_coordinate(1);
    ret[2]= d.get_coordinate(2);
    return ret;
  }
  unsigned int ParticleStatesEmbedding::get_number_of_points() const {
    return allowed_.size();
  }

  Ints
  get_state_clusters(Particle *p,
                     ParticleStates *ps,
                     const Ints &allowed_states,
                     double resolution) {
    IMP::internal::OwnerPointer<ParticleStatesEmbedding> pse
      = new ParticleStatesEmbedding(p, ps, allowed_states, "domino2 embedding");
    IMP::internal::OwnerPointer<statistics::PartitionalClusteringWithCenter> c
      = get_connectivity_clustering(pse, resolution);
    Ints ret(ps->get_number_of_particle_states(), -1);
    IMP_LOG(TERSE, "For particle " << p->get_name()
            << " there are " << allowed_states.size()
            << " states which clustered to " << c->get_number_of_clusters()
            << std::endl);
    for (unsigned int i=0; i < c->get_number_of_clusters(); ++i) {
      Ints is= c->get_cluster(i);
      int rep= allowed_states[c->get_cluster_representative(i)];
      for (unsigned int j=0; j< is.size(); ++j) {
        ret[allowed_states[is[j]]]=rep;
      }
    }
    return ret;
  }

  SubsetStates filter_states(const Subset &subset,
                             const SubsetStates &in,
                             const std::vector<Ints> &clustering) {
    SubsetStates ret;
    for (unsigned int i=0; i< in.size(); ++i) {
      Ints cur(in[i].size(), -1);
      for (unsigned int j=0; j< in[i].size(); ++j) {
        cur[j]= clustering[j][in[i][j]];
      }
      SubsetState curs(cur.begin(), cur.end());
      ret.push_back(curs);
    }
    std::sort(ret.begin(), ret.end());
    ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
    IMP_LOG(TERSE, "Filtered from " << in.size()
            << " to " << ret.size() << std::endl);
    return ret;
  }
}
SubsetStates get_state_clusters(const Subset &subset,
                                const SubsetStates &states,
                                ParticleStatesTable *pst,
                                double resolution) {
  std::vector<Ints> rotated(subset.size(), Ints(states.size(), -1));
  for (unsigned int i=0; i< states.size(); ++i) {
    for (unsigned int j=0; j< states[i].size(); ++j) {
      rotated[j][i]= states[i][j];
    }
  }
  for (unsigned int i=0; i< rotated.size(); ++i) {
    std::sort(rotated[i].begin(), rotated[i].end());
    rotated[i].erase(std::unique(rotated[i].begin(), rotated[i].end()),
                     rotated[i].end());
  }
  std::vector<Ints> clustering(states.size());
  for (unsigned int i=0; i< subset.size(); ++i) {
    IMP::internal::OwnerPointer<ParticleStates> ps
      =pst->get_particle_states(subset[i]);
    Ints c= get_state_clusters(subset[i], ps,
                            rotated[i], resolution);
    clustering[i]=c;
  }

  return filter_states(subset, states, clustering);
}


IMPDOMINO2_END_NAMESPACE
