/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
#include "IMP/internal/evaluate_utility.h"
#include "IMP/ScoringFunction.h"
#include "IMP/ScoreState.h"
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

class ScoreDependencies: public boost::default_dfs_visitor {
  Ints &bs_;
  const compatibility::map<base::Object*, int> &ssindex_;
  DependencyGraphConstVertexName vm_;
public:
  ScoreDependencies(Ints &bs,
                    const compatibility::map<base::Object*, int> &ssindex,
                    DependencyGraphConstVertexName vm): bs_(bs),
                                                        ssindex_(ssindex),
                                                        vm_(vm) {}
  template <class G>
  void discover_vertex(DependencyGraphTraits::vertex_descriptor u,
                       const G&) {
    base::Object *o= vm_[u];
    compatibility::map<base::Object*, int>::const_iterator it= ssindex_.find(o);
    if (it != ssindex_.end()) {
      bs_.push_back(it->second);
    } else {
      IMP_INTERNAL_CHECK(!dynamic_cast<ScoreState*>(o),
                         "Score state jot in index");
    }
  }
};

void Model::clear_caches() {
  IMP_USAGE_CHECK(cur_stage_== internal::NOT_EVALUATING,
                  "The dependencies cannot be reset during evaluation or"
                  << " dependency computation.");
  if (!dependencies_dirty_) {
    IMP_LOG(WARNING, "Reseting dependencies" << std::endl);
  }
  ordered_score_states_.clear();
  first_call_=true;
  dependencies_dirty_=true;
}

void Model::compute_dependencies() {
  IMP_OBJECT_LOG;
  IMP_USAGE_CHECK(!get_has_dependencies(),
                  "Already has dependencies when asked to compute them.");
  internal::SFSetIt<IMP::internal::Stage>
    reset(&cur_stage_, internal::COMPUTING_DEPENDENCIES);
  IMP_LOG(TERSE, "Computing restraint dependencies" << std::endl);
  IMP_LOG(VERBOSE, "Reason is " << dependencies_dirty_ << " "
          << ModelObjectTracker::get_changed_description()
          << std::endl);
  DependencyGraph dg
    = get_dependency_graph(this);
  // attempt to get around boost/gcc bug and the most vexing parse
  DependencyGraphVertexIndex index((IMP::get_vertex_index(dg)));
  //internal::show_as_graphviz(boost::make_reverse_graph(dg), std::cout);
  set_score_state_update_order(dg, index);
  // to prevent infinite recursion when updating ScoringFunctions
  dependencies_dirty_=false;
  ModelObjectTracker::set_is_dirty(false);
  IMP_INTERNAL_CHECK(!ModelObjectTracker::get_is_dirty(),
                     "Cleaning the tracked list did not make it clean");
  IMP_LOG(TERSE, "Score states are " << get_score_states() << std::endl);
  // must go after dependencies dirty to avoid recursion
  ScoreStatesTemp score_states;
  DependencyGraphVertexName name= boost::get(boost::vertex_name, dg);
  for (unsigned int i=0; i< boost::num_vertices(dg); ++i) {
    ModelObject *mo=name[i];
    if (dynamic_cast<ScoreState*>(mo)) {
      score_states.push_back(dynamic_cast<ScoreState*>(mo));
    }
  }
  ordered_score_states_=get_update_order(score_states);

  for (ModelObjectTracker::TrackedIterator it
           = ModelObjectTracker::tracked_begin();
       it != ModelObjectTracker::tracked_end(); ++it) {
    ModelObject *sf= *it;
    IMP_CHECK_OBJECT(sf);
    sf->update_dependencies(dg, index);
  }
  IMP_INTERNAL_CHECK(get_has_dependencies(),
                     "Computing the dependencies did not result in the "
                     << "model having them");
}



ModelObjectsTemp Model::get_optimized_particles() const {
  compatibility::set<ModelObject*> ret;
  FloatIndexes fix= internal::FloatAttributeTable::get_optimized_attributes();
  for (unsigned int i=0; i< fix.size(); ++i) {
    ret.insert(get_particle(fix[i].get_particle()));
  }
  return ModelObjectsTemp(ret.begin(), ret.end());
}


IMP_END_NAMESPACE
