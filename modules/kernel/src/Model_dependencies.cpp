/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/Model.h"
#include "IMP/kernel/Particle.h"
#include "IMP/base/log.h"
#include "IMP/kernel/Restraint.h"
#include "IMP/kernel/DerivativeAccumulator.h"
#include "IMP/kernel/ScoreState.h"
#include "IMP/kernel/RestraintSet.h"
#include "IMP/kernel/internal/graph_utility.h"
#include "IMP/kernel/file.h"
#include "IMP/base//map.h"
#include "IMP/kernel/dependency_graph.h"
#include "IMP/kernel/internal/evaluate_utility.h"
#include "IMP/kernel/ScoringFunction.h"
#include "IMP/kernel/ScoreState.h"
#include <boost/timer.hpp>
#include <IMP/kernel/utility.h>
#include "IMP/base//set.h"
#include <numeric>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/visitors.hpp>
IMP_GCC_PRAGMA(diagnostic ignored "-Wunused-parameter")
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/dynamic_bitset.hpp>
//#include <boost/graph/lookup_edge.hpp>
#include <IMP/base/vector_property_map.h>

IMPKERNEL_BEGIN_NAMESPACE

class ScoreDependencies : public boost::default_dfs_visitor {
  Ints &bs_;
  const base::map<base::Object *, int> &ssindex_;
  DependencyGraphConstVertexName vm_;

 public:
  ScoreDependencies(Ints &bs, const base::map<base::Object *, int> &ssindex,
                    DependencyGraphConstVertexName vm)
      : bs_(bs), ssindex_(ssindex), vm_(vm) {}
  template <class G>
  void discover_vertex(DependencyGraphTraits::vertex_descriptor u, const G &) {
    base::Object *o = vm_[u];
    base::map<base::Object *, int>::const_iterator it = ssindex_.find(o);
    if (it != ssindex_.end()) {
      bs_.push_back(it->second);
    } else {
      IMP_INTERNAL_CHECK(!dynamic_cast<ScoreState *>(o),
                         "Score state jot in index");
    }
  }
};

const DependencyGraph &Model::get_dependency_graph() {
  set_has_dependencies(true);
  return dependency_graph_;
}
const DependencyGraphVertexIndex &Model::get_dependency_graph_vertex_index() {
  set_has_dependencies(true);
  return dependency_graph_index_;
}

void Model::compute_required_score_states() {
  base::Vector<int> upstream_counts(boost::num_vertices(dependency_graph_));
  // initialize upstream counts
  typedef DependencyGraphTraits::in_edge_iterator IIT;
  typedef DependencyGraphTraits::out_edge_iterator OIT;
  for (unsigned int i = 0; i < upstream_counts.size(); ++i) {
    std::pair<IIT, IIT> be = boost::in_edges(i, dependency_graph_);
    upstream_counts[i] = std::distance(be.first, be.second);
  }
  // find all with counts of 0, add to queue
  Ints queue;
  for (unsigned int i = 0; i < boost::num_vertices(dependency_graph_); ++i) {
    if (upstream_counts[i] == 0) {
      queue.push_back(i);
    }
  }
  DependencyGraphVertexName vm =
      boost::get(boost::vertex_name, dependency_graph_);
  // while queue is not empty, pop one
  while (!queue.empty()) {
    int cur = queue.back();
    queue.pop_back();
    // update it from its neighbors
    ScoreStatesTemp required;
    for (std::pair<IIT, IIT> be = boost::in_edges(cur, dependency_graph_);
         be.first != be.second; ++be.first) {
      int source = boost::source(*be.first, dependency_graph_);
      required += vm[source]->get_required_score_states();
      ScoreState *ss = dynamic_cast<ScoreState *>(vm[source]);
      if (ss) {
        IMP_LOG_VERBOSE(ss->get_name() << " is a score state" << std::endl);
        required.push_back(ss);
      } else {
        IMP_LOG_VERBOSE(vm[source]->get_name() << " is not a score state"
                                               << std::endl);
      }
    }
    std::sort(required.begin(), required.end());
    required.erase(std::unique(required.begin(), required.end()),
                   required.end());
    IMP_LOG_VERBOSE("Required states for " << *vm[cur] << " are " << required
                                           << std::endl);

    vm[cur]->set_has_dependencies(true, get_update_order(required));

    // update its downstream neighbors
    for (std::pair<OIT, OIT> be = boost::out_edges(cur, dependency_graph_);
         be.first != be.second; ++be.first) {
      int target = boost::target(*be.first, dependency_graph_);
      --upstream_counts[target];
      if (upstream_counts[target] == 0) {
        queue.push_back(target);
      }
    }
  }
}

void Model::compute_dependencies() {
  IMP_OBJECT_LOG;
  IMP_LOG_TERSE("Computing dependencies." << std::endl);
  internal::SFSetIt<IMP::kernel::internal::Stage> reset(
      &cur_stage_, internal::COMPUTING_DEPENDENCIES);
  dependency_graph_ = IMP::kernel::get_dependency_graph(this);
  // attempt to get around boost/gcc bug and the most vexing parse
  dependency_graph_index_ = IMP::kernel::get_vertex_index(dependency_graph_);
  // internal::show_as_graphviz(boost::make_reverse_graph(dg), std::cout);
  set_score_state_update_order(dependency_graph_, dependency_graph_index_);
  IMP_LOG_TERSE("Score states are " << get_score_states() << std::endl);
  // must go after dependencies dirty to avoid recursion
  ScoreStatesTemp score_states;
  DependencyGraphVertexName name =
      boost::get(boost::vertex_name, dependency_graph_);
  for (unsigned int i = 0; i < boost::num_vertices(dependency_graph_); ++i) {
    ModelObject *mo = name[i];
    if (dynamic_cast<ScoreState *>(mo)) {
      score_states.push_back(dynamic_cast<ScoreState *>(mo));
    }
  }

  has_dependencies_ = true;

  ordered_score_states_ = get_update_order(score_states);

  compute_required_score_states();
}

ModelObjectsTemp Model::get_optimized_particles() const {
  base::set<ModelObject *> ret;
  FloatIndexes fix = internal::FloatAttributeTable::get_optimized_attributes();
  for (unsigned int i = 0; i < fix.size(); ++i) {
    ret.insert(get_particle(fix[i].get_particle()));
  }
  return ModelObjectsTemp(ret.begin(), ret.end());
}

void Model::set_has_dependencies(bool tf) {
  if (tf == get_has_dependencies()) return;
  if (tf) {
    has_dependencies_ = true;
    compute_dependencies();
  } else {
    IMP_LOG_TERSE("Resetting dependencies" << std::endl);
    IMP_USAGE_CHECK(cur_stage_ == internal::NOT_EVALUATING,
                    "The dependencies cannot be reset during evaluation or"
                        << " dependency computation.");
    has_dependencies_ = false;
    for (ModelObjectTracker::TrackedIterator it =
             ModelObjectTracker::tracked_begin();
         it != ModelObjectTracker::tracked_end(); ++it) {
      ModelObject *sf = *it;
      sf->set_has_dependencies(false);
    }
  }
}

IMPKERNEL_END_NAMESPACE
