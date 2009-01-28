/**
 *  \file JNode.cpp \brief Handles all functionalities of a junction tree node.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/domino/JNode.h>

#include <numeric>
#include <climits>
#include <algorithm>

IMPDOMINO_BEGIN_NAMESPACE

//TODO: for all the intersection operations the sets should be sorted,
//make sure that in the initialization of the nodes the number of
//nodes come sorted.
JNode::JNode(const Particles &ps, int node_ind): ds_(NULL)
{
  node_ind_ = node_ind;
  //enter the particles by the order of their indexes.
  sorted_particle_indexes_ = std::vector<Int>();
  particles_ = Particles();
  for (Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
    particles_.push_back(*it);
    sorted_particle_indexes_.push_back((*it)->get_index());
  }
  std::sort(sorted_particle_indexes_.begin(), sorted_particle_indexes_.end());
  comb_states_ = std::map<std::string, CombState *>();
}

void JNode::init_sampling(DiscreteSampler &ds)
{
  ds_ = &ds;
  populate_states_of_particles(&particles_, &comb_states_);
  //create a vector of the keys, needed for fast
  //implementation of get_state function
  std::pair<std::string, CombState *> it;
  BOOST_FOREACH(it,comb_states_) {
    comb_states_keys_.push_back(it.first);
  }
}

CombState* JNode::get_state(unsigned int index, bool move2state_){
  CombState *cs = comb_states_.find(comb_states_keys_[index])->second;
  if (move2state_) {
    move2state(cs);
  }
  return cs;
}

void JNode::populate_states_of_particles(Particles *particles,
                                         std::map<std::string,
                                         CombState *> *states)
{
  ds_->populate_states_of_particles(particles, states);
}
void JNode::show_sampling_space(std::ostream& out) const
{
  out << std::endl << " sampling space of JNode with index: " << node_ind_;
  for (Particles::const_iterator pi = particles_.begin();
       pi != particles_.end(); pi++) {
    out << std::endl << " states for particle name : " <<
      //    (*pi)->get_value(IMP::StringKey("name")) << ":" << std::endl;
      (*pi)->get_index() << ":" << std::endl;
    ds_->show_space(*pi, out);
  }
}

void JNode::show(std::ostream& out) const
{
  out << "=========================JNode  " <<  " node_index: " << node_ind_
      << std::endl;
  out << "particle_indices: " << std::endl;
  for (Particles::const_iterator it = particles_.begin(); it !=
       particles_.end(); it++) {
    //    out << (*it)->get_value(IMP::StringKey("name")) << " || " ;
    out << (*it)->get_index() << " || " ;
  }
  out << std::endl << "==combinations ( " << comb_states_.size();
  out << " ): " << std::endl;
  for (std::map<std::string, CombState *>::const_iterator it =
         comb_states_.begin(); it != comb_states_.end(); it++) {
    out << " (" << it->first << " , " << it->second->get_total_score() << ") ";
  }
  out << std::endl;
}
bool JNode::is_part(const Particles &ps) const
{
  std::cout<<"JNode::is_part " << std::endl;
  std::cout<<" the particles are : "<< std::endl;
  for(Particles::const_iterator it = ps.begin(); it != ps.end();it++) {
    std::cout<<(*it)->get_index()<<std::endl;
  }
  std::cout << " ========= and the JNOde is : " << std::endl;
  show();
  std::vector<IMP::Int> intersection, other_sorted_particle_indexes;
  for (Particles::const_iterator it = ps.begin(); it != ps.end(); it++) {
    other_sorted_particle_indexes.push_back((*it)->get_index());
  }
  sort(other_sorted_particle_indexes.begin(),
       other_sorted_particle_indexes.end());
  set_intersection(sorted_particle_indexes_.begin(),
                   sorted_particle_indexes_.end(),
                   other_sorted_particle_indexes.begin(),
                   other_sorted_particle_indexes.end(),
                   std::inserter(intersection, intersection.begin()));
  if (intersection.size() == ps.size()) {
    return true;
  }
  return false;
}

void JNode::get_intersection(const JNode &other, Particles &in) const
{
  // since the list should be sorted we use the indexes and not the pointers,
  // as we can not predict the order of the pointers.
  std::vector<unsigned int> inter_indexes;
  set_intersection(sorted_particle_indexes_.begin(),
                   sorted_particle_indexes_.end(),
                   other.sorted_particle_indexes_.begin(),
                   other.sorted_particle_indexes_.end(),
                   std::inserter(inter_indexes, inter_indexes.begin()));
  //TODO - do this more efficient
  for (std::vector<unsigned int>::const_iterator it = inter_indexes.begin();
       it != inter_indexes.end(); it++) {
    for (Particles::const_iterator pi = particles_.begin();
         pi != particles_.end(); pi++) {
      if (*it == (*pi)->get_index()) {
        in.push_back(*pi);
      }
    }
  }
}

void JNode::move2state(CombState *cs)
{
  ds_->move2state(cs);
}


void JNode::realize(Restraint *r, Particles *ps, Float weight)
{
  std::cout <<" JNode::realize restraint start in node with index ";
  std::cout << node_ind_ << std::endl;
  std::map<std::string, float> temp_calculations;
  // stores calculated discrete values. It might be that each appears more
  // than once, since the node may contain more particles than the ones
  // indicated by the restraint.
  std::map<std::string, float> result_cache; // to avoid multiple calculation
                                             // of the same configuration.
  float score;
//   Particles r_particles;
//   ParticlesList pl = r->get_interacting_particles();
//   for(ParticlesList::iterator it1 = pl.begin();
//       it1 != pl.end(); it1++){
//     for(Particles::iterator it2 = it1->begin(); it2 != it1->end(); it2++) {
//       r_particles.push_back(*it2);
//      }
//   }
  std::string partial_key;
  for (std::map<std::string, CombState *>::iterator it =  comb_states_.begin();
       it != comb_states_.end(); it++) {
    partial_key = it->second->partial_key(ps);
    if (result_cache.find(partial_key) == result_cache.end()) {
      move2state(it->second);
      std::cout<<"The value is : " << r->evaluate(NULL) << std::endl;
      score = r->evaluate(NULL) * weight;
      result_cache[partial_key] = score;
    } else {
      score = result_cache.find(partial_key)->second;
    }
    it->second->update_total_score(0.0, score);
    it->second->show();
  }
}

std::vector<CombState *> JNode::min_marginalize(const CombState &s,
                                                bool move_to_state)
{
  float min_score = INT_MAX;
  std::vector<CombState *> min_comb;
  min_comb = std::vector<CombState *>();
  for (std::map<std::string, CombState *>::iterator it =  comb_states_.begin();
       it != comb_states_.end(); it++) {
    if (it->second->is_part(s)) {
      // #TODO: too expensive , should be the other way around -
      // build all combinations according to separator.comb_key
      if (it->second->get_total_score() < min_score) {
        min_score = it->second->get_total_score();
      }
    }
  }

  //get all of the combinations of the minimum score
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
  IMP_assert(min_score < INT_MAX, error_message.str());
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
    std::string s_key = it->second->partial_key(&intersection_particles);
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
  std::stringstream err_msg;
  err_msg<<"JNode::find_minimum the number of requested solutions (";
  err_msg<<num_of_solutions << ")is larger than the enumerated solution ";
  err_msg<<"by the node (" << all_states.size() << ")";
  IMP_assert(all_states.size()>num_of_solutions,err_msg.str().c_str());
  //allocate min_combs with the top best solutions
  int min_num = all_states.size() * (all_states.size() < num_of_solutions) +
                num_of_solutions * (all_states.size() >= num_of_solutions);
  for(unsigned int i=0;i<min_num;i++) {
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

IMPDOMINO_END_NAMESPACE
