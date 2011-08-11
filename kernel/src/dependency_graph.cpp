/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/dependency_graph.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/visitors.hpp>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/dynamic_bitset.hpp>
//#include <boost/graph/lookup_edge.hpp>
#include <IMP/compatibility/vector_property_map.h>

IMP_BEGIN_NAMESPACE
namespace {
 typedef boost::graph_traits<DependencyGraph> DGTraits;
  typedef DGTraits::vertex_descriptor DGVertex;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::type DGVertexMap;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::const_type
  DGConstVertexMap;
}


template <class Graph, class Type>
class DirectCollectVisitor: public boost::default_dfs_visitor {
  typename boost::property_map<Graph,
                      boost::vertex_name_t>::const_type vm_;
  std::vector<Type*> &vals_;
public:
  const std::vector<Type*> &get_collected() {
    std::sort(vals_.begin(), vals_.end());
    vals_.erase(std::unique(vals_.begin(), vals_.end()), vals_.end());
    return vals_;
  }
  DirectCollectVisitor(const Graph &g, std::vector<Type*> &vals): vals_(vals)
    {
      vm_=boost::get(boost::vertex_name, g);
    }
  void discover_vertex(typename boost::graph_traits<Graph>::vertex_descriptor u,
                       const Graph&) {
    Object *o= vm_[u];
    //std::cout << "Visiting " << o->get_name() << std::endl;
    Type *p=dynamic_cast<Type*>(o);
    if (p) {
      vals_.push_back(p);
    }
  }
};







ParticlesTemp get_dependent_particles(Particle *p,
                                      const ParticlesTemp &all,
                                      const DependencyGraph &dg) {
  // find p in graph, ick
  DGConstVertexMap dpm= boost::get(boost::vertex_name, dg);
  std::pair<DGTraits::vertex_iterator, DGTraits::vertex_iterator> be
    = boost::vertices(dg);
  IMP::compatibility::set<Object*> block(all.begin(), all.end());
  boost::vector_property_map<int> color(boost::num_vertices(dg));
  int start=-1;
  for (; be.first != be.second; ++be.first) {
    if (dpm[*be.first]==p) {
      start=*be.first;
    } else if (block.find(dpm[*be.first]) != block.end()) {
      // block traversal though the other nodes
      color[*be.first]= boost::color_traits<int>::black();
    }
  }
  if (start==-1) {
    return ParticlesTemp();
  }
  ParticlesTemp pt;
  DirectCollectVisitor<DependencyGraph, Particle> cv(dg, pt);
  boost::depth_first_visit(dg, start, cv, color);
  return cv.get_collected();
}

ParticlesTemp get_dependent_particles(Particle *p,
                                      const ParticlesTemp &all) {
  Model *m= p->get_model();
  DependencyGraph dg
    = get_dependency_graph(get_restraints(m->restraints_begin(),
                                          m->restraints_end()));
  return get_dependent_particles(p, all, dg);
}


RestraintsTemp get_dependent_restraints(Particle *p,
                                      const ParticlesTemp &all,
                                      const DependencyGraph &dg) {
  // find p in graph, ick
  DGConstVertexMap dpm= boost::get(boost::vertex_name, dg);
  std::pair<DGTraits::vertex_iterator, DGTraits::vertex_iterator> be
    = boost::vertices(dg);
  IMP::compatibility::set<Object*> block(all.begin(), all.end());
  boost::vector_property_map<int> color(boost::num_vertices(dg));
  int start=-1;
  for (; be.first != be.second; ++be.first) {
    if (dpm[*be.first]==p) {
      start=*be.first;
    } else if (block.find(dpm[*be.first]) != block.end()) {
      // block traversal though the other nodes
      color[*be.first]= boost::color_traits<int>::black();
    }
  }
  if (start==-1) {
    return RestraintsTemp();
  }
  RestraintsTemp pt;
  DirectCollectVisitor<DependencyGraph, Restraint> cv(dg, pt);
  boost::depth_first_visit(dg, start, cv, color);
  return cv.get_collected();
}


typedef compatibility::map<Object*, DGVertex> DGIndex;
typedef boost::property_map<DependencyGraph, boost::vertex_name_t>::const_type
DGConstVertexMap;



namespace {


  template <class C>
  C filter(C c) {
    std::sort(c.begin(), c.end());
    c.erase(std::unique(c.begin(), c.end()), c.end());
    IMP_INTERNAL_CHECK(c.empty() || c[0] != NULL,
                       "NULL returned for dependencies.");
    return c;
  }

