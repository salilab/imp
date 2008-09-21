/**
 * \file OptimizationWorkFlow.h
 * \brief handles the dicrete optimization work flow
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OPTIMIZATION_WORK_FLOW_H
#define __IMP_OPTIMIZATION_WORK_FLOW_H

#include "DiscreteSampler.h"
#include "RestraintGraph.h"
#include "IMP/Model.h"
#include "domino_exports.h"

namespace IMP
{

namespace domino
{

class IMPDOMINOEXPORT OptimizationWorkFlow
{
public:
  OptimizationWorkFlow() {
    rest_g = RestraintGraph();
  }
  void initialize_jt_graph(int number_of_nodes) {
    rest_g.initialize_graph(number_of_nodes);
  }
  //! Creates a new node and add it to the graphs that take part in the
  //! optimization
  /** \param[in] node_index the index of the node
      \param[in] particles  the particles that are part of the node
   */
  void add_jt_node(int node_index, std::vector<Int> &particles_ind, Model &m) {
    Particles particles = Particles();
    for (std::vector<Int>::const_iterator it = particles_ind.begin();
         it != particles_ind.end(); it++) {
      particles.push_back(m.get_particle(*it));
    }
    rest_g.add_node(node_index, particles);
  }

  //! Adds an undirected edge between a pair of nodes
  /** \param[in] node1_ind  the index of the first node
      \param[in] node2_ind  the index of the second node
  */
  void add_jt_edge(int node1_ind, int node2_ind) {
    rest_g.add_edge(node1_ind, node2_ind);
  }

  void set_sampling_space(const DiscreteSampler &ds) {
    rest_g.set_sampling_space(ds);
  }
  void set_model(IMP::Model *m_) {
    m = m_;
    rest_g.set_model(m);
  }
  void show_restraint_graph(std::ostream& out = std::cout) const {
    rest_g.show(out);
  }

  void show_sampling_space(std::ostream& out = std::cout) const {
    rest_g.show_sampling_space(out);
  }

  void infer() {
    //TODO - clear
    //    set_sampling_space(*ds);
    //the combination graph is if for a specific discretization
    //you want to try all of the combinations.
    rest_g.infer();
  }

  //! Initalize potentials according to the input restraint set.
  /** \param[in] rs  the restraint set
   */
  void initialize_potentials(const RestraintSet &rs) {
    for (Model::RestraintConstIterator it = rs.restraints_begin();
         it != rs.restraints_end(); it++) {
      rest_g.initialize_potentials(**it, rs.get_weight());
    }
  }
  const RestraintGraph & get_restraint_graph() {
    return rest_g;
  }
  //! Set the optimizable attributes of the optimizable components to the
  //! values that are part of the global minimum
  void move_model2global_minimum() const {
    rest_g.move_model2global_minimum();
  }
  //! Sets the optimizable attributes of the optimizable components to the
  //! values that build the minimum of the scoring function when the state
  //! of the root of the junction tree is of a spcific index.
  /** \param[in] state_index the index of the state of the root node of
                 the junction tree.
   */
  void move_model2state(unsigned int state_index) const {
    rest_g.move_model2state(state_index);
  }
  Float get_global_minimum_score() const {
    return rest_g.get_minimum_score();
  }
protected:
  RestraintGraph rest_g;
  Model *m; // the particles model.
};

} // namespace domino

} // namespace IMP

#endif  /* __IMP_OPTIMIZATION_WORK_FLOW_H */
