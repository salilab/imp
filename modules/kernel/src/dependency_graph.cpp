/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/dependency_graph.h"
#include "IMP/kernel/Model.h"
#include "IMP/kernel/RestraintSet.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/visitors.hpp>
#include <IMP/kernel/internal/graph_utility.h>
IMP_GCC_PRAGMA(diagnostic ignored "-Wunused-parameter")
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/dynamic_bitset.hpp>
#include <IMP/base//warning_macros.h>
#include <boost/foreach.hpp>
#include <IMP/base//file.h>
//#include <boost/graph/lookup_edge.hpp>
#include <IMP/base//vector_property_map.h>
#include <boost/graph/reverse_graph.hpp>

IMPKERNEL_BEGIN_NAMESPACE

template <class Graph, class Type, class Types>
class DirectCollectVisitor: public boost::default_dfs_visitor {
  typename boost::property_map<Graph,
                      boost::vertex_name_t>::const_type vm_;
  Types &vals_;
public:
  const Types &get_collected() {
    std::sort(vals_.begin(), vals_.end());
    vals_.erase(std::unique(vals_.begin(), vals_.end()), vals_.end());
    return vals_;
  }
  DirectCollectVisitor(const Graph &g, Types &vals): vals_(vals)
    {
      vm_=boost::get(boost::vertex_name, g);
    }
  template <class TG>
  void discover_vertex(typename boost::graph_traits<Graph>::vertex_descriptor u,
                       const TG&) {
    ModelObject *o= vm_[u];
    //std::cout << "Visiting " << o->get_name() << std::endl;
    Type *p=dynamic_cast<Type*>(o);
    if (p) {
      //IMP_LOG_VERBOSE( "Found vertex " << o->get_name() << std::endl);
      vals_.push_back(p);
    } else {
      //IMP_LOG_VERBOSE( "Visited vertex " << o->get_name() << std::endl);
    }
  }
};

namespace {
  template <class ResultType, class Type, bool REVERSE>
  ResultType get_dependent(const ModelObjectsTemp &p,
                         const ModelObjectsTemp &all,
                         const DependencyGraph &dg,
                         const DependencyGraphVertexIndex &index) {
  IMP_FUNCTION_LOG;
  boost::vector_property_map<int> color(boost::num_vertices(dg));
  for (unsigned int i=0; i< all.size(); ++i) {
    IMP_USAGE_CHECK(index.find(all[i]) != index.end(),
                    "Blocker node not found in index");
    DependencyGraphVertex blocked=
      index.find(all[i])->second;
    IMP_INTERNAL_CHECK(color[blocked]==boost::color_traits<int>::white(),
                       "Vertex does not start white");
    color[blocked]= boost::color_traits<int>::black();
  }
  ResultType pt;
  DirectCollectVisitor<DependencyGraph, Type, ResultType> cv(dg, pt);
  for (unsigned int i=0; i< p.size(); ++i) {
    IMP_USAGE_CHECK(index.find(p[i]) != index.end(),
                    "Object " << p[i] << " not found in dependency graph");
    DependencyGraphVertex start= index.find(p[i])->second;
    if (REVERSE) {
      boost::depth_first_visit(boost::make_reverse_graph(dg), start, cv, color);
    } else {
      boost::depth_first_visit(dg, start, cv, color);
    }
  }
  return cv.get_collected();
}


}







ParticlesTemp get_dependent_particles(ModelObject *p,
                                      const ModelObjectsTemp &all,
                                      const DependencyGraph &dg,
                       const DependencyGraphVertexIndex &index) {
  return get_dependent<ParticlesTemp, Particle, false>(ModelObjectsTemp(1,p),
                                                       all, dg,index);
}



RestraintsTemp get_dependent_restraints(ModelObject *p,
                                        const ModelObjectsTemp &all,
                                        const DependencyGraph &dg,
                       const DependencyGraphVertexIndex &index) {
  return get_dependent<RestraintsTemp, Restraint, false>(ModelObjectsTemp(1,p),
                                                         all, dg, index);
}
ScoreStatesTemp get_dependent_score_states(ModelObject *p,
                                           const ModelObjectsTemp &all,
                                           const DependencyGraph &dg,
                       const DependencyGraphVertexIndex &index) {
  return get_dependent<ScoreStatesTemp,
      ScoreState, false>(ModelObjectsTemp(1,p),all, dg, index);
}





ParticlesTemp get_required_particles(ModelObject *p,
                                     const ModelObjectsTemp &all,
                                     const DependencyGraph &dg,
                       const DependencyGraphVertexIndex &index) {
  return get_dependent<ParticlesTemp, Particle, true>(ModelObjectsTemp(1,p),
                                                      all, dg, index);
}