  bool get_has_edge(const DependencyGraph &graph,
                    DGTraits::vertex_descriptor va,
                    DGTraits::vertex_descriptor vb) {
    /*std::pair<MDGTraits::out_edge_iterator,
      MDGTraits::out_edge_iterator> edges= boost::out_edges(va, graph);
    for (; edges.first != edges.second;++edges.first) {
      if (boost::target(*edges.first, graph) == vb) return true;
    }
    return false;*/
    return boost::edge(va, vb, graph).second;
  }

  void add_edge(DependencyGraph &graph,
                DGTraits::vertex_descriptor va,
                DGTraits::vertex_descriptor vb) {
    if (get_has_edge(graph, va, vb)) return;
    IMP_INTERNAL_CHECK(va != vb, "Can't dependend on itself " << va);
    IMP_INTERNAL_CHECK(!get_has_edge(graph, va, vb),
                       "Already has edge between " << va << " and " << vb);
    boost::add_edge(va, vb, graph);
    IMP_INTERNAL_CHECK(get_has_edge(graph, va, vb),
                       "No has edge between " << va << " and " << vb);

  }

  DGTraits::vertex_descriptor get_vertex(DependencyGraph &dg,
                                         DGIndex &dgi,
                                         Object *o) {
    DGIndex::const_iterator it=dgi.find(o);
    if (it==dgi.end()) {
      boost::property_map<DependencyGraph, boost::vertex_name_t>::type vm
        = boost::get(boost::vertex_name, dg);
      DGTraits::vertex_descriptor v= boost::add_vertex(dg);
      vm[v]=o;
      dgi[o]=v;
      return v;
    } else {
      return it->second;
    }
  }

  template <class It>
  void build_inputs_graph(It b, It e,
                          DependencyGraph &dg,
                          DGIndex &dgi) {
    for (It c= b; c != e; ++c) {
      DGTraits::vertex_descriptor rv= dgi.find(*c)->second;
      IMP_LOG(VERBOSE, "Processing inputs for \""
        << (*c)->get_name() << "\" ");
      {
        ContainersTemp ct= filter((*c)->get_input_containers());
        if (!ct.empty()) {
          IMP_LOG(VERBOSE, ", containers are "
                  << Containers(ct));
                  }
        for (unsigned int j=0; j < ct.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, ct[j]);
          //if (!get_has_edge(dg, rv, cv)) {
          add_edge(dg, cv, rv);
          //}
        }
      }
      {
        ParticlesTemp pt= filter((*c)->get_input_particles());
        if (!pt.empty()) {
          IMP_LOG(VERBOSE, ", particles are " << Particles(pt));
          }
        for (unsigned int j=0; j < pt.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, pt[j]);
          //if (!get_has_edge(dg, rv, cv)) {
          add_edge(dg, cv, rv);
          //}
        }
      }
      IMP_LOG(VERBOSE, std::endl);
    }
  }

  template <class It>
  void build_outputs_graph(It b, It e,
                           DependencyGraph &dg,
                           DGIndex &dgi) {
    for (It c= b; c != e; ++c) {
      DGTraits::vertex_descriptor rv= dgi.find(*c)->second;
      IMP_LOG(VERBOSE, "Processing outputs for \""
        << (*c)->get_name()  << "\"");
      {
        ContainersTemp ct= filter((*c)->get_output_containers());
        IMP_IF_LOG(VERBOSE) {
          if (!ct.empty()) {
            IMP_LOG(VERBOSE, ", containers are "
                    << Containers(ct));
          }
        }
        for (unsigned int j=0; j < ct.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, ct[j]);
          if (!get_has_edge(dg, cv, rv)) {
            add_edge(dg, rv, cv);
          }
        }
      }
      {
        ParticlesTemp pt= filter((*c)->get_output_particles());
        if (!pt.empty()) {
          IMP_LOG(VERBOSE, ", particles are "
                  << Particles(pt));
        }
        for (unsigned int j=0; j < pt.size(); ++j) {
          DGTraits::vertex_descriptor cv= get_vertex(dg, dgi, pt[j]);
          if (!get_has_edge(dg, cv, rv)) {
            add_edge(dg, rv, cv);
          }
        }
      }
      IMP_LOG(VERBOSE, std::endl);
    }
  }
}
DependencyGraph
get_dependency_graph(const ScoreStatesTemp &ss,
                     const RestraintsTemp &rs) {
  IMP_LOG(VERBOSE, "Making dependency graph on " << rs.size()
          << " restraints and " << ss.size() << " score states."
          << std::endl);
  DGIndex index;
  DependencyGraph ret(ss.size()+rs.size());
  boost::property_map<DependencyGraph, boost::vertex_name_t>::type
    vm = boost::get(boost::vertex_name, ret);
  for (unsigned int i=0; i< ss.size(); ++i) {
    vm[i]= ss[i];
    index[ss[i]]=i;
  }
  for (unsigned int i=0; i< rs.size(); ++i) {
    vm[i+ss.size()]= rs[i];
    index[rs[i]]=i+ss.size();
  }
  // Very important to do inputs first
  build_inputs_graph(ss.begin(), ss.end(), ret, index);
  build_outputs_graph(ss.begin(), ss.end(), ret, index);
  build_inputs_graph(rs.begin(), rs.end(), ret, index);
  std::vector<std::pair<Object*, Object*> > extra;
  if (!ss.empty()) {
    extra=ss[0]->get_model()->get_extra_dependency_edges();
  } else if (!rs.empty()) {
    extra=rs[0]->get_model()->get_extra_dependency_edges();
  }
  for (unsigned int i=0; i< extra.size(); ++i) {
    int va= index[extra[i].first];
    int vb= index[extra[i].second];
    boost::add_edge(va, vb, ret);
  }
  return ret;
}

