/**
 *  \file graph_base.h     \brief Internal use only.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_GRAPH_BASE_H
#define IMPCORE_GRAPH_BASE_H

#include <IMP/core/core_config.h>

#include <IMP/Particle.h>
#include <IMP/Model.h>

#include <vector>
#include <set>


IMPCORE_BEGIN_NAMESPACE

namespace internal
{

/** \internal */
struct IMPCOREEXPORT GraphData
{
  GraphData() {}
  GraphData(std::string prefix) {
    node_keys_[0]=ParticleIndexKey((prefix+" node 0").c_str());
    node_keys_[1]=ParticleIndexKey((prefix+" node 1").c_str());
    edges_key_=ParticleIndexesKey(prefix+" edges");
  }
  ParticleIndexKey node_keys_[2];
  ParticleIndexesKey edges_key_;
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

/** \internal */
IMPCOREEXPORT ParticleIndexes graph_get_edges(Particle* a,
                                           const GraphData &d);

IMPCOREEXPORT Particle* graph_get_neighbor(Particle* a, int i,
                                          const GraphData &d);

/** \internal */
IMPCOREEXPORT unsigned int graph_get_number_of_edges(Particle *a,
                                                    const GraphData &d);

/** \internal */
inline Particle* graph_get_node(Particle *a, int i,
                                      const GraphData &d)
{
  IMP_INTERNAL_CHECK(i<2, "bad node requested");
  return a->get_value(d.node_keys_[i]);
}


/** \internal */
inline bool graph_is_edge(Particle *a, const GraphData &d) {
  IMP_INTERNAL_CHECK((a->has_attribute(d.node_keys_[0])
              && a->has_attribute(d.node_keys_[1])) ||
             (!a->has_attribute(d.node_keys_[0])
              && !a->has_attribute(d.node_keys_[1])),
             "Potential graph edge is in invalid state.");
  return a->has_attribute(d.node_keys_[0]);
}

/** \internal */
IMPCOREEXPORT void graph_initialize_edge(Particle *a, const GraphData &d);

/** \internal */
template <class F>
inline F graph_traverse(Particle *start, F f, const GraphData &d)
{
  std::set<Particle*> visited;
  base::Vector<Particle*> queue;
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
inline void graph_connected_component(Particle* start,
                                      const GraphData &d, Oit out)
{
  graph_traverse(start, GraphGather<Oit>(out), d);
}

} // namespace internal

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_GRAPH_BASE_H */
