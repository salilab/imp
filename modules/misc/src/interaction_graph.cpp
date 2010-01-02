/**
 *  \file interaction_graph.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/misc/interaction_graph.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/ScoreState.h>
#include <map>

IMPMISC_BEGIN_NAMESPACE

typedef boost::graph_traits<InteractionGraph>::edge_descriptor Edge;
typedef boost::graph_traits<InteractionGraph>::vertex_descriptor Vertex;

typedef boost::property_map<InteractionGraph,
                            boost::vertex_name_t>::type ParticleMap;
typedef boost::property_map<InteractionGraph,
                            boost::edge_name_t>::type ObjectMap;

namespace {
  void add_edges(const ParticlesList &pl, const std::map<Particle*, int> &map,
                 Object *blame,
                 InteractionGraph &g) {
    ObjectMap om= boost::get(boost::edge_name, g);
    for (unsigned int i=0; i< pl.size(); ++i) {
      for (unsigned int j=0; j< pl[i].size(); ++j) {
        Vertex vj=map.find(pl[i][j])->second;
        for (unsigned int k=0; k< j; ++k) {
          Vertex vk=map.find(pl[i][k])->second;
          Edge e;
          bool inserted;
          boost::tie(e, inserted)= boost::add_edge(vj, vk, g);
          if (inserted) {
            om[e]=blame;
          }
        }
      }
    }
  }
}

InteractionGraph get_interaction_graph(Model *m) {
  InteractionGraph ret(m->get_number_of_particles());
  ParticleMap pm= boost::get(boost::vertex_name, ret);
  std::map<Particle*, int> map;
  for (Model::ParticleIterator it= m->particles_begin();
       it != m->particles_end(); ++it) {
    int i= map.size();
    map[*it]= i;
    pm[i]= *it;
  }
  for (Model::RestraintIterator it= m->restraints_begin();
       it != m->restraints_end(); ++it) {
    ParticlesList pl= (*it)->get_interacting_particles();
    add_edges(pl, map, *it, ret);
  }
  for (Model::ScoreStateIterator it= m->score_states_begin();
       it != m->score_states_end(); ++it) {
    ParticlesList pl= (*it)->get_interacting_particles();
    add_edges(pl, map, *it, ret);
  }
  return ret;
}

IMPMISC_END_NAMESPACE
