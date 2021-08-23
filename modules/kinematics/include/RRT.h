/**
 * \file IMP/kinematics/RRT.h
 * \brief Implementation of Randomly-Exploring Random Trees
 *
 *  \authors Dina Schneidman, Barak Raveh
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKINEMATICS_RR_T_H
#define IMPKINEMATICS_RR_T_H

#include <IMP/kinematics/kinematics_config.h>
#include <IMP/kinematics/DOF.h>
#include <IMP/kinematics/DOFValues.h>
#include <IMP/kinematics/local_planners.h>

#include <IMP/Sampler.h>
#include <IMP/ConfigurationSet.h>

#include <boost/shared_ptr.hpp>

IMPKINEMATICS_BEGIN_NAMESPACE

// TODO: should move out of kinematics, or be in kinematic_algorithms

//! Simple implementation of the Rapidly-exploring Random Trees algorithm
class IMPKINEMATICSEXPORT RRT : public IMP::Sampler {
 public:
  IMP_OBJECT_METHODS(RRT);

  //! Simple RRT node implementation
  /** we may replace it with something from boost so we can use boost graph */
  class RRTNode {
   public:
    RRTNode(const DOFValues& vec) : vec_(vec) {
      id_ = node_counter_;
      node_counter_++;
      score_ = 0;
    }

    // Accessors
    const DOFValues& get_DOFValues() const { return vec_; }

    const std::vector<std::pair<const RRTNode*, float> >& get_edges() const {
      return edges_;
    }

    int get_id() const { return id_; }

    float get_score() const { return score_; }

    bool is_edge(const RRTNode* node) const {
      for (unsigned int i = 0; i < edges_.size(); i++)
        if (node == edges_[i].first) return true;
      return false;
    }

    // Modifiers
    void add_edge(const RRTNode* node, float distance) {
      if (id_ > node->id_) std::cerr << "wrong direction edge" << std::endl;
      edges_.push_back(std::make_pair(node, distance));
    }

    void set_score(float score) { score_ = score; }

    struct less_node {
      bool operator()(const RRTNode* n1, const RRTNode* n2) {
        return n1->get_score() < n2->get_score();
      }
    };

    friend std::ostream& operator<<(std::ostream& s, const RRTNode& n);

   private:
    const DOFValues vec_;
    // a node and a distance between two dofs
    std::vector<std::pair<const RRTNode*, float> > edges_;
    int id_;
    float score_;
    static int node_counter_;
  };

 private:
  // counters for stop condition...
  class Parameters {
   public:
    Parameters()
        : number_of_iterations_(0),
          actual_tree_size_(0),
          tree_size_(0),
          number_of_collisions_(0) {}

    Parameters(unsigned int number_of_iterations,
               unsigned int actual_tree_size = 100,
               unsigned int tree_size = 100,
               unsigned int number_of_collisions = 10000)
        : number_of_iterations_(number_of_iterations),
          actual_tree_size_(actual_tree_size),
          tree_size_(tree_size),
          number_of_collisions_(number_of_collisions) {}

    unsigned int number_of_iterations_;
    unsigned int actual_tree_size_;  // not including path nodes
    unsigned int tree_size_;         // all nodes
    unsigned int number_of_collisions_;
  };

  friend std::ostream& operator<<(std::ostream& s, const Parameters& p);

 public:
  //! Constructor
  RRT(Model* m, DOFsSampler* sampler, LocalPlanner* planner,
      const DOFs& cspace_dofs, unsigned int iteration_number = 1000,
      unsigned int tree_size = 100,
      unsigned int number_of_sampled_dofs = 0); // 0 for all dofs

  // function required by Sampler
  // TODO: think how to save configurations in internal coords
  // to be more memory efficient
  IMP::ConfigurationSet* do_sample() const IMP_OVERRIDE {
    const_cast<RRT*>(this)->run();
    return nullptr;
  }

  //! Run the specified number of iterations
  /** If zero, will run for default iteration number.DOFValuesVector
      This option allows to output the already sampled
      configurations during the sampling run. */
  bool run(unsigned int number_of_iterations = 0);

  DOFValuesList get_DOFValuesList();

  /* Parameters for controlling RRT stop condition */

  //! Set number of RRT iterations
  void set_number_of_iterations(unsigned int num) {
    default_parameters_.number_of_iterations_ = num;
  }

  //! Set tree size
  void set_tree_size(unsigned int num) { default_parameters_.tree_size_ = num; }

  //! Set the actual tree size - not including path nodes
  void set_actual_tree_size(unsigned int num) {
    default_parameters_.actual_tree_size_ = num;
  }

  //! Set the number of collisions
  void set_number_of_collisions(unsigned int num) {
    default_parameters_.number_of_collisions_ = num;
  }

  void check_initial_configuration(ScoringFunction *sf) const;

 private:
  RRTNode* get_q_near(const DOFValues& q_rand) const;



  void add_nodes(RRTNode* q_near, const std::vector<DOFValues>& new_nodes);

  // TODO: make it more general
  bool is_stop_condition(const Parameters& parameters,
                         const Parameters& counters) {
    return (counters.number_of_iterations_ > parameters.number_of_iterations_ ||
            counters.actual_tree_size_ > parameters.actual_tree_size_ ||
            counters.tree_size_ > parameters.tree_size_ ||
            counters.number_of_collisions_ > parameters.number_of_collisions_);
  }

 private:
  PointerMember<DOFsSampler> dofs_sampler_;
  PointerMember<LocalPlanner> local_planner_;
  typedef boost::shared_ptr<RRTNode> RRTNodePtr;
  typedef std::vector<RRTNodePtr> RRTTree;
  RRTTree tree_;
  DOFs cspace_dofs_;               // configuration space dofs
  Parameters default_parameters_;  // limits for stop condition
  Parameters current_counters_;

  // for sampling subsets of dofs
  unsigned int number_of_sampled_dofs_;
  std::vector<bool> active_dofs_;
};

IMPKINEMATICS_END_NAMESPACE

#endif /* IMPKINEMATICS_RR_T_H */
