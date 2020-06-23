/**
 * \file RRT \brief
 *
 * Copyright 2007-2020 IMP Inventors. All rights reserved.
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

namespace {

int myrandom (int i) { return std::rand()%i;}

std::vector<bool> select_k_out_of_n_dofs(unsigned int k, unsigned int n) {
  std::vector<unsigned int> arr(n);
  for(unsigned int i=0; i<n; i++) arr[i] = i;
  std::random_shuffle(arr.begin(), arr.end(), myrandom);
  std::vector<bool> ret(n, false);
  for(unsigned int i=0; i<k; i++) ret[arr[i]] = true;
  //std::vector<unsigned int> ret(k);
  //for(unsigned int i=0; i<k; i++) ret[i] = arr[i];
  return ret;
}

}

RRT::RRT(Model* m, DOFsSampler* dofs_sampler, LocalPlanner* planner,
         const DOFs& cspace_dofs, unsigned int iteration_number,
         unsigned int tree_size, unsigned int number_of_sampled_dofs)
    : Sampler(m, "rrt_sampler"),
      dofs_sampler_(dofs_sampler),
      local_planner_(planner),
      cspace_dofs_(cspace_dofs),
      default_parameters_(iteration_number, tree_size, tree_size),
      number_of_sampled_dofs_(number_of_sampled_dofs) {
  // add q_init to the RRT tree
  DOFValues q_init(cspace_dofs_);
  RRTNodePtr new_node(new RRTNode(q_init));
  tree_.push_back(new_node);
}

void RRT::check_initial_configuration(ScoringFunction *sf) const {
  // define q_init and check if it is a valid configuration
  DOFValues q_init(cspace_dofs_);
  if (!local_planner_->is_valid(q_init, sf)) {  // TODO throw IMP exception
    std::cerr << "Initial configuration in the forbidden space!!!" << std::endl;
    std::cerr << "Try to decrease radii scaling parameter" << std::endl;
    exit(1);
  }
}

RRT::RRTNode* RRT::get_q_near(const DOFValues& q_rand) const {
  double shortest_distance = std::numeric_limits<double>::max();
  RRTNode* q_near = nullptr;
  for (unsigned int i = 0; i < tree_.size(); i++) {
    double curr_distance = q_rand.get_distance(tree_[i]->get_DOFValues(), active_dofs_);
    //double curr_distance = q_rand.get_distance(tree_[i]->get_DOFValues());
    if (curr_distance < shortest_distance) {
      shortest_distance = curr_distance;
      q_near = tree_[i].get();
    }
  }
  return q_near;
}

void RRT::add_nodes(RRTNode* q_near, const std::vector<DOFValues>& new_nodes) {
  RRTNode* prev_node = q_near;
  for (unsigned int i = 0; i < new_nodes.size(); i++) {
    RRTNodePtr new_node(new RRTNode(new_nodes[i]));
    tree_.push_back(new_node);
    // add edge
    double distance = prev_node->get_DOFValues().get_distance(new_nodes[i]);
    prev_node->add_edge(new_node.get(), distance);
    prev_node = new_node.get();
  }
}

bool RRT::run(unsigned int number_of_iterations) {
  //std::ofstream myfile;
  //myfile.open ("AA_m0.4_pio180.dat");
  set_was_used(true);
  if (dofs_sampler_->get_number_of_dofs() == 0) {
    throw IMP::ValueException("No degrees of freedom to sample!");
  }

  ScoringFunction *sf = get_scoring_function();
  if(tree_.size() == 1) check_initial_configuration(sf);
  unsigned int iter_counter = 0;
  //Parameters current_counters;
  while (!is_stop_condition(default_parameters_, current_counters_)) {
    //std::cerr << current_counters_.number_of_iterations_ << std::endl;
    DOFValues q_rand = dofs_sampler_->get_sample();

    // random selection of active dofs; change after every 10.
    if (number_of_sampled_dofs_ > 0 &&
        current_counters_.number_of_iterations_ % 10 == 0) {
      active_dofs_ = select_k_out_of_n_dofs(number_of_sampled_dofs_, q_rand.size());
    }

    RRTNode* q_near_node = get_q_near(q_rand);

    if (number_of_sampled_dofs_ > 0) { // take non-active dof values from q_near
      const DOFValues& dof_values = q_near_node->get_DOFValues();
      for (unsigned int i=0; i<dof_values.size(); i++) {
        if (!active_dofs_[i]) q_rand[i] = dof_values[i];
      }
    }

    std::vector<DOFValues> new_nodes =
        local_planner_->plan(q_near_node->get_DOFValues(), q_rand, sf);

    //std::cout << "RRT new nodes: " << new_nodes.size() << std::endl;
    add_nodes(q_near_node, new_nodes);
    
    //myfile << "RRT Step | ";
    
    for (unsigned int i = 0; i < new_nodes.size(); i++) {
      //myfile << " " << new_nodes[i];
    }    

    // update counters
    current_counters_.number_of_iterations_++;
    iter_counter++;
    if (new_nodes.size() > 0) current_counters_.actual_tree_size_++;
    current_counters_.tree_size_ = tree_.size();
    if (current_counters_.number_of_iterations_ % 100 == 0 ||
        new_nodes.size() > 0) {
      std::cerr << "RRT done iteration, added " << new_nodes.size()
                << " new nodes " << current_counters_ << " q_near "
                << q_near_node->get_id() << std::endl;

      //myfile << " | q_near | "
      //          << q_near_node->get_DOFValues() << " | q_rand | " << q_rand;
    }
    //myfile << std::endl;
    // try to output
    if(number_of_iterations > 0 && tree_.size() > 1
       && iter_counter == number_of_iterations)
      return true;
  }
  return false; //done running

}

DOFValuesList RRT::get_DOFValuesList() {
  DOFValuesList ret(tree_.size());
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
