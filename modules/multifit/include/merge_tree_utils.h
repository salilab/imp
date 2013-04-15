/**
 *  \file merge_tree_utils.h
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPMULTIFIT_MERGE_TREE_UTILS_H
#define IMPMULTIFIT_MERGE_TREE_UTILS_H

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/graph/graph_utility.hpp>
#include <IMP/multifit/multifit_config.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/base/map.h>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>

IMPMULTIFIT_BEGIN_NAMESPACE

namespace MTU {
  typedef boost::adjacency_matrix<boost::undirectedS, boost::no_property,
                        boost::property<boost::edge_weight_t, double>
                                > DependencyGraph;
  typedef boost::graph_traits<DependencyGraph>::edge_descriptor DGEdge;
  typedef DependencyGraph::edge_property_type DGWeight;
  typedef boost::graph_traits<DependencyGraph>::vertex_descriptor DGVertex;
  typedef base::map<Particle *, DGVertex> PVMAP;
  typedef base::map<DGVertex,Particle *> VPMAP;
};

class IMPMULTIFITEXPORT DummyRestraint : public Restraint {
public:
  DummyRestraint(Particle *a,Particle *b) : p1_(a),p2_(b){}
  IMP_RESTRAINT(DummyRestraint);
protected:
  Particle *p1_,*p2_;
};

class IMPMULTIFITEXPORT MergeTreeBuilder {
public:
  MergeTreeBuilder(const atom::Hierarchies &mhs) : g_(mhs.size()),
                                                   mhs_(mhs) {
    //add mapping from mol to node

    typedef boost::graph_traits<MTU::DependencyGraph>::vertex_iterator
          VertexIterator;
    VertexIterator v_it, v_it_end;
    std::tie(v_it, v_it_end) = boost::vertices(g_);
    int ind=0;
    for (; v_it != v_it_end; ++v_it) {
      mol2node_[mhs_[ind]]=*v_it;
      node2mol_[*v_it]=mhs_[ind];
      ind=ind+1;
    }
  }//end constructor

  //add +1 to the edge weight, creates the edge if does not exist
  void increase_edge(atom::Hierarchy mh1,atom::Hierarchy mh2) {
    //do not add self edges
    if (mh1.get_particle()==mh2.get_particle())
      return;
    //get the corresponding nodes
    MTU::DGVertex u,v;
    u=mol2node_[mh1.get_particle()];
    v=mol2node_[mh2.get_particle()];
    if (!boost::edge(u,v,g_).second) {//edge does not exist
      boost::add_edge(u,v,MTU::DGWeight(0.),g_);
    }
    //increase edge by one
    MTU::DGEdge e;
    e=boost::edge(u,v,g_).first;
    boost::put(boost::edge_weight_t(),g_,e,
               boost::get(boost::edge_weight_t(),g_,e)-1);
  }
  void show(std::ostream& out=std::cout) const {
    out << "vertices:";
    typedef boost::graph_traits<MTU::DependencyGraph>::vertex_iterator
           vertex_iter;
    std::pair<vertex_iter, vertex_iter> vp;
    for (vp = vertices(g_); vp.first != vp.second; ++vp.first) {
      out << node2mol_.find(*vp.first)->second->get_name() <<  " ";
    }
    out << std::endl;
    out << "edges:";
    boost::graph_traits<MTU::DependencyGraph>::edge_iterator ei, ei_end;
    for (std::tie(ei, ei_end) = edges(g_); ei != ei_end; ++ei)
      out << "(" << node2mol_.find(source(*ei, g_))->second->get_name()
          << "," << node2mol_.find(target(*ei, g_))->second->get_name()
          << ","<<boost::get(boost::edge_weight_t(),g_,*ei)<<")"<<std::endl;
    out << std::endl;
  }
  ParticlePairsTemp get_mst_dependency() const {
    std::vector<MTU::DGEdge> mst;
    boost::kruskal_minimum_spanning_tree(g_, std::back_inserter(mst));
    //go over the edges and get the pairs
    ParticlePairsTemp ret;
    for(int i=0;i<(int)mst.size();i++) {
      ParticlePair pp;
      pp[0]=node2mol_.find(boost::source(mst[i],g_))->second;
      pp[1]=node2mol_.find(boost::target(mst[i],g_))->second;
      ret.push_back(pp);
    }
    return ret;
  }
protected:
  //the graph
  MTU::DependencyGraph g_;
  atom::Hierarchies mhs_;
  MTU::PVMAP mol2node_;
  MTU::VPMAP node2mol_;
};

IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_MERGE_TREE_UTILS_H */
