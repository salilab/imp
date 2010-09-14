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
#include <boost/graph/copy.hpp>
#include <IMP/domino2/internal/maximal_cliques.h>
#include <IMP/internal/graph_utility.h>
#include <IMP/domino2/internal/restraint_evaluator.h>
#include <IMP/RestraintSet.h>
#include <IMP/core/internal/CoreClosePairContainer.h>
#include <IMP/domino2/particle_states.h>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif
#include <boost/pending/disjoint_sets.hpp>


IMPDOMINO2_BEGIN_NAMESPACE


template <class Graph>
class CollectVisitor: public boost::default_dfs_visitor {
  const IMP::internal::Map<Particle*, Ints> *lu_;
  typename boost::property_map<Graph,
                      boost::vertex_name_t>::const_type vm_;
  Ints &vals_;
public:
  const Ints &get_collected() {
    std::sort(vals_.begin(), vals_.end());
    vals_.erase(std::unique(vals_.begin(), vals_.end()), vals_.end());
    return vals_;
  }
  CollectVisitor(const Graph &g,
                 const IMP::internal::Map<Particle*, Ints>&lu,
                 Ints &vals):
    lu_(&lu),
    vm_(boost::get(boost::vertex_name, g)),
    vals_(vals){}
  void discover_vertex(typename boost::graph_traits<Graph>::vertex_descriptor u,
                       const Graph& g) {
    Object *o= vm_[u];
    Particle *p=dynamic_cast<Particle*>(o);
    if (p) {
      IMP::internal::Map<Particle*, Ints>::const_iterator it= lu_->find(p);
      if (it != lu_->end()) {
        vals_.insert(vals_.end(),it->second.begin(),
                     it->second.end());
      }
    }
  }
};

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
  typedef boost::graph_traits<InteractionGraph> IGTraits;
  typedef IGTraits::edge_descriptor IGEdge;
  typedef IGTraits::vertex_descriptor IGVertex;

  typedef boost::property_map<InteractionGraph,
                              boost::vertex_name_t>::type IGVertexMap;
  typedef boost::property_map<InteractionGraph,
                              boost::edge_name_t>::type IGEdgeMap;
 typedef boost::property_map<InteractionGraph,
                              boost::vertex_name_t>::const_type
 IGVertexConstMap;
  typedef boost::property_map<InteractionGraph,
                              boost::edge_name_t>::const_type IGEdgeConstMap;


  typedef boost::graph_traits<DependencyGraph> DGTraits;
  typedef DGTraits::vertex_descriptor DGVertex;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::type DGVertexMap;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::const_type
  DGConstVertexMap;



  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::undirectedS,
                                boost::property<boost::vertex_name_t,
                                                std::string>,
                                boost::property<boost::edge_weight_t,
                                                double> > CliqueGraph;
typedef boost::graph_traits<CliqueGraph> CGTraits;
typedef CGTraits::vertex_descriptor CGVertex;
typedef CGTraits::edge_descriptor CGEdge;
typedef boost::property_map<CliqueGraph,
                            boost::vertex_name_t>::type CGVertexMap;


typedef boost::property_map<SubsetGraph,
                            boost::vertex_name_t>::type SGVertexMap;
typedef boost::property_map<SubsetGraph,
                            boost::vertex_name_t>::const_type SGConstVertexMap;
typedef boost::graph_traits<SubsetGraph> SGTraits;



bool get_has_edge(InteractionGraph &graph,
                  IGVertex va,
                  IGVertex vb) {
  std::pair<IGTraits::out_edge_iterator,
    IGTraits::out_edge_iterator> edges= boost::out_edges(va, graph);
  for (; edges.first != edges.second;++edges.first) {
    if (boost::target(*edges.first, graph) == vb) return true;
  }
  return false;
}

  void add_edges( const ParticlesTemp &ps,
                  ParticlesTemp pt,
                  const IMP::internal::Map<Particle*, int> &map,
                Object *blame,
                InteractionGraph &g) {
  IGEdgeMap om= boost::get(boost::edge_name, g);
  std::sort(pt.begin(), pt.end());
  pt.erase(std::unique(pt.begin(), pt.end()), pt.end());
  for (unsigned int i=0; i< pt.size(); ++i) {
    if (map.find(pt[i]) == map.end()) continue;
    int vj=map.find(pt[i])->second;
    for (unsigned int k=0; k< i; ++k) {
      if (map.find(pt[k]) == map.end()) continue;
      int vk= map.find(pt[k])->second;
      if (vj != vk && !get_has_edge(g, vj, vk)) {
        IMP_LOG(VERBOSE, "Adding edge between \"" << ps[vj]->get_name()
                << "\" and \"" << ps[vk]->get_name()
                << "\" due to \"" << blame->get_name() << "\"" << std::endl);
        IGEdge e;
        bool inserted;
        boost::tie(e, inserted)= boost::add_edge(vj, vk, g);
        if (inserted) {
          om[e]=blame;
        }
      }
    }
  }
}


