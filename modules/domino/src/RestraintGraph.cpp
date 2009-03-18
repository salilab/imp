/**
*  \file RestraintGraph.cpp
*  \brief creates a MRF from a set of particles and restraints
*
*  Copyright 2007-9 Sali Lab. All rights reserved.
*
*/

#include <IMP/domino/RestraintGraph.h>

#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>

IMPDOMINO_BEGIN_NAMESPACE

  void RestraintGraph::parse_jt_file(const std::string &filename, Model *mdl)
  {
    StringKey name_key("name");
    // for fast access to particle by its name, create a map
    std::map<std::string, Particle *> p_map;
    IMP_LOG(VERBOSE,"start checking particles"<< std::endl);
    for (Model::ParticleIterator it = mdl->particles_begin();
         it != mdl->particles_end(); it++ ) {
       if ((*it)->has_attribute(name_key)) {
         IMP_LOG(VERBOSE,"adding index for particle named: "
                 << (*it)->get_value(name_key)<< std::endl);
         p_map[(*it)->get_value(name_key)] = *it;
       }
    }
    IMP_LOG(VERBOSE,"end checking particles"<< std::endl);
    // load the nodes and edges
    IMP_LOG(VERBOSE,"going to open file: " << filename<< std::endl);
    std::ifstream myfile(filename.c_str());
    IMP_assert(myfile.is_open(),
               "could not open junction tree file: " << filename<< std::endl);
    char line[1024];
    unsigned int number_of_nodes=0;
    unsigned int status = 0;
    std::vector<std::string> v;
    std::stringstream err_msg;
    while (myfile.getline(line, 1024) && status != 2) {
      IMP_LOG(VERBOSE,"line: " << line);
      v.clear();
      char *pch = strtok(line, " ");
      while (pch != NULL) {
        v.push_back(std::string(pch));
        pch = strtok(NULL, " ");
      }
      if (status == 0) {
        if (v[0] == "nodes:") {
          status = 1;
        }
        else {
          err_msg.clear();
          err_msg << "RestraintGraph::parse_jt_file wrong file format:";
          IMP_assert(true, err_msg.str());
        }
      }
      else if (status==1) {
        if  (v[0]=="edges:") {
          status=2;
        }
        else {
          number_of_nodes++;
        }
      }
    }
    initialize_graph(number_of_nodes);
    std::pair<int, int> last_key;
    status = 0;
    unsigned int node_index = 0;
    // move back to the beginning of the file
    myfile.seekg (0);
    //TODO - remove this when possible
    std::ifstream myfile1(filename.c_str());
    IMP_assert(myfile1.is_open(),
               "could not open junction tree file: " << filename<< std::endl);
    //TODO END
    while (myfile1.getline(line, 1024)) {
      v.clear();
      char *pch = strtok(line, " ");
      while (pch != NULL) {
        v.push_back(std::string(pch));
        pch = strtok(NULL, " ");
      }
      if (status == 0) {
        if (v[0] == "nodes:") {
          status = 1;
        }
        else {
          err_msg.clear();
          err_msg << "RestraintGraph::parse_jt_file wrong file format:";
          IMP_assert(true, err_msg.str());
        }
      }
      else if (status==1) {
        if  (v[0]=="edges:") {
          status=2;
        }
        else {
          Particles ps;
          for(std::vector<std::string>::iterator it =  v.begin();
              it != v.end(); it++) {
            IMP_assert(p_map.find(*it) != p_map.end(), "node with key : " <<
                       *it << " was not found" << std::endl);
            ps.push_back(p_map[*it]);
          }
          add_node(node_index, ps);
          node_index = node_index+1;
        }
      }
      else if (status ==2) {
        IMP_assert(v.size()==2,
           "wrong format of edge line:" << line << std::endl);
        add_edge(atoi(v[0].c_str()),atoi(v[1].c_str()));
      }
    }
  }

