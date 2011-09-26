/**
 *  \file Restraint.cpp   \brief Abstract base class for all restraints.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container_base.h"
#include "IMP/internal/utility.h"
#include "IMP/Particle.h"
#include "IMP/Model.h"
#include "IMP/internal/graph_utility.h"
#include "IMP/dependency_graph.h"

IMP_BEGIN_NAMESPACE

Container::Container(Model *m, std::string name):
  Object(name),
  m_(m)
{
}

bool Container::is_ok(Particle *p) {
  return p && p->get_model()==m_;
}


Model *Container::get_model(Particle *p) {return p->get_model();}


namespace {
 typedef boost::graph_traits<DependencyGraph> DGTraits;
  typedef DGTraits::vertex_descriptor DGVertex;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::type DGVertexMap;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::const_type
  DGConstVertexMap;
}

bool
get_is_static_container(Container *c,
                        const DependencyGraph &dg,
                        const ParticlesTemp &pst) {
  typedef DGTraits::in_edge_iterator IEIt;
  typedef DGTraits::vertex_iterator DVIt;
  DGConstVertexMap pm=boost::get(boost::vertex_name, dg);
  int cv=-1;
  for (std::pair<DVIt, DVIt> be= boost::vertices(dg);
       be.first != be.second; ++be.first) {
    if (boost::get(pm, *be.first)== c) {
      cv=*be.first;
      break;
    }
  }
  if (cv==-1) {
    IMP_THROW("Container \"" << c->get_name()
              << "\" not in graph.", ValueException);
  }
  return !internal::get_has_ancestor(dg, cv, pst);
}


IMP_END_NAMESPACE
