/**
 *  \file DominoOptimizer.h   \brief An exact inference optimizer
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_DOMINO_OPTIMIZER_H
#define __IMP_DOMINO_OPTIMIZER_H

#include "domino_config.h"
#include "IMP/Optimizer.h"
#include "IMP/Restraint.h"
#include "IMP/Model.h"
#include "IMP/restraints/RestraintSet.h"
#include "RestraintGraph.h"

namespace IMP
{

//! An exact inference optimizer.
/** \ingroup optimize
 */
class DOMINODLLEXPORT DominoOptimizer : public Optimizer
{
public:
  DominoOptimizer(Model *m);
  IMP_OPTIMIZER(internal::kernel_version_info)
public:
  void show(std::ostream &out = std::cout) const {
    out << "DOMINO optimizer" << std::endl;
  }
  void set_sampling_space(DiscreteSampler *ds);
  void initialize_jt_graph(int number_of_nodes) {
    g_->initialize_graph(number_of_nodes);
  }

  //! Creates a new node and add it to the graphs that take part in the
  //! optimization
  /** \param[in] node_index the index of the node
      \param[in] particles  the particles that are part of the node
   */
  void add_jt_node(int node_index, std::vector<Int>  &particles_ind,
                   Model &m) {
    Particles particles = Particles();
    for (std::vector<Int>::const_iterator it = particles_ind.begin();
         it != particles_ind.end(); it++) {
      particles.push_back(m.get_particle(*it));
    }
    g_->add_node(node_index, particles);
  }

  //! Adds an undirected edge between a pair of nodes
  /** \param[in] node1_ind  the index of the first node
      \param[in] node2_ind  the index of the second node
   */
  void add_jt_edge(int node1_ind, int node2_ind) {
    g_->add_edge(node1_ind, node2_ind);
  }
  void show_restraint_graph(std::ostream& out = std::cout) const {
    g_->show(out);
  }
protected:
  void realize_rec(RestraintSet *rs, Float weight);
  DiscreteSampler *ds_;
  RestraintGraph *g_;
};

} // namespace IMP

#endif  /* __IMP_DOMINO_OPTIMIZER_H */
