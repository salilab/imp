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
#include "IMP/ScoringFunction.h"
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
  dependencies_dirty_=true;
}

void Model::compute_dependencies() {
  IMP_OBJECT_LOG;
  IMP_LOG(VERBOSE, "Ordering score states. Input list is: ");
  RestraintsTemp all_restraints
      = RestraintsTemp(RestraintTracker::tracked_begin(),
                       RestraintTracker::tracked_end());
  ScoreStates score_states= access_score_states();
  IMP_LOG(VERBOSE, "Making dependency graph on " << all_restraints.size()
          << " restraints " << score_states.size() << " score states "
          << " and " << get_number_of_particles()
          << " particles." << std::endl);
  DependencyGraph dg
    = get_dependency_graph(get_as<ScoreStatesTemp>(score_states),
                           get_as<RestraintsTemp>(all_restraints));
  //internal::show_as_graphviz(boost::make_reverse_graph(dg), std::cout);
  ordered_score_states_=IMP::get_ordered_score_states(dg);
  compute_restraint_dependencies(dg, all_restraints,
                               get_as<ScoreStatesTemp>(ordered_score_states_));
  IMP_LOG(VERBOSE, "Ordered score states are "
          << ordered_score_states_ << std::endl);
  // to prevent infinite recursion when updating ScoringFunctions
  dependencies_dirty_=false;
  RestraintTracker::set_is_dirty(false);
  ScoringFunctionTracker::set_is_dirty(false);

  for (ScoringFunctionTracker::TrackedIterator it
           = ScoringFunctionTracker::tracked_begin();
       it != ScoringFunctionTracker::tracked_end(); ++it) {
    ScoringFunction *sf= *it;
    IMP_CHECK_OBJECT(sf);
    sf->ss_= get_score_states(sf->get_restraints());
  }
}


ScoreStatesTemp
Model::get_score_states(const RestraintsTemp &restraints) {
  if (!get_has_dependencies()) {
    compute_dependencies();
  }
  Ints bs;
  for (unsigned int i=0; i< restraints.size(); ++i) {
    IMP_USAGE_CHECK(restraints[i]->get_model(),
                    "Restraint must have the model set: "
                    << restraints[i]->get_name());
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


IMP_END_NAMESPACE
