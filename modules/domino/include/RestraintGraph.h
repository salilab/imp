/**
 *  \file RestraintGraph.h
 *  \brief creates a MRF from a set of particles and restraints
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMPDOMINO_RESTRAINT_GRAPH_H
#define __IMPDOMINO_RESTRAINT_GRAPH_H

#include "domino_exports.h"
#include "IMP/Model.h"
#include "IMP/Restraint.h"
#include "IMP/restraints/RestraintSet.h"
#include "JNode.h"
#include "JEdge.h"
#include "DiscreteSampler.h"

#include <vector>
#include <map>
#include <sstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>

namespace IMP
{

namespace domino
{

template < typename TimeMap > class dfs_time_visitor :
      public boost::default_dfs_visitor
{
  typedef typename boost::property_traits <TimeMap >::value_type T;
public:
  dfs_time_visitor(TimeMap dmap, TimeMap fmap, T & t)
      :  m_dtimemap(dmap), m_ftimemap(fmap), m_time(t) {
  }
  template < typename Vertex, typename Graph >
  void discover_vertex(Vertex u, const Graph & g) const {
    put(m_dtimemap, u, m_time++);
  }
  template < typename Vertex, typename Graph >
  void finish_vertex(Vertex u, const Graph & g) const {
    put(m_ftimemap, u, m_time++);
  }
  TimeMap m_dtimemap;
  TimeMap m_ftimemap;
  T & m_time;
};

// should think about something more general here, since each level of
// hierarchy will have its own state.
typedef std::pair<unsigned int, unsigned int> Pair;

class IMPDOMINOEXPORT RestraintGraph
{
public:
  //! Constructor
  /** \param[in] filename the file holds the graph structure (nodes and edges)
   */
  RestraintGraph(const std::string & filename, Model *mdl);
  //    void clear_states();
  void set_model(IMP::Model *m_);
  void initialize_graph(int number_of_nodes);
  void parse_jt_file(const std::string &filename, Model *mdl);
  void move_model2global_minimum() const;
  CombState * get_minimum_comb() const;
  //! Creates a new node and add it to the graph
  /** \param[in] node_index the index of the node
      \param[in] particles  the particles that are part of the node
   */
  void add_node(unsigned int node_index, Particles &particles);

  //! Creates an undirected edge between two nodes
  /** \param[in] node1_ind  the index of the first node
      \param[in] node2_ind  the index of the second node
   */
  void add_edge(unsigned int node1_ind, unsigned int node2_ind);
  void set_sampling_space(const DiscreteSampler &ds);

  //! Initalize potentials according to the input restraint set.
  /** \param[in] rs  the restraint set
   */
  void initialize_potentials(Restraint &r, Float weight);
  unsigned int number_of_nodes() const {
    return  num_vertices(g);
  }
  unsigned int number_of_edges() const {
    return  num_edges(g);
  }
  void infer();

  //! Show the restraint graph
  void show(std::ostream& out = std::cout) const;
  //! Prints the value of each restraint encoded in the graph for a state of
  //! the global minimum
  void analyse(std::ostream& out = std::cout) const;
  //! Sets the optimizable attributes of the optimizable components to the
  //! values that build the minimum of the scoring function when the state
  //! of the root of the junction tree is of a spcific index.
  /** \param[in] state_index the index of the state of the root node of
                 the junction tree.
   */
  void show_sampling_space(std::ostream& out = std::cout) const;

  //  float move_model2state(unsigned int state_index) const;

  Float get_minimum_score() const {
    std::stringstream err_msg;
    err_msg << "RestraintGraph::get_minimum_score the graph has not been"
            << " infered";
    IMP_assert(infered, err_msg.str());
    return (*(min_combs->begin()))->get_total_score();
  }
  void clear();
protected:
  //! Determine a DFS
  /** \param[in]  root_ind the index of the node from which the DFS starts
      \param[out] discover_time stores the discover order of the nodes.
                  discover_time[i] is the discover time in the DFS of node
                  with index i.
   */
  void dfs_order(unsigned int root_ind);
  void move_model2state_rec(unsigned int father_ind,
                            CombState &best_state) const;
  //! \return a node that contains the input set of particles.
  /** It might be that there is more than one such one. The function returns
      the first it finds.
     \param[in] p the set of particles
     \return a node that contains the input set of particles
     \exception IMP exception if none of the graph nodes contain the given
                set of particles.
   */
  JNode * get_node(const Particles &p);
  JEdge* get_edge(unsigned int n1, unsigned int n2) const {
    return edge_data.find(get_edge_key(n1, n2))->second;
  }

  //! Recursive Collect Evidence, father is the cluster that invoked
  //! CollectEvidence
  /** \param[in]  father_ind the index of the node to start collecting from
   */
  unsigned int collect_evidence(unsigned int father_ind);

  //! Recursive Distribution of evidence. father is the cluster that
  //! invoked distribute_evidence
  /** \param[in]  father_ind  the index of the node to start collecting from
   */
  void  distribute_evidence(unsigned int father_ind);

  //! Recursive Distribution of minimum state.
  /** \param[in]  father_ind the index of the node to start the min_dist from
      \param[in]  min_comb       the minimum combination so far.
      Each child node will add the states of its particles.
   */
  void distribute_minimum(unsigned int father_ind, CombState *min_comb);

  //! Updates node with index w based on the evidence in the node with index v
  /** \param[in]  father_ind     the index of the node to start collecting from
   */
  void update(unsigned int w, unsigned int v);

  typedef boost::adjacency_list < boost::vecS, boost::vecS,
                                  boost::undirectedS, boost::no_property,
                                  boost::property<boost::edge_weight_t,
                                                  boost::vecS> > Graph;
  Pair get_edge_key(unsigned int node1_ind, unsigned int node2_ind) const;
 protected:
  typedef boost::graph_traits<Graph>::edge_descriptor Edge;
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
  std::map<Pair, JEdge *> edge_data;
  std::vector<JNode *> node_data; // the i'th entry holds the i'th jnode.
  Model *m;
  Graph g;
  std::map<int, int> particle2node; //for quick graph building
  std::vector<int> node2particle;
  //inference support data structures
  std::vector<unsigned int> discover_time;
  // discover_order[i] , the discover time of node number i
  unsigned int root;
  bool infered;
  std::vector<CombState *> *min_combs;
};

} // namespace domino

} // namespace IMP

#endif  /* __IMPDOMINO_RESTRAINT_GRAPH_H */
