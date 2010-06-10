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

IMPDOMINO2_BEGIN_NAMESPACE


namespace {
typedef boost::graph_traits<InteractionGraph> IGTraits;
typedef IGTraits::edge_descriptor Edge;
typedef IGTraits::vertex_descriptor Vertex;

typedef boost::property_map<InteractionGraph,
                            boost::vertex_name_t>::type ParticleMap;
typedef boost::property_map<InteractionGraph,
                            boost::edge_name_t>::type ObjectMap;


bool get_has_edge(InteractionGraph &graph,
                  Vertex va,
                  Vertex vb) {
  std::pair<IGTraits::out_edge_iterator,
    IGTraits::out_edge_iterator> edges= boost::out_edges(va, graph);
  for (; edges.first != edges.second;++edges.first) {
    if (boost::target(*edges.first, graph) == vb) return true;
  }
  return false;
}

  void add_edges(const ParticlesList &pl, const std::map<Particle*, int> &map,
                 Object *blame,
                 InteractionGraph &g) {
    ObjectMap om= boost::get(boost::edge_name, g);
    for (unsigned int i=0; i< pl.size(); ++i) {
      ParticlesTemp pt= pl[i];
      std::sort(pt.begin(), pt.end());
      pt.erase(std::unique(pt.begin(), pt.end()), pt.end());
      for (unsigned int j=0; j< pt.size(); ++j) {
        int vj=map.find(pt[j])->second;
        for (unsigned int k=0; k< j; ++k) {
          int vk= map.find(pt[k])->second;
          if (vj != vk && !get_has_edge(g, vj, vk)) {
            Edge e;
            bool inserted;
            boost::tie(e, inserted)= boost::add_edge(vj, vk, g);
            if (inserted) {
              om[e]=blame;
            }
          }
        }
      }
    }
  }
  typedef boost::graph_traits<Model::DependencyGraph> DTraits;
  typedef DTraits::vertex_descriptor DVertex;


  Ints find_parents(const std::map<Particle*, Ints>  &map,
                    DVertex v,
                    Model::DependencyGraph &dg) {
    std::vector<DVertex> front;
    front.push_back(v);
    Ints ret;
    typedef boost::property_map<Model::DependencyGraph,
      boost::vertex_name_t>::type DParticleMap;
    DParticleMap dpm= boost::get(boost::vertex_name, dg);
    do {
      DVertex v= front.back();
      front.pop_back();
      Object *o= boost::get(dpm, v);
      Particle *p=dynamic_cast<Particle*>(o);
      if (p && map.find(p) != map.end()) {
        ret.insert(ret.end(), map.find(p)->second.begin(),
                  map.find(p)->second.end());
      } else {
        typedef DTraits::in_edge_iterator InIt;
        std::pair<InIt, InIt> be= boost::in_edges(v, dg);
        IMP_USAGE_CHECK(std::distance(be.first, be.second)>0,
                        "Encountered leaf which was not known particle "
                        <<o->get_name());
        for (; be.first != be.second; ++be.first) {
          DVertex s= boost::source(*be.first, dg);
          front.push_back(s);
        }
      }
    } while (!front.empty());
    std::sort(ret.begin(), ret.end());
    ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
    return ret;
  }

class ObjectNameWriter {
     ParticleMap om_;
  public:
    ObjectNameWriter( ParticleMap om): om_(om){}
    void operator()(std::ostream& out, int v) const {
      out << "[label=\"" << boost::get(om_, v)->get_name() << "\"]";
    }
  };
}

InteractionGraph get_interaction_graph(Model *m,
                                       const ParticlesTemp &ps) {
  InteractionGraph ret(ps.size());
  std::map<Particle*, int> map;
  ParticleMap pm= boost::get(boost::vertex_name, ret);
  Model::DependencyGraph dg= m->get_dependency_graph();
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
    pm[i]= ps[i];
  }
  for (Model::RestraintIterator it= m->restraints_begin();
       it != m->restraints_end(); ++it) {
    ParticlesList pl= (*it)->get_interacting_particles();
    add_edges(pl, map, *it, ret);
  }
  for (Model::ScoreStateIterator it= m->score_states_begin();
       it != m->score_states_end(); ++it) {
    ParticlesList pl= (*it)->get_interacting_particles();
    add_edges(pl, map, *it, ret);
  }
  IMP_INTERNAL_CHECK(boost::num_vertices(ret) == ps.size(),
                     "Wrong number of vertices "
                     << boost::num_vertices(ret)
                     << " vs " << ps.size());
  return ret;
}

