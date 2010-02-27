/**
*  \file RestraintGraph.cpp
*  \brief creates a MRF from a set of particles and restraints
*
*  Copyright 2007-2010 Sali Lab. All rights reserved.
*
*/

#include <IMP/domino/RestraintGraph.h>

#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>


IMPDOMINO_BEGIN_NAMESPACE

StringKey node_name_key() {
  static StringKey k("name");
  return k;
}

void RestraintGraph::load_data(const JunctionTree &jt,Model *mdl,
                               RestraintEvaluatorI *r_eval) {

  IMP_INTERNAL_CHECK(jt.get_number_of_nodes()>0,
                     "empty junction tree" << std::endl);
  initialize_graph(jt.get_number_of_nodes());
  std::map<std::string, Particle*> p_map;
  //a mapping between names and particles. Notice that the name
  // is not the default particle one, but a string name attribute
  IMP_LOG(VERBOSE,"RestraintGraph::load_data from model with : " <<
                  mdl->get_number_of_particles() << " particles "<<std::endl);
  for (Model::ParticleIterator it = mdl->particles_begin();
         it != mdl->particles_end(); it++ ) {
    if ((*it)->has_attribute(node_name_key())) {
      IMP_LOG(VERBOSE,"adding particle " << **it << " with key: " <<
              (*it)->get_value(node_name_key())<<std::endl);
      p_map[(*it)->get_value(node_name_key())] = *it;
    }
  }
  IMP_INTERNAL_CHECK(p_map.size()>0,
             "no node was assigned with name attribute" << std::endl);
  // load nodes
  for(int i=0;i<jt.get_number_of_nodes();i++) {
    Particles ps;
    for(int j=0;j<jt.get_number_of_components(i);j++) {
      std::string comp_name = jt.get_component_name(i,j);
      //TODO - talk with Ben about problems with this assert check!
      IMP_INTERNAL_CHECK(p_map.find(comp_name) != p_map.end(),
                 "node with key : " <<
                 comp_name << " was not found" << std::endl);
      IMP_LOG(VERBOSE,"comp: " << comp_name << " is mapped to : " <<
              p_map[comp_name]->get_name()<<std::endl);
      ps.push_back(p_map[comp_name]);
    }
    add_node(i, ps, r_eval);
  }
  //load edges
  for(int n1=0;n1<jt.get_number_of_nodes();n1++) {
    for(int n2=n1+1;n2<jt.get_number_of_nodes();n2++) {
      if (jt.has_edge(n1,n2)) {
        IMP_LOG(VERBOSE,"in Restraint Graph adding edge between : "
                        << n1 << " and " << n2 << std::endl);
        add_edge(n1,n2);
      }
    }
  }
}

RestraintGraph::RestraintGraph(const JunctionTree &jt,Model *mdl,
                 RestraintEvaluatorI *r_eval) {
  load_data(jt,mdl,r_eval);
  infered_ = false;
  sampling_space_set_=false;
  min_combs_ = new std::vector<CombState *>();
}

void RestraintGraph::initialize_graph(int number_of_nodes)
{
  // TODO: here you need the check if the graph has already been initialize
  // - and if so clear everything.
  g_ = Graph(number_of_nodes);
  node_data_ = std::vector<JNode *>(number_of_nodes, (JNode *)NULL);
  edge_data_ = std::map<Pair, JEdge *>();
}

void RestraintGraph::add_node(unsigned int node_index,
                              Particles &particles,
                              RestraintEvaluatorI *rstr_eval)
{
  std::stringstream error_message;
  error_message << " RestraintGraph::add_node the input node_index: "
                << node_index << " is out of range ( " << num_vertices(g_)
                << " ) " ;
  IMP_INTERNAL_CHECK(node_index < num_vertices(g_), error_message.str());
  node_data_[node_index] = new JNode(particles,
                                    node_index);
  node_data_[node_index]->set_restraint_evaluator(rstr_eval);
}

