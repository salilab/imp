/**
 *  \file interaction_graph.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino2/utility.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/ScoreState.h>
#include <map>

IMPDOMINO2_BEGIN_NAMESPACE


namespace {
typedef boost::graph_traits<InteractionGraph> IGTraits;
typedef IGTraits::edge_descriptor Edge;
typedef IGTraits::vertex_descriptor Vertex;

typedef boost::property_map<InteractionGraph,
                            boost::vertex_name_t>::type ParticleMap;
typedef boost::property_map<InteractionGraph,
                            boost::edge_name_t>::type ObjectMap;

  void add_edges(const ParticlesList &pl, const std::map<Particle*, int> &map,
                 Object *blame,
                 InteractionGraph &g) {
    ObjectMap om= boost::get(boost::edge_name, g);
    for (unsigned int i=0; i< pl.size(); ++i) {
      ParticlesTemp pt= pl[i];
      std::sort(pt.begin(), pt.end());
      pt.erase(std::unique(pt.begin(), pt.end()), pt.end());
      for (unsigned int j=0; j< pt.size(); ++j) {
        int vj=map.find(pt[j])->second;
        for (unsigned int k=0; k< j; ++k) {
          int vk= map.find(pt[k])->second;
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
  typedef boost::graph_traits<Model::DependencyGraph> DTraits;
  typedef DTraits::vertex_descriptor DVertex;


  Ints find_parents(const std::map<Particle*, Ints>  &map,
                    DVertex v,
                    Model::DependencyGraph &dg) {
    std::vector<DVertex> front;
    front.push_back(v);
    Ints ret;
    typedef boost::property_map<Model::DependencyGraph,
      boost::vertex_name_t>::type DParticleMap;
    DParticleMap dpm= boost::get(boost::vertex_name, dg);
    do {
      DVertex v= front.back();
      front.pop_back();
      Object *o= boost::get(dpm, v);
      Particle *p=dynamic_cast<Particle*>(o);
      if (p && map.find(p) != map.end()) {
        ret.insert(ret.end(), map.find(p)->second.begin(),
                  map.find(p)->second.end());
      } else {
        typedef DTraits::in_edge_iterator InIt;
        std::pair<InIt, InIt> be= boost::in_edges(v, dg);
        IMP_USAGE_CHECK(std::distance(be.first, be.second)>0,
                        "Encountered leaf which was not known particle "
                        <<o->get_name());
        for (; be.first != be.second; ++be.first) {
          DVertex s= boost::source(*be.first, dg);
          front.push_back(s);
        }
      }
    } while (!front.empty());
    std::sort(ret.begin(), ret.end());
    ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
    return ret;
  }

}

InteractionGraph get_interaction_graph(Model *m,
                                       const ParticlesTemp &ps) {
  InteractionGraph ret(ps.size());
  std::map<Particle*, int> map;
  ParticleMap pm= boost::get(boost::vertex_name, ret);
  Model::DependencyGraph dg= m->get_dependency_graph();
  for (unsigned int i=0; i< ps.size(); ++i) {
    ParticlesTemp t= get_dependent_particles(ps[i], dg);
    for (unsigned int j=0; j< t.size(); ++j) {
      IMP_USAGE_CHECK(map.find(t[j]) == map.end(),
                      "Currently particles which depend on more "
                      << "than one particle "
                      << "from the input set are not supported."
                      << "  Particle " << t[j]->get_name()
                      << " depends on " << ps[i]->get_name()
                      << " and " << ps[map.find(t[i])->second]->get_name());
      map[t[j]]= i;
    }
    pm[i]= ps[i];
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

ParticlesTemp get_dependent_particles(Particle *p,
                                      Model::DependencyGraph &dg) {
  // find p in graph, ick
  typedef boost::property_map<Model::DependencyGraph,
    boost::vertex_name_t>::type DParticleMap;
  DParticleMap dpm= boost::get(boost::vertex_name, dg);
  typedef DTraits::vertex_iterator InIt;
  std::pair<InIt, InIt> be= boost::vertices(dg);
  std::vector<DVertex> front;
  for (; be.first != be.second; ++be.first) {
    if (dpm[*be.first]==p) {
      front.push_back(*be.first);
      break;
    }
  }
  ParticlesTemp ret(1,p);
  do {
    DVertex v= front.back();
    front.pop_back();
    Object *o= boost::get(dpm, v);
    Particle *p=dynamic_cast<Particle*>(o);
    if (p) {
      ret.push_back(p);
    }
    typedef DTraits::out_edge_iterator InIt;
    std::pair<InIt, InIt> be= boost::out_edges(v, dg);
    for (; be.first != be.second; ++be.first) {
      DVertex s= boost::target(*be.first, dg);
      front.push_back(s);
    }
  } while (!front.empty());
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
  }

ParticlesTemp get_dependent_particles(Particle *p) {
  Model *m= p->get_model();
  Model::DependencyGraph dg= m->get_dependency_graph();
  return get_dependent_particles(p, dg);
}

IMPDOMINO2_END_NAMESPACE
