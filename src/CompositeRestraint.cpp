/**
 *  \file CompositeRestraint.cpp  \brief Composite restraint.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <IMP/npc/CompositeRestraint.h>

IMPNPC_BEGIN_NAMESPACE

namespace {

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

  void generate_connected_subgraphs(
             std::set<Vertex> vertices_not_yet_considered,
             std::vector<Vertex> subset_so_far, std::set<Vertex> neighbors,
             Graph &g, std::set<Vertex> &min_vertices, double &min_score) {
  }

  double get_best_scoring_subgraph(Graph &g, double max_score) {
    double min_score = max_score;
    std::set<Vertex> min_vertices;
    //generate_connected_subgraphs(g, min_vertices, min_score);
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
