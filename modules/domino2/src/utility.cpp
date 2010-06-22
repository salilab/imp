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
#include <map>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <IMP/domino2/internal/maximal_cliques.h>
#include <IMP/internal/graph_utility.h>
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

IMPDOMINO2_BEGIN_NAMESPACE


template <class Graph>
class CollectVisitor: public boost::default_dfs_visitor {
  const std::map<Particle*, Ints> *lu_;
  typename boost::property_map<Graph,
                      boost::vertex_name_t>::const_type vm_;
  Ints vals_;
public:
  const Ints &get_collected() {
    std::sort(vals_.begin(), vals_.end());
    vals_.erase(std::unique(vals_.begin(), vals_.end()), vals_.end());
    return vals_;
  }
  CollectVisitor(const Graph &g,
                 const std::map<Particle*, Ints>&lu,
                 Ints &vals):
    lu_(&lu),
    vm_(boost::get(boost::vertex_name, g)),
    vals_(vals){}
  void discover_vertex(typename boost::graph_traits<Graph>::vertex_descriptor u,
                       const Graph& g) {
    Object *o= vm_[u];
    Particle *p=dynamic_cast<Particle*>(o);
    if (p) {
      std::map<Particle*, Ints>::const_iterator it= lu_->find(p);
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
    std::cout << "Visiting " << o->get_name() << std::endl;
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



  template <class It>
  RestraintsTemp get_restraints(It b, It e) {
    RestraintsTemp ret;
    for (It c=b; c!= e; ++c) {
      RestraintSet *rs=dynamic_cast<RestraintSet*>(*c);
      if (rs) {
        IMP_LOG(TERSE, "Restraint set " << rs->get_name()
                << std::endl);
        RestraintsTemp o=get_restraints(rs->restraints_begin(),
                                        rs->restraints_end(), 1.0);
        ret.insert(ret.end(), o.begin(), o.end());
      } else {
        ret.push_back(*c);
      }
    }
    return ret;
  }

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

void add_edges( ParticlesTemp pt, const std::map<Particle*, int> &map,
                Object *blame,
                InteractionGraph &g) {
  IGEdgeMap om= boost::get(boost::edge_name, g);
  std::sort(pt.begin(), pt.end());
  pt.erase(std::unique(pt.begin(), pt.end()), pt.end());
  for (unsigned int i=0; i< pt.size(); ++i) {
    int vj=map.find(pt[i])->second;
    for (unsigned int k=0; k< i; ++k) {
      int vk= map.find(pt[k])->second;
      if (vj != vk && !get_has_edge(g, vj, vk)) {
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


Ints find_parents(const std::map<Particle*, Ints>  &map,
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

InteractionGraph get_interaction_graph(Model *m,
                                       const ParticlesTemp &ps) {
  InteractionGraph ret(ps.size());
  std::map<Particle*, int> map;
  IGVertexMap pm= boost::get(boost::vertex_name, ret);
  DependencyGraph dg
    = get_dependency_graph(ScoreStatesTemp(m->score_states_begin(),
                                           m->score_states_end()),
                           get_restraints(m->restraints_begin(),
                                          m->restraints_end(),
                                          1.0).first);
  for (unsigned int i=0; i< ps.size(); ++i) {
    ParticlesTemp t= get_dependent_particles(ps[i], dg);
    for (unsigned int j=0; j< t.size(); ++j) {
      IMP_USAGE_CHECK(map.find(t[j]) == map.end(),
                      "Currently particles which depend on more "
                      << "than one particle "
                      << "from the input set are not supported."
                      << "  Particle " << t[j]->get_name()
                      << " depends on " << ps[i]->get_name()
                      << " and " << ps[map.find(t[i])->second]->get_name());
      map[t[j]]= i;
    }
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE, "Particle " << ps[i]->get_name() << " controls ");
      for (unsigned int i=0; i< t.size(); ++i) {
        IMP_LOG(VERBOSE, t[i]->get_name() << " ");
      }
      IMP_LOG(VERBOSE, std::endl);
    }
    pm[i]= ps[i];
  }
  for (Model::RestraintIterator it= m->restraints_begin();
       it != m->restraints_end(); ++it) {
    ParticlesTemp pl= (*it)->get_input_particles();
    add_edges(pl, map, *it, ret);
  }
  for (Model::ScoreStateIterator it= m->score_states_begin();
       it != m->score_states_end(); ++it) {
    ParticlesTemp pl= (*it)->get_input_particles();
    add_edges(pl, map, *it, ret);
  }
  IMP_INTERNAL_CHECK(boost::num_vertices(ret) == ps.size(),
                     "Wrong number of vertices "
                     << boost::num_vertices(ret)
                     << " vs " << ps.size());
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
    = get_dependency_graph(ScoreStatesTemp(m->score_states_begin(),
                                           m->score_states_end()),
                           get_restraints(m->restraints_begin(),
                                          m->restraints_end(),
                                          1.0).first);
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
    std::map<Particle*, int> vmap;
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
  std::vector<CGEdge> mst(cliques.size()-1);
  boost::kruskal_minimum_spanning_tree(cg, mst.begin());
  SubsetGraph jt(cliques.size());
  SGVertexMap cm= boost::get(boost::vertex_name, jt);
  for (unsigned int i=0; i< cliques.size(); ++i) {
    ParticlesTemp ps;
    for (unsigned int j=0; j< cliques[i].size(); ++j) {
      ps.push_back(boost::get(pm, cliques[i][j]));
    }
    Subset lsc(ps, true);
    boost::put(cm, i, lsc);
  }
  for (unsigned int i=0; i< mst.size(); ++i) {
    boost::add_edge(boost::source(mst[i], cg),
                    boost::target(mst[i], cg), jt);
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
  const ParticleStatesTable* pst_;
  typename boost::property_map<Graph,
                               boost::vertex_name_t>::const_type vm_;
public:
  AncestorVisitor(){}
  AncestorVisitor(const ParticleStatesTable *pst,
                  const Graph&g): pst_(pst),
                                  vm_(boost::get(boost::vertex_name, g)){}
  void discover_vertex(typename boost::graph_traits<Graph>::vertex_descriptor u,
                       const Graph& g) {
    Object *o= vm_[u];
    std::cout << "Visiting " << o->get_name() << std::endl;
    if (pst_->get_has_particle(dynamic_cast<Particle*>(o))) {
      throw AncestorException(o);
    }
  }
  template <class Edge>
  void tree_edge(Edge e, const Graph &g) {
    typename boost::graph_traits<Graph>::vertex_descriptor s
      = boost::source(e, g);
    typename boost::graph_traits<Graph>::vertex_descriptor t
      = boost::target(e, g);
    std::cout << "Tree edge " << vm_[s]->get_name()
              << "->" << vm_[t]->get_name() << std::endl;
  }
};

namespace {

  bool get_has_ancestor(const DependencyGraph &g,
                        unsigned int v,
                        const ParticleStatesTable *pst) {
    typedef boost::reverse_graph<DependencyGraph>  RG;
    RG rg(g);
    AncestorVisitor<RG> av(pst,g);
    boost::vector_property_map<int> color(boost::num_vertices(g));
    try {
      std::cout << "Searching for dependents of " << v << std::endl;
      boost::depth_first_visit(rg, v, av, color);
      return false;
    } catch (AncestorException e) {
      IMP_LOG(VERBOSE, "Vertex has ancestor " << e.o->get_name() << std::endl);
      return true;
    }
  }
}


bool
get_is_static_container(Container *c,
                        const DependencyGraph &dg,
                        const ParticleStatesTable *pst) {
  typedef DGTraits::in_edge_iterator IEIt;
  typedef DGTraits::vertex_iterator DVIt;
  DGConstVertexMap pm=boost::get(boost::vertex_name, dg);
  int cv;
  for (std::pair<DVIt, DVIt> be= boost::vertices(dg);
       be.first != be.second; ++be.first) {
    if (boost::get(pm, *be.first)== c) {
      cv=*be.first;
      break;
    }
  }
  return get_has_ancestor(dg, cv, pst);
}


namespace {
  template <class Parent>
  void optimize_restraint(Restraint *r, Parent *p,
                          const DependencyGraph &dg,
                          const std::map<Object*, unsigned int> &index,
                          const ParticleStatesTable *pst,
                          Restraints &removed,
                          Restraints &added) {
    ContainersTemp ic= r->get_input_containers();
    for (unsigned int i=0; i< ic.size(); ++i) {
      IMP_LOG(TERSE, "Checking container " << ic[i]->get_name() << std::endl);
      if (get_has_ancestor(dg, index.find(ic[i])->second, pst)) {
        IMP_LOG(TERSE, "Restraint " << r->get_name()
                << " depends on dynamic container " << ic[i]->get_name()
                << " not decomposed." << std::endl);
        return;
      }
    }
    Restraints rs= r->get_decomposition();
    if (rs.size()==0 || (rs.size() >=1 && rs[0] != r)) {
      IMP_LOG(TERSE, "Restraint " << r->get_name()
              << " is being decompsed into " << rs.size() << " restraints"
              << std::endl);
      removed.push_back(r);
      p->remove_restraint(r);
      for (unsigned int i=0; i< rs.size(); ++i) {
        p->add_restraint(rs[i]);
      }
      added.insert(added.end(), rs.begin(), rs.end());
    } else {
      IMP_LOG(TERSE, "Restraint " << r->get_name()
              << " cannot be decomposed" << std::endl);
    }
  }

  template <class Parent>
  void optimize_restraint_parent(Parent *p,
                                 const DependencyGraph &dg,
                                 const std::map<Object*, unsigned int> &index,
                                 const ParticleStatesTable *pst,
                      std::map<Pointer<Object>, Restraints> &reverse_removed,
                      std::map<Pointer<Object>, Restraints> &reverse_added) {
    Restraints all(p->restraints_begin(), p->restraints_end());
    for (unsigned int i=0; i < all.size(); ++i) {
      Restraint *r=all[i];
      RestraintSet *rs= dynamic_cast<RestraintSet*>(r);
      if (rs) {
        optimize_restraint_parent(rs, dg, index, pst,
                                  reverse_removed, reverse_added);
      } else {
        Restraints added, removed;
        optimize_restraint(all[i], p, dg, index, pst,
                           removed, added);
        reverse_removed[p].insert(reverse_removed[p].end(),
                                  removed.begin(), removed.end());
        reverse_added[p].insert(reverse_added[p].end(),
                                added.begin(), added.end());
      }
    }
  }

  void make_static_cpc(core::internal::CoreClosePairContainer *cpc,
                       const ParticlesTemp &optimized,
                       ParticleStatesTable *pst) {
    IMP_LOG(TERSE, "Making container " << cpc->get_name()
            << "static " << std::endl);
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
        psl[j]->load_state(std::min(i, psl[j]->get_number_of_states()),
                           optimized[j]);
      }
      for (unsigned int j=0; j< inputs.size(); ++j) {
        core::XYZ d(inputs[j]);
        bbs[j]+= d.get_coordinates();
      }
    }
    cpc->set_is_static(true, bbs);
    IMP_LOG(VERBOSE, cpc->get_number_of_particle_pairs()
            << "pairs" << std::endl);
  }

  template <class Parent>
  void optimize_container_parent(Parent *p,
                                 const DependencyGraph &dg,
                                 const ParticlesTemp &optimized,
                                 ParticleStatesTable *pst) {
    DGConstVertexMap pm=boost::get(boost::vertex_name, dg);
    for (std::pair<typename DGTraits::vertex_iterator,
           typename DGTraits::vertex_iterator> be
           = boost::vertices(dg); be.first != be.second; ++be.first) {
      core::internal::CoreClosePairContainer *cpc
        = dynamic_cast<core::internal::CoreClosePairContainer*>(pm[*be.first]);
      if (cpc && get_has_ancestor(dg, *be.first, pst)) {
        make_static_cpc(cpc, optimized, pst);
      }
    }
  }
}


void optimize_model(Model *m,
                    ParticleStatesTable *pst) {
  ParticlesTemp optimized_particles= pst->get_particles();
  optimize_container_parent(m,
      get_dependency_graph(ScoreStatesTemp(m->score_states_begin(),
                                           m->score_states_end()),
                           get_restraints(m->restraints_begin(),
                                          m->restraints_end(),
                                          1.0).first),
                            optimized_particles, pst);
  std::map<Pointer<Object>, Restraints> added, removed;
  std::map<Object*, unsigned int> index;
  //std::cout << "new gra[j is \n";
  //IMP::internal::show_as_graphviz(m->get_dependency_graph(), std::cout);
  const DependencyGraph dg
    =get_dependency_graph(ScoreStatesTemp(m->score_states_begin(),
                                          m->score_states_end()),
                          get_restraints(m->restraints_begin(),
                                         m->restraints_end(),
                                         1.0).first);
  DGConstVertexMap vm= boost::get(boost::vertex_name,dg);
  unsigned int nv=boost::num_vertices(dg);
  for (unsigned int i=0; i< nv; ++i) {
    index[vm[i]]= i;
  }
  optimize_restraint_parent(m, dg, index,
                            pst, removed, added);
}


IMPDOMINO2_END_NAMESPACE
