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
#include <boost/foreach.hpp>
#include <IMP/kernel/utility.h>
#include "IMP/base/set.h"

IMPKERNEL_BEGIN_NAMESPACE


ModelObjectsTemp Model::get_optimized_particles() const {
  base::set<ModelObject *> ret;
  FloatIndexes fix = internal::FloatAttributeTable::get_optimized_attributes();
  for (unsigned int i = 0; i < fix.size(); ++i) {
    ret.insert(get_particle(fix[i].get_particle()));
  }
  return ModelObjectsTemp(ret.begin(), ret.end());
}

ScoreStatesTemp Model::get_ancestor_score_states(const ModelObject *mo) const {
  ModelObjectsTemp all_in = get_dependency_graph_inputs(mo);
  ScoreStatesTemp ret;
  for (unsigned int i = 0; i< all_in.size(); ++i) {
    ScoreState *ss = dynamic_cast<ScoreState*>(all_in[i].get());
    if (ss) ret.push_back(ss);
      ret += get_ancestor_score_states(all_in[i]);
  }
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

ScoreStatesTemp
Model::get_descendent_score_states(const ModelObject *mo) const {
  ModelObjectsTemp all_out = get_dependency_graph_outputs(mo);
  ScoreStatesTemp ret;
  for (unsigned int i = 0; i< all_out.size(); ++i) {
    ScoreState *ss = dynamic_cast<ScoreState*>(all_out[i].get());
    if (ss) ret.push_back(ss);
    ret += get_descendent_score_states(all_out[i]);
  }
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}


void Model::do_check_inputs_and_outputs(const ModelObject *mo) const {
  {
    ModelObjectsTemp inputs = (mo)->get_inputs();
    std::sort(inputs.begin(), inputs.end());
    inputs.erase(std::unique(inputs.begin(), inputs.end()), inputs.end());
    ModelObjectsTemp saved_inputs
      = dependency_graph_.find(mo)->second.get_inputs()
      +  dependency_graph_.find(mo)->second.get_input_outputs();
    std::sort(saved_inputs.begin(), saved_inputs.end());
    IMP_USAGE_CHECK(inputs.size() == saved_inputs.size(),
                    "Input sizes don't match.");
    IMP_USAGE_CHECK(std::equal(inputs.begin(), inputs.end(),
                               saved_inputs.begin()),
                    "Input sets don't match.");
    BOOST_FOREACH(ModelObject *i, dependency_graph_.find(mo)
                  ->second.get_inputs()) {
      ModelObjectsTemp readers
        = dependency_graph_.find(i)->second.get_readers();
      IMP_INTERNAL_CHECK(std::find(readers.begin(), readers.end(), mo)
                         != readers.end(),
                         "Input not found in readers for " << mo->get_name()
                         << " and " << i->get_name());
    }
  }
  {
    ModelObjectsTemp outputs = (mo)->get_outputs();
    std::sort(outputs.begin(), outputs.end());
    outputs.erase(std::unique(outputs.begin(), outputs.end()), outputs.end());
    const ModelObjectsTemp &saved_outputs
      = dependency_graph_.find(mo)->second.get_outputs();
    IMP_USAGE_CHECK(outputs.size() == saved_outputs.size(),
                    "Output sizes don't match.");
    IMP_USAGE_CHECK(std::equal(outputs.begin(), outputs.end(),
                               saved_outputs.begin()),
                    "Output sets don't match.");
    BOOST_FOREACH(ModelObject *o, outputs) {
      ModelObjectsTemp writers
        = dependency_graph_.find(o)->second.get_writers();
      IMP_INTERNAL_CHECK(std::find(writers.begin(), writers.end(), mo)
                         != writers.end(),
                         "Input not found in writers for " << (mo)->get_name()
                         << " and " << o->get_name());
    }
  }
}

void Model::do_check_update_order(const ScoreState *ss) const {
  ScoreStatesTemp ancestors = get_ancestor_score_states(ss);
  for (unsigned int j = 0; j < ancestors.size(); ++j) {
    if (ancestors[j]->get_has_update_order()) {
      IMP_INTERNAL_CHECK(ancestors[j]->get_update_order()
                         < ss->get_update_order(),
                         "Mis-ordered score states ancestors: "
                         << ss->get_name()
                         << " at " << ss->get_update_order()
                         << " vs " << ancestors[j]->get_name()
                         << " at " << ancestors[j]->get_update_order());
    }
  }
  ScoreStatesTemp descendents = get_descendent_score_states(ss);
  for (unsigned int j = 0; j < descendents.size(); ++j) {
    if (descendents[j]->get_has_update_order()) {
      IMP_INTERNAL_CHECK(descendents[j]->get_update_order()
                         > ss->get_update_order(),
                           "Mis-ordered score states descendents: "
                         << ss->get_name()
                         << " at " << ss->get_update_order()
                         << " vs " << descendents[j]->get_name()
                         << " at " << descendents[j]->get_update_order());
    }
  }
}
namespace {
template <class It, class DG>
  void check_not_in_readers_and_writers(const ModelObject *mo,
                                        const DG& dg,
                                        It b,
                                        It e) {
  IMP_UNUSED(mo); // for fast builds
    for (It it = b; it != e; ++it) {
      if (dg.find(*it) == dg.end()) continue;
      ModelObjectsTemp readers = dg.find(*it)->second.get_readers();
      IMP_INTERNAL_CHECK(std::find(readers.begin(), readers.end(), mo)
                         == readers.end(),
                         "ModelObject " << (mo)->get_name()
                         << " has no dependencies but is in readers list for "
                         << (*it)->get_name());
      ModelObjectsTemp writers = dg.find(*it)->second.get_writers();
      IMP_INTERNAL_CHECK(std::find(writers.begin(), writers.end(), mo)
                         == writers.end(),
                         "ModelObject " << (mo)->get_name()
                         << " has no dependencies but is in writers list for "
                         << (*it)->get_name());
    }
  }
}
void Model::do_check_required_score_states(const ModelObject *mo) const {
    ScoreStatesTemp ancestors = get_ancestor_score_states(mo);
    ScoreStatesTemp required = mo->get_required_score_states();
    for (unsigned int i = 0; i < required.size(); ++i) {
      IMP_INTERNAL_CHECK(required[i]->get_has_update_order(),
                         "Required score state missing update order.");
      if (i > 0) {
        IMP_INTERNAL_CHECK(required[i - 1]->get_update_order()
                           <= required[i]->get_update_order(),
                           "Mis-ordered required scores states");
      }
    }
    std::sort(required.begin(), required.end());
    std::sort(ancestors.begin(), ancestors.end());
    IMP_USAGE_CHECK(required.size() == ancestors.size(),
                    "Bad required score states " << required << " vs "
                    << ancestors);
    IMP_USAGE_CHECK(std::equal(required.begin(), required.end(),
                    ancestors.begin()),
                    "Bad required score states " << required << " vs "
                    << ancestors);
  }

void Model::check_dependency_invariants() const {
  IMP_OBJECT_LOG;
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    IMP_LOG_TERSE("Checking dependency invariants" << std::endl);
    ModelObjectsTemp without;
    for (ModelObjectTracker::TrackedIterator it =
             ModelObjectTracker::tracked_begin();
         it != ModelObjectTracker::tracked_end(); ++it) {
      if ((*it)->get_has_dependencies()) {
        do_check_inputs_and_outputs(*it);
      } else {
        check_not_in_readers_and_writers(*it,
                                         dependency_graph_,
                                         tracked_begin(),
                                         tracked_end());
      }
      if ((*it)->get_has_required_score_states()) {
        ScoreState *ss = dynamic_cast<ScoreState*>(*it);
        if (ss) {
          IMP_INTERNAL_CHECK(ss->get_has_update_order(), "No update order for "
                             << ss->get_name());
          do_check_update_order(ss);
        }
        do_check_required_score_states(*it);
      }
    }
    BOOST_FOREACH(const ModelObject *mo, no_dependencies_) {
      IMP_INTERNAL_CHECK(std::find(tracked_begin(), tracked_end(),
                                   mo) != tracked_end(),
                         "The object " << mo->get_name()
                         << " is in the no dependencies list but not "
                         << "in the model.");
    }
  }
}