RestraintGraph::RestraintGraph(const std::string &filename, Model *mdl)
{
  parse_jt_file(filename, mdl);
  infered_ = false;
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
                              Particles &particles)
{
  std::stringstream error_message;
  error_message << " RestraintGraph::add_node the input node_index: "
                << node_index << " is out of range ( " << num_vertices(g_)
                << " ) " ;
  IMP_assert(node_index < num_vertices(g_), error_message.str());
  node_data_[node_index] = new JNode(particles,
                                    node_index);
}

RestraintGraph::Pair RestraintGraph::get_edge_key(unsigned int node1_ind,
                                  unsigned int node2_ind) const
{
  std::stringstream error_message;
  error_message << " RestraintGraph::get_edge_key trying to create a self"
                << " loop for node " << node1_ind;
  IMP_assert(node1_ind != node2_ind, error_message.str());
  if (node1_ind < node2_ind) {
    return Pair(node1_ind, node2_ind);
  }
  return Pair(node2_ind, node1_ind);
}

void RestraintGraph::add_edge(unsigned int node1_ind, unsigned int node2_ind)
{
  IMP_assert(node1_ind < num_vertices(g_),
             "RestraintGraph::add_node the input node_index: "
             << node1_ind << " is out of range ( " << num_vertices(g_)<<" )");
  IMP_assert(node2_ind < num_vertices(g_),
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
    IMP_check(j != NULL,
             "the nodes of the restraint graph have not been initialized",
             ErrorException);
    j->init_sampling(ds_);
  }
  for (std::map<Pair, JEdge *>::iterator it = edge_data_.begin();
       it != edge_data_.end(); it++) {
    it->second->init_separators();
  }
  clear_infered_data();
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
  std::stringstream error_message;
  error_message << "RestraintGraph::get_node the input set of particles"
                << " is not part of any of the nodes in the graph";
  IMP_assert(true, error_message.str());
  return NULL;
}

void RestraintGraph::initialize_potentials(Restraint *r, Particles *ps,
                                           Float weight) {
  JNode *jn = get_node(*ps);
  if (jn == NULL) {
    std::cerr << "PROBLEM - no node - the restraint : ";
    r->show(std::cerr);
    std::cerr << " between particles: ";
    for (Particles::const_iterator ii = ps->begin();ii < ps->end();ii++) {
      std::cerr << (*ii)->get_index() << " ("
      <<(*ii)->get_value(StringKey("name"))<<"):: ";
    }
    std::cerr << " has not been realized." << std::endl;
  }
  else {
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
  discover_time.insert(discover_time.begin(), num_vertices(g_), 0);
  typedef size_type* Iiter;
  std::copy(r.begin(), r.end(), discover_order.begin());
  std::sort(discover_order.begin(), discover_order.end(),
            boost::indirect_cmp < Iiter, std::less < size_type > >(&dtime[0]));
  for (unsigned int i = 0; i < num_vertices(g_); ++i) {
    discover_time[discover_order[i]] = i;
  }
}

void  RestraintGraph::infer(unsigned int num_of_solutions)
{
  IMP_LOG(VERBOSE,"RestraintGraph::infer number of solutions : "
          <<num_of_solutions <<std::endl);
  std::stringstream err_msg;
  err_msg << "RestraintGraph::infer the graph has already been infered."
          << " Please reset the graph before calling infer";
  IMP_assert(infered_ == false, err_msg.str());
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
  IMP_assert(temp_min_combs->size()>0, "RestraintGraph::infer the number of"
             <<" minimum solutions is 0");
  // distribute the minimu combinations and return the final full comb state.
  CombState *min_comb;
  for(std::vector<CombState *>::iterator it =  temp_min_combs->begin();
      it != temp_min_combs->end(); it++) {
    min_comb = new CombState(**(it));
    distribute_minimum(root_, min_comb);
    IMP_LOG(VERBOSE,"====MINIMUM COMBINATION number " <<
            it-temp_min_combs->begin()<<" : ============== " << std::endl);
    min_comb->show(); //TODO _ move to log
    min_combs_->push_back(min_comb);
  }
  delete temp_min_combs;
  infered_ = true;
  move_model2global_minimum();
}

void RestraintGraph::distribute_minimum(unsigned int father_ind,
                                        CombState *min_comb)
{
  boost::graph_traits<Graph>::adjacency_iterator adj_first, adj_last;
  boost::tie(adj_first, adj_last) = adjacent_vertices(father_ind, g_);
  for (boost::graph_traits<Graph>::adjacency_iterator child_it = adj_first;
       child_it != adj_last;  child_it++) {
    if (!(discover_time[*child_it] > discover_time[father_ind])) {
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
      } catch (IMP::ErrorException e) {}
      it++;
    }
    if (!passed) {
      std::cerr << " could not combine any of the " << child_min_state.size()
                << " states " << std::endl;
      throw 1;
    }
    //TODO - should we free here min_father_separator  and child_min_state
    distribute_minimum(*child_it, min_comb);
  }
}