ScoreStatesTemp get_required_score_states(ModelObject *p,
                                          const ModelObjectsTemp &all,
                                          const DependencyGraph &dg,
                                 const DependencyGraphVertexIndex &index ) {
  return get_dependent<ScoreStatesTemp, ScoreState,
      true>(ModelObjectsTemp(1,p),all, dg,index);
}




namespace {


  template <class C, class O>
  C filter(C c, O*o) {
    std::sort(c.begin(), c.end());
    c.erase(std::unique(c.begin(), c.end()), c.end());
    IMP_INTERNAL_CHECK_VARIABLE(o);
    IMP_INTERNAL_CHECK(c.empty() || c[0],
                       "nullptr returned for dependencies of "
                       << o->get_name());
    return c;
  }

  bool get_has_edge(const DependencyGraph &graph,
                    DependencyGraphTraits::vertex_descriptor va,
                    DependencyGraphTraits::vertex_descriptor vb) {
    /*std::pair<MDependencyGraphTraits::out_edge_iterator,
      MDependencyGraphTraits::out_edge_iterator> edges
      = boost::out_edges(va, graph);
    for (; edges.first != edges.second;++edges.first) {
      if (boost::target(*edges.first, graph) == vb) return true;
    }
    return false;*/
    return boost::edge(va, vb, graph).second;
  }

  void add_edge(DependencyGraph &graph,
                DependencyGraphTraits::vertex_descriptor va,
                DependencyGraphTraits::vertex_descriptor vb) {
    if (get_has_edge(graph, va, vb)) return;
    // const conversion broken
#if IMP_HAS_CHECKS >= IMP_INTERNAL
    DependencyGraphVertexName names = boost::get(boost::vertex_name, graph);
    IMP_INTERNAL_CHECK(va != vb, "Can't depend on itself "
                       << names[va]->get_name());
    IMP_INTERNAL_CHECK(!get_has_edge(graph, va, vb),
                       "Already has edge between " << names[va]->get_name()
                       << " and " << names[vb]->get_name());
#endif
    boost::add_edge(va, vb, graph);
    IMP_INTERNAL_CHECK(get_has_edge(graph, va, vb),
                       "No has edge between " << va << " and " << vb);

  }

  DependencyGraphTraits::vertex_descriptor get_vertex(DependencyGraph &,
                                          const DependencyGraphVertexIndex &dgi,
                                                      ModelObject *o) {
    DependencyGraphVertexIndex::const_iterator it=dgi.find(o);
    IMP_USAGE_CHECK(it != dgi.end(),
                    "Found unregistered ModelObject " << Showable(o));
    return it->second;
  }

template <class It>
void add_out_edges(DependencyGraphTraits::vertex_descriptor rv,
                   It b, It e,
                   DependencyGraph &dg,
                   const DependencyGraphVertexIndex &dgi) {
  for (It c=b; c!= e; ++c) {
    DependencyGraphTraits::vertex_descriptor cv= get_vertex(dg, dgi, *c);
    if (!get_has_edge(dg, rv, cv)) {
      add_edge(dg, cv, rv);
    }
  }
}

  void build_inputs_graph(const ModelObjectsTemp &mos,
                          DependencyGraph &dg,
                          const DependencyGraphVertexIndex &dgi) {
    for (unsigned int i=0; i< mos.size(); ++i) {
      DependencyGraphTraits::vertex_descriptor rv= dgi.find(mos[i])->second;
      ModelObject *o= mos[i];
      ModelObjectsTemp ct= filter(o->get_inputs(), o);
      add_out_edges(rv, ct.begin(), ct.end(), dg, dgi);
    }
  }

  void build_outputs_graph(const ModelObjectsTemp mos,
                           DependencyGraph &dg,
                           const DependencyGraphVertexIndex &dgi) {
    for (unsigned int i=0; i< mos.size(); ++i) {
      DependencyGraphTraits::vertex_descriptor rv= dgi.find(mos[i])->second;
      {
        ModelObjectsTemp ct= filter(mos[i]->get_outputs(),
                                    static_cast<Object*>(mos[i]));
        for (unsigned int j=0; j < ct.size(); ++j) {
          DependencyGraphTraits::vertex_descriptor cv
            = get_vertex(dg, dgi, ct[j]);
          if (!get_has_edge(dg, cv, rv)) {
            add_edge(dg, rv, cv);
          }
        }
      }
    }
  }
}
DependencyGraph
get_dependency_graph(Model *m) {
  ModelObjectsTemp mos= m->get_model_objects();
  DependencyGraphVertexIndex index;
  DependencyGraph ret(mos.size());
  DependencyGraphVertexName
    vm = boost::get(boost::vertex_name, ret);
  for (unsigned int i=0; i< mos.size(); ++i) {
    vm[i]= mos[i];
    index[mos[i]]=i;
  }
  build_outputs_graph(mos, ret, index);
  build_inputs_graph(mos, ret, index);
  base::Vector<std::pair<ModelObject*, ModelObject*> > extra;
  for (unsigned int i=0; i< extra.size(); ++i) {
    int va= index[extra[i].first];
    int vb= index[extra[i].second];
    boost::add_edge(va, vb, ret);
  }
  return ret;
}