bool Model::get_has_all_dependencies() const {
  check_dependency_invariants();
  return no_dependencies_.empty();
}

void Model::do_set_has_all_dependencies(bool tf) {
  if (tf) {
    while (!no_dependencies_.empty()) {
      do_set_has_dependencies(*no_dependencies_.begin(), true);
    }
  }
}

void Model::set_has_all_dependencies(bool tf) {
  IMP_USAGE_CHECK(tf, "Only works for true.");
  check_dependency_invariants();
  do_set_has_all_dependencies(tf);
  IMP_INTERNAL_CHECK(get_has_all_dependencies() == tf,
                     "Post condition not satisfied");
  check_dependency_invariants();
}

void Model::update() {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  set_has_all_dependencies(true);
  ScoreStatesTemp sst;
   for (ModelObjectTracker::TrackedIterator it =
         ModelObjectTracker::tracked_begin();
         it != ModelObjectTracker::tracked_end(); ++it) {
     ScoreState *ss = dynamic_cast<ScoreState*>(*it);
     if (ss) {
       ss->set_has_required_score_states(true);
       sst.push_back(ss);
     }
   }
   ScoreStatesTemp ordered_score_states
     = get_update_order(sst);
  before_evaluate(ordered_score_states);
}

void Model::do_set_has_dependencies(const ModelObject *mo, bool tf) {
  if (tf == do_get_has_dependencies(mo)) return;
  IMP_OBJECT_LOG;
  if (!tf) {
    remove_dependency_graph_node(mo);
    ModelObject *ncmo = const_cast<ModelObject*>(mo);
    ncmo->do_set_has_required_score_states(false);
  } else {
    add_dependency_graph_node(mo);
    mo->validate_inputs();
    mo->validate_outputs();
  }
}

