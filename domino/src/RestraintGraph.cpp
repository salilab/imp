/**
*  \file RestraintGraph.cpp
*  \brief creates a MRF from a set of particles and restraints
*
*  Copyright 2007-8 Sali Lab. All rights reserved.
*
*/

#include "IMP/domino/RestraintGraph.h"
#include <algorithm>
#include <boost/graph/copy.hpp>
#include <boost/pending/indirect_cmp.hpp>

namespace IMP
{
namespace domino
{
  void RestraintGraph::parse_jt_file(const std::string &filename, Model *mdl)
  {
    // for fast access to particle by its name, create a map
    std::map<std::string, Particle *> p_map;
    for (Model::ParticleIterator it = mdl->particles_begin();
         it != mdl->particles_end(); it++ ) {
      p_map[(*it)->get_value(IMP::StringKey("name"))] = *it;
    }
    // load the nodes and edges
    std::ifstream myfile(filename.c_str());
    char line[1024];
    unsigned int number_of_nodes=0;
    unsigned int status = 0;
    std::vector<std::string> v;
    std::stringstream err_msg;
    while (myfile.getline(line, 1024) and (status != 2)) {
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
    while (myfile.getline(line, 1024)) {
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
            ps.push_back(p_map[*it]);
          }
          add_node(node_index, ps);
          node_index = node_index+1;
        }
      }
      else if (status ==2) {
        if (v.size() != 2) {
          err_msg.clear();
          err_msg << "RestraintGraph::parse_jt_file wrong format of edge line:";
          IMP_assert(true, err_msg.str());
        }
        add_edge(atoi(v[0].c_str()),atoi(v[1].c_str()));
      }
    }
  }
RestraintGraph::RestraintGraph(const std::string &filename, Model *mdl)
{
  parse_jt_file(filename, mdl);
  infered = false;
  min_combs = new std::vector<CombState *>();
}

void RestraintGraph::set_model(IMP::Model *m_)
{
  m = m_;
}

void RestraintGraph::initialize_graph(int number_of_nodes)
{
  // TODO: here you need the check if the graph has already been initialize
  // - and if so clear everything.
  g = Graph(number_of_nodes);
  node_data = std::vector<JNode *>(number_of_nodes, (JNode *)NULL);
  edge_data = std::map<Pair, JEdge *>();
}

void RestraintGraph::add_node(unsigned int node_index,
                              Particles &particles)
{
  std::stringstream error_message;
  error_message << " RestraintGraph::add_node the input node_index: "
                << node_index << " is out of range ( " << num_vertices(g)
                << " ) " ;
  IMP_assert(node_index < num_vertices(g), error_message.str());
  node_data[node_index] = new JNode(particles,
                                    node_index);
}

Pair RestraintGraph::get_edge_key(unsigned int node1_ind,
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
  std::stringstream error_message;
  error_message << " RestraintGraph::add_node the input node_index: "
                << node1_ind << " is out of range ( " << num_vertices(g)
                << " ) " ;
  IMP_assert(node1_ind < num_vertices(g), error_message.str());
  error_message.clear();
  error_message << " RestraintGraph::add_node the input node_index: "
                << node2_ind << " is out of range ( " << num_vertices(g)
                << " ) " ;
  IMP_assert(node2_ind < num_vertices(g), error_message.str());
  boost::add_edge(node1_ind, node2_ind, g);
  edge_data[get_edge_key(node1_ind,node2_ind)] =
    new JEdge(node_data[node1_ind], node_data[node2_ind]);
}

void RestraintGraph::analyse(std::ostream &out) const
{
  out << "===========RestraintGraph Analysis=========" << std::endl;
}

void RestraintGraph::show(std::ostream& out) const
{
  out << "===========RestraintGraph=========" << std::endl;
  out << " Nodes: " << std::endl;
  for (unsigned int vi = 0;vi < num_vertices(g);vi++) {
    JNode *j = node_data[vi];
    j->show(out);
  }
  out << " Edges: " << std::endl;
  boost::graph_traits<Graph>::edge_iterator ei_begin, ei_end, ei;
  tie(ei_begin, ei_end) = edges(g);
  for (ei = ei_begin; ei != ei_end; ei++) {
    out << *ei << std::endl;
    edge_data.find(get_edge_key(source(*ei, g),
                                target(*ei, g)))->second->show(out);
  }
  out << "==================================" << std::endl;
  out << "==================================" << std::endl;
}