RestraintGraph::Pair RestraintGraph::get_edge_key(unsigned int node1_ind,
                                  unsigned int node2_ind) const
{
  std::stringstream error_message;
  error_message << " RestraintGraph::get_edge_key trying to create a self"
                << " loop for node " << node1_ind;
  IMP_INTERNAL_CHECK(node1_ind != node2_ind, error_message.str());
  if (node1_ind < node2_ind) {
    return Pair(node1_ind, node2_ind);
  }
  return Pair(node2_ind, node1_ind);
}

void RestraintGraph::add_edge(unsigned int node1_ind, unsigned int node2_ind)
{
  IMP_INTERNAL_CHECK(node1_ind < num_vertices(g_),
             "RestraintGraph::add_node the input node_index: "
             << node1_ind << " is out of range ( " << num_vertices(g_)<<" )");
  IMP_INTERNAL_CHECK(node2_ind < num_vertices(g_),
             " RestraintGraph::add_node the input node_index: "
             << node2_ind << " is out of range ( " << num_vertices(g_)<<" )");
  boost::add_edge(node1_ind, node2_ind, g_);
  edge_data_[get_edge_key(node1_ind,node2_ind)] =
    new JEdge(node_data_[node1_ind], node_data_[node2_ind]);
}

void RestraintGraph::analyse(std::ostream &out) const
{
  out << "===========RestraintGraph Analysis=========" << std::endl;
}

void RestraintGraph::show(std::ostream& out) const
{
  out << "===========RestraintGraph=========" << std::endl;
  out << " Nodes: " << std::endl;
  for (unsigned int vi = 0;vi < num_vertices(g_);vi++) {
    JNode *j = node_data_[vi];
    j->show(out);
  }
  out << " Edges: " << std::endl;
  boost::graph_traits<Graph>::edge_iterator ei_begin, ei_end, ei;
  tie(ei_begin, ei_end) = edges(g_);
  for (ei = ei_begin; ei != ei_end; ei++) {
    out << *ei << std::endl;
    edge_data_.find(get_edge_key(source(*ei, g_),
                                target(*ei, g_)))->second->show(out);
  }
  out << "==================================" << std::endl;
  out << "==================================" << std::endl;
}

void RestraintGraph::set_sampling_space(DiscreteSampler &ds_)
{
  for (unsigned int vi = 0; vi < num_vertices(g_); vi++) {
    IMP_LOG(VERBOSE,"Initializing node number " << vi <<std::endl);
    JNode *j = node_data_[vi];
    IMP_USAGE_CHECK(j != NULL,
             "the nodes of the restraint graph have not been initialized");
    j->init_sampling(ds_);
  }
  for (std::map<Pair, JEdge *>::iterator it = edge_data_.begin();
       it != edge_data_.end(); it++) {
    it->second->init_separators();
  }
  clear_infered_data();
  sampling_space_set_=true;
}
void RestraintGraph::clear_infered_data() {
  infered_=false;
  for(std::vector<CombState *>::iterator it = min_combs_->begin();
      it != min_combs_->end();it++) {
    delete *it;
  }
  min_combs_->clear();
}
void RestraintGraph::show_sampling_space(std::ostream& out) const
{
  for (unsigned int vi = 0; vi < num_vertices(g_); vi++) {
    JNode *j = node_data_[vi];
    j->show_sampling_space(out);
  }
}
JNode * RestraintGraph::get_node(const Particles &ps)
{
  std::vector<int> inter;
  for (unsigned int vi = 0; vi < num_vertices(g_); vi++) {
    JNode *j = node_data_[vi];
    if (j->is_part(ps)) {
      return j;
    }
  }
  IMP_INTERNAL_CHECK(true,
    "RestraintGraph::get_node the input set of particles"
     << " is not part of any of the nodes in the graph"<<std::endl);
  return NULL;
}

