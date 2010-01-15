/**
 *  \file ConnectivityRestraint.cpp  \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/ConnectivityRestraint.h>

#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/log.h>
#include <IMP/PairScore.h>
#include <IMP/internal/utility.h>
#include <IMP/core/ListSingletonContainer.h>

#include <climits>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>

#include <limits>

IMPCORE_BEGIN_NAMESPACE

ConnectivityRestraint::ConnectivityRestraint(PairScore *ps,
                                             SingletonContainer *sc):
  Restraint("ConnectivityRestraint %1%"),
  ps_(ps)
{
  if (sc) {
    sc_= sc;
  } else {
    sc_= new ListSingletonContainer();
  }
}

namespace {
  ListSingletonContainer* get_list(SingletonContainer *sc) {
    ListSingletonContainer *ret= dynamic_cast<ListSingletonContainer*>(sc);
    if (!ret) {
      IMP_THROW("Can only use the set and add methods when no container"
                << " was passed on construction of ConnectivityRestraint.",
                ValueException);
    }
    return ret;
  }
}

void ConnectivityRestraint::set_particles(const Particles &ps) {
  get_list(sc_)->set_particles(ps);
}

void ConnectivityRestraint::add_particles(const Particles &ps) {
  get_list(sc_)->add_particles(ps);
}

void ConnectivityRestraint::add_particle(Particle *ps) {
  get_list(sc_)->add_particle(ps);
}

namespace {
  /*typedef boost::adjacency_list<boost::vecS, boost::vecS,
                        boost::undirectedS, boost::no_property,
                        boost::property<boost::edge_weight_t, double> > Graph;*/
  typedef boost::adjacency_matrix<boost::undirectedS, boost::no_property,
                        boost::property<boost::edge_weight_t, double> > Graph;
  typedef boost::graph_traits<Graph>::edge_descriptor Edge;
  typedef Graph::edge_property_type Weight;
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

  void compute_mst(const SingletonContainer *a,
                   PairScore *ps,
                   Graph &g,
                   std::vector<Edge> &mst) {
    for (unsigned int i=0; i< a->get_number_of_particles(); ++i) {
      for (unsigned int j=0; j<i; ++j) {
        double d= ps->evaluate(ParticlePair(a->get_particle(i),
                                            a->get_particle(j)), NULL);
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

  Graph g(sc_->get_number_of_particles());
  compute_mst(sc_, ps_, g, mst);
  double sum=0;
  // could be more clever if accum is NULL
  for (unsigned int index=0; index< mst.size(); ++index) {
    int i= boost::target(mst[index], g);
    int j= boost::source(mst[index], g);
    IMP_LOG(VERBOSE, "ConnectivityRestraint edge between "
            << sc_->get_particle(i)->get_name()
            << " and " << sc_->get_particle(j)->get_name() << std::endl);
    if (accum) {
      sum+= ps_->evaluate(ParticlePair(sc_->get_particle(i),
                                       sc_->get_particle(j)),
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
  Graph g(sc_->get_number_of_particles());
  compute_mst(sc_, ps_, g, mst);
  ParticlePairs ret(mst.size());
  for (unsigned int index=0; index< mst.size(); ++index) {
    int i= boost::target(mst[index], g);
    int j= boost::source(mst[index], g);
    ret.set(index, ParticlePair(sc_->get_particle(i),
                                sc_->get_particle(j)));
  }
  return ret;
}

ParticlesList ConnectivityRestraint::get_interacting_particles() const {
  ParticlePairs pps= get_connected_pairs();
  ParticlesList pl(pps.size());
  for (unsigned int i=0; i< pps.size(); ++i) {
    pl[i]= IMP::internal::get_union(ps_->get_interacting_particles(pps[i]));
  }
  return pl;
}

ParticlesTemp ConnectivityRestraint::get_input_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
    for (unsigned int j=0; j<i; ++j) {
      ParticlesTemp cs
        = ps_->get_input_particles(ParticlePair(sc_->get_particle(i),
                                                sc_->get_particle(j)));
      ret.insert(ret.end(), cs.begin(), cs.end());
      ret.push_back(sc_->get_particle(i));
      ret.push_back(sc_->get_particle(j));
    }
  }
  return ret;
}

ContainersTemp ConnectivityRestraint::get_input_containers() const {
  ContainersTemp ret;
  for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
    for (unsigned int j=0; j<i; ++j) {
      ContainersTemp cs
        = ps_->get_input_containers(ParticlePair(sc_->get_particle(i),
                                                 sc_->get_particle(j)));
      ret.insert(ret.end(), cs.begin(), cs.end());
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