void RestraintGraph::set_sampling_space(const DiscreteSampler &ds_)
{
  for (unsigned int vi = 0; vi < num_vertices(g); vi++) {
    JNode *j = node_data[vi];
    j->init_sampling(ds_);
  }
  for (std::map<Pair, JEdge *>::iterator it = edge_data.begin();
       it != edge_data.end(); it++) {
    it->second->init_separators();
  }
  infered=false;
}

void RestraintGraph::show_sampling_space(std::ostream& out) const
{
  for (unsigned int vi = 0; vi < num_vertices(g); vi++) {
    JNode *j = node_data[vi];
    j->show_sampling_space(out);
  }
}

JNode * RestraintGraph::get_node(const Particles &p)
{
  std::vector<int> inter;
  for (unsigned int vi = 0; vi < num_vertices(g); vi++) {
    JNode *j = node_data[vi];
    if (j->is_part(p)) {
      return j;
    }
  }
  std::stringstream error_message;
  error_message << "RestraintGraph::get_node the input set of particles"
                << " is not part of any of the nodes in the graph";
  IMP_assert(true, error_message.str());
  return NULL;
}

void RestraintGraph::initialize_potentials(Restraint &r, Float weight)
{
  Particles r_particles;
  ParticlesList pl = r.get_interacting_particles();
  for(ParticlesList::iterator it1 = pl.begin();
      it1 != pl.end(); it1++){
    for(Particles::iterator it2 = it1->begin(); it2 != it1->end(); it2++) {
      r_particles.push_back(*it2);
     }
  }
  JNode *jn = get_node(r_particles);
  if (jn == NULL) {
    std::cerr << "PROBLEM - no node - the restraint : ";
    r.show(std::cerr);
    std::cerr << " between particles: ";
    for (Particles::const_iterator ii = r_particles.begin();
         ii < r_particles.end();ii++) {
      std::cerr << (*ii)->get_index().get_index() << ","
                << (*ii)->get_value(IMP::StringKey("name")) << " :: ";
    }
    std::cerr << " has not been realized." << std::endl;
  }
  else {
    jn->realize(&r, weight);
  }
}

void RestraintGraph::dfs_order(unsigned int root_ind)
{
  //http://www.boost.org/doc/libs/1_35_0/libs/graph/example/dfs-example.cpp
  typedef boost::graph_traits <Graph>::vertices_size_type size_type;
  std::vector < size_type > dtime(num_vertices(g));
  std::vector < size_type > ftime(num_vertices(g));
  size_type t = 0;
  dfs_time_visitor < size_type * >vis(&dtime[0], &ftime[0], t);
  depth_first_search(g, visitor(vis));
  // use std::sort to order the vertices by their discover time
  std::vector < size_type > discover_order(num_vertices(g));
  boost::integer_range < size_type > r(0, num_vertices(g));
  discover_time.insert(discover_time.begin(), num_vertices(g), 0);
  typedef size_type* Iiter;
  std::copy(r.begin(), r.end(), discover_order.begin());
  std::sort(discover_order.begin(), discover_order.end(),
            boost::indirect_cmp < Iiter, std::less < size_type > >(&dtime[0]));
  for (unsigned int i = 0; i < num_vertices(g); ++i) {
    discover_time[discover_order[i]] = i;
  }
}

void  RestraintGraph::infer()
{
  show();
  min_combs->clear();
  std::stringstream err_msg;
  err_msg << "RestraintGraph::infer the graph has already been infered."
          << " Please reset the graph before calling infer";
  IMP_assert(infered == false, err_msg.str());
  root = 0;
  dfs_order(root);
  collect_evidence(root);
  distribute_evidence(root);
  //min_combs = node_data[root]->find_minimum();
  std::vector<CombState *>*  temp_min_combs = node_data[root]->find_minimum();
  err_msg.clear();
  err_msg << "RestraintGraph::infer the number of minimum solutions is 0";
  IMP_assert(temp_min_combs->size()>0, err_msg.str());
  // distribute the minimu combinations and return the final full comb state.
  CombState *min_comb = new CombState(**(temp_min_combs->begin()));
  distribute_minimum(root, min_comb);
  std::cout << "==THE MINIMUM COMBINATION: ============== " << std::endl;
  min_comb->show();
  min_combs->push_back(min_comb);
  delete temp_min_combs;
  infered = true;
  move_model2global_minimum();
}