void Model::add_dependency_graph_node(const ModelObject *cmo) {
  IMP_USAGE_CHECK(!cmo->get_has_dependencies(),
                 "Object " << cmo->get_name() << " does have dependencies.");
  IMP_USAGE_CHECK(!cmo->get_has_required_score_states(),
                 "Object " << cmo->get_name()
                  << " has required score states before addition.");
  IMP_LOG_VERBOSE("Add " << cmo->get_name() << " to dependency graph."
                  << std::endl);
  ModelObject *mo = const_cast<ModelObject*>(cmo);
  ModelObjectsTemp outputs = mo->get_outputs();
  {
    std::sort(outputs.begin(), outputs.end());
    outputs.erase(std::unique(outputs.begin(), outputs.end()),
                   outputs.end());
    BOOST_FOREACH(ModelObject *out, outputs) {
      dependency_graph_[out].access_writers().push_back(mo);
      do_set_has_dependencies(out, false);
    }
    dependency_graph_[mo].set_outputs(outputs);
  }
  {
    ModelObjectsTemp inputs = mo->get_inputs();
    std::sort(inputs.begin(), inputs.end());
    inputs.erase(std::unique(inputs.begin(), inputs.end()),
                   inputs.end());
    ModelObjectsTemp filtered_inputs;
    std::set_difference(inputs.begin(), inputs.end(),
                        outputs.begin(), outputs.end(),
                        std::back_inserter(filtered_inputs));
    ModelObjectsTemp input_outputs;
    std::set_difference(inputs.begin(), inputs.end(),
                        filtered_inputs.begin(), filtered_inputs.end(),
                        std::back_inserter(input_outputs));
    IMP_INTERNAL_CHECK(filtered_inputs.size() + input_outputs.size()
                       == inputs.size(), "Sizes don't add up");
    BOOST_FOREACH(ModelObject *in, filtered_inputs) {
      dependency_graph_[in].access_readers().push_back(mo);
    }
    dependency_graph_[mo].set_inputs(filtered_inputs);
    dependency_graph_[mo].set_input_outputs(input_outputs);
  }
  if (no_dependencies_.find(mo) != no_dependencies_.end()) {
    no_dependencies_.erase(mo);
  }
}