Ints find_parents(const IMP::internal::Map<Particle*, Ints>  &map,
                  DGVertex v,
                  const DependencyGraph &dg) {
  typedef boost::reverse_graph<DependencyGraph> RG;
  Ints vals;
  CollectVisitor<RG> cv(dg, map, vals);
  RG rg(dg);
  boost::vector_property_map<int> color(boost::num_vertices(rg));
  boost::depth_first_visit(rg, v, cv, color);
  return cv.get_collected();
}
}

InteractionGraph get_interaction_graph(const ParticlesTemp &ps,
                                       const RestraintsTemp &irs) {
  InteractionGraph ret(ps.size());
  RestraintsTemp rs= get_restraints(irs.begin(), irs.end());
  //Model *m= ps[0]->get_model();
  IMP::internal::Map<Particle*, int> map;
  IGVertexMap pm= boost::get(boost::vertex_name, ret);
  DependencyGraph dg = get_dependency_graph(rs);
  /*IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "dependency graph is \n");
    IMP::internal::show_as_graphviz(dg, std::cout);
    }*/
  for (unsigned int i=0; i< ps.size(); ++i) {
    ParticlesTemp t= get_dependent_particles(ps[i], dg);
    for (unsigned int j=0; j< t.size(); ++j) {
      IMP_USAGE_CHECK(map.find(t[j]) == map.end(),
                      "Currently particles which depend on more "
                      << "than one particle "
                      << "from the input set are not supported."
                      << "  Particle \"" << t[j]->get_name()
                      << "\" depends on \"" << ps[i]->get_name()
                      << "\" and \"" << ps[map.find(t[i])->second]->get_name()
                      << "\"");
      map[t[j]]= i;
    }
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE, "Particle \"" << ps[i]->get_name() << "\" controls ");
      for (unsigned int i=0; i< t.size(); ++i) {
        IMP_LOG(VERBOSE, "\""<< t[i]->get_name() << "\" ");
      }
      IMP_LOG(VERBOSE, std::endl);
    }
    pm[i]= ps[i];
  }
  for (RestraintsTemp::const_iterator it= rs.begin();
       it != rs.end(); ++it) {
    ParticlesTemp pl= (*it)->get_input_particles();
    add_edges(ps, pl, map, *it, ret);
  }
  ScoreStatesTemp ss= get_required_score_states(rs);
  for (ScoreStatesTemp::const_iterator it= ss.begin();
       it != ss.end(); ++it) {
    ParticlesTemp pl= (*it)->get_input_particles();
    add_edges(ps, pl, map, *it, ret);
    ParticlesTemp opl= (*it)->get_output_particles();
    add_edges(ps, opl, map, *it, ret);
  }
  IMP_INTERNAL_CHECK(boost::num_vertices(ret) == ps.size(),
                     "Wrong number of vertices "
                     << boost::num_vertices(ret)
                     << " vs " << ps.size());
  return ret;
}


display::Geometries
get_interaction_graph_geometry(const InteractionGraph &ig) {
  display::Geometries ret;
  IGVertexConstMap vm= boost::get(boost::vertex_name, ig);
  IGEdgeConstMap em= boost::get(boost::edge_name, ig);
  IMP::internal::Map<std::string, display::Color> colors;
  for (std::pair<IGTraits::vertex_iterator,
         IGTraits::vertex_iterator> be= boost::vertices(ig);
       be.first != be.second; ++be.first) {
    Particle *p= dynamic_cast<Particle*>(vm[*be.first]);
    core::XYZ pd(p);
    for (std::pair<IGTraits::out_edge_iterator,
           IGTraits::out_edge_iterator> ebe= boost::out_edges(*be.first, ig);
         ebe.first != ebe.second; ++ebe.first) {
      unsigned int target= boost::target(*ebe.first, ig);
      if (target > *be.first) continue;
      Particle *op= dynamic_cast<Particle*>(vm[target]);
      core::XYZ od(op);
      std::string on= em[*ebe.first]->get_name();
      IMP_NEW(display::SegmentGeometry, cg,
              (algebra::Segment3D(pd.get_coordinates(),
                                  od.get_coordinates())));
      if (colors.find(em[*ebe.first]->get_name()) == colors.end()) {
        colors[em[*ebe.first]->get_name()]
          = display::get_display_color(colors.size());
      }
      cg->set_color(colors[em[*ebe.first]->get_name()]);
      cg->set_name(on);
      ret.push_back(cg);
    }
  }
  return ret;
}


