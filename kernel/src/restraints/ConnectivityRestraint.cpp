/**
 *  \file ConnectivityRestraint.cpp  \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/restraints/ConnectivityRestraint.h"
#include "IMP/PairScore.h"

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <limits>

namespace IMP
{


ConnectivityRestraint::ConnectivityRestraint(PairScore *ps): ps_(ps)
{
  clear_sets();
}


ConnectivityRestraint::~ConnectivityRestraint()
{
}

struct Pair
{
  typedef Pair This;
  Pair(){p_[0]=-1; p_[1]=-1;}
  Pair(int a, int b){
    p_[0]= std::min(a,b);
    p_[1]= std::max(a,b);
  }
  int operator[](unsigned int i) const {
    return p_[i];
  }
  bool is_default() const {
    return p_[0] < 0 && p_[1] < 0;
  }
  IMP_COMPARISONS_2(p_[0], p_[1]);
  int p_[2];
};

void ConnectivityRestraint::add_set(const Particles &ps)
{
  IMP_check(!ps.empty(), "Cannot add empty set to ConnectivityRestraint",
            InvalidStateException("Empty set"));
  unsigned int sz= ps.size();
  Restraint::add_particles(ps);
  set_offsets_.push_back(set_offsets_.back()+sz);
}

void ConnectivityRestraint::clear_sets()
{
  set_offsets_.clear();
  Restraint::clear_particles();
  set_offsets_.push_back(0);
}

Float ConnectivityRestraint::evaluate(DerivativeAccumulator *accum)
{
  IMP_CHECK_OBJECT(ps_.get());
  unsigned int num_sets= set_offsets_.size()-1;

  typedef boost::adjacency_list<boost::vecS, boost::vecS,
    boost::undirectedS, boost::no_property,
    boost::property<boost::edge_weight_t, float> > Graph;
  typedef boost::graph_traits<Graph>::edge_descriptor Edge;
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
  Graph g(Restraint::number_of_particles());

  const float tag_weight= -std::numeric_limits<float>::max();
  for (unsigned int i=0; i< num_sets; ++i) {
    for (unsigned int ii= set_offsets_[i]+1; ii < set_offsets_[i+1]; ++ii) {
      Edge e = boost::add_edge(ii-1, ii, g).first;
      boost::put(boost::edge_weight_t(), g, e, tag_weight);
    }
    for (unsigned int j=0; j< i; ++j) {
      for (unsigned int ii= set_offsets_[i]; ii != set_offsets_[i+1]; ++ii) {
        for (unsigned int ij= set_offsets_[j]; ij != set_offsets_[j+1]; ++ij) {
          float d= ps_->evaluate(get_particle(ii), get_particle(ij), NULL);
          IMP_check(d != tag_weight, "ConnectivityRestraint uses the distance "
                    << tag_weight  << " as a marker, so the distance function "
                    << " should not return it. This can be fixed if you "
                    << " complain.",
                    ValueException("Tag weight matches distance in"\
                                   " ConnectivityRestraint"));
          IMP_LOG(VERBOSE, "ConnectivityRestraint edge between "
                  << get_particle(ii)->get_index() << " and "
                  << get_particle(ij)->get_index() << " with weight "
                  << d << std::endl);
          Edge e = boost::add_edge(ii, ij, g).first;
          boost::put(boost::edge_weight_t(), g, e, d);
        }
      }
    }
  }

  std::vector<Edge> mst;
  boost::kruskal_minimum_spanning_tree(g, std::back_inserter(mst));

  float sum=0;
  // could be more clever if accum is NULL
  for (unsigned int index=0; index< mst.size(); ++index) {
    if (boost::get(boost::edge_weight_t(), g, mst[index]) == tag_weight) {
      continue;
    }
    int i= boost::target(mst[index], g);
    int j= boost::source(mst[index], g);
    IMP_LOG(VERBOSE, "ConnectivityRestraint edge between "
            << get_particle(i)->get_index()
            << " and " << get_particle(j)->get_index() << std::endl);
    sum+= ps_->evaluate(get_particle(i),
                        get_particle(j),
                        accum);
  }
  return sum;
}


void ConnectivityRestraint::show(std::ostream& out) const
{
  if (get_is_active()) {
    out << "connectivity restraint (active):" << std::endl;
  } else {
    out << "connectivity restraint (inactive):" << std::endl;
  }

  get_version_info().show(out);
}

}  // namespace IMP
