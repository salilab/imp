/**
 *  \file CompositeRestraint.cpp  \brief Composite restraint.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

// Work around Boost bug with adjacency_matrix in 1.60:
// https://svn.boost.org/trac/boost/ticket/11880
#include <boost/version.hpp>
#if BOOST_VERSION == 106000
# include <boost/type_traits/ice.hpp>
#endif

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/random/uniform_int.hpp>
#include <algorithm>

#include <IMP/random.h>
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
  typedef FullGraph::edge_property_type FullGraphWeight;

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

  void compute_mst(Model *m, const TypedParticles &tps,
                   PairScore *ps, Graph &g) {
    // Create fully connected graph
    FullGraph full_g(tps.size());
    for (unsigned int i = 0; i < tps.size(); ++i) {
      for (unsigned int j = 0; j < i; ++j) {
        double d = ps->evaluate_index(m,
                         ParticleIndexPair(tps[i].second, tps[j].second),
                         nullptr);
        boost::add_edge(i, j, static_cast<FullGraphWeight>(d), full_g);
      }
    }
    std::vector<FullGraphEdge> mst;
    boost::kruskal_minimum_spanning_tree(full_g, std::back_inserter(mst));

    // Build new graph containing only the mst edges
    FullGraphWeightMap weight_map = boost::get(boost::edge_weight, full_g);
    for (std::vector<FullGraphEdge>::const_iterator it = mst.begin();
         it != mst.end(); ++it) {
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
      if (weight < max_score) {
        neighbors.insert(t);
      }
    }
  }

  bool got_all_particle_types(const std::vector<Vertex> &subset_so_far,
                              const TypedParticles &tps,
                              int num_particle_types) {
    boost::dynamic_bitset<> have_types(num_particle_types);
    have_types.set();
    for (std::vector<Vertex>::const_iterator it = subset_so_far.begin();
         it != subset_so_far.end(); ++it) {
      have_types.reset(tps[*it].first);
    }
    return have_types.none();
  }

  void update_minimum_subgraph(std::vector<Vertex> &subgraph, Graph &g,
                               std::vector<Edge> &min_edges,
                               double &min_score) {
    std::set<Vertex> vertices;
    vertices.insert(subgraph.begin(), subgraph.end());

    // Find the set of edges that connect the subgraph vertices
    std::vector<Edge> edges;
    WeightMap weight_map = boost::get(boost::edge_weight, g);
    double score = 0.;
    boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei) {
      Vertex s = boost::source(*ei, g);
      Vertex t = boost::target(*ei, g);
      if (vertices.find(s) != vertices.end()
          && vertices.find(t) != vertices.end()) {
        edges.push_back(*ei);
        score += weight_map[*ei];
        if (score >= min_score) {
          return;
        }
      }
    }
    // The new graph is the best scoring, so update
    min_edges = edges;
    min_score = score;
  }

  void generate_connected_subgraphs(
             std::set<Vertex> &vertices_not_yet_considered,
             std::vector<Vertex> &subset_so_far, std::set<Vertex> &neighbors,
             Graph &g, const TypedParticles &tps, int num_particle_types,
             std::vector<Edge> &min_edges, double &min_score,
             double max_score) {
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
    if (got_all_particle_types(subset_so_far, tps, num_particle_types)) {
      update_minimum_subgraph(subset_so_far, g, min_edges, min_score);
    } else if (!candidates.empty()) {
      // Pick one of the candidates at random
      boost::uniform_int<unsigned> randint(0, candidates.size() - 1);
      unsigned cnum = randint(random_number_generator);
      std::set<Vertex> new_to_consider = vertices_not_yet_considered;
      new_to_consider.erase(candidates[cnum]);;
      generate_connected_subgraphs(new_to_consider, subset_so_far, neighbors,
                                   g, tps, num_particle_types, min_edges,
                                   min_score, max_score);

      std::vector<Vertex> new_subset_so_far = subset_so_far;
      new_subset_so_far.push_back(candidates[cnum]);

      std::set<Vertex> new_neighbors = neighbors;
      add_neighbors(new_neighbors, candidates[cnum], g, max_score);
      generate_connected_subgraphs(new_to_consider, new_subset_so_far,
                                   new_neighbors, g, tps, num_particle_types,
                                   min_edges, min_score, max_score);
    }
  }

  double get_best_scoring_subgraph(Graph &g, const TypedParticles &tps,
                                   int num_particle_types, double max_score,
                                   std::vector<Edge> &edges) {
    double min_score = max_score;
    std::set<Vertex> vertices_not_yet_considered;
    std::vector<Vertex> subset_so_far;
    std::set<Vertex> neighbors;
    boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = boost::vertices(g); vi != vi_end; ++vi) {
      vertices_not_yet_considered.insert(*vi);
    }
    generate_connected_subgraphs(vertices_not_yet_considered, subset_so_far,
                                 neighbors, g, tps, num_particle_types,
                                 edges, min_score, max_score);
    return min_score;
  }

  void get_particles_from_edges(const std::vector<Edge> &edges, const Graph &g,
                                const TypedParticles &tps,
                                ParticleIndexPairs &pis) {
    pis.reserve(edges.size());
    for (std::vector<Edge>::const_iterator it = edges.begin();
         it != edges.end(); ++it) {
      int i = boost::target(*it, g);
      int j = boost::source(*it, g);
      pis.push_back(ParticleIndexPair(tps[i].second, tps[j].second));
    }
  }

} // anonymous namespace

double CompositeRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
  IMP_CHECK_OBJECT(ps_.get());
  Graph g;
  compute_mst(get_model(), tps_, ps_, g);
  std::vector<Edge> edges;
  double score = get_best_scoring_subgraph(g, tps_, num_particle_types_,
                                           get_maximum_score(), edges);
  IMP_IF_LOG(VERBOSE) {
    ParticleIndexPairs pis;
    get_particles_from_edges(edges, g, tps_, pis);
    IMP_LOG_VERBOSE("Minimum subtree is [");
    for (ParticleIndexPairs::const_iterator it = pis.begin(); it != pis.end();
         ++it) {
      IMP_LOG_VERBOSE("(" << get_model()->get_particle_name((*it)[0])
                      << ", " << get_model()->get_particle_name((*it)[1])
                      << ") ");
    }
    IMP_LOG_VERBOSE("]" << std::endl);
  }
  if (accum) {
    // Need to reevaluate the score for each edge to get derivatives
    ParticleIndexPairs pis;
    get_particles_from_edges(edges, g, tps_, pis);
    return ps_->evaluate_indexes(get_model(), pis, accum, 0, pis.size());
  } else {
    return score;
  }
}

ParticleIndexPairs CompositeRestraint::get_connected_pairs() const
{
  IMP_CHECK_OBJECT(ps_.get());
  Graph g;
  compute_mst(get_model(), tps_, ps_, g);
  std::vector<Edge> edges;
  get_best_scoring_subgraph(g, tps_, num_particle_types_,
                            get_maximum_score(), edges);
  ParticleIndexPairs pis;
  get_particles_from_edges(edges, g, tps_, pis);
  return pis;
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
