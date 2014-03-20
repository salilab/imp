/**
 * \file RRT \brief
 *
 * Copyright 2007-2014 Sali Lab. All rights reserved.
 *
 */
#include <IMP/kinematics/RRT.h>

IMPKINEMATICS_BEGIN_NAMESPACE

int RRT::RRTNode::node_counter_ = 0;

std::ostream& operator<<(std::ostream& s, const RRT::Parameters& p) {
  s << "Number_of_iterations = " << p.number_of_iterations_
    << " actual_tree_size = " << p.actual_tree_size_
    << " tree_size = " << p.tree_size_;
  return s;
}

RRT::RRT(kernel::Model* m, DOFsSampler* dofs_sampler, LocalPlanner* planner,
         const DOFs& cspace_dofs, unsigned int iteration_number,
         unsigned int tree_size)
    : Sampler(m, "rrt_sampler"),
      dofs_sampler_(dofs_sampler),
      local_planner_(planner),
      cspace_dofs_(cspace_dofs),
      default_parameters_(iteration_number, tree_size, tree_size) {
  // define q_init and check if it is a valid configuration
  DOFValues q_init(cspace_dofs_);
  if (!local_planner_->is_valid(q_init)) {  // TODO throw IMP exception
    std::cerr << "Initial configuration in the forbiden space!!!" << std::endl;
    std::cerr << "Try to decrease radii scaling parameter" << std::endl;
    exit(1);
  }

  // add q_init to the RRT tree
  RRTNode* new_node = new RRTNode(q_init);
  tree_.push_back(new_node);
}

RRT::RRTNode* RRT::get_q_near(const DOFValues& q_rand) const {
  double shortest_distance = std::numeric_limits<double>::max();
  RRTNode* q_near = nullptr;
  for (unsigned int i = 0; i < tree_.size(); i++) {
    double curr_distance = q_rand.get_distance(tree_[i]->get_DOFValues());
    if (curr_distance < shortest_distance) {
      shortest_distance = curr_distance;
      q_near = tree_[i];
    }
  }
  return q_near;
}

void RRT::add_nodes(RRTNode* q_near, const std::vector<DOFValues>& new_nodes) {
  RRTNode* prev_node = q_near;
  for (unsigned int i = 0; i < new_nodes.size(); i++) {
    RRTNode* new_node = new RRTNode(new_nodes[i]);
    tree_.push_back(new_node);
    // add edge
    double distance = prev_node->get_DOFValues().get_distance(new_nodes[i]);
    prev_node->add_edge(new_node, distance);
    prev_node = new_node;
  }
}

void RRT::run() {
  Parameters current_counters;
  while (!is_stop_condition(default_parameters_, current_counters)) {
    DOFValues q_rand = dofs_sampler_->get_sample();
    RRTNode* q_near_node = get_q_near(q_rand);
    std::vector<DOFValues> new_nodes =
        local_planner_->plan(q_near_node->get_DOFValues(), q_rand);
    add_nodes(q_near_node, new_nodes);
    // update counters
    current_counters.number_of_iterations_++;
    if (new_nodes.size() > 0) current_counters.actual_tree_size_++;
    current_counters.tree_size_ = tree_.size();
    if (current_counters.number_of_iterations_ % 100 == 0 ||
        new_nodes.size() > 0) {
      std::cerr << "RRT done iteration, added " << new_nodes.size()
                << " new nodes " << current_counters << " q_near "
                << q_near_node->get_id() << std::endl;
    }
  }
}

std::vector<DOFValues> RRT::get_DOFValues() {
  std::vector<DOFValues> ret(tree_.size());
  for (unsigned int i = 0; i < tree_.size(); i++) {
    ret[i] = tree_[i]->get_DOFValues();
  }
  return ret;
}

std::ostream& operator<<(std::ostream& s, const RRT::RRTNode& n) {
  s << n.id_ << " = " << n.score_ << " = ";
  n.vec_.show(s);
  return s;
}

IMPKINEMATICS_END_NAMESPACE
