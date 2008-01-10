/**
 *  \file graph_base.cpp   \brief classes for implementing a graph.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <sstream>

#include "IMP/decorators/graph_base.h"

namespace IMP
{

namespace internal
{

static void graph_add_edge_key(unsigned int i, const GraphData &d)
{
  while (i >= d.edge_keys_.size()) {
    std::ostringstream oss;
    oss << d.prefix_ << " edge " << i;
    d.edge_keys_.push_back(IntKey(oss.str().c_str()));
  }
}

static IntKey graph_get_edge_key(unsigned int i, const GraphData &d)
{
  if (i >= d.edge_keys_.size()) graph_add_edge_key(i, d);
  return d.edge_keys_[i];
}

Particle* graph_connect(Particle* a, Particle* b, const GraphData &d)
{
  Model *m= a->get_model();
  Particle *p= new Particle();
  ParticleIndex pi=m->add_particle(p);
  p->add_attribute(d.node_keys_[0], a->get_index().get_index());
  p->add_attribute(d.node_keys_[1], b->get_index().get_index());
  {
    int nc= graph_get_number_of_edges(a, d);
    IntKey nm=graph_get_edge_key(nc, d);
    a->add_attribute(nm, p->get_index().get_index());
    if (a->has_attribute(d.num_edges_key_)) {
      a->set_value(d.num_edges_key_, nc+1);
    } else {
      a->add_attribute(d.num_edges_key_, nc+1);
    }
  }
  {
    int nc= graph_get_number_of_edges(b, d);
    IntKey nm=graph_get_edge_key(nc, d);
    b->add_attribute(nm, p->get_index().get_index());
    if (b->has_attribute(d.num_edges_key_)) {
      b->set_value(d.num_edges_key_, nc+1);
    } else {
      b->add_attribute(d.num_edges_key_, nc+1);
    }
  }

  return a->get_model()->get_particle(pi);
}

Particle* graph_get_edge(Particle* a, int i, const GraphData &d)
{
  IntKey nm= graph_get_edge_key(i, d);
  return a->get_model()->get_particle(ParticleIndex(a->get_value(nm)));
}

Particle* graph_get_neighbor(Particle* a, int i, const GraphData &d)
{
  IntKey nm= graph_get_edge_key(i, d);
  Particle *edge= a->get_model()->get_particle(ParticleIndex(a->get_value(nm)));
  if (graph_get_node(edge, 0, d) == a) {
    return graph_get_node(edge, 1, d);
  } else {
    IMP_assert(graph_get_node(edge, 1, d) == a,
               "Broken graph");
    return graph_get_node(edge, 0, d);
  }
}

unsigned int graph_get_number_of_edges(Particle *a, const GraphData &d)
{
  if (a->has_attribute(d.num_edges_key_)) {
    return a->get_value(d.num_edges_key_);
  } else {
    return 0;
  }
}

Particle* graph_get_node(Particle *a, int i, const GraphData &d)
{
  IMP_assert(i<2, "bad node requested");
  return a->get_model()->get_particle(a->get_value(d.node_keys_[i]));
}

bool graph_is_edge(Particle *a, const GraphData &d)
{
  return a->has_attribute(d.node_keys_[0])
         && a->has_attribute(d.node_keys_[1]);
}

} // namespace internal

} // namespace IMP