DependencyGraph
get_dependency_graph(const RestraintsTemp &irs) {
  IMP_FUNCTION_LOG;
  if (irs.empty()) return DependencyGraph();
  RestraintsTemp rs= get_restraints(irs.begin(), irs.end());
  ScoreStatesTemp ss
    = irs[0]->get_model()->get_score_states(rs);
  return get_dependency_graph(ss, rs);
}


namespace {
  struct Connections {
    Ints in, out;
    Connections(int v, const DependencyGraph &g) {
      typedef boost::graph_traits<DependencyGraph> T;
      {
        typedef T::in_edge_iterator IIT;
        for (std::pair<IIT,IIT> be= boost::in_edges(v, g);
             be.first != be.second; ++be.first) {
          in.push_back(boost::source(*be.first, g));
        }
        std::sort(in.begin(), in.end());
      }
      {
        typedef T::out_edge_iterator OIT;
        for (std::pair<OIT,OIT> be= boost::out_edges(v, g);
             be.first != be.second; ++be.first) {
          out.push_back(boost::target(*be.first, g));
        }
        std::sort(out.begin(), out.end());
      }
      IMP_INTERNAL_CHECK(*this == *this, "Not equal");
    }
    int compare(const Connections &o) const {
      if (in.size() < o.in.size()) return -1;
      else if (in.size() > o.in.size()) return 1;
      else if (out.size() < o.out.size()) return -1;
      else if (out.size() > o.out.size()) return 1;
      else {
        for (unsigned int i=0; i< in.size(); ++i) {
          if (in[i] < o.in[i]) return -1;
          else if (in[i] > o.in[i]) return 1;
        }
        for (unsigned int i=0; i< out.size(); ++i) {
          if (out[i] < o.out[i]) return -1;
          else if (out[i] > o.out[i]) return 1;
        }
        return 0;
      }
    }
    IMP_HASHABLE_INLINE(Connections,
                        return boost::hash_range(in.begin(),
                                                 in.end())
                        + boost::hash_range(out.begin(),
                                            out.end()));
    IMP_COMPARISONS(Connections);
  };
  //IMP_VALUES(Connections, ConnectionsList);
#if !defined(__clang__)
  inline std::size_t hash_value(const Connections &t) {
    return t.__hash__();
  }
#endif
}

DependencyGraph
get_pruned_dependency_graph(const RestraintsTemp &irs) {
  IMP_FUNCTION_LOG;
  DependencyGraph full= get_dependency_graph(irs);
  typedef boost::graph_traits<DependencyGraph> T;
  bool changed=true;
  while (changed) {
    changed=false;
    IMP_LOG(VERBOSE, "Searching for vertices to prune" << std::endl);
    compatibility::set<Connections> connections;
    for (unsigned int i=0; i< boost::num_vertices(full); ++i) {
      Connections c(i, full);
      if (connections.find(c) != connections.end()) {
        boost::property_map<DependencyGraph, boost::vertex_name_t>::type
          vm = boost::get(boost::vertex_name, full);
        IMP_LOG(VERBOSE, "Removing object " << vm[i]->get_name() << std::endl);
        for (unsigned int j=0; j< c.in.size(); ++j) {
          for (unsigned int k=0; k< c.out.size(); ++k) {
            //if (!boost::lookup_edge(c.in[j], c.out[k], full).second) {
              // why am I doing this anyway?
              //boost::add_edge(c.in[j], c.out[k], full);
            //}
          }
        }
        boost::clear_vertex(i, full);
        boost::remove_vertex(i, full);
        changed=true;
      } else {
        connections.insert(c);
      }
    }
  }
  return full;
}



DependencyGraph
get_pruned_dependency_graph(Model *m) {
  RestraintsTemp rt
    = get_restraints(RestraintsTemp(1, m->get_root_restraint_set()));
  return get_pruned_dependency_graph(rt);
}


IMP_END_NAMESPACE
