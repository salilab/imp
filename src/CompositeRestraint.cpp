/**
 *  \file CompositeRestraint.cpp  \brief Composite restraint.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/random/uniform_int.hpp>
#include <algorithm>

#include <IMP/random.h>
#include <IMP/npc/CompositeRestraint.h>

IMPNPC_BEGIN_NAMESPACE

namespace {

  template <class LIST> void debug_print(std::string caption, const LIST &ls) {
    std::cout << caption << " ";
    for (typename LIST::const_iterator it = ls.begin(); it != ls.end(); ++it) {
      std::cout << *it << " " ;
    }
    std::cout << std::endl;
  }

  typedef std::pair<int, ParticleIndex> TypedParticle;
  typedef std::vector<TypedParticle> TypedParticles;

  /* Graph type used for the original fully-connected graph */
  typedef boost::adjacency_matrix<boost::undirectedS, boost::no_property,
                                  boost::property<boost::edge_weight_t,
                                                  double> > FullGraph;
  typedef boost::property_map<FullGraph, boost::edge_weight_t>::type
                                                 FullGraphWeightMap;
  typedef boost::graph_traits<FullGraph>::edge_descriptor FullGraphEdge;

  /* Graph type used for MST */
  typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                boost::undirectedS, boost::no_property,
                                boost::property<boost::edge_weight_t,
                                                double> > Graph;
  typedef boost::property_map<Graph, boost::edge_weight_t>::type
                                                 WeightMap;
  typedef boost::graph_traits<Graph>::edge_descriptor Edge;
  typedef Graph::edge_property_type Weight;
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

  /* debugging */
  void show_graph(const Graph &g, const TypedParticles &tps, Model *m) {
    std::cout << boost::num_vertices(g) << " vertices" << std::endl;
    std::cout << boost::num_edges(g) << " edges = ";
    boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei) {
        int s = boost::source(*ei, g);
        int t = boost::target(*ei, g);
        std::cout << "(" << m->get_particle_name(tps[s].second)
                  << "," << m->get_particle_name(tps[t].second) << ") ";
    }
    std::cout << std::endl;
  }

  void compute_mst(Model *m, const TypedParticles &tps,
                   PairScore *ps, Graph &g) {
    // Create fully connected graph
    FullGraph full_g(tps.size());
    for (unsigned int i = 0; i < tps.size(); ++i) {
      for (unsigned int j = 0; j < i; ++j) {
        double d = ps->evaluate_index(m,
                         ParticleIndexPair(tps[i].second, tps[j].second),
                         nullptr);
//      std::cout << "add edge " << i << "," << j << " => " << d << std::endl;
        boost::add_edge(i, j, d, full_g);
      }
    }
    std::vector<FullGraphEdge> mst;
    boost::kruskal_minimum_spanning_tree(full_g, std::back_inserter(mst));

    // Build new graph containing only the mst edges
    FullGraphWeightMap weight_map = boost::get(boost::edge_weight, full_g);
    for (std::vector<FullGraphEdge>::const_iterator it = mst.begin();
         it != mst.end(); ++it) {
//    std::cout << boost::source(*it, full_g) << " " << boost::target(*it, full_g) << " " << weight_map[*it] << std::endl;
      boost::add_edge(boost::source(*it, full_g),
                      boost::target(*it, full_g), weight_map[*it], g);
    }
  }

  void add_neighbors(std::set<Vertex> &neighbors, Vertex v, Graph &g,
                     double max_score) {
    WeightMap weight_map = boost::get(boost::edge_weight, g);
    boost::graph_traits<Graph>::out_edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::out_edges(v, g);
         ei != ei_end; ++ei) {
      Vertex t = boost::target(*ei, g);
      double weight = weight_map[*ei];
//      std::cout << " consider neighbor " << t << " of " << v << " weight " << weight << std::endl;
      if (weight < max_score) {
        neighbors.insert(t);
      }
    }
  }

  void generate_connected_subgraphs(
             std::set<Vertex> &vertices_not_yet_considered,
             std::vector<Vertex> &subset_so_far, std::set<Vertex> &neighbors,
             Graph &g, std::set<Vertex> &min_vertices, double &min_score,
             double max_score) {
    /*debug_print<std::set<Vertex> >("vertices_not_yet_considered",
                                   vertices_not_yet_considered);
    debug_print<std::vector<Vertex> >("subset_so_far",
                                      subset_so_far);
    debug_print<std::set<Vertex> >("neighbors",
                                   neighbors);*/
    std::vector<Vertex> candidates;
    if (subset_so_far.empty()) {
      candidates.insert(candidates.end(),
                        vertices_not_yet_considered.begin(),
                        vertices_not_yet_considered.end());
    } else {
      std::set_intersection(vertices_not_yet_considered.begin(),
                            vertices_not_yet_considered.end(),
                            neighbors.begin(), neighbors.end(),
                            std::back_inserter(candidates));
    }
    /*debug_print<std::vector<Vertex> >("candidates",
                                   candidates);*/
    if (candidates.empty()) {
      std::cout << "found subgraph ";
      for (unsigned i = 0; i < subset_so_far.size(); ++i) {
        std::cout << subset_so_far[i] << " " ;
      }
      std::cout << std::endl;
    } else {
      // Pick one of the candidates at random
      boost::uniform_int<unsigned> randint(0, candidates.size() - 1);
      unsigned cnum = randint(random_number_generator);
//    std::cout << "picked candidate #" << cnum << " = " << candidates[cnum] << std::endl;
      std::set<Vertex> new_to_consider;
      for (unsigned i = 0; i < candidates.size(); ++i) {
        if (i != cnum) {
          new_to_consider.insert(candidates[i]);
        }
      }
      generate_connected_subgraphs(new_to_consider, subset_so_far, neighbors,
                                   g, min_vertices, min_score, max_score);

      std::vector<Vertex> new_subset_so_far = subset_so_far;
      new_subset_so_far.push_back(candidates[cnum]);

      std::set<Vertex> new_neighbors = neighbors;
      add_neighbors(new_neighbors, candidates[cnum], g, max_score);
      generate_connected_subgraphs(new_to_consider, new_subset_so_far,
                                   new_neighbors, g, min_vertices, min_score,
                                   max_score);
    }
  }

  double get_best_scoring_subgraph(Graph &g, double max_score) {
    double min_score = max_score;
    std::set<Vertex> min_vertices;
    std::set<Vertex> vertices_not_yet_considered;
    std::vector<Vertex> subset_so_far;
    std::set<Vertex> neighbors;
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = boost::vertices(g); vi != vi_end; ++vi) {
      vertices_not_yet_considered.insert(*vi);
    }
    generate_connected_subgraphs(vertices_not_yet_considered, subset_so_far,
                                 neighbors, g, min_vertices, min_score,
                                 max_score);
    // score over all edges that connect min_vertices in g
  }

} // anonymous namespace

double CompositeRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
  IMP_CHECK_OBJECT(ps_.get());
  Graph g;
  compute_mst(get_model(), tps_, ps_, g);
  show_graph(g, tps_, get_model());
  double score = get_best_scoring_subgraph(g, get_maximum_score());
  return score;
}

ModelObjectsTemp CompositeRestraint::do_get_inputs() const {
  ModelObjectsTemp ret;
  ret.reserve(tps_.size());
  Model *m = get_model();
  for (TypedParticles::const_iterator it = tps_.begin(); it != tps_.end();
       ++it) {
    ret.push_back(m->get_particle(it->second));
  }
  return ret;
}

IMPNPC_END_NAMESPACE
