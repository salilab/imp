/**
 *  \file graph_base.h     \brief Internal use only.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPCORE_GRAPH_BASE_H
#define IMPCORE_GRAPH_BASE_H

#include "../config.h"
#include "ArrayOnAttributesHelper.h"

#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <set>


IMPCORE_BEGIN_NAMESPACE

namespace internal
{

/** \internal */
struct IMPCOREEXPORT GraphData:
    public ArrayOnAttributesHelper<ParticleKey, Particle*>
{
  typedef ArrayOnAttributesHelper<ParticleKey, Particle*> P;
  GraphData() {}
  GraphData(std::string prefix): P(prefix) {
    node_keys_[0]=ParticleKey((P::get_prefix()+" node 0").c_str());
    node_keys_[1]=ParticleKey((P::get_prefix()+" node 1").c_str());
  }
  ParticleKey node_keys_[2];
};


/** \internal */
IMPCOREEXPORT void graph_initialize_node(Particle* a,
                                        const GraphData &d);

/** \internal */
IMPCOREEXPORT bool graph_is_node(Particle* a,
                                const GraphData &d);


/** \internal */
IMPCOREEXPORT Particle* graph_connect(Particle* a, Particle* b,
                                     GraphData &d);

/** \internal */
IMPCOREEXPORT void graph_disconnect(Particle* bond,
                                   const GraphData &d);

/** \internal */
IMPCOREEXPORT Particle* graph_get_edge(Particle* a, int i,
                                      const GraphData &d);

IMPCOREEXPORT Particle* graph_get_neighbor(Particle* a, int i,
                                          const GraphData &d);

/** \internal */
IMPCOREEXPORT unsigned int graph_get_number_of_edges(Particle *a,
                                                    const GraphData &d);

/** \internal */
IMPCOREEXPORT Particle* graph_get_node(Particle *a, int i,
                                      const GraphData &d);

/** \internal */
IMPCOREEXPORT bool graph_is_edge(Particle *a, const GraphData &d);

/** \internal */
IMPCOREEXPORT void graph_initialize_edge(Particle *a, const GraphData &d);

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

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_GRAPH_BASE_H */