void RestraintGraph::initialize_potentials(Restraint *r, Particles *ps,
                                           Float weight) {
  JNode *jn = NULL;
  jn = get_node(*ps);
  if (jn == NULL) { // TODO - should use IMP_INTERNAL_CHECK
    IMP_WARN(" no node - the restraint : " << *r
             << " between particles: ");
    for (Particles::const_iterator ii = ps->begin();ii < ps->end();ii++) {
      IMP_WARN( (*ii)->get_value(node_name_key()) << " ("
                <<(*ii)->get_value(node_name_key()) <<"):: ");
    }
    IMP_WARN( " has not been realized." << std::endl);
  }
  else {
    IMP_IF_LOG(TERSE) {
      IMP_LOG(TERSE,"restraint : " );
      IMP_LOG_WRITE(TERSE,r->show());
      IMP_LOG(TERSE,"is realized by node with index : "<<
                     jn->get_node_index()<<std::endl);
    }
    jn->realize(r, ps, weight);
  }
}

void RestraintGraph::dfs_order(unsigned int root_ind)
{
  //http://www.boost.org/doc/libs/1_35_0/libs/graph/example/dfs-example.cpp
  typedef boost::graph_traits <Graph>::vertices_size_type size_type;
  std::vector < size_type > dtime(num_vertices(g_));
  std::vector < size_type > ftime(num_vertices(g_));
  size_type t = 0;
  dfs_time_visitor < size_type * >vis(&dtime[0], &ftime[0], t);
  depth_first_search(g_, visitor(vis));
  // use std::sort to order the vertices by their discover time
  std::vector < size_type > discover_order(num_vertices(g_));
  boost::integer_range < size_type > r(0, num_vertices(g_));
  discover_time_.insert(discover_time_.begin(), num_vertices(g_), 0);
  typedef size_type* Iiter;
  std::copy(r.begin(), r.end(), discover_order.begin());
  std::sort(discover_order.begin(), discover_order.end(),
            boost::indirect_cmp < Iiter, std::less < size_type > >(&dtime[0]));
  for (unsigned int i = 0; i < num_vertices(g_); ++i) {
    discover_time_[discover_order[i]] = i;
  }
}

void  RestraintGraph::infer(unsigned int num_of_solutions)
{
  IMP_INTERNAL_CHECK(sampling_space_set_,
                     "The sampling space was not set"<<std::endl);
  IMP_LOG(VERBOSE,"RestraintGraph::infer number of solutions : "
          <<num_of_solutions <<std::endl);
  std::stringstream err_msg;
  err_msg << "RestraintGraph::infer the graph has already been infered."
          << " Please reset the graph before calling infer";
  IMP_INTERNAL_CHECK(infered_ == false, err_msg.str());
  root_ = 0;
  IMP_LOG(VERBOSE,"RestraintGraph::infer before dfs_order " << std::endl);
  dfs_order(root_);
  IMP_LOG(VERBOSE,"RestraintGraph::infer before collect_evidence"<<std::endl);
  collect_evidence(root_);
  IMP_LOG(VERBOSE,"RestraintGraph::infer before distribute_evidence"
          <<std::endl);
  distribute_evidence(root_);
  IMP_LOG(VERBOSE,"RestraintGraph::infer after distribute_evidence"<<std::endl);
  std::vector<CombState *>*  temp_min_combs =
      node_data_[root_]->find_minimum(false,num_of_solutions);
  IMP_INTERNAL_CHECK(temp_min_combs->size()>0,
                     "RestraintGraph::infer the number of"
             <<" minimum solutions is 0");
  // distribute the minimum combinations and return the final full comb state.
  //CombState *min_comb;
  for(std::vector<CombState *>::iterator it =  temp_min_combs->begin();
      it != temp_min_combs->end(); it++) {
    CombState *min_comb = new CombState(**(it));
    distribute_minimum(root_, min_comb);
    IMP_IF_LOG(TERSE) {
      IMP_LOG(TERSE,"====MINIMUM COMBINATION number " <<
            it-temp_min_combs->begin()<<" : ============== " << std::endl);
      IMP_LOG_WRITE(TERSE,min_comb->show(IMP_STREAM));
    }
    min_combs_->push_back(min_comb);
  }
  delete temp_min_combs;
  infered_ = true;
  IMP_LOG(VERBOSE,"RestraintGraph::going to move to global minimum"
                  << std::endl);
  move_to_global_minimum_configuration();
  IMP_LOG(VERBOSE,"the model is at its global minimum"<< std::endl);
}

