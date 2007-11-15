/**
 *  \file graph_base.h     \brief Internal use only.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_GRAPH_DECORATORS_H
#define __IMP_GRAPH_DECORATORS_H


#include "../Particle.h"
#include "../Model.h"
#include "utility.h"
#include <vector>

namespace IMP
{

namespace internal
{

struct IMPDLLEXPORT GraphData {
  GraphData() {}
  GraphData(const char *prefix): prefix_(prefix) {
    node_keys_[0]=IntKey((prefix_+" node 0").c_str());
    node_keys_[1]=IntKey((prefix_+" node 1").c_str());
    num_edges_key_= IntKey((prefix_+" num edges").c_str());
  }
  IntKey node_keys_[2];
  IntKey num_edges_key_;
  mutable std::vector<IntKey> edge_keys_;
  std::string prefix_;
};



IMPDLLEXPORT ParticleIndex graph_connect(Particle* a, Particle* b,
                                         const GraphData &d);

IMPDLLEXPORT ParticleIndex graph_get_edge(Particle* a, int i,
                                          const GraphData &d);

IMPDLLEXPORT unsigned int graph_get_number_of_edges(Particle *a,
                                                    const GraphData &d);

IMPDLLEXPORT ParticleIndex graph_get_node(Particle *a, int i,
                                          const GraphData &d);

IMPDLLEXPORT bool graph_is_edge(Particle *a, const GraphData &d);

} // namespace internal

} // namespace IMP

#endif  /* __IMP_GRAPH_DECORATORS_H */
