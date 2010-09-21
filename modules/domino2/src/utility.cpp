/**
 *  \file interaction_graph.cpp
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino2/utility.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Restraint.h>
#include <IMP/ScoreState.h>
#include <IMP/internal/map.h>
#include <boost/graph/graphviz.hpp>
#include <IMP/internal/graph_utility.h>
#include <IMP/domino2/internal/restraint_evaluator.h>
#include <IMP/RestraintSet.h>
#include <IMP/domino2/particle_states.h>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif


IMPDOMINO2_BEGIN_NAMESPACE



template <class Graph>
class DirectCollectVisitor: public boost::default_dfs_visitor {
  typename boost::property_map<Graph,
                      boost::vertex_name_t>::const_type vm_;
  ParticlesTemp &vals_;
public:
  const ParticlesTemp &get_collected() {
    std::sort(vals_.begin(), vals_.end());
    vals_.erase(std::unique(vals_.begin(), vals_.end()), vals_.end());
    return vals_;
  }
  DirectCollectVisitor(const Graph &g, ParticlesTemp &vals): vals_(vals)
    {
      vm_=boost::get(boost::vertex_name, g);
    }
  void discover_vertex(typename boost::graph_traits<Graph>::vertex_descriptor u,
                       const Graph& g) {
    Object *o= vm_[u];
    //std::cout << "Visiting " << o->get_name() << std::endl;
    Particle *p=dynamic_cast<Particle*>(o);
    if (p) {
      vals_.push_back(p);
    }
  }
};


namespace {
 typedef boost::graph_traits<DependencyGraph> DGTraits;
  typedef DGTraits::vertex_descriptor DGVertex;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::type DGVertexMap;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::const_type
  DGConstVertexMap;
}





ParticlesTemp get_dependent_particles(Particle *p,
                                      const DependencyGraph &dg) {
  // find p in graph, ick
  DGConstVertexMap dpm= boost::get(boost::vertex_name, dg);
  std::pair<DGTraits::vertex_iterator, DGTraits::vertex_iterator> be
    = boost::vertices(dg);
  for (; be.first != be.second; ++be.first) {
    if (dpm[*be.first]==p) {
      break;
    }
  }
  if (be.first == be.second) {
    return ParticlesTemp();
  }
  ParticlesTemp pt;
  DirectCollectVisitor<DependencyGraph> cv(dg, pt);
  boost::vector_property_map<int> color(boost::num_vertices(dg));
  boost::depth_first_visit(dg, *be.first, cv, color);
  return cv.get_collected();
}

ParticlesTemp get_dependent_particles(Particle *p) {
  Model *m= p->get_model();
  DependencyGraph dg
    = get_dependency_graph(get_restraints(m->restraints_begin(),
                                          m->restraints_end()));
  return get_dependent_particles(p, dg);
}



void load_particle_states(const Subset &s,
                          const SubsetState &ss,
                          const ParticleStatesTable *pst) {
  internal::load_particle_states(s.begin(), s.end(), ss, pst);
}
// lazily defined in optimize_restraints.cpp
bool get_has_ancestor(const DependencyGraph &g,
                      unsigned int v,
                      const ParticlesTemp &pst);

namespace {
RestraintsAndWeights get_restraints_and_weights(const Subset &s,
                              const ParticleStatesTable *pst,
                              const DependencyGraph &dg,
                              RestraintSet *rs) {
  RestraintsAndWeights rw= get_restraints_and_weights(rs);
  ParticlesTemp other=pst->get_particles();
  std::sort(other.begin(), other.end());
  ParticlesTemp oms;
  std::set_difference(other.begin(), other.end(),
                      s.begin(), s.end(),
                      std::back_inserter(oms));
  IMP::internal::Map<Restraint*, int> index
    = IMP::internal::get_graph_index<Restraint>(dg);
  Ints to_remove;
  for (unsigned int i=0; i< rw.first.size(); ++i) {
    if (get_has_ancestor(dg, index[rw.first[i]], oms)) {
      to_remove.push_back(i);
    }
  }
  for (int i=to_remove.size()-1; i >=0; --i) {
    rw.first.erase(rw.first.begin()+to_remove[i]);
    rw.second.erase(rw.second.begin()+to_remove[i]);
  }
  return rw;
}
}

RestraintsTemp get_restraints(const Subset &s,
                              const ParticleStatesTable *pst,
                              const DependencyGraph &dg,
                              RestraintSet *rs) {
  return get_restraints_and_weights(s, pst, dg, rs).first;
}




IMPDOMINO2_END_NAMESPACE