void RestraintGraph::distribute_minimum(unsigned int father_ind,
                                        CombState *min_comb)
{
  IMP_LOG(VERBOSE,"going to distribute the minimum for node : "
          << father_ind << "with " << boost::degree(father_ind,g_)
          << " children the total number of edges is : "
          << boost::num_edges(g_) << std::endl);
 boost::graph_traits<Graph>::adjacency_iterator adj_first, adj_last;
  boost::tie(adj_first, adj_last) = adjacent_vertices(father_ind, g_);
  for (boost::graph_traits<Graph>::adjacency_iterator child_it = adj_first;
       child_it != adj_last;  child_it++) {
    IMP_LOG(VERBOSE,"discover time of father : (" << father_ind << ") is : "
            << discover_time_[father_ind] << " discover time of child (" <<
            *child_it << ") is: " << discover_time_[*child_it] << std::endl);
    if (!(discover_time_[*child_it] > discover_time_[father_ind])) {
      continue;
    }
    JNode *child_data = node_data_[*child_it];
    //get the separator that corresponds to the father's minimum state.
    JEdge *e = get_edge(father_ind, *child_it);
    CombState *min_father_separator = e->get_separator(*min_comb);
    std::vector<CombState *> child_min_state;
    child_min_state = child_data->min_marginalize(*min_father_separator);
    std::vector<CombState*>::const_iterator it = child_min_state.begin();
    bool passed = false;
    while (it != child_min_state.end() && !passed) {
      try {
        min_comb->combine(*it);
        passed = true;
      } catch (IMP::InternalException e) {
         IMP_WARN("problem distributing minimum");
        }
      it++;
    }
    IMP_INTERNAL_CHECK(passed,
                       "could not combine any of the " << child_min_state.size()
               << " states " << std::endl);
    //TODO - should we free here min_father_separator  and child_min_state
    distribute_minimum(*child_it, min_comb);
  }
}

unsigned int RestraintGraph::collect_evidence(unsigned int father_ind)
{
  //go over all of the children and collect evidence
  boost::graph_traits<Graph>::adjacency_iterator adj_first, adj_last;
  boost::tie(adj_first, adj_last) = adjacent_vertices(father_ind, g_);
  for (boost::graph_traits<Graph>::adjacency_iterator child_it = adj_first;
       child_it != adj_last;  child_it++) {
    // if the child appears before the father in the dfs doscovery order,
    //continue
    if (discover_time_[*child_it] <= discover_time_[father_ind]) {
      continue;
    }
    update(father_ind, collect_evidence(*child_it));
  }
  return father_ind;
}

void RestraintGraph::distribute_evidence(unsigned int father_ind)
{
  boost::graph_traits<Graph>::adjacency_iterator adj_first, adj_last;
  boost::tie(adj_first, adj_last) = adjacent_vertices(father_ind, g_);
  for (boost::graph_traits<Graph>::adjacency_iterator child_it = adj_first;
       child_it != adj_last; child_it++) {
    // if the child appears before the father in the dfs doscovery
    //order, continue
    if (!(discover_time_[*child_it] < discover_time_[father_ind])) {
      continue;
    }
    update(*child_it, father_ind);
    distribute_evidence(*child_it);
  }
}

void RestraintGraph::update(unsigned int w, unsigned int v)
{
  IMP_LOG(VERBOSE,"RestraintGraph::update start from "<< w <<" to "
                  <<v<< std::endl);
  // update node with index w  based on node with index: v
  // check if there is an edge between w and v
  IMP_INTERNAL_CHECK(edge_data_.find(get_edge_key(w, v)) != edge_data_.end(),
             " RestraintGraph::update the nodes with indexes : "
             << w << " and " << v
             << " are not neighbors. Can not perform the update"
             <<std::endl);
  JNode *w_data = node_data_[w];
  JNode *v_data = node_data_[v];
  //minimize over all sub-configurations in  v that do not involve the w
  JEdge *e = edge_data_[get_edge_key(w,v)];
  IMP_LOG(VERBOSE,"RestraintGraph::update before min_marginalize"<< std::endl);
  e->min_marginalize(v_data, w_data);
  //now update the to_node according to the new separators
  Particles intersection_set;
  v_data->get_intersection(*w_data, intersection_set);
  IMP_LOG(VERBOSE,"RestraintGraph::update before update potentials"
                  << std::endl);
  w_data->update_potentials(*(e->get_old_separators(w_data)),
                            *(e->get_new_separators(w_data)),
                            intersection_set);
  IMP_LOG(VERBOSE,"RestraintGraph::update end from "<< w
                  <<" to "<<v<< std::endl);

}

