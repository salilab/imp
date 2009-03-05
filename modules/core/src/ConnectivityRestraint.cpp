/**
 *  \file ConnectivityRestraint.cpp  \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/ConnectivityRestraint.h>

#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/log.h>
#include <IMP/PairScore.h>

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

#include <limits>

IMPCORE_BEGIN_NAMESPACE

ConnectivityRestraint::ConnectivityRestraint(PairScore *ps): ps_(ps)
{
}

IMP_LIST_IMPL(ConnectivityRestraint, Particle, particle,Particle*,  {
              IMP_assert(get_number_of_particles()==0
                         || obj->get_model()
                          == (*particles_begin())->get_model(),
                         "All particles in Restraint must belong to the "
                         "same Model.");
              },,);


Float ConnectivityRestraint::evaluate(DerivativeAccumulator *accum)
{
  IMP_CHECK_OBJECT(ps_.get());

  typedef boost::adjacency_list<boost::vecS, boost::vecS,
    boost::undirectedS, boost::no_property,
    boost::property<boost::edge_weight_t, float> > Graph;
  typedef boost::graph_traits<Graph>::edge_descriptor Edge;
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
  Graph g(get_number_of_particles());

  for (unsigned int i=0; i< get_number_of_particles(); ++i) {
    for (unsigned int j=0; j<i; ++j) {
      float d= ps_->evaluate(get_particle(i), get_particle(j), NULL);
      IMP_LOG(VERBOSE, "ConnectivityRestraint edge between "
              << get_particle(i)->get_name() << " and "
              << get_particle(j)->get_name() << " with weight "
              << d << std::endl);
      Edge e = boost::add_edge(i, j, g).first;
      boost::put(boost::edge_weight_t(), g, e, d);
    }
  }

  std::vector<Edge> mst;
  boost::kruskal_minimum_spanning_tree(g, std::back_inserter(mst));

  float sum=0;
  // could be more clever if accum is NULL
  for (unsigned int index=0; index< mst.size(); ++index) {
    int i= boost::target(mst[index], g);
    int j= boost::source(mst[index], g);
    IMP_LOG(VERBOSE, "ConnectivityRestraint edge between "
            << get_particle(i)->get_name()
            << " and " << get_particle(j)->get_name() << std::endl);
    if (accum) {
      sum+= ps_->evaluate(get_particle(i),
                          get_particle(j),
                          accum);
    } else {
      sum += boost::get(boost::edge_weight_t(), g, mst[index]);
    }
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

IMPCORE_END_NAMESPACE
