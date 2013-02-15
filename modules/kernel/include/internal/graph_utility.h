/**
 *  \file internal/graph_utility.h
 *  \brief Various useful utilities
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_GRAPH_UTILITY_H
#define IMPKERNEL_INTERNAL_GRAPH_UTILITY_H

#include <IMP/kernel/kernel_config.h>
#include "../Particle.h"
#include "../Decorator.h"
#include <IMP/base/internal/graph_utility.h>

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

struct IMPKERNELEXPORT AncestorException{
  base::Object *o;
  AncestorException(base::Object *oi): o(oi){};
  virtual ~AncestorException();
};

// gcc 4.2 objects if this does not have external linkage
template <class Graph>
class AncestorVisitor: public boost::default_dfs_visitor {
  std::set<Particle*> pst_;
  typename boost::property_map<Graph,
                               boost::vertex_name_t>::const_type vm_;
public:
  AncestorVisitor(){}
  AncestorVisitor(const ParticlesTemp& pst,
                  const Graph&g): pst_(pst.begin(), pst.end()),
                                  vm_(boost::get(boost::vertex_name, g)){}
  void discover_vertex(typename boost::graph_traits<Graph>::vertex_descriptor u,
                       const Graph& ) {
    base::Object *o= vm_[u];
    //std::cout << "Visiting " << o->get_name() << std::endl;
    if (pst_.find(dynamic_cast<Particle*>(o)) != pst_.end()) {
      throw AncestorException(o);
    }
  }
  /*template <class Edge>
  void tree_edge(Edge e, const Graph &g) {
    typename boost::graph_traits<Graph>::vertex_descriptor s=
      = boost::source(e, g);
    typename boost::graph_traits<Graph>::vertex_descriptor t
      = boost::target(e, g);
    std::cout << "Tree edge " << vm_[s]->get_name()
      << "->" << vm_[t]->get_name() << std::endl;
      }*/
};

template <class G>
inline bool get_has_ancestor(const G &g,
                      unsigned int v,
                      const ParticlesTemp &pst) {
  typedef boost::reverse_graph<G>  RG;
  RG rg(g);
  AncestorVisitor<RG> av(pst,g);
  boost::vector_property_map<int> color(boost::num_vertices(g));
  try {
    //std::cout << "Searching for dependents of " << v << std::endl;
    boost::depth_first_visit(rg, v, av, color);
    return false;
  } catch (AncestorException e) {
    /*IMP_LOG_VERBOSE( "Vertex has ancestor \"" << e.o->get_name()
      << "\"" << std::endl);*/
    return true;
  }
}

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_GRAPH_UTILITY_H */
