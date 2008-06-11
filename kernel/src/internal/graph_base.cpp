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

IMPDLLEXPORT void graph_initialize_node(Particle* a,
                                        const GraphData &d)
{
  d.add_required_attributes(a);
}

/** \internal */
IMPDLLEXPORT bool graph_is_node(Particle* a,
                                const GraphData &d)
{
  return d.has_required_attributes(a);
}


Particle* graph_connect(Particle* a, Particle* b, GraphData &d)
{
  Model *m= a->get_model();
  Particle *p= new Particle();
  ParticleIndex pi=m->add_particle(p);
  p->add_attribute(d.node_keys_[0], a);
  p->add_attribute(d.node_keys_[1], b);
  for (int i=0; i< 2; ++i) {
    Particle *cp=((i==0)?a:b);
    d.push_back(cp, p);
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
    for (unsigned int j=0; j< d.get_size(p[i]); ++j) {
      if (d.get_value(p[i], j) == e) {
        d.erase(p[i], j);
        break;
      }
    }
  }
  e->set_is_active(false);
  e->get_model()->remove_particle(e->get_index());
}



Particle* graph_get_edge(Particle* a, int i, const GraphData &d)
{
  return d.get_value(a,i);
}

Particle* graph_get_neighbor(Particle* a, int i, const GraphData &d)
{
  Particle *edge= d.get_value(a,i);
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
  return d.get_size(a);
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