void RestraintGraph::distribute_minimum(unsigned int father_ind,
                                        CombState *min_comb)
{
  boost::graph_traits<Graph>::adjacency_iterator adj_first, adj_last;
  boost::tie(adj_first, adj_last) = adjacent_vertices(father_ind, g);
  for (boost::graph_traits<Graph>::adjacency_iterator child_it = adj_first;
       child_it != adj_last;  child_it++) {
    if (!(discover_time[*child_it] > discover_time[father_ind])) {
      continue;
    }
    JNode *child_data = node_data[*child_it];
    //get the separator that corresponds to the father's minimum state.
    JEdge *e = get_edge(father_ind, *child_it);
    CombState *min_father_separator = e->get_separator(*min_comb);
    std::vector<CombState *> child_min_state;
    child_min_state = child_data->min_marginalize(*min_father_separator);
    std::vector<CombState*>::const_iterator it = child_min_state.begin();
    bool passed = false;
    while ((it != child_min_state.end()) and not passed) {
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
  boost::tie(adj_first, adj_last) = adjacent_vertices(father_ind, g);
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
  boost::tie(adj_first, adj_last) = adjacent_vertices(father_ind, g);
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
  // update node with index w  based on node with index: v
  // check if there is an edge between w and v
  std::stringstream error_message;
  error_message << " RestraintGraph::update the nodes with indexes : "
                << w << " and " << v
                << " are not neighbors. Can not perform the update" ;
  IMP_assert(edge_data.find(get_edge_key(w, v)) != edge_data.end(),
             error_message.str());
  JNode *w_data = node_data[w];
  JNode *v_data = node_data[v];
  //minimize over all sub-configurations in  v that do not involve the w
  JEdge *e = edge_data[get_edge_key(w,v)];
  e->min_marginalize(v_data, w_data);
  //now update the to_node according to the new separators
  Particles intersection_set;
  v_data->get_intersection(*w_data, intersection_set);
  w_data->update_potentials(*(e->get_old_separators(w_data)),
                            *(e->get_new_separators(w_data)),
                            intersection_set);
}

void RestraintGraph::move_model2state_rec(unsigned int father_ind,
                                          CombState &best_state) const
{
  for (std::vector<JNode *>::const_iterator it = node_data.begin();
       it != node_data.end(); it++) {
    CombState *node_state = best_state.get_partial(*((*it)->get_particles()));
    (*it)->move2state(*node_state);
    delete(node_state);
  }
}
  /*
float RestraintGraph::move_model2state(unsigned int state_index) const
{
  std::stringstream err_msg;
  err_msg << "RestraintGraph::move_model2state the graph"
          << " has not been infered";
  IMP_assert(infered, err_msg.str());
  CombState *best_state = node_data[root]->get_state(state_index, true);
  move_model2state_rec(root, *best_state);
  return best_state->get_total_score();
}
  */
void RestraintGraph::move_model2global_minimum() const
{
  std::stringstream err_msg;
  err_msg << "RestraintGraph::move_model2global_minimum the "
          << "graph has not been infered";
  IMP_assert(infered, err_msg.str());
  CombState *best_state = *(min_combs->begin());
  move_model2state_rec(root, *best_state);
}
CombState *RestraintGraph::get_minimum_comb() const
{
  std::stringstream err_msg;
  err_msg << "RestraintGraph::move_model2global_minimum the "
          << "graph has not been infered";
  IMP_assert(infered, err_msg.str());
  return *(min_combs->begin());
}
void RestraintGraph::clear() {
  for(std::map<Pair, JEdge *>::iterator it = edge_data.begin();
      it != edge_data.end();it++) {
    it->second->clear();
  }
  for(std::vector<JNode *>::iterator it = node_data.begin();
      it != node_data.end();it++) {
    (*it)->clear();
  }
  infered = false;
  min_combs->clear();
}
} // namespace domino

} // namespace IMP