display::Geometries
get_subset_graph_geometry(const SubsetGraph &ig) {
  display::Geometries ret;
  SGConstVertexMap vm= boost::get(boost::vertex_name, ig);
  for (std::pair<IGTraits::vertex_iterator,
         IGTraits::vertex_iterator> be= boost::vertices(ig);
       be.first != be.second; ++be.first) {
    Subset s= vm[*be.first];
    display::Color c= display::get_display_color(*be.first);
    for (unsigned int i=0; i< s.size(); ++i) {
      core::XYZ pi(s[i]);
      IMP_NEW(display::SphereGeometry, cg,
              (algebra::Sphere3D(pi.get_coordinates(), 1)));
      cg->set_color(c);
      cg->set_name(s.get_name());
      ret.push_back(cg);
    }
  }
  return ret;
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


namespace {
  struct LessDegree {
    const InteractionGraph &ig_;
    LessDegree(InteractionGraph &ig): ig_(ig){}
    bool operator()(int a, int b) const {
      return boost::degree(a, ig_) > boost::degree(b, ig_);
    }
  };
  void triangulate(InteractionGraph &ig) {
    typedef std::pair<IGTraits::adjacency_iterator,
      IGTraits::adjacency_iterator>
      AdjacencyRange;
    typedef std::pair<IGTraits::vertex_iterator, IGTraits::vertex_iterator>
      VertexRange;
    typedef std::pair<IGTraits::out_edge_iterator, IGTraits::out_edge_iterator>
      EdgeRange;
    InteractionGraph mig;
    boost::copy_graph(ig, mig);
    IMP::internal::Map<Particle*, int> vmap;
    IGVertexMap mpm= boost::get(boost::vertex_name, mig);
    for(VertexRange be = boost::vertices(ig);
        be.first != be.second; ++be.first) {
      /*std::cout << "Vertex " << *be.first
        << " is particle " << boost::get(mpm, *be.first)->get_name()
        << std::endl;*/
      vmap[boost::get(mpm, *be.first)]=*be.first;
    }
    while (boost::num_vertices(mig) >0) {
      int maxv=-1;
      int maxd=std::numeric_limits<int>::max();
      for ( VertexRange be = boost::vertices(mig);
            be.first != be.second; ++be.first) {
        int d= boost::degree(*be.first, mig);
        if (d < maxd) {
          maxd=d;
          maxv=*be.first;
        }
      }
      //unsigned int v= vmap.find(mpm[maxv])->second;
      /*std::cout << "Triangulating vertex "
        << vmap.find(mpm[maxv])->second
        << " with degree " << maxd << std::endl;
        std::cout << boost::num_vertices(mig)
        << " remaining" << std::endl;*/
      AdjacencyRange be  = boost::adjacent_vertices(maxv, mig);
      const std::vector<unsigned int> neighbors(be.first, be.second);
      /*std::cout << "Neighbors are ";
        for (unsigned int i=0; i < neighbors.size(); ++i) {
        std::cout << neighbors[i] << " ";
        }
        std::cout << std::endl;*/
      for (unsigned int i=1; i< neighbors.size(); ++i) {
        //std::cout << "neighbor 0 is "
        // << boost::get(mpm, neighbors[i])->get_name() << std::endl;
        IGVertex o0 =  vmap.find(boost::get(mpm, neighbors[i]))->second;
        for (unsigned int j=0; j<i; ++j) {
          /*std::cout << "neighbor 1 is "
            << boost::get(mpm, neighbors[j])->get_name() << std::endl;*/
          IGVertex o1 = vmap.find(boost::get(mpm, neighbors[j]))->second;
          // check for adjacency in ig, ick. painful
          AdjacencyRange be01 = boost::adjacent_vertices(o0, ig);
          for (;be01.first != be01.second; ++be01.first) {
            if (*be01.first == o1) break;
          }
          // connect if not adjacent
          if (be01.first == be01.second) {
            //std::cout << "Connecting " << o0 << " " << o1 << std::endl;
            //std::cout << "and " << neighbors[i] << " "
            //<< neighbors[j] << std::endl;
            boost::add_edge(neighbors[i], neighbors[j], mig);
            boost::add_edge(o0, o1, ig);
          } else {
            //std::cout << "Already connected " << o0
            //<< " " << o1 << std::endl;
          }
        }
      }
      EdgeRange er=boost::out_edges(maxv, mig);
      while (er.first != er.second) {
        boost::remove_edge(*er.first, mig);
        er= boost::out_edges(maxv, mig);
      }
      boost::remove_vertex(maxv, mig);
      /*std::cout << "MIG graph is " << std::endl;
        IMP::internal::show_as_graphviz(mig, std::cout);
        std::cout << "Output graph is " << std::endl;
        IMP::internal::show_as_graphviz(ig, std::cout);*/
    }
  }

}

SubsetGraph get_junction_tree(const InteractionGraph &ig) {
  InteractionGraph cig;
  IGVertexMap pm= boost::get(boost::vertex_name, cig);
  boost::copy_graph(ig, cig);
  /*std::cout << "Input graph is " << std::endl;
    IMP::internal::show_as_graphviz(ig, std::cout);*/
  triangulate(cig);
  /* std::cout << "Triangulated graph is " << std::endl;
     IMP::internal::show_as_graphviz(cig, std::cout);*/
  typedef std::vector<IGVertex> Clique;
  std::vector<Clique> cliques;
  internal::maximal_cliques(cig, std::back_inserter(cliques));
  for (unsigned int i=0; i< cliques.size(); ++i) {
    /*std::cout << "Clique is ";
      for (unsigned int j=0; j< cliques[i].size(); ++j) {
      std::cout << cliques[i][j] << " ";
      }*/
    std::sort(cliques[i].begin(), cliques[i].end());
  }
  CliqueGraph cg(cliques.size());
  for (unsigned int i=0; i< cliques.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      Clique intersection;
      std::set_intersection(cliques[i].begin(),
                            cliques[i].end(),
                            cliques[j].begin(),
                            cliques[j].end(),
                            std::back_inserter(intersection));
      if (!intersection.empty()) {
        double minus_weight=intersection.size();
        /*std::cout << "edge " << i << " " << j
          << " has weight "
          << -static_cast<int>(intersection.size()) << std::endl;*/
        boost::add_edge(i, j,
                        CliqueGraph::edge_property_type(-minus_weight),
                        cg);
      }
    }
  }
  std::vector<CGEdge> mst;
  boost::kruskal_minimum_spanning_tree(cg, std::back_inserter(mst));
  SubsetGraph jt(cliques.size());
  SGVertexMap cm= boost::get(boost::vertex_name, jt);
  for (unsigned int i=0; i< cliques.size(); ++i) {
    ParticlesTemp ps;
    for (unsigned int j=0; j< cliques[i].size(); ++j) {
      ps.push_back(boost::get(pm, cliques[i][j]));
    }
    Subset lsc(ps);
    boost::put(cm, i, lsc);
  }
  for (unsigned int i=0; i< mst.size(); ++i) {
    boost::add_edge(boost::source(mst[i], cg),
                    boost::target(mst[i], cg), jt);
  }
  // find disjoint sets and connect them arbitrarily
  {
    typedef boost::vector_property_map<unsigned int> Index;
    typedef Index Parent;
    typedef boost::disjoint_sets<Index,Parent> UF;
    Index index;
    Parent parent;
    UF uf(index, parent);
    for (unsigned int i=0; i< cliques.size(); ++i) {
      uf.make_set(i);
    }
    for (std::pair<SGTraits::edge_iterator,
           SGTraits::edge_iterator> be= boost::edges(jt);
         be.first != be.second; ++be.first) {
      uf.union_set(boost::source(*be.first, jt),
                   boost::target(*be.first, jt));
    }
    for (unsigned int i=1; i< cliques.size(); ++i) {
      if (uf.find_set(i) != uf.find_set(i-1)) {
        boost::add_edge(i,i-1, jt);
        uf.union_set(i,i-1);
      }
    }
  }

  /*std::cout << "JT graph is " << std::endl;
    IMP::internal::show_as_graphviz(jt, std::cout);
    {
    SubsetGraph njt=jt;
    std::cout << "JT graph is " << std::endl;
    IMP::internal::show_as_graphviz(njt, std::cout);
    }
    std::cout << "JT graph is " << std::endl;
    IMP::internal::show_as_graphviz(jt, std::cout);*/
  return jt;
}


