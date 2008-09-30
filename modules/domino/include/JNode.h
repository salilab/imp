/**
 *  \file JNode.h   \brief Handles all functionalities of a junction tree node.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMPDOMINO_JNODE_H
#define __IMPDOMINO_JNODE_H

#include "domino_exports.h"
#include "IMP/Particle.h"
#include "IMP/Restraint.h"
#include "IMP/restraints/RestraintSet.h"
#include "IMP/decorators/XYZDecorator.h"
#include "DiscreteSampler.h"
#include "CombState.h"
#include <vector>
#include <sstream>
#include <algorithm>

namespace IMP
{

namespace domino
{

//! Handles all functionalities of a junction tree node.
/** Holds multiple particles, their states and the current marginalization
    state.
 */
class IMPDOMINOEXPORT JNode
{
public:
  //! Constructor
  /** \param [in] p_    a vector of IMP particles that are part of the node
      \param [in] node_ind_  The index of the JNode
   */
  JNode(const Particles &p, int node_ind);
  ~JNode() {
    //    free(opt_state);
  }

  //! Set the discrete sampling space of each of particles in the node
  /** \param [in] ds the sampler data
   */
  void init_sampling(const DiscreteSampler &ds);

  //! Get the set of intersecting particles between two nodes
  /** \param [in] other   the second node
      \param[out] in      the intersection set
   */
  void get_intersection(const JNode &other, Particles &in) const;
  void get_intersection2(const JNode &other, Particles in) const {}

  //! checks if the input set of particles is part of the node
  /** \param [in] p   a set of particles
      \return True if the node contains the input set of nodes, False otherwise
   */
  bool is_part(const Particles &p) const;

  //! Fill states as encoded in the node for the input subset of particles
  /** \param[in] particles_   a set of particles
      \param[in] states_      the dataset to be filled with states.
   */
  void populate_states_of_particles(const Particles &particles_,
                                    std::map<std::string,
                                             CombState *> *states_) const;
  void realize(Restraint *r, float weight);

  //! Finds the minimum combination in the node.
  /** \param[in]   move2state     true if the model should move to the new state
      \return all of the combinations that reach the global minimum
   */
  std::vector<CombState *> * find_minimum(bool move2state_ = false) const;

  CombState* get_state(unsigned int index, bool move2state_ = false) const;

  void show(std::ostream& out = std::cout) const;
  void show_sampling_space(std::ostream& out = std::cout) const;
  unsigned int get_node_index() const {
    return node_ind_;
  }
  const Particles *get_particles() const {
    return &particles_;
  }
  std::vector<CombState *> min_marginalize(const CombState &s,
      bool move2state_ = false);

  //! Update the potentials
  /** \param[in] old_score_separators
      \param[in] new_score_separators
      \param[in] intersection_particles
   */
  void update_potentials(
    const std::map<std::string, float>  &old_score_separators,
    const std::map<std::string, float>  &new_score_separators,
    const Particles &intersection_particles);
  const DiscreteSampler* get_sampler() {
    return ds_;
  }
  long number_of_states() const {
    long number_of_states = 1;
    for (Particles::const_iterator it = particles_.begin();
         it != particles_.end(); it++) {
      number_of_states *=  ds_->get_space_size(**it);
    }
    return number_of_states;
  }
  //! Move the system to the state encoded in the class
  void move2state(const CombState &cs) const;
  void clear();
protected:
  std::vector<Int> sorted_particle_indexes_; // needed for calculating
                                            // intersections with other nodes.
  Particles particles_; //the particles that are part of the node
  unsigned int node_ind_;
  std::map<std::string, CombState *> comb_states_;
  const DiscreteSampler *ds_;
};

} // namespace domino

} // namespace IMP

#endif  /* __IMPDOMINO_JNODE_H */