void Model::do_clear_required_score_states(ModelObject *mo) {
  // get_model() is for during shutdown, in case undecorators are not used
  if (!mo->get_model() || !mo->get_has_required_score_states()) return;
  mo->do_set_has_required_score_states(false);
  BOOST_FOREACH(ModelObject *in,
                dependency_graph_.find(mo)->second.get_readers()) {
    do_clear_required_score_states(in);
  }
  BOOST_FOREACH(ModelObject *in,
                dependency_graph_.find(mo)->second.get_outputs()) {
    do_clear_required_score_states(in);
  }
}

void Model::remove_dependency_graph_node(const ModelObject *cmo) {
  IMP_INTERNAL_CHECK(dependency_graph_.find(cmo) != dependency_graph_.end(),
                     "Model object " << cmo->get_name()
                     << " not in graph.");
  IMP_LOG_VERBOSE("Removing " << cmo->get_name() << " from dependency graph."
                  << std::endl);
  ModelObject *mo = const_cast<ModelObject*>(cmo);
  BOOST_FOREACH(ModelObject *in,
                dependency_graph_.find(mo)->second.get_inputs()) {
    base::Vector<ModelObject*> &cur
      = dependency_graph_.find(in)->second.access_readers();
    IMP_INTERNAL_CHECK(std::find(cur.begin(), cur.end(), mo) != cur.end(),
                       "Not in list");
    cur.erase(std::find(cur.begin(), cur.end(), mo));
  }
  BOOST_FOREACH(ModelObject *out,
                dependency_graph_.find(mo)->second.get_outputs()) {
    base::Vector<ModelObject*> &cur
      = dependency_graph_.find(out)->second.access_writers();
    IMP_INTERNAL_CHECK(std::find(cur.begin(), cur.end(), mo) != cur.end(),
                       "Not in list");
    cur.erase(std::find(cur.begin(), cur.end(), mo));
  }
  // since it will be changed during the process
  ModelObjectsTemp inreaders = dependency_graph_.find(mo)->second.get_readers();
  BOOST_FOREACH(ModelObject *in, inreaders) {
    // they can't have score states since they read an object without state
    do_clear_required_score_states(in);
    do_set_has_dependencies(in, false);
  }

  dependency_graph_.find(mo)->second.set_inputs(ModelObjectsTemp());
  dependency_graph_.find(mo)->second.set_outputs(ModelObjectsTemp());
  no_dependencies_.insert(mo);
}

 ModelObjectsTemp
Model::get_dependency_graph_inputs(const ModelObject *mo) const {
  IMP_USAGE_CHECK(do_get_has_dependencies(mo),
                 "Object " << mo->get_name() << " does not have dependencies.");
  IMP_INTERNAL_CHECK(dependency_graph_.find(mo) != dependency_graph_.end(),
                     "Node not in dependency_graph.");
  return dependency_graph_.find(mo)->second.get_inputs()
    + dependency_graph_.find(mo)->second.get_writers();
}

 ModelObjectsTemp
Model::get_dependency_graph_outputs(const ModelObject *mo) const {
  IMP_USAGE_CHECK(mo->get_has_dependencies(),
                 "Object " << mo->get_name() << " does not have dependencies.");
  IMP_INTERNAL_CHECK(dependency_graph_.find(mo) != dependency_graph_.end(),
                     "Node not in dependency_graph.");
  return dependency_graph_.find(mo)->second.get_outputs()
    + dependency_graph_.find(mo)->second.get_readers();
}

void Model::do_add_tracked(ModelObject*mo) {
  IMP_LOG_VERBOSE("Adding " << mo->get_name() << " to model." << std::endl);
  no_dependencies_.insert(mo);
}

void Model::do_remove_tracked(ModelObject*mo) {
  IMP_LOG_VERBOSE("Removing " << mo->get_name() << " from model." << std::endl);
  if (dependency_graph_.find(mo) != dependency_graph_.end()) {
    remove_dependency_graph_node(mo);
  }
  // must be after to clean up
  if (no_dependencies_.find(mo) != no_dependencies_.end()) {
    no_dependencies_.erase(mo);
  }
}


IMPKERNEL_END_NAMESPACE
