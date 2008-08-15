/**
 *  \file JEdge.cpp
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "JEdge.h"

namespace IMP
{

JEdge::JEdge(JNode *source_, JNode *target_)
{
  if (source_->get_node_index() < target_->get_node_index()) {
    source = source_;
    target = target_;
  } else {
    target = source_;
    source = target_;
  }
}

const std::map<std::string, float> * JEdge::get_old_seperators(JNode *n) const
{
  if (source->get_node_index() == n->get_node_index()) {
    return &source_old_score_seperators;
  }
  return &target_old_score_seperators;
}

const std::map<std::string, float> * JEdge::get_new_seperators(JNode *n) const
{
  if (source->get_node_index() == n->get_node_index()) {
    return &source_new_score_seperators;
  }
  return &target_new_score_seperators;
}

void JEdge::init_seperators()
{
  //get the set of interacing particles
  Particles *intersection_set = new Particles();
  source->get_intersection(*target, *intersection_set);
  source->populate_states_of_particles(*intersection_set, &seperators);
  source_old_score_seperators = std::map<std::string, float>();
  target_old_score_seperators = std::map<std::string, float>();
  source_new_score_seperators = std::map<std::string, float>();
  target_new_score_seperators = std::map<std::string, float>();;
  for (std::map<std::string, CombState *>::iterator e  =  seperators.begin();
       e != seperators.end(); e++) {
    source_old_score_seperators[e->first] = 0.0;
    target_old_score_seperators[e->first] = 0.0;
    source_new_score_seperators[e->first] = 0.0;
    target_new_score_seperators[e->first] = 0.0;
  }
}

void JEdge::min_marginalize(JNode *from_node, JNode *to_node)
{
  JNode *fn, *tn;
  std::map<std::string, float> *fnoss, *tnoss, *fnnss, *tnnss;
  fn = source;
  tn = target;
  fnoss = &source_old_score_seperators;
  tnoss = &target_old_score_seperators;
  fnnss = &source_new_score_seperators;
  tnnss = &target_new_score_seperators;
  if (!(source->get_node_index() == from_node->get_node_index())) {
    tn = source;
    fn = target;
    tnoss = &source_old_score_seperators;
    fnoss = &target_old_score_seperators;
    tnnss = &source_new_score_seperators;
    fnnss = &target_new_score_seperators;
  }
  for (std::map<std::string, CombState *>::iterator e  =  seperators.begin();
       e != seperators.end(); e++) {
    std::vector<CombState *> min_p_all;
    //marginize over all particles except for those that are part
    //of the seperator.
    min_p_all = fn->min_marginalize(*(e->second));
    CombState *min_p = min_p_all[0];
    //      (*fnmp)[e->first]=min_p;
    std::cout << "JEdge::min_marginilize for seperator : " << e->first
              <<  " : the optimal from combination is : " << min_p << std::endl;
    (*tnoss)[e->first] = (*tnnss)[e->first];
    (*tnnss)[e->first] = min_p->get_total_score();
    // I think that we should release min_p here - it was allocated
    //in min_marginilize
  }
}
CombState * JEdge::get_seperator(const CombState &other_comb) const
{
  std::cout << " JEdge::get_seperator an edge between nodes "
            << source->get_node_index() << "  " << target->get_node_index()
            << std::endl;
  std::string key = generate_key(other_comb);
  std::cout << " JEdge::get_seperator start2 " << key << std::endl;
  std::cout << " JEdge::get_seperator start33 " << seperators.size()
            << std::endl;
  std::cout << " JEdge::get_seperator start333 " << seperators.begin()->first
            << std::endl;
  std::stringstream error_message;
  error_message << " JEdge::get_seperator a combination with index  : "
                << key << " is not part of the edge seperators" ;
  IMP_assert(seperators.find(key) != seperators.end(), error_message.str());
  std::cout << " JEdge::get_seperator start3 " << std::endl;
  return seperators.find(key)->second;
}

const std::string JEdge::generate_key(const CombState &other_comb) const
{
  Particles *intersection_set = new Particles();
  source->get_intersection(*target, *intersection_set);
  return other_comb.partial_key(intersection_set);
}

void JEdge::show(std::ostream& out) const
{
  out << "=========================JEdge bewteen  " << source->get_node_index();
  out << " and " << target->get_node_index() << " seperator keys and values : ";
  out << std::endl;
  for (std::map<std::string, CombState *>::const_iterator it =
         seperators.begin();it != seperators.end(); it++) {
    out << "(" << it->first << " , " << it->second->get_total_score() << ") ";
  }
  out << std::endl;
}

} // namespace IMP