struct AncestorException{
  Object *o;
  AncestorException(Object *oi): o(oi){};
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
                       const Graph& g) {
    Object *o= vm_[u];
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

namespace {

  bool get_has_ancestor(const DependencyGraph &g,
                        unsigned int v,
                        const ParticlesTemp &pst) {
    typedef boost::reverse_graph<DependencyGraph>  RG;
    RG rg(g);
    AncestorVisitor<RG> av(pst,g);
    boost::vector_property_map<int> color(boost::num_vertices(g));
    try {
      //std::cout << "Searching for dependents of " << v << std::endl;
      boost::depth_first_visit(rg, v, av, color);
      return false;
    } catch (AncestorException e) {
      /*IMP_LOG(VERBOSE, "Vertex has ancestor \"" << e.o->get_name()
        << "\"" << std::endl);*/
      return true;
    }
  }
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
  return !get_has_ancestor(dg, cv, pst);
}


namespace {
  void optimize_restraint(Restraint *r, RestraintSet *p,
                          const DependencyGraph &dg,
                          const IMP::internal::Map<Object*,
                          unsigned int> &index,
                          const ParticlesTemp &pst,
                          boost::ptr_vector<ScopedRemoveRestraint> &removed,
                          boost::ptr_vector<ScopedRestraint> &added) {
    ContainersTemp ic= r->get_input_containers();
    for (unsigned int i=0; i< ic.size(); ++i) {
      if (get_has_ancestor(dg, index.find(ic[i])->second, pst)) {
        return;
      }
    }
    Restraints rs= r->get_decomposition();
    if (rs.size()==0 || (rs.size() >=1 && rs[0] != r)) {
      IMP_LOG(TERSE, "Restraint \"" << r->get_name()
              << "\" is being decompsed into " << rs.size() << " restraints"
              << std::endl);
      IMP_NEW(RestraintSet, rss, (r->get_name()));
      rss->add_restraints(rs);
      double max= r->get_model()->get_maximum_score(r);
      if (max < std::numeric_limits<double>::max()) {
        for (unsigned int i=0; i< rs.size(); ++i) {
          r->get_model()->set_maximum_score(rs[i], max);
        }
      }
      removed.push_back(new ScopedRemoveRestraint(r, p));
      added.push_back(new ScopedRestraint(rss, p));
    } else {
    }
  }

  void optimize_restraint_parent(RestraintSet *p,
                                 const DependencyGraph &dg,
                                 const IMP::internal::Map<Object*,
                                 unsigned int> &index,
                                 const ParticlesTemp &pst,
                           boost::ptr_vector<ScopedRemoveRestraint> &removed,
                                 boost::ptr_vector<ScopedRestraint> &added) {
    Restraints all(p->restraints_begin(), p->restraints_end());
    for (unsigned int i=0; i < all.size(); ++i) {
      Restraint *r=all[i];
      RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
      if (rs) {
        optimize_restraint_parent(rs, dg, index, pst,
                                  removed,
                                  added);
      } else {
        optimize_restraint(all[i], p, dg, index, pst,
                           removed, added);
      }
    }
  }

  void make_static_cpc(core::internal::CoreClosePairContainer *cpc,
                       const ParticlesTemp &optimized,
                       const ParticleStatesTable *pst,
                       core::internal::CoreClosePairContainers &staticed) {
    IMP_LOG(TERSE, "Making container \"" << cpc->get_name()
            << "\" static " << std::endl);
    ParticleStatesList psl(optimized.size());
    unsigned int max=0;
    for (unsigned int i=0; i< optimized.size(); ++i) {
      psl[i]= pst->get_particle_states(optimized[i]);
      max= std::max(psl[i]->get_number_of_states(), max);
    }
    ParticlesTemp inputs
      = cpc->get_singleton_container()->get_contained_particles();
    algebra::BoundingBox3Ds bbs(inputs.size());
    for (unsigned int i=0; i< max; ++i) {
      for (unsigned int j=0; j< optimized.size(); ++j) {
        psl[j]->load_state(std::min(i, psl[j]->get_number_of_states()-1),
                           optimized[j]);
      }
      // make sure invariants are updated
      cpc->get_model()->update();
      for (unsigned int j=0; j< inputs.size(); ++j) {
        core::XYZ d(inputs[j]);
        bbs[j]+= d.get_coordinates();
      }
    }
    cpc->set_is_static(true, bbs);
    staticed.push_back(cpc);
    IMP_LOG(VERBOSE, cpc->get_number_of_particle_pairs()
            << "pairs" << std::endl);
  }

  void optimize_container_parent(RestraintSet *p,
                                 const DependencyGraph &dg,
                                 const ParticlesTemp &optimized,
                                 const ParticleStatesTable *pst,
                   core::internal::CoreClosePairContainers &staticed) {
    DGConstVertexMap pm=boost::get(boost::vertex_name, dg);
    ParticlesTemp particles=pst->get_particles();
    for (std::pair< DGTraits::vertex_iterator,
            DGTraits::vertex_iterator> be
           = boost::vertices(dg); be.first != be.second; ++be.first) {
      core::internal::CoreClosePairContainer *cpc
        = dynamic_cast<core::internal::CoreClosePairContainer*>(pm[*be.first]);
      if (cpc && get_has_ancestor(dg, *be.first, particles)) {
        make_static_cpc(cpc, optimized,pst, staticed);
      }
    }
  }
}


void OptimizeRestraints::optimize_model(RestraintSet *m,
                                        const ParticlesTemp &particles) {
  IMP::internal::Map<Object*, unsigned int> index;
  //std::cout << "new gra[j is \n";
  //IMP::internal::show_as_graphviz(m->get_dependency_graph(), std::cout);
  const DependencyGraph dg
    =get_dependency_graph(get_restraints(m));
  DGConstVertexMap vm= boost::get(boost::vertex_name,dg);
  unsigned int nv=boost::num_vertices(dg);
  for (unsigned int i=0; i< nv; ++i) {
    index[vm[i]]= i;
  }
  optimize_restraint_parent(m,
                            dg, index,
                            particles, removed_,  added_);
}



void OptimizeContainers::optimize_model(RestraintSet *m,
                                        const ParticleStatesTable *pst) {
  optimize_container_parent(m,
                            get_dependency_graph(get_restraints(m)),
                            pst->get_particles(), pst, staticed_);
}

void OptimizeContainers::unoptimize_model() {
  for (unsigned int i=0; i< staticed_.size(); ++i) {
    staticed_[i]->set_is_static(false, algebra::BoundingBox3Ds());
  }
}



void load_particle_states(const Subset &s,
                          const SubsetState &ss,
                          const ParticleStatesTable *pst) {
  internal::load_particle_states(s.begin(), s.end(), ss, pst);
}



RestraintSet* create_restraint_set(const Subset &s,
                                   ParticlesTemp other,
                                   const DependencyGraph &dg,
                                   RestraintSet *rs) {
  std::sort(other.begin(), other.end());
  ParticlesTemp oms;
  std::set_difference(other.begin(), other.end(),
                      s.begin(), s.end(),
                      std::back_inserter(oms));
  RestraintsTemp allr= get_restraints(rs);
  std::sort(allr.begin(), allr.end());
  DGConstVertexMap vm= boost::get(boost::vertex_name,dg);
  IMP_NEW(RestraintSet, ret, (std::string("Restraints for ") +s.get_name()));
  rs->get_model()->add_restraint(ret);
  IMP::internal::Map<double, Pointer<RestraintSet> > rss;
  for (unsigned int i=0; i< boost::num_vertices(dg); ++i) {
    Restraint *r=dynamic_cast<Restraint*>(boost::get(vm, i));
    if (r && std::binary_search(allr.begin(), allr.end(), r)) {
      if (get_has_ancestor(dg, i, oms)) {
        // pass
      } else {
        double w= rs->get_model()->get_weight(r);
        if (rss.find(w)==rss.end()) {
          rss[w]=new RestraintSet();
          rss[w]->add_restraint(r);
          rss[w]->set_weight(w);
          ret->add_restraint(rss[w]);
        } else {
          rss.find(w)->second->add_restraint(r);
        }
      }
    }
  }
  return ret.release();
}


IMPDOMINO2_END_NAMESPACE
