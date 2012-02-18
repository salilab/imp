/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include "IMP/RestraintSet.h"
#include "IMP/internal/graph_utility.h"
#include "IMP/file.h"
#include "IMP/compatibility/map.h"
#include "IMP/dependency_graph.h"
#include <boost/timer.hpp>
#include <IMP/utility.h>
#include "IMP/compatibility/set.h"
#include <numeric>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/visitors.hpp>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/dynamic_bitset.hpp>
//#include <boost/graph/lookup_edge.hpp>
#include <IMP/compatibility/vector_property_map.h>

IMP_BEGIN_NAMESPACE

EvaluationCache::EvaluationCache(const RestraintsTemp &rs){
  RestraintsTemp rrs;
  boost::tie(rrs, weights_)= get_restraints_and_weights(rs.begin(),
                                                        rs.end());
  rs_=get_as<Restraints>(rrs);
  if (!rs.empty()) {
    Model *m= rs[0]->get_model();
    IMP_USAGE_CHECK(m, "Restraints must be registered with the model.");
    ss_=get_as<ScoreStates>(m->get_score_states(rrs));
  }
}

EvaluationCache::EvaluationCache(const RestraintsTemp &rs,
                                 double w){
  RestraintsTemp rrs;
  boost::tie(rrs, weights_)= get_restraints_and_weights(rs.begin(),
                                                        rs.end(), w);
  rs_=get_as<Restraints>(rrs);
  if (!rs.empty()) {
    Model *m= rs[0]->get_model();
    IMP_USAGE_CHECK(m, "Restraints must be registered with the model.");
    ss_=get_as<ScoreStates>(m->get_score_states(rrs));
  }
}

EvaluationCache::EvaluationCache(const ScoreStatesTemp &ss,
                                 const RestraintsTemp &rs,
                                 const Floats &floats):
    ss_(ss.begin(), ss.end()),
    rs_(rs.begin(), rs.end()), weights_(floats) {
}

EvaluationCache Model::get_evaluation_cache() const {
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  Floats weights(scoring_restraints_.size());
  for (unsigned int i=0; i< scoring_restraints_.size(); ++i) {
    weights[i]=scoring_restraints_[i]->model_weight_;
  }
  return EvaluationCache(ordered_score_states_,
                         scoring_restraints_,
                         weights);
}

  //#pragma GCC diagnostic warn "-Wunused-parameter"
namespace {
  compatibility::map<Object*, int> get_index(const DependencyGraph &dg) {
    compatibility::map<Object*, int> ret;
    MDGConstVertexMap om= boost::get(boost::vertex_name, dg);
    for (std::pair<MDGTraits::vertex_iterator,
           MDGTraits::vertex_iterator> be= boost::vertices(dg);
         be.first != be.second; ++be.first) {
      Object *o= om[*be.first];
      ret[o]= *be.first;
    }
    return ret;
  }
}
class ScoreDependencies: public boost::default_dfs_visitor {
  Ints &bs_;
  const compatibility::map<Object*, int> &ssindex_;
  MDGConstVertexMap vm_;
public:
  ScoreDependencies(Ints &bs,
                    const compatibility::map<Object*, int> &ssindex,
                    MDGConstVertexMap vm): bs_(bs), ssindex_(ssindex),
                                          vm_(vm) {}
  template <class G>
  void discover_vertex(MDGTraits::vertex_descriptor u,
                       const G&) {
    Object *o= vm_[u];
    //std::cout << "visiting " << o->get_name() << std::endl;
    compatibility::map<Object*, int>::const_iterator it= ssindex_.find(o);
    if (it != ssindex_.end()) {
      //std::cout << "setting " << it->second << std::endl;
      bs_.push_back(it->second);
    } else {
      IMP_INTERNAL_CHECK(!dynamic_cast<ScoreState*>(o),
                         "Score state jot in index");
    }
  }
};

namespace {
  void
  compute_restraint_dependencies(const DependencyGraph &dg,
                                 const RestraintsTemp &ordered_restraints,
                                 const ScoreStatesTemp &ordered_score_states) {
    compatibility::map<Object *, int> ssindex;
    for (unsigned int i=0; i < ordered_score_states.size(); ++i) {
      ssindex[ordered_score_states[i]]=i;
    }
    compatibility::map<Object*, int> index= get_index(dg);
    MDGConstVertexMap om= boost::get(boost::vertex_name, dg);
    for (unsigned int i=0; i< ordered_restraints.size(); ++i) {
      // make sure it is in the loop so it gets reset
      boost::vector_property_map<int> color(boost::num_vertices(dg));
      /*std::cout << "Finding dependencies for "
        << ordered_restraints[i]->get_name()
        << std::endl;*/
      IMP_USAGE_CHECK(static_cast<Object*>(ordered_restraints[i])
                      ==
     om[index.find(static_cast<Object*>(ordered_restraints[i]))->second],
                      "Restraints and vertices don't match");
      Ints cur;
      boost::depth_first_visit(boost::make_reverse_graph(dg),
                               index.find(ordered_restraints[i])->second,
                               ScoreDependencies(cur, ssindex, om),
                               color);
      std::sort(cur.begin(), cur.end());
      cur.erase(std::unique(cur.begin(), cur.end()), cur.end());
      ordered_restraints[i]->model_dependencies_=cur;
    }
  }
}