unsigned int RestraintGraph::collect_evidence(unsigned int father_ind)
{
  //go over all of the childs and collect evidence
  boost::graph_traits<Graph>::adjacency_iterator adj_first, adj_last;
  boost::tie(adj_first, adj_last) = adjacent_vertices(father_ind, g_);
  for (boost::graph_traits<Graph>::adjacency_iterator child_it = adj_first;
       child_it != adj_last;  child_it++) {
    // if the child appears before the father in the dfs doscovery order,
    //continue
    if (discover_time[*child_it] <= discover_time[father_ind]) {
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
    if (!(discover_time[*child_it] < discover_time[father_ind])) {
      continue;
    }
    update(*child_it, father_ind);
    distribute_evidence(*child_it);
  }
}

void RestraintGraph::update(unsigned int w, unsigned int v)
{
  std::cout<<"RestraintGraph::update start from "<< w <<" to " <<v<< std::endl;
  // update node with index w  based on node with index: v
  // check if there is an edge between w and v
  std::stringstream error_message;
  error_message << " RestraintGraph::update the nodes with indexes : "
                << w << " and " << v
                << " are not neighbors. Can not perform the update" ;
  IMP_assert(edge_data_.find(get_edge_key(w, v)) != edge_data_.end(),
             error_message.str());
  JNode *w_data = node_data_[w];
  JNode *v_data = node_data_[v];
  //minimize over all sub-configurations in  v that do not involve the w
  JEdge *e = edge_data_[get_edge_key(w,v)];
  std::cout<<"RestraintGraph::update before min_marginalize"<< std::endl;
  e->min_marginalize(v_data, w_data);
  //now update the to_node according to the new separators
  Particles intersection_set;
  v_data->get_intersection(*w_data, intersection_set);
  std::cout<<"RestraintGraph::update before update potentials"<< std::endl;
  w_data->update_potentials(*(e->get_old_separators(w_data)),
                            *(e->get_new_separators(w_data)),
                            intersection_set);
  std::cout<<"RestraintGraph::update end from "<< w <<" to "<<v<< std::endl;

}

void RestraintGraph::move_model2state_rec(unsigned int father_ind,
                                          CombState &best_state) const
{
  for (std::vector<JNode *>::const_iterator it = node_data_.begin();
       it != node_data_.end(); it++) {
    CombState *node_state = best_state.get_partial(*((*it)->get_particles()));
    (*it)->move2state(node_state);
    delete(node_state);
  }
}
void RestraintGraph::move_model2global_minimum() const
{
  IMP_assert(infered_, "RestraintGraph::move_model2global_minimum the "
             << "graph has not been infered");
  CombState *best_state = *(min_combs_->begin());
  move_model2state_rec(root_, *best_state);
}
CombState *RestraintGraph::get_minimum_comb() const
{
  std::stringstream err_msg;
  err_msg << "RestraintGraph::move_model2global_minimum the "
          << "graph has not been infered";
  IMP_assert(infered_, err_msg.str());
  return *(min_combs_->begin());
}
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
  IMP_assert(infered_, err_msg.str());
  err_msg.clear();
  err_msg << "RestraintGraph::get_opt_combination the requested combination";
  err_msg <<" index is out of range " << i << " >= " << min_combs_->size();
  IMP_assert(i<min_combs_->size(), err_msg.str());
  return (*min_combs_)[i];
}


IMPDOMINO_END_NAMESPACE
