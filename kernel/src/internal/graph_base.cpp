/**
 *  \file graph_base.cpp   \brief classes for implementing a graph.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <sstream>

#include "IMP/internal/graph_base.h"

namespace IMP
{

namespace internal
{

static void graph_add_edge_key(unsigned int i, const GraphData &d)
{
  while (i >= d.edge_keys_.size()) {
    std::ostringstream oss;
    oss << d.prefix_ << " edge " << i;
    d.edge_keys_.push_back(ParticleKey(oss.str().c_str()));
  }
}

static ParticleKey graph_get_edge_key(unsigned int i, const GraphData &d)
{
  if (i >= d.edge_keys_.size()) graph_add_edge_key(i, d);
  return d.edge_keys_[i];
}

Particle* graph_connect(Particle* a, Particle* b, const GraphData &d)
{
  Model *m= a->get_model();
  Particle *p= new Particle();
  ParticleIndex pi=m->add_particle(p);
  p->add_attribute(d.node_keys_[0], a);
  p->add_attribute(d.node_keys_[1], b);
  for (int i=0; i< 2; ++i) {
    Particle *cp=((i==0)?a:b);
    int nc= graph_get_number_of_edges(cp, d);
    ParticleKey nm=graph_get_edge_key(nc, d);
    if (!cp->has_attribute(nm)) {
      cp->add_attribute(nm, p);
    } else {
      cp->set_value(nm, p);
    }
    if (cp->has_attribute(d.num_edges_key_)) {
      cp->set_value(d.num_edges_key_, nc+1);
    } else {
      cp->add_attribute(d.num_edges_key_, nc+1);
    }
  }

  return a->get_model()->get_particle(pi);
}

void graph_disconnect(Particle* e, const GraphData &d)
{
  ParticleIndex pi=e->get_index();
  Particle *p[2];
  p[0]= graph_get_node(e, 0, d);
  p[1]= graph_get_node(e, 1, d);
  for (int i=0; i< 2; ++i) {
    int shift=0;
    Int nc= p[i]->get_value(d.num_edges_key_);
    for (int j=0; j< nc; ++j) {
      if (graph_get_edge(p[i], j, d) == e) {
        IMP_assert(shift==0, "duplicate edges found in graph_base");
        shift=-1;
      } else {
        Particle* v = p[i]->get_value(graph_get_edge_key(j, d));
        p[i]->set_value(graph_get_edge_key(j+shift, d), v);
      }
    }
    p[i]->remove_attribute(graph_get_edge_key(nc-1, d));
    IMP_assert(shift==-1, "no edge found");
    IMP_assert(nc > 0, "Too few edges");
    p[i]->set_value(d.num_edges_key_, nc-1);
  }
  e->set_is_active(false);
  e->get_model()->remove_particle(e->get_index());
}



Particle* graph_get_edge(Particle* a, int i, const GraphData &d)
{
  ParticleKey nm= graph_get_edge_key(i, d);
  return a->get_value(nm);
}

Particle* graph_get_neighbor(Particle* a, int i, const GraphData &d)
{
  ParticleKey nm= graph_get_edge_key(i, d);
  Particle *edge= a->get_value(nm);
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
  return a->get_value(d.node_keys_[i]);
}

bool graph_is_edge(Particle *a, const GraphData &d)
{
  return a->has_attribute(d.node_keys_[0])
         && a->has_attribute(d.node_keys_[1]);
}

} // namespace internal

} // namespace IMP
