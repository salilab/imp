/**
 *  \file graph_base.cpp   \brief classes for implementing a graph.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/internal/graph_base.h>
#include <IMP/core/core_macros.h>

#include <sstream>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

void graph_initialize_node(Particle *, const GraphData &) {}

/** \internal */
bool graph_is_node(Particle *, const GraphData &) { return true; }

Particle *graph_connect(Particle *a, Particle *b, GraphData &d) {
  Model *m = a->get_model();
  Particle *p = new Particle(m);
  p->add_attribute(d.node_keys_[0], a);
  p->add_attribute(d.node_keys_[1], b);
  for (int i = 0; i < 2; ++i) {
    Particle *cp = ((i == 0) ? a : b);
    if (m->get_has_attribute(d.edges_key_, cp->get_index())) {
      ParticleIndexes c = m->get_attribute(d.edges_key_, cp->get_index());
      c.push_back(p->get_index());
      m->set_attribute(d.edges_key_, cp->get_index(), c);
    } else {
      m->add_attribute(d.edges_key_, cp->get_index(),
                       ParticleIndexes(1, p->get_index()));
    }
  }

  return p;
}

void graph_disconnect(Particle *e, const GraphData &d) {
  Particle *p[2];
  p[0] = graph_get_node(e, 0, d);
  p[1] = graph_get_node(e, 1, d);
  for (int i = 0; i < 2; ++i) {
    ParticleIndexes pis =
        e->get_model()->get_attribute(d.edges_key_, p[i]->get_index());
    pis.erase(std::find(pis.begin(), pis.end(), e->get_index()));
    if (!pis.empty()) {
      e->get_model()->set_attribute(d.edges_key_, p[i]->get_index(), pis);
    } else {
      e->get_model()->remove_attribute(d.edges_key_, p[i]->get_index());
    }
  }
  e->get_model()->remove_particle(e);
}

Particle *graph_get_edge(Particle *a, int i, const GraphData &d) {
  IMP_USAGE_CHECK(
      a->get_model()->get_has_attribute(d.edges_key_, a->get_index()),
      "Particle " << a->get_name() << " does not have "
                  << "enough edges");
  ParticleIndexes all =
      a->get_model()->get_attribute(d.edges_key_, a->get_index());
  IMP_USAGE_CHECK(
      all.size() > static_cast<unsigned int>(i),
      "Particle " << a->get_name() << " does not have enough edges");
  return a->get_model()->get_particle(all[i]);
}

ParticleIndexes graph_get_edges(Particle *a, const GraphData &d) {
  if (!a->get_model()->get_has_attribute(d.edges_key_, a->get_index())) {
    return ParticleIndexes();
  }
  ParticleIndexes all =
      a->get_model()->get_attribute(d.edges_key_, a->get_index());
  return all;
}

Particle *graph_get_neighbor(Particle *a, int i, const GraphData &d) {
  Particle *edge = graph_get_edge(a, i, d);
  if (graph_get_node(edge, 0, d) == a) {
    return graph_get_node(edge, 1, d);
  } else {
    IMP_INTERNAL_CHECK(graph_get_node(edge, 1, d) == a, "Broken graph");
    return graph_get_node(edge, 0, d);
  }
}

unsigned int graph_get_number_of_edges(Particle *a, const GraphData &d) {
  if (!a->get_model()->get_has_attribute(d.edges_key_, a->get_index()))
    return 0;
  else
    return a->get_model()->get_attribute(d.edges_key_, a->get_index()).size();
}

void graph_initialize_edge(Particle *a, const GraphData &d) {
  a->add_attribute(d.node_keys_[0], nullptr);
  a->add_attribute(d.node_keys_[1], nullptr);
}

IMPCORE_END_INTERNAL_NAMESPACE