void RestraintGraph::move_to_global_minimum_configuration() const
{
  IMP_INTERNAL_CHECK(infered_,
      "RestraintGraph::move_to_global_minimum_configuration "
      <<" the graph has not been infered");
  CombState *best_state = *(min_combs_->begin());
  move_to_configuration(*best_state);
}

Float RestraintGraph::move_to_configuration(const CombState &comb) const {
  IMP_IF_LOG(VERBOSE) {
    IMP_LOG(VERBOSE,
           "RestraintGraph::move_to_configuration start moving to "
           << "configuration: "<<std::endl);
    IMP_LOG_WRITE(VERBOSE,comb.show());
    IMP_LOG(VERBOSE,std::endl);
  }
  Float score=0.0;
  for (std::vector<JNode *>::const_iterator it = node_data_.begin();
       it != node_data_.end(); it++) {
    CombState *node_state = comb.get_partial(*((*it)->get_particles()));
    (*it)->move2state(node_state);
    score += (*it)->get_score(comb);
    delete(node_state);
  }
  return score;
}

//! TODO - check that this is still correct
CombState *RestraintGraph::get_minimum_configuration() const
{
  IMP_INTERNAL_CHECK(infered_, "RestraintGraph::move_model2global_minimum the "
             << "graph has not been infered");
  CombState *best_state = *(min_combs_->begin());
  return best_state;
  //move_model2state_rec(root_, *best_state);
}
// CombState *RestraintGraph::get_minimum_comb() const
// {
// >>>>>>> .r3915
//   std::stringstream err_msg;
//   err_msg << "RestraintGraph::move_model2global_minimum the "
//           << "graph has not been infered";
//   IMP_INTERNAL_CHECK(infered_, err_msg.str());
//   return *(min_combs_->begin());
// }
void RestraintGraph::clear() {
  for(std::map<Pair, JEdge *>::iterator it = edge_data_.begin();
      it != edge_data_.end();it++) {
    it->second->clear();
  }
  for(std::vector<JNode *>::iterator it = node_data_.begin();
      it != node_data_.end();it++) {
    (*it)->clear();
  }
  clear_infered_data();
}

const CombState *RestraintGraph::get_opt_combination(unsigned int i) const {
  std::stringstream err_msg;
  err_msg << "RestraintGraph::get_opt_combination no combinations have ";
  err_msg <<" been infered";
  IMP_INTERNAL_CHECK(infered_, err_msg.str());
  err_msg.clear();
  err_msg << "RestraintGraph::get_opt_combination the requested combination";
  err_msg <<" index is out of range " << i << " >= " << min_combs_->size();
  IMP_INTERNAL_CHECK(i<min_combs_->size(), err_msg.str());
  return (*min_combs_)[i];
}

Particles RestraintGraph::get_particles() const {
  Particles ps;
  std::map<std::string,Particle *> build_ps;
  for (unsigned int vi = 0;vi < num_vertices(g_);vi++) {
    JNode *j = node_data_[vi];
    const Particles *node_particles = j->get_particles();
    for(Particles::const_iterator it = node_particles->begin();
        it != node_particles->end();it++) {
      if (build_ps.find((*it)->get_name()) == build_ps.end()) {
        build_ps[(*it)->get_name()]=*it;
        ps.push_back(*it);
      }
    }
  }
  return ps;
}
IMPDOMINO_END_NAMESPACE
