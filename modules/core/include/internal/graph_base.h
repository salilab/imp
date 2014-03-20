/**
 *  \file graph_base.h     \brief Internal use only.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_GRAPH_BASE_H
#define IMPCORE_GRAPH_BASE_H

#include <IMP/core/core_config.h>

#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>

#include <vector>
#include <set>

IMPCORE_BEGIN_NAMESPACE

namespace internal {

/** \internal */
struct IMPCOREEXPORT GraphData {
  GraphData() {}
  GraphData(std::string prefix) {
    setup_key_ = kernel::IntKey((prefix + " setup").c_str());
    node_keys_[0] = kernel::ParticleIndexKey((prefix + " node 0").c_str());
    node_keys_[1] = kernel::ParticleIndexKey((prefix + " node 1").c_str());
    edges_key_ = kernel::ParticleIndexesKey(prefix + " edges");
  }
  kernel::IntKey setup_key_;
  kernel::ParticleIndexKey node_keys_[2];
  kernel::ParticleIndexesKey edges_key_;
};

/** \internal */
IMPCOREEXPORT void graph_initialize_node(kernel::Particle *a,
                                         const GraphData &d);

/** \internal */
IMPCOREEXPORT bool graph_is_node(kernel::Particle *a, const GraphData &d);

/** \internal */
IMPCOREEXPORT kernel::Particle *graph_connect(kernel::Particle *a,
                                              kernel::Particle *b,
                                              GraphData &d);

/** \internal */
IMPCOREEXPORT void graph_disconnect(kernel::Particle *bond, const GraphData &d);

/** \internal */
IMPCOREEXPORT kernel::Particle *graph_get_edge(kernel::Particle *a, int i,
                                               const GraphData &d);

/** \internal */
IMPCOREEXPORT kernel::ParticleIndexes graph_get_edges(kernel::Particle *a,
                                                      const GraphData &d);

IMPCOREEXPORT kernel::Particle *graph_get_neighbor(kernel::Particle *a, int i,
                                                   const GraphData &d);

/** \internal */
IMPCOREEXPORT unsigned int graph_get_number_of_edges(kernel::Particle *a,
                                                     const GraphData &d);

/** \internal */
inline kernel::Particle *graph_get_node(kernel::Particle *a, int i,
                                        const GraphData &d) {
  IMP_INTERNAL_CHECK(i < 2, "bad node requested");
  return a->get_value(d.node_keys_[i]);
}

/** \internal */
inline bool graph_is_edge(kernel::Particle *a, const GraphData &d) {
  IMP_INTERNAL_CHECK((a->has_attribute(d.node_keys_[0]) &&
                      a->has_attribute(d.node_keys_[1])) ||
                         (!a->has_attribute(d.node_keys_[0]) &&
                          !a->has_attribute(d.node_keys_[1])),
                     "Potential graph edge is in invalid state.");
  return a->has_attribute(d.node_keys_[0]);
}

/** \internal */
IMPCOREEXPORT void graph_initialize_edge(kernel::Particle *a,
                                         const GraphData &d);

/** \internal */
template <class F>
inline F graph_traverse(kernel::Particle *start, F f, const GraphData &d) {
  std::set<kernel::Particle *> visited;
  base::Vector<kernel::Particle *> queue;
  queue.push_back(start);
  visited.insert(start);
  do {
    kernel::Particle *cur = queue.back();
    queue.pop_back();
    f(cur);
    for (unsigned int i = 0; i < graph_get_number_of_edges(cur, d); ++i) {
      kernel::Particle *n = graph_get_neighbor(cur, i, d);
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
struct GraphGather {
  GraphGather(Oit out) : out_(out) {}
  void operator()(kernel::Particle *p) {
    *out_ = p;
    ++out_;
  }

  Oit get_out() const { return out_; }
  Oit out_;
};

/** \internal */
template <class Oit>
inline void graph_connected_component(kernel::Particle *start,
                                      const GraphData &d, Oit out) {
  graph_traverse(start, GraphGather<Oit>(out), d);
}

}  // namespace internal

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_GRAPH_BASE_H */
