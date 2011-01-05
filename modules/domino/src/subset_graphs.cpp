/**
 *  \file domino/DominoSampler.h \brief A beyesian infererence-based
 *  sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/domino/domino_config.h>
#include <IMP/domino/subset_graphs.h>
#include <IMP/domino/internal/inference_utility.h>
#include <IMP/domino/utility.h>
#include <IMP/domino/optimize_restraints.h>
#include <IMP/domino/particle_states.h>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/copy.hpp>
#include <IMP/domino/internal/maximal_cliques.h>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif
#include <boost/pending/disjoint_sets.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>



IMPDOMINO_BEGIN_NAMESPACE


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

}

Subsets get_subsets(const SubsetGraph &g){
    boost::property_map< SubsetGraph, boost::vertex_name_t>::const_type
      subset_map= boost::get(boost::vertex_name, g);
    Subsets output;
    for (unsigned int vi = 0;vi < boost::num_vertices(g);vi++) {
      output.push_back(boost::get(subset_map,vi));
  }
    return output;
  }







SubsetGraph get_restraint_graph(RestraintSet *irs,
                                const ParticleStatesTable *pst) {
  OptimizeRestraints ro(irs, pst);
  RestraintsTemp rs= get_restraints(irs);
  //ScoreStatesTemp ss= get_required_score_states(rs);
  SubsetGraph ret(rs.size());// + ss.size());
  IMP_LOG(TERSE, "Creating restraint graph on "
          << rs.size() << " restraints." << std::endl);
  IMP::internal::Map<Particle*, int> map;
  SGVertexMap pm= boost::get(boost::vertex_name, ret);
  DependencyGraph dg = get_dependency_graph(rs);
  /*IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE, "dependency graph is \n");
    IMP::internal::show_as_graphviz(dg, std::cout);
    }*/
  Subset ps= pst->get_subset();
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
  }
  for (unsigned int i=0; i < rs.size(); ++i) {
    ParticlesTemp pl= rs[i]->get_input_particles();
    std::sort(pl.begin(), pl.end());
    pl.erase(std::unique(pl.begin(), pl.end()), pl.end());
    Subset os(pl);
    for (unsigned int j=0; j< pl.size(); ++j) {
      pl[j]= ps[map[pl[j]]];
    }
    std::sort(pl.begin(), pl.end());
    pl.erase(std::unique(pl.begin(), pl.end()), pl.end());
    Subset s(pl);
    IMP_LOG(VERBOSE, "Subset for restraint " << rs[i]->get_name()
            << " is " << s << " from " << os << std::endl);
    pm[i]=s;
  }
                  /*ScoreStatesTemp ss= get_required_score_states(rs);
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
                     << " vs " << ps.size());*/
  for (unsigned int i=0; i< boost::num_vertices(ret); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      if (internal::get_intersection(pm[i], pm[j]).size() >0) {
        boost::add_edge(i,j,ret);
        IMP_LOG(VERBOSE, "Connecting " << rs[i]->get_name()
                << " with " << rs[j]->get_name() << std::endl);
      }
    }
  }
  return ret;
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




namespace {


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

  void add_edges( const Subset &ps,
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

InteractionGraph get_interaction_graph(RestraintSet *irs,
                                       const ParticleStatesTable* pst) {
  Subset ps = pst->get_subset();
  InteractionGraph ret(ps.size());
  RestraintsTemp rs= get_restraints(irs);
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


IMPDOMINO_END_NAMESPACE