namespace {
  template <class P>
  bool get_range_is_empty(const P &p) {
    return p.first==p.second;
  }
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
IMP_CLANG_PRAGMA(diagnostic ignored "-Wunused-member-function")
    IMP_COMPARISONS(Connections);
  };
  //IMP_VALUES(Connections, ConnectionsList);
  inline std::size_t hash_value(const Connections &t) {
    return t.__hash__();
  }
}




DependencyGraph
get_pruned_dependency_graph(Model *m) {
  IMP_FUNCTION_LOG;
  DependencyGraph full= get_dependency_graph(m);
  typedef boost::graph_traits<DependencyGraph> T;
  bool changed=true;
  while (changed) {
    changed=false;
    IMP_LOG_VERBOSE( "Searching for vertices to prune" << std::endl);
    base::set<Connections> connections;
    for (unsigned int i=0; i< boost::num_vertices(full); ++i) {
      Connections c(i, full);
      if (connections.find(c) != connections.end()) {
#if IMP_HAS_LOG >= IMP_VERBOSE
        DependencyGraphVertexName
          vm = boost::get(boost::vertex_name, full);
        IMP_LOG_VERBOSE( "Removing object " << vm[i]->get_name() << std::endl);
#endif
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




struct cycle_detector : public boost::default_dfs_visitor {
  base::Vector<DependencyGraphVertex> cycle_;
  template <class DGEdge>
  void tree_edge(DGEdge e, const DependencyGraph&g) {
    DependencyGraphVertex t= boost::target(e, g);
    //MDGVertex s= boost::source(e, g);
    cycle_.push_back(t);
  }
  template <class DependencyGraphVertex>
  void finish_vertex(DependencyGraphVertex v, const DependencyGraph&) {
    IMP_USAGE_CHECK_VARIABLE(v);
    IMP_USAGE_CHECK(cycle_.back()==v, "They don't match");
    cycle_.pop_back();
  }
  template <class ED>
  void back_edge(ED e, const DependencyGraph&g) {
    DependencyGraphVertex t= boost::target(e, g);
    //MDGVertex s= boost::source(e, g);
    base::Vector<DependencyGraphVertex>::iterator it
        = std::find(cycle_.begin(), cycle_.end(), t);
    //std::cout << s << " " << cycle_.back() << std::endl;
    if (it != cycle_.end()) {
      cycle_.erase(cycle_.begin(), it);
      cycle_.push_back(t);
      throw cycle_;
    } else {
      //std::cout << "non-loop " << s << " " << t << std::endl;
    }
  }
};



ScoreStatesTemp get_required_score_states(const ModelObjectsTemp &irs,
                                          const DependencyGraph &dg,
                             const DependencyGraphVertexIndex &index) {
  ScoreStatesTemp sst
      =  get_dependent<ScoreStatesTemp, ScoreState, true>(irs,
                                                          ModelObjectsTemp(),
                                                          dg, index);
  return get_update_order(sst);
}


void set_score_state_update_order(const DependencyGraph& dg,
                                  const DependencyGraphVertexIndex &index) {
  DependencyGraphConstVertexName map= boost::get(boost::vertex_name, dg);
  /** Iteratively, add all score states with no score state input
      to current set. Remove score states that have been added.

      But we fake it by simply checking for each score state if
      it has an input that is not already added.*/
  // find all score states
  ModelObjectsTemp added;
  base::set<ModelObject*> not_added;
  for (unsigned int i=0; i < boost::num_vertices(dg); ++i) {
    if (dynamic_cast<ScoreState*>(map[i])) {
      not_added.insert(map[i]);
    }
  }
  int round=0;
  while (!not_added.empty()) {
    ScoreStatesTemp cur;
    BOOST_FOREACH(ModelObject *s, not_added) {
      ScoreStatesTemp inputs= get_required_score_states(s,
                                                        added,
                                                        dg, index);
      IMP_LOG_TERSE( Showable(s) << " depends on " << inputs << std::endl);
      // includes self
      if (inputs.size() <=1) {
        cur.push_back(dynamic_cast<ScoreState*>(s));
      }
    }
    IMP_INTERNAL_CHECK(!cur.empty(), "There must be something");
    for (unsigned int i=0; i< cur.size(); ++i) {
      cur[i]->order_=round;
    }
    added.insert(added.end(), cur.begin(), cur.end());
    for (unsigned int i=0; i< cur.size(); ++i) {
      not_added.erase(cur[i]);
    }
    ++round;
  }
}

IMPKERNEL_END_NAMESPACE
