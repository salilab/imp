/**
 *  \file ConnectivityRestraint.cpp  \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/ConnectivityRestraint.h>

#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/log.h>
#include <IMP/PairScore.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/internal/InternalListSingletonContainer.h>

#include <climits>

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>

#include <limits>

IMPCORE_BEGIN_NAMESPACE

ConnectivityRestraint::ConnectivityRestraint(PairScore *ps,
                                             SingletonContainer*sc):
  Restraint(sc->get_model(), "ConnectivityRestraint %1%"),
  ps_(ps)
{
  sc_= sc;
}


ConnectivityRestraint::ConnectivityRestraint(PairScore *ps):
    Restraint("ConnectivityRestraint %1%"),
  ps_(ps)
{
}

namespace {
  IMP::internal::InternalListSingletonContainer*
  get_list(SingletonContainer *sc) {
    IMP::internal::InternalListSingletonContainer *ret
      = dynamic_cast<IMP::internal::InternalListSingletonContainer*>(sc);
    if (!ret) {
      IMP_THROW("Can only use the set and add methods when no container"
                << " was passed on construction of ConnectivityRestraint.",
                base::ValueException);
    }
    return ret;
  }
}

void ConnectivityRestraint::set_particles(const ParticlesTemp &ps) {
  if (!sc_ && !ps.empty()) {
    sc_= new IMP::internal::InternalListSingletonContainer(ps[0]->get_model(),
                                                  "connectivity list");
  }
  get_list(sc_)->set(IMP::internal::get_index(ps));
}

void ConnectivityRestraint::add_particles(const ParticlesTemp &ps) {
  if (!sc_&& !ps.empty()) {
    sc_= new IMP::internal::InternalListSingletonContainer(ps[0]->get_model(),
                                                  "connectivity list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
}

void ConnectivityRestraint::add_particle(Particle *ps) {
  if (!sc_) {
    sc_= new IMP::internal::InternalListSingletonContainer(ps->get_model(),
                                                  "connectivity list");
  }
  get_list(sc_)->add(IMP::internal::get_index(ps));
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

  void compute_mst(Model *m,
                   const ParticleIndexes &pis,
                   PairScore *ps,
                   Graph &g,
                   base::Vector<Edge> &mst) {
    try {
      for (unsigned int i=0; i< pis.size(); ++i) {
        for (unsigned int j=0; j<i; ++j) {
          double d= ps->evaluate_index(m,
                                       ParticleIndexPair(pis[i],
                                                         pis[j]), nullptr);
          IMP_LOG_VERBOSE( "ConnectivityRestraint edge between "
                  << ParticleIndexPair(pis[i],
                                       pis[j]) << " with weight "
                  << d << std::endl);
          /*Edge e =*/ boost::add_edge(i, j, Weight(d), g);
          //boost::put(boost::edge_weight_t(), g, e, d);
        }
      }
      mst.resize(pis.size()-1);
    } catch (std::bad_alloc&) {
      IMP_FAILURE("Out of memory in ConnectivityRestraint.");
    }
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

  ParticleIndexPairs get_edges(const SingletonContainer *a,
                              PairScore *ps) {
    ParticleIndexes pis= a->get_indexes();
    Graph g(pis.size());
    base::Vector<Edge> mst;
    compute_mst(a->get_model(), pis, ps, g, mst);
    ParticleIndexPairs ret(mst.size());
    for (unsigned int index=0; index< mst.size(); ++index) {
      int i= boost::target(mst[index], g);
      int j= boost::source(mst[index], g);
      IMP_LOG_VERBOSE( "ConnectivityRestraint edge between "
            << pis[i]
            << " and " << pis[j] << std::endl);
      ret[index]= ParticleIndexPair(pis[i], pis[j]);
    }
    return ret;
  }
}


double
ConnectivityRestraint::unprotected_evaluate(DerivativeAccumulator *accum) const
{
  IMP_CHECK_OBJECT(ps_.get());
  IMP_OBJECT_LOG;
  base::Vector<Edge> mst;
  if (!sc_) return 0;
  ParticleIndexPairs edges= get_edges(sc_, ps_);
  return ps_->evaluate_indexes(get_model(),
                               edges,
                               accum, 0, edges.size());
}


Restraints ConnectivityRestraint::do_create_current_decomposition() const {
  ParticlePairsTemp pp= get_connected_pairs();
  Restraints ret(pp.size());
  for (unsigned int i=0; i< pp.size(); ++i) {
    IMP_NEW(PairRestraint, pr, (ps_, pp[i]));
    std::ostringstream oss;
    oss << get_name() << " " << i;
    pr->set_name(oss.str());
    ret[i]=pr;
    ret[i]->set_model(get_model());
  }
  return ret;
}


ParticlePairsTemp ConnectivityRestraint::get_connected_pairs() const {
  IMP_CHECK_OBJECT(ps_.get());
  ParticleIndexPairs edges= get_edges(sc_, ps_);
  return IMP::internal::get_particle(get_model(), edges);
}

ModelObjectsTemp ConnectivityRestraint::do_get_inputs() const {
  if (!sc_) return ModelObjectsTemp();
  ModelObjectsTemp ret;
  ret+=ps_->get_inputs(get_model(),
                       sc_->get_all_possible_indexes());
  ret.push_back(sc_);
  return ret;
}

IMPCORE_END_NAMESPACE