void Model::reset_dependencies() {
  scoring_restraints_.clear();
  ordered_score_states_.clear();
  first_call_=true;
  has_dependencies_=false;
}

void Model::compute_dependencies() const {
  IMP_OBJECT_LOG;
  IMP_LOG(VERBOSE, "Ordering score states. Input list is: ");
  Floats weights;
  // kind of a hack, make sure it is overwritten for ones that are part of the
  // model
  RestraintsTemp all_restraints(tracked_restraints_.begin(),
                                tracked_restraints_.end());
  for (unsigned int i=0; i< all_restraints.size(); ++i) {
    all_restraints[i]->model_weight_= 1;
  }
  boost::tie(scoring_restraints_,
             weights)
    = get_restraints_and_weights(restraints_begin(),
                                 restraints_end());
  for (unsigned int i=0; i< scoring_restraints_.size(); ++i) {
    scoring_restraints_[i]->model_weight_= weights[i];
  }
  ScoreStates score_states= access_score_states();
  IMP_LOG(VERBOSE, "Making dependency graph on " << weights.size()
          << " restraints " << score_states.size() << " score states "
          << " and " << get_number_of_particles()
          << " particles." << std::endl);
  DependencyGraph dg
    = get_dependency_graph(get_as<ScoreStatesTemp>(score_states),
                           get_as<RestraintsTemp>(all_restraints));
  //internal::show_as_graphviz(boost::make_reverse_graph(dg), std::cout);
  ordered_score_states_= IMP::get_ordered_score_states(dg);
  compute_restraint_dependencies(dg, all_restraints,
                                 ordered_score_states_);
  IMP_LOG(VERBOSE, "Ordered score states are "
          << ordered_score_states_ << std::endl);
  has_dependencies_=true;
}


ScoreStatesTemp
Model::get_score_states(const RestraintsTemp &restraints) const {
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i= 0; i< restraints.size(); ++i) {
      IMP_CHECK_CODE(Restraint *r=restraints[i]);
      IMP_USAGE_CHECK(!dynamic_cast<RestraintSet*>(r),
                      "Cannot pass restraint sets to get_score_states()");
    }
  }
  Ints bs;
  for (unsigned int i=0; i< restraints.size(); ++i) {
    IMP_USAGE_CHECK(restraints[i]->get_is_part_of_model(),
                    "Restraint must be added to model: "
                    << restraints[i]->get_name());
    // weight 0
    if (restraints[i]->model_weight_==0) {
      IMP_LOG(VERBOSE, "Restraint " << restraints[i]->get_name()
              << " has weight 0" << std::endl);
    }
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE, restraints[i]->get_name() << " depends on ");
      for (unsigned int j=0; j< restraints[i]->model_dependencies_.size();
           ++j) {
        IMP_LOG(VERBOSE,
                ordered_score_states_[restraints[i]->model_dependencies_[j]]
                ->get_name()
                << " ");
      }
      IMP_LOG(VERBOSE, std::endl);
    }
    bs.insert(bs.end(), restraints[i]->model_dependencies_.begin(),
              restraints[i]->model_dependencies_.end());
  }
  std::sort(bs.begin(), bs.end());
  bs.erase(std::unique(bs.begin(), bs.end()), bs.end());
  ScoreStatesTemp ss(bs.size());
  for (unsigned int i=0; i< bs.size(); ++i) {
    ss[i]=ordered_score_states_[bs[i]];
  }
  return ss;
}


ScoreStatesTemp get_required_score_states(const RestraintsTemp &irs) {
  if (irs.empty()) return ScoreStatesTemp();
  RestraintsTemp rs= get_restraints(irs.begin(),
                                    irs.end());
  if (rs.empty()) return ScoreStatesTemp();
  return rs[0]->get_model()->get_score_states(rs);
}



void Model::add_dependency_edge(ScoreState *from, ScoreState *to) {
  extra_edges_.push_back(std::pair<Object*, Object*>(from, to));
}

IMP_END_NAMESPACE