ParticlesTemp get_dependent_particles(Particle *p,
                                      Model::DependencyGraph &dg) {
  // find p in graph, ick
  typedef boost::property_map<Model::DependencyGraph,
    boost::vertex_name_t>::type DParticleMap;
  DParticleMap dpm= boost::get(boost::vertex_name, dg);
  typedef DTraits::vertex_iterator InIt;
  std::pair<InIt, InIt> be= boost::vertices(dg);
  std::vector<DVertex> front;
  for (; be.first != be.second; ++be.first) {
    if (dpm[*be.first]==p) {
      front.push_back(*be.first);
      break;
    }
  }
  ParticlesTemp ret(1,p);
  do {
    DVertex v= front.back();
    front.pop_back();
    Object *o= boost::get(dpm, v);
    Particle *p=dynamic_cast<Particle*>(o);
    if (p) {
      ret.push_back(p);
    }
    typedef DTraits::out_edge_iterator InIt;
    std::pair<InIt, InIt> be= boost::out_edges(v, dg);
    for (; be.first != be.second; ++be.first) {
      DVertex s= boost::target(*be.first, dg);
      front.push_back(s);
    }
  } while (!front.empty());
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
  }

ParticlesTemp get_dependent_particles(Particle *p) {
  Model *m= p->get_model();
  Model::DependencyGraph dg= m->get_dependency_graph();
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
    ParticleMap mpm= boost::get(boost::vertex_name, mig);
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
        Vertex o0 =  vmap.find(boost::get(mpm, neighbors[i]))->second;
        for (unsigned int j=0; j<i; ++j) {
          /*std::cout << "neighbor 1 is "
            << boost::get(mpm, neighbors[j])->get_name() << std::endl;*/
          Vertex o1 = vmap.find(boost::get(mpm, neighbors[j]))->second;
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

  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::undirectedS,
                                boost::property<boost::vertex_name_t,
                                                std::string>,
                                boost::property<boost::edge_weight_t,
                                                double> > CliqueGraph;
  typedef boost::graph_traits<CliqueGraph> CliqueTraits;
  typedef CliqueTraits::vertex_descriptor CliqueVertex;
  typedef CliqueTraits::edge_descriptor CliqueEdge;
  typedef boost::property_map<CliqueGraph,
                              boost::vertex_name_t>::type CliqueMap;
  typedef boost::property_map<SubsetGraph,
                              boost::vertex_name_t>::type JTMap;

}

SubsetGraph get_junction_tree(const InteractionGraph &ig) {
  InteractionGraph cig;
  ParticleMap pm= boost::get(boost::vertex_name, cig);
  boost::copy_graph(ig, cig);
  /*std::cout << "Input graph is " << std::endl;
    IMP::internal::show_as_graphviz(ig, std::cout);*/
  triangulate(cig);
  /* std::cout << "Triangulated graph is " << std::endl;
     IMP::internal::show_as_graphviz(cig, std::cout);*/
  typedef std::vector<Vertex> Clique;
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
  std::vector<CliqueEdge> mst(cliques.size()-1);
  boost::kruskal_minimum_spanning_tree(cg, mst.begin());
  SubsetGraph jt(cliques.size());
  JTMap cm= boost::get(boost::vertex_name, jt);
  for (unsigned int i=0; i< cliques.size(); ++i) {
    ParticlesTemp ps;
    for (unsigned int j=0; j< cliques[i].size(); ++j) {
      ps.push_back(boost::get(pm, cliques[i][j]));
    }
    IMP_NEW(Subset, lsc, (ps));
    lsc->set_was_used(true);
    std::ostringstream oss;
    for (unsigned int j=0; j< ps.size(); ++j) {
      oss << ps[j]->get_name();
      if (j != ps.size()-1) {
        oss << ", ";
      }
    }
    lsc->set_name(oss.str());
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

IMPDOMINO2_END_NAMESPACE
