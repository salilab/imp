/**
 *  \file DominoOptimizer.h   \brief An exact inference optimizer
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO2_DOMINO_OPTIMIZER_H
#define IMPDOMINO2_DOMINO_OPTIMIZER_H

#include "../domino2_config.h"
#include "RestraintGraph.h"
#include "JunctionTree.h"
#include "RestraintEvaluator.h"
#include "CombState.h"
#include <IMP/RestraintSet.h>

#include <IMP/Optimizer.h>
#include <IMP/Restraint.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/Model.h>

IMPDOMINO2_BEGIN_INTERNAL_NAMESPACE

//! An exact inference optimizer.
/** \ingroup optimizer
 */
class IMPDOMINO2EXPORT DominoOptimizer : public Optimizer
{
  typedef boost::tuple<Restraint *,
                       container::ListSingletonContainer *,
                       Float> OptTuple;
public:
  //! Constructor
  /**
  \param[in] jt A special type of tree decomposition that holds the
                graph on which inference will be run
  \param[in] m  The model to be optimized
  \param[in] r_eval evaluator used to evaluate the restraints
   */
  //TODO - add here much more!
  DominoOptimizer(container::ListSingletonContainer *ps,
                  const JunctionTree &jt, Model *m,
                  RestraintEvaluator *r_eval);

  void set_sampling_space(DiscreteSampler *ds);
  //TODO : set a discrete sampling space for one node
  //void set_sampling_space(int node_ind, DiscreteSampler *ds);

  void show_restraint_graph(std::ostream& out = std::cout) const {
    g_->show(out);
  }
  void show_optimum_configuration(unsigned int conf_ind,
                                   std::ostream& out = std::cout) const {
    out<<"OPT_CONF:";
    g_->get_opt_combination(conf_ind)->show(out);
  }
  DiscreteSampler *get_sampling_space() const {return ds_;}
  RestraintGraph  *get_graph() const {return g_;}
  void set_restraint_evaluator(RestraintEvaluator *rstr_eval) {
    rstr_eval_=rstr_eval;
  }
  inline unsigned int get_number_of_solutions() const {
      return num_of_solutions_;}
  inline void set_number_of_solutions(unsigned int n){num_of_solutions_=n;}

  IMP_OPTIMIZER(DominoOptimizer);

protected:
  //! Creates a new node and add it to the graphs that take part in the
  //! optimization
  /*
    \param[in] node_index the index of the node
    \param[in] particles_ind  the particles that are part of the node
  */
  void add_jt_node(int node_index, std::vector<Int>  &particles_ind,
                   Model &m);
  //! Adds an undirected edge between a pair of nodes
  /** \param[in] node1_ind  the index of the first node
      \param[in] node2_ind  the index of the second node
   */
  void add_jt_edge(int node1_ind, int node2_ind);

  void initialize_jt_graph(int number_of_nodes);
  DiscreteSampler *ds_;
  RestraintGraph *g_;
  unsigned int num_of_solutions_;
  std::vector<OptTuple> rs_;
  RestraintEvaluator *rstr_eval_;
  Pointer<container::ListSingletonContainer> all_particles_;
};
IMPDOMINO2_END_INTERNAL_NAMESPACE

#endif  /* IMPDOMINO2_DOMINO_OPTIMIZER_H */
