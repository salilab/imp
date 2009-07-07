/**
 *  \file JunctionTree.cpp
 *  \brief Stores a junction tree
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
#include "IMP/domino/JunctionTree.h"
#include <fstream>
#include <IMP/log.h>
IMPDOMINO_BEGIN_NAMESPACE
void read_junction_tree(const std::string &filename, JunctionTree *jt) {
  std::ifstream jt_f (filename.c_str());
  if (! jt_f.is_open()) {
    std::cerr<<"problem opening junction tree file: "<< filename << std::endl;
    exit(0);
  }
  std::string line;
  int status=0; //0 - nodes header, 1 - nodes, 2 - edges header, 3 - edges
  int node_ind = 0;
  while (!jt_f.eof()) {
    getline (jt_f,line);
    std::vector<std::string> ls;
    boost::split(ls, line, boost::is_any_of(" "));
    if (status==0) {
      if (ls[0] == "nodes:") {
        status=1;
        jt->set_nodes(boost::lexical_cast<int>(ls[1]));
      }
      else {
        IMP_assert(true, "wrong junction tree format");
      }
    }
    else if (status == 1) {
      if (ls[0] == "edges:") {
        status=2;
      }
      else {
        for(std::vector<std::string>::iterator it =  ls.begin();
            it != ls.end(); it++) {
          boost::trim(*it);
          if (it->size() >0) {
            jt->set_node_name(node_ind,*it);
            IMP_LOG(VERBOSE,"adding component "<< *it <<
                            " to node " << node_ind << std::endl);
          }
        }
        node_ind = node_ind +1;
      }
    }
    else if (status == 2) {
      if (ls.size() == 2) {
        IMP_LOG(VERBOSE,"adding edge"<<std::endl);
        jt->add_edge(boost::lexical_cast<int>(ls[0]),
                   boost::lexical_cast<int>(ls[1]));
      }
      else {
        IMP_WARN("edge line was not parsed: ");
      }
    }
  }
  IMP_check(status==2,"wrong junction tree format",ValueException);
  jt_f.close();
}

//! Initialize a graph with N nodes
void JunctionTree::set_nodes(int number_of_nodes) {
  IMP_LOG(VERBOSE,"setting a graph with " << number_of_nodes << std::endl);
  g_=Graph(number_of_nodes);
  data_.clear();
  data_.insert(data_.end(),number_of_nodes,std::vector<std::string>());
}
void JunctionTree::show(std::ostream& out) const {
  out << "===========Junction Tree=========" << std::endl;
  out << " Nodes: " << std::endl;
  for (unsigned int vi = 0;vi < boost::num_vertices(g_);vi++) {
    for(unsigned int j=0;j<data_[vi].size();j++) {
      out<<data_[vi][j]<<" || ";
    }
    out<<std::endl;
  }
  out << " Edges: " << std::endl;
  boost::graph_traits<Graph>::edge_iterator ei_begin, ei_end, ei;
  tie(ei_begin, ei_end) = edges(g_);
  for (ei = ei_begin; ei != ei_end; ei++) {
    out << *ei << std::endl;
  }
  out << "==================================" << std::endl;
  out << "==================================" << std::endl;

}
IMPDOMINO_END_NAMESPACE
