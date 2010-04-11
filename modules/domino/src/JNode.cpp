/**
 *  \file JNode.cpp \brief Handles all functionalities of a junction tree node.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/domino/JNode.h>
#include <IMP/ScoreState.h>


#include <numeric>
#include <climits>
#include <algorithm>

IMPDOMINO_BEGIN_NAMESPACE

//TODO: for all the intersection operations the sets should be sorted,
//make sure that in the initialization of the nodes the number of
//nodes come sorted.
JNode::JNode(const Particles &ps, int node_ind): ds_(NULL)
{
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,"Constructing a JNode with "<<
                     ps.size() << " particles: ");
    for(Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
      IMP_LOG(VERBOSE,(*it)->get_name()<<",");
    }
    IMP_LOG(VERBOSE,std::endl);
  }
  node_ind_ = node_ind;
  //create a sorted list of particles
  Particles temp = ps;
  std::sort(temp.begin(), temp.end());
  particles_ = new container::ListSingletonContainer(temp);
  comb_states_ = std::map<std::string, CombState *>();
  rstr_eval_=NULL;
}

void JNode::init_sampling(DiscreteSampler &ds)
{
  IMP_LOG(VERBOSE,"Start sampling initialization for node number: "
          << node_ind_ << " with: " << comb_states_.size()<<" states"
          <<std::endl);
  ds_ = &ds;
  populate_states_of_particles(particles_, &comb_states_);

  //create a vector of the keys, needed for fast
  //implementation of get_state function
  std::map<std::string, CombState *>::const_iterator it;
  for (it = comb_states_.begin(); it != comb_states_.end(); it++) {
    comb_states_keys_.push_back(it->first);
  }
  IMP_LOG(VERBOSE,"End sampling initialization for node number: "
                   << node_ind_ <<std::endl);
}

CombState* JNode::get_state(unsigned int index, bool move2state_){
  CombState *cs = comb_states_.find(comb_states_keys_[index])->second;
  if (move2state_) {
    move2state(cs);
  }
  return cs;
}

void JNode::populate_states_of_particles(container::ListSingletonContainer *ps,
                                         std::map<std::string,
                                         CombState *> *states)
{
  ds_->populate_states_of_particles(ps, states);
  IMP_USAGE_CHECK(comb_states_.size()>0,"no state added to node: "
            << node_ind_ << std::endl);
}
void JNode::show_sampling_space(std::ostream& out) const
{
  out << std::endl << " sampling space of JNode with index: " << node_ind_;
  for (unsigned int i=0;i<particles_->get_number_of_particles();i++){
    Particle *pi = particles_->get_particle(i);
    out << std::endl << " states for particle name : " <<
      pi->get_value(node_name_key()) << ":" << std::endl;
    ds_->show_space(pi, out);
  }
}

void JNode::show(std::ostream& out) const
{
  out << "=========================JNode  " <<  " node_index: " << node_ind_
      << std::endl;
  out << "particle_indices: " << std::endl;
  for (unsigned int i=0;i<particles_->get_number_of_particles();i++){
    Particle *p=particles_->get_particle(i);
    out << p->get_value(node_name_key()) << " || " ;
  }
  out << std::endl << "==combinations ( " << comb_states_.size();
  out << " ): " << std::endl;
  for (std::map<std::string, CombState *>::const_iterator it =
         comb_states_.begin(); it != comb_states_.end(); it++) {
    out << " (" << it->first << " , " << it->second->get_total_score() << ") ";
  }
  out << std::endl;
}
bool JNode::is_part(const ParticlesTemp &ps) const
{
  ParticlesTemp intersection, other_sorted_particle;
  for (ParticlesTemp::const_iterator it = ps.begin(); it != ps.end(); it++) {
    other_sorted_particle.push_back(*it);
  }
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,"check if particles : " << std::endl);
    for (Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
      IMP_LOG(VERBOSE,(*it)->get_value(node_name_key()) << " : ");
    }
    IMP_LOG(VERBOSE,"are part of  : " << std::endl);
    for (unsigned int i=0;i<particles_->get_number_of_particles();i++){
      IMP_LOG(VERBOSE,
              particles_->get_particle(i)->get_value(node_name_key())<< " : ");
    }
    IMP_LOG(VERBOSE,std::endl);
  }
  sort(other_sorted_particle.begin(),
       other_sorted_particle.end());
  ParticlesTemp my_ps = particles_->get_particles();
  set_intersection(my_ps.begin(),
                   my_ps.end(),
                   other_sorted_particle.begin(),
                   other_sorted_particle.end(),
                   std::inserter(intersection, intersection.begin()));
  if (intersection.size() == ps.size()) {
    IMP_LOG(VERBOSE,"return true\n");
    return true;
  }
    IMP_LOG(VERBOSE,"return false\n");
  return false;
}

Particles JNode::get_intersection(const JNode &other) const
{
  Particles in;
  // since the list should be sorted we use the indexes and not the pointers,
  // as we can not predict the order of the pointers.
  ParticlesTemp inter_indexes;
  ParticlesTemp my_ps = particles_->get_particles();
  ParticlesTemp other_ps = other.particles_->get_particles();
  set_intersection(my_ps.begin(),
                   my_ps.end(),
                   other_ps.begin(),
                   other_ps.end(),
                   std::inserter(inter_indexes, inter_indexes.begin()));
  //TODO - do this more efficient
  for (Particles::const_iterator it = inter_indexes.begin();
       it != inter_indexes.end(); it++) {
    for (ParticlesTemp::const_iterator pi = my_ps.begin();
         pi != my_ps.end(); pi++) {
      if (*it == *pi) {
        in.push_back(*pi);
      }
    }
  }
  return in;
}

void JNode::move2state(CombState *cs)
{
  ds_->move2state(cs);
}

void JNode::realize(Restraint *r, container::ListSingletonContainer *ps,
                    Float weight)
{
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,"start realize node: " << node_ind_ << " with restraint: ");
    IMP_LOG_WRITE(VERBOSE,r->show());
    IMP_LOG(VERBOSE," , weight : " << weight);
    IMP_LOG(VERBOSE, " and number of states : " << comb_states_.size());
    IMP_LOG(VERBOSE,std::endl);
  }
  std::map<std::string, float> temp_calculations;
  // stores calculated discrete values. It might be that each appears more
  // than once, since the node may contain more particles than the ones
  // indicated by the restraint.
  CombinationValues result_cache; // to avoid multiple calculation
                                             // of the same configuration.
  IMP_INTERNAL_CHECK(rstr_eval_!=NULL,
                     "restraint evaluator was not initialized"<<std::endl);
  rstr_eval_->calc_scores(comb_states_,result_cache,r,ps);
  std::string partial_key;
  Float score;
  for (std::map<std::string, CombState *>::iterator it =  comb_states_.begin();
       it != comb_states_.end(); it++) {
    partial_key = it->second->get_partial_key(ps);
    score = result_cache.find(partial_key)->second;
    it->second->update_total_score(0.0, weight*score);
    //rethink how to handle weights.
    //should be part of reading restraints directly from the model
  }
  IMP_LOG(VERBOSE,"end realize node : " << node_ind_ << std::endl);
}

std::vector<CombState *> JNode::min_marginalize(const CombState &s,
                                                bool move_to_state)
{
  Float min_score = std::numeric_limits<Float>::max();
  std::vector<CombState *> min_comb;
  min_comb = std::vector<CombState *>();
  IMP_LOG(VERBOSE,"min_marginalize for node : " << node_ind_ <<
          " number of states: " << comb_states_.size() << std::endl);
  for (std::map<std::string, CombState *>::iterator it =  comb_states_.begin();
       it != comb_states_.end(); it++) {
    IMP_LOG(VERBOSE,"checking state: " << it->first);
    IMP_LOG_WRITE(VERBOSE,it->second->show());
    IMP_LOG(VERBOSE,"std::endl");

    if (it->second->is_part(s)) {
      // #TODO: too expensive , should be the other way around -
      // build all combinations according to separator.comb_key
      if (it->second->get_total_score() < min_score) {
        min_score = it->second->get_total_score();
      }
    }
  }

  //get all of the combinations of the minimum score
  IMP_LOG(VERBOSE,"min_marginalize for node : " << node_ind_ <<
          " the minimum score is : "<<min_score << std::endl);
  for (std::map<std::string, CombState *>::iterator it = comb_states_.begin();
       it != comb_states_.end(); it++) {
    if (it->second->is_part(s)) {
      if (it->second->get_total_score() ==  min_score) {
        min_comb.push_back(it->second);
      }
    }
  }
  std::stringstream error_message;
  error_message<<"JNode::min_marginalize could not marginalize over separator:";
  s.show(error_message);
  error_message << " in node with index : " << node_ind_;
  error_message << " the minimum score is : " << min_score << std::endl;
  IMP_INTERNAL_CHECK(min_score < INT_MAX, error_message.str());
  if (move_to_state) {
    move2state(min_comb[0]);
  }
  return min_comb;
}

void JNode::update_potentials(
  const std::map<std::string, float> &old_score_separators,
  const std::map<std::string, float> &new_score_separators,
  const Particles &intersection_particles)
{
  for (std::map<std::string, CombState *>::iterator it = comb_states_.begin();
       it != comb_states_.end(); it++) {
    std::string s_key = it->second->get_partial_key(intersection_particles);
    //separator_comb=v.comb_key.intersection_by_feature_key(edge_data.s_ij)
    // s_key=separator_comb.key()
    //TODO - add validation tests
    //check if the node is the from or to of the edge:
    it->second->update_total_score(old_score_separators.find(s_key)->second,
                                   new_score_separators.find(s_key)->second);
  }
}

bool my_comp(const std::map<std::string,CombState *>::iterator &i1,
             const std::map<std::string,CombState *>::iterator &i2) {
  return (i1->second->get_total_score() < i2->second->get_total_score());
}


std::vector<CombState *>* JNode::find_minimum(bool move_to_state,
                                              unsigned int num_of_solutions)
{
  //sort all of the combinations of the node by their score
  std::vector<std::pair<float,std::string> > all_states;
  for (std::map<std::string,CombState *>::const_iterator it=
    comb_states_.begin();it != comb_states_.end(); it++) {
    all_states.push_back(
       std::pair<float,std::string>(it->second->get_total_score(),it->first));
  }
  std::sort(all_states.begin(),all_states.end());
  std::vector<CombState *>* min_combs = new std::vector<CombState *>;
  IMP_INTERNAL_CHECK(all_states.size()>num_of_solutions,
             "JNode::find_minimum for node: " << node_ind_
             <<" the number of requested solutions ("
             <<num_of_solutions << ")is larger than the enumerated solution "
             <<"by the node (" << all_states.size() << ")");
  //allocate min_combs with the top best solutions
  //  int min_num = all_states.size() * (all_states.size() < num_of_solutions) +
  //              num_of_solutions * (all_states.size() >= num_of_solutions);
  for(unsigned int i=0;i<num_of_solutions;i++) {
    std::string key = all_states[i].second;
    min_combs->push_back(comb_states_[key]);
  }
  if (move_to_state) {
    move2state(*(min_combs->begin()));
  }
  return min_combs;
}

void JNode::clear() {
  for(std::map<std::string, CombState *>::iterator it =  comb_states_.begin();
      it != comb_states_.end();it++) {
    delete(it->second);
  }
  comb_states_.clear();
  ds_=NULL;
}

//! Get the score for this combination
Float JNode::get_score(const CombState &comb) {
  IMP_IF_LOG(VERBOSE){
    IMP_LOG(VERBOSE,"JNode::get_score for combination:"<<std::endl);
    IMP_LOG_WRITE(VERBOSE,comb.show());
    IMP_LOG(VERBOSE,std::endl);
  }

  std::string key = comb.get_partial_key(particles_);
  IMP_USAGE_CHECK(comb_states_.find(key) != comb_states_.end(),
       "The combination was not realized by the node"<<std::endl);
  IMP_LOG(VERBOSE,"JNode::get_score partial key: " <<
          key << " score: " <<
          comb_states_.find(key)->second->get_total_score() <<std::endl);
  return comb_states_.find(key)->second->get_total_score();
}


IMPDOMINO_END_NAMESPACE
