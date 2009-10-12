/**
 *  \file ConnectivityRestraint.cpp  \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/ConnectivityRestraint.h>

#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/log.h>
#include <IMP/PairScore.h>

#include <climits>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>

#include <limits>

IMPCORE_BEGIN_NAMESPACE

ConnectivityRestraint::ConnectivityRestraint(PairScore *ps):
  Restraint("ConnectivityRestraint %1%"),
  ps_(ps)
{
}

IMP_LIST_IMPL(ConnectivityRestraint, Particle, particle,Particle*,
              Particles,
              {
              IMP_INTERNAL_CHECK(get_number_of_particles()==0
                         || obj->get_model()
                          == (*particles_begin())->get_model(),
                         "All particles in Restraint must belong to the "
                         "same Model.");
              },,);


namespace {
  /*typedef boost::adjacency_list<boost::vecS, boost::vecS,
                        boost::undirectedS, boost::no_property,
                        boost::property<boost::edge_weight_t, double> > Graph;*/
  typedef boost::adjacency_matrix<boost::undirectedS, boost::no_property,
                        boost::property<boost::edge_weight_t, double> > Graph;
  typedef boost::graph_traits<Graph>::edge_descriptor Edge;
  typedef Graph::edge_property_type Weight;
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

  void compute_mst(const ConnectivityRestraint *a,
                   PairScore *ps,
                   Graph &g,
                   std::vector<Edge> &mst) {
    for (unsigned int i=0; i< a->get_number_of_particles(); ++i) {
      for (unsigned int j=0; j<i; ++j) {
        double d= ps->evaluate(a->get_particle(i), a->get_particle(j), NULL);
        IMP_LOG(VERBOSE, "ConnectivityRestraint edge between "
                << a->get_particle(i)->get_name() << " and "
                << a->get_particle(j)->get_name() << " with weight "
                << d << std::endl);
        /*Edge e =*/ boost::add_edge(i, j, Weight(d), g);
        //boost::put(boost::edge_weight_t(), g, e, d);
      }
    }
    mst.resize(a->get_number_of_particles()-1);
    boost::kruskal_minimum_spanning_tree(g, mst.begin());

    /*
    boost::property_map<Graph, boost::vertex_index_t>::type indexmap
      = get(boost::vertex_index, g);
    boost::prim_minimum_spanning_tree(g, indexmap);
    for (unsigned int i=0; i< a->get_number_of_particles(); ++i) {
      Vertex cv= boost::vertex(i,g);
      if (boost::get(indexmap, cv) !=  cv) {
        mst.push_back(boost::edge(cv, boost::get(indexmap, cv), g).first);
      }
      }*/
  }

}


double
ConnectivityRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(ps_.get());
  std::vector<Edge> mst;

  Graph g(get_number_of_particles());
  compute_mst(this, ps_, g, mst);
  double sum=0;
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


ParticlePairs ConnectivityRestraint::get_connected_pairs() const {
  IMP_CHECK_OBJECT(ps_.get());
  std::vector<Edge> mst;
  Graph g(get_number_of_particles());
  compute_mst(this, ps_, g, mst);
  ParticlePairs ret(mst.size());
  for (unsigned int index=0; index< mst.size(); ++index) {
    int i= boost::target(mst[index], g);
    int j= boost::source(mst[index], g);
    ret.set(index, ParticlePair(get_particle(i),
                                get_particle(j)));
  }
  return ret;
}

ParticlesList ConnectivityRestraint::get_interacting_particles() const {
  ParticlePairs pps= get_connected_pairs();
  ParticlesList pl(pps.size());
  for (unsigned int i=0; i< pps.size(); ++i) {
    pl[i]= get_union(ps_->get_interacting_particles(pps[i][0],
                                                    pps[i][1]));
  }
  return pl;
}

ParticlesTemp ConnectivityRestraint::get_used_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< get_number_of_particles(); ++i) {
    for (unsigned int j=0; j<i; ++j) {
      ParticlesTemp cs= ps_->get_used_particles(get_particle(i),
                                                get_particle(j));
      ret.insert(ret.end(), cs.begin(), cs.end());
      ret.push_back(get_particle(i));
      ret.push_back(get_particle(j));
    }
  }
  return ret;
}



void ConnectivityRestraint::show(std::ostream& out) const
{
  out << "connectivity restraint:" << std::endl;
  get_version_info().show(out);
}

IMPCORE_END_NAMESPACE
