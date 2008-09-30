/**
 *  \file DominoOptimizer.h   \brief An exact inference optimizer
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMPDOMINO_DOMINO_OPTIMIZER_H
#define __IMPDOMINO_DOMINO_OPTIMIZER_H

#include "domino_exports.h"
#include "domino_version_info.h"
#include "IMP/Optimizer.h"
#include "IMP/Restraint.h"
#include "IMP/Model.h"
#include "IMP/restraints/RestraintSet.h"
#include "RestraintGraph.h"

namespace IMP
{
namespace domino
{

//! An exact inference optimizer.
/** \ingroup optimizer
 */
class IMPDOMINOEXPORT DominoOptimizer : public Optimizer
{
public:
  DominoOptimizer(std::string jt_filename, Model *m);
  IMP_OPTIMIZER(internal::domino_version_info)
public:
  void show(std::ostream &out = std::cout) const {
    out << "DOMINO optimizer" << std::endl;
  }
  void set_sampling_space(DiscreteSampler *ds);

  void show_restraint_graph(std::ostream& out = std::cout) const {
    g_->show(out);
  }
  DiscreteSampler *get_sampling_space() const {return ds_;}
  RestraintGraph  *get_graph() const {return g_;}
protected:
  void clear(); //TODO implement!
  //! Creates a new node and add it to the graphs that take part in the
  //! optimization
  /**
    \param[in] node_index the index of the node
    \param[in] particles  the particles that are part of the node
  */
  void add_jt_node(int node_index, std::vector<Int>  &particles_ind,
                   Model &m);
  //! Adds an undirected edge between a pair of nodes
  /** \param[in] node1_ind  the index of the first node
      \param[in] node2_ind  the index of the second node
   */
  void add_jt_edge(int node1_ind, int node2_ind);

  void realize_rec(RestraintSet *rs, Float weight);
  void initialize_jt_graph(int number_of_nodes);
  DiscreteSampler *ds_;
  RestraintGraph *g_;
};

} // namespace domino

} // namespace IMP

#endif  /* __IMPDOMINO_DOMINO_OPTIMIZER_H */
