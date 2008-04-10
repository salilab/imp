/**
 *  \file graph_base.h     \brief Internal use only.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_GRAPH_BASE_H
#define __IMP_GRAPH_BASE_H

#include <vector>
#include <set>

#include "../Particle.h"
#include "../Model.h"

namespace IMP
{

namespace internal
{

/** \internal */
struct IMPDLLEXPORT GraphData
{
  GraphData() {}
  GraphData(const char *prefix): prefix_(prefix) {
    node_keys_[0]=ParticleKey((prefix_+" node 0").c_str());
    node_keys_[1]=ParticleKey((prefix_+" node 1").c_str());
    num_edges_key_= IntKey((prefix_+" num edges").c_str());
  }
  ParticleKey node_keys_[2];
  IntKey num_edges_key_;
  mutable std::vector<ParticleKey> edge_keys_;
  std::string prefix_;
};

/** \internal */
IMPDLLEXPORT Particle* graph_connect(Particle* a, Particle* b,
                                     const GraphData &d);

/** \internal */
IMPDLLEXPORT void graph_disconnect(Particle* bond,
                                   const GraphData &d);

/** \internal */
IMPDLLEXPORT Particle* graph_get_edge(Particle* a, int i,
                                      const GraphData &d);

IMPDLLEXPORT Particle* graph_get_neighbor(Particle* a, int i,
                                          const GraphData &d);

/** \internal */
IMPDLLEXPORT unsigned int graph_get_number_of_edges(Particle *a,
                                                    const GraphData &d);

/** \internal */
IMPDLLEXPORT Particle* graph_get_node(Particle *a, int i,
                                      const GraphData &d);

/** \internal */
IMPDLLEXPORT bool graph_is_edge(Particle *a, const GraphData &d);

/** \internal */
template <class F>  
F graph_traverse(Particle *start, F f, const GraphData &d)
{
  std::set<Particle*> visited;
  std::vector<Particle*> queue;
  queue.push_back(start);
  visited.insert(start);
  do {
    Particle *cur= queue.back();
    queue.pop_back();
    f(cur);
    for (unsigned int i=0; i< graph_get_number_of_edges(cur, d); ++i) {
      Particle *n= graph_get_neighbor(cur, i, d);
      if (visited.find(n) == visited.end()) {
        visited.insert(n);
        queue.push_back(n);
      }
    }
  } while (!queue.empty());
  return f;
}

/** \internal */
template <class Oit>
struct GraphGather
{
  GraphGather(Oit out): out_(out){}
  void operator()( Particle *p)
  {
    *out_=p;
    ++out_;
  }

  Oit get_out() const {return out_;}
  Oit out_;
};

/** \internal */
template <class Oit>
void graph_connected_component(Particle* start, const GraphData &d, Oit out)
{
  graph_traverse(start, GraphGather<Oit>(out), d);
}

} // namespace internal

} // namespace IMP

#endif  /* __IMP_GRAPH_BASE_H */
