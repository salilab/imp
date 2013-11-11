/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/kernel/DerivativeAccumulator.h"
#include "IMP/kernel/Model.h"
#include "IMP/kernel/Particle.h"
#include "IMP/kernel/Restraint.h"
#include "IMP/kernel/RestraintSet.h"
#include "IMP/kernel/ScoreState.h"
#include "IMP/kernel/ScoreState.h"
#include "IMP/kernel/ScoringFunction.h"
#include "IMP/kernel/dependency_graph.h"
#include "IMP/kernel/file.h"
#include "IMP/kernel/internal/evaluate_utility.h"
#include "IMP/kernel/internal/graph_utility.h"
#include <IMP/kernel/utility.h>
#include "IMP/base/map.h"
#include "IMP/base/log.h"
#include "IMP/base/set.h"
#include "IMP/base/statistics.h"


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
  ModelObjectsTemp all_in = dependency_graph_.find(mo)->second.get_inputs() +
                            dependency_graph_.find(mo)->second.get_writers();
  ScoreStatesTemp ret;
  for (unsigned int i = 0; i < all_in.size(); ++i) {
    ScoreState *ss = dynamic_cast<ScoreState *>(all_in[i].get());
    if (ss) ret.push_back(ss);
    ret += get_ancestor_score_states(all_in[i]);
  }
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

ScoreStatesTemp Model::get_descendent_score_states(
    const ModelObject *mo) const {
  ModelObjectsTemp all_out = dependency_graph_.find(mo)->second.get_outputs() +
                             dependency_graph_.find(mo)->second.get_readers();
  ScoreStatesTemp ret;
  for (unsigned int i = 0; i < all_out.size(); ++i) {
    ScoreState *ss = dynamic_cast<ScoreState *>(all_out[i].get());
    if (ss) ret.push_back(ss);
    ret += get_descendent_score_states(all_out[i]);
  }
  std::sort(ret.begin(), ret.end());
  ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
  return ret;
}

void Model::do_check_inputs_and_outputs(const ModelObject *mo) const {
  {
    IMP_FOREACH(kernel::ModelObject * i,
                  dependency_graph_.find(mo)->second.get_inputs()) {
      ModelObjectsTemp readers =
          dependency_graph_.find(i)->second.get_readers();
      IMP_INTERNAL_CHECK(
          std::find(readers.begin(), readers.end(), mo) != readers.end(),
          "Input not found in readers for " << mo->get_name() << " and "
                                            << i->get_name());
    }
  }
  {
    IMP_FOREACH(kernel::ModelObject * o,
                  dependency_graph_.find(mo)->second.get_outputs()) {
      ModelObjectsTemp writers =
          dependency_graph_.find(o)->second.get_writers();
      IMP_INTERNAL_CHECK(
          std::find(writers.begin(), writers.end(), mo) != writers.end(),
          "Input not found in writers for " << (mo)->get_name() << " and "
                                            << o->get_name());
    }
  }
}

void Model::do_check_readers_and_writers(const ModelObject *mo) const {
  {
    ModelObjectsTemp readers = dependency_graph_.find(mo)->second.get_readers();
    IMP_FOREACH(kernel::ModelObject * r, readers) {
      IMP_INTERNAL_CHECK(dependency_graph_.find(r) != dependency_graph_.end(),
                         "Reader " << r->get_name() << " of " << mo->get_name()
                                   << " not in graph" << std::endl);
      const ModelObjectsTemp &inputs =
          dependency_graph_.find(r)->second.get_inputs();
      IMP_UNUSED(inputs);
      IMP_INTERNAL_CHECK(
          std::find(inputs.begin(), inputs.end(), mo) != inputs.end(),
          "Reader not found in inputs for " << mo->get_name() << " and "
                                            << r->get_name());
    }
  }
  {
    const ModelObjectsTemp &writers =
        dependency_graph_.find(mo)->second.get_writers();
    IMP_UNUSED(writers);
    IMP_FOREACH(kernel::ModelObject * w, writers) {
      IMP_INTERNAL_CHECK(dependency_graph_.find(w) != dependency_graph_.end(),
                         "Reader " << w->get_name() << " of " << mo->get_name()
                                   << " not in graph" << std::endl);
      const ModelObjectsTemp &outputs =
          dependency_graph_.find(w)->second.get_outputs();
      IMP_UNUSED(outputs);
      IMP_INTERNAL_CHECK(
          std::find(outputs.begin(), outputs.end(), mo) != outputs.end(),
          "Writer not found in outputs for " << (mo)->get_name() << " and "
                                             << w->get_name());
    }
  }
}

void Model::do_check_update_order(const ScoreState *ss) const {
  ScoreStatesTemp ancestors = get_ancestor_score_states(ss);
  for (unsigned int j = 0; j < ancestors.size(); ++j) {
    if (ancestors[j]->get_has_update_order()) {
      IMP_INTERNAL_CHECK(
          ancestors[j]->get_update_order() < ss->get_update_order(),
          "Mis-ordered score states ancestors: "
              << ss->get_name() << " at " << ss->get_update_order() << " vs "
              << ancestors[j]->get_name() << " at "
              << ancestors[j]->get_update_order());
    }
  }
  ScoreStatesTemp descendents = get_descendent_score_states(ss);
  for (unsigned int j = 0; j < descendents.size(); ++j) {
    if (descendents[j]->get_has_update_order()) {
      IMP_INTERNAL_CHECK(
          descendents[j]->get_update_order() > ss->get_update_order(),
          "Mis-ordered score states descendents: "
              << ss->get_name() << " at " << ss->get_update_order() << " vs "
              << descendents[j]->get_name() << " at "
              << descendents[j]->get_update_order());
    }
  }
}

void Model::do_check_not_in_readers_and_writers(const ModelObject *mo) const {
  IMP_UNUSED(mo);  // for fast builds
  IMP_FOREACH(DependencyGraph::value_type vt, dependency_graph_) {
    ModelObjectsTemp readers = vt.second.get_readers();
    IMP_INTERNAL_CHECK(
        std::find(readers.begin(), readers.end(), mo) == readers.end(),
        "ModelObject " << (mo)->get_name()
                       << " has no dependencies but is in readers list for "
                       << (vt.first)->get_name());
    ModelObjectsTemp writers = vt.second.get_writers();
    IMP_INTERNAL_CHECK(
        std::find(writers.begin(), writers.end(), mo) == writers.end(),
        "ModelObject " << (mo)->get_name()
                       << " has no dependencies but is in writers list for "
                       << (vt.first)->get_name());
  }
}

void Model::do_check_required_score_states(const ModelObject *mo) const {
  ScoreStatesTemp ancestors = get_ancestor_score_states(mo);
  ScoreStatesTemp required = do_get_required_score_states(mo);
  for (unsigned int i = 0; i < required.size(); ++i) {
    IMP_INTERNAL_CHECK(required[i]->get_has_update_order(),
                       "Required score state missing update order.");
    if (i > 0) {
      IMP_INTERNAL_CHECK(required[i - 1]->get_update_order() <=
                             required[i]->get_update_order(),
                         "Mis-ordered required scores states");
    }
  }
  std::sort(required.begin(), required.end());
  std::sort(ancestors.begin(), ancestors.end());
  IMP_USAGE_CHECK(required.size() == ancestors.size(),
                  "Bad required score states " << required << " vs "
                                               << ancestors);
  IMP_USAGE_CHECK(
      std::equal(required.begin(), required.end(), ancestors.begin()),
      "Bad required score states " << required << " vs " << ancestors);
}

void Model::check_dependency_invariants(const ModelObject *mo) const {
  if (do_get_has_dependencies(mo)) {
    do_check_inputs_and_outputs(mo);
  } else {
    do_check_not_in_readers_and_writers(mo);
  }
  do_check_readers_and_writers(mo);
  if (do_get_has_required_score_states(mo)) {
    const ScoreState *ss = dynamic_cast<const ScoreState *>(mo);
    if (ss) {
      IMP_INTERNAL_CHECK(ss->get_has_update_order(), "No update order for "
                                                         << ss->get_name());
      do_check_update_order(ss);
    }
    do_check_required_score_states(mo);
  }
}

void Model::check_dependency_invariants() const {
  IMP_OBJECT_LOG;
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    IMP_LOG_TERSE("Checking dependency invariants" << std::endl);
    ModelObjectsTemp without;
    IMP_FOREACH(const DependencyGraph::value_type & vt, dependency_graph_) {
      check_dependency_invariants(vt.first);
    }
    IMP_FOREACH(const ModelObject * mo, no_dependencies_) {
      IMP_UNUSED(mo);
      IMP_INTERNAL_CHECK(
          dependency_graph_.find(mo) != dependency_graph_.end(),
          "The object " << mo->get_name()
                        << " is in the no dependencies list but not "
                        << "in the model.");
    }
  }
}

bool Model::get_has_all_dependencies() const {
  // check_dependency_invariants();
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
  base::Timer t(this, "set_has_all_dependencies");
  IMP_USAGE_CHECK(tf, "Only works for true.");
  // check_dependency_invariants();
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
  IMP_FOREACH(DependencyGraph::value_type & vt, dependency_graph_) {
    ModelObject *mo = const_cast<ModelObject *>(vt.first);
    ScoreState *ss = dynamic_cast<ScoreState *>(mo);
    if (ss) {
      do_set_has_required_score_states(ss, true);
      sst.push_back(ss);
    }
  }
  ScoreStatesTemp ordered_score_states = get_update_order(sst);
  before_evaluate(ordered_score_states);
}

void Model::do_set_has_dependencies(const ModelObject *mo, bool tf) {
  IMP_CHECK_OBJECT(mo);
  if (tf == do_get_has_dependencies(mo)) return;
  IMP_OBJECT_LOG;
  if (!tf) {
    IMP_INTERNAL_CHECK(dependency_graph_.find(mo) != dependency_graph_.end(),
                       "Model object " << mo->get_name() << " not in graph.");
    IMP_LOG_VERBOSE("Removing " << mo->get_name() << " from dependency graph."
                                << std::endl);
    do_clear_dependencies(mo);
  } else {
    do_add_dependencies(mo);
    mo->validate_inputs();
    mo->validate_outputs();
  }
}

void Model::do_add_dependencies(const ModelObject *cmo) {
  static base::set<const ModelObject *> computed;
  if (computed.find(cmo) != computed.end()) {
    IMP_THROW("Loop in dependencies at " << cmo->get_name(), ModelException);
  }
  computed.insert(cmo);
  IMP_LOG_VERBOSE("Add " << cmo->get_name() << " to dependency graph."
                         << std::endl);
  IMP_INTERNAL_CHECK(no_dependencies_.find(cmo) != no_dependencies_.end(),
                     "Already has dependencies");
  ModelObject *mo = const_cast<ModelObject *>(cmo);
  ModelObjectsTemp outputs = mo->get_outputs();
  {
    std::sort(outputs.begin(), outputs.end());
    outputs.erase(std::unique(outputs.begin(), outputs.end()), outputs.end());
    IMP_FOREACH(kernel::ModelObject * out, outputs) {
      dependency_graph_[out].access_writers().push_back(mo);
      do_clear_required_score_states(out);
    }
    dependency_graph_[mo].set_outputs(outputs);
  }
  {
    ModelObjectsTemp inputs = mo->get_inputs();
    std::sort(inputs.begin(), inputs.end());
    inputs.erase(std::unique(inputs.begin(), inputs.end()), inputs.end());
    ModelObjectsTemp filtered_inputs;
    std::set_difference(inputs.begin(), inputs.end(), outputs.begin(),
                        outputs.end(), std::back_inserter(filtered_inputs));
    ModelObjectsTemp input_outputs;
    std::set_difference(inputs.begin(), inputs.end(), filtered_inputs.begin(),
                        filtered_inputs.end(),
                        std::back_inserter(input_outputs));
    IMP_INTERNAL_CHECK(
        filtered_inputs.size() + input_outputs.size() == inputs.size(),
        "Sizes don't add up");
    IMP_FOREACH(kernel::ModelObject * in, filtered_inputs) {
      dependency_graph_[in].access_readers().push_back(mo);
    }
    dependency_graph_[mo].set_inputs(filtered_inputs);
    dependency_graph_[mo].set_input_outputs(input_outputs);
  }
  // needs to be at end to not trigger input/output validation
  no_dependencies_.erase(cmo);
  computed.erase(cmo);
}

void Model::do_clear_required_score_states(kernel::ModelObject *mo) {
  IMP_CHECK_OBJECT(mo);
  if (required_score_states_.find(mo) == required_score_states_.end()) return;
  required_score_states_.erase(mo);
  IMP_CHECK_OBJECT(mo);
  mo->handle_set_has_required_score_states(false);
  IMP_FOREACH(kernel::ModelObject * in,
                dependency_graph_.find(mo)->second.get_readers()) {
    do_clear_required_score_states(in);
  }
  IMP_FOREACH(kernel::ModelObject * in,
                dependency_graph_.find(mo)->second.get_outputs()) {
    do_clear_required_score_states(in);
  }
}

void Model::do_clear_dependencies(const ModelObject *cmo) {
  IMP_CHECK_OBJECT(this);
  if (dependency_graph_.find(cmo) == dependency_graph_.end()) return;
  ModelObject *mo = const_cast<ModelObject *>(cmo);
  {  // do first before edges go away
    do_clear_required_score_states(mo);
  }
  NodeInfo &ni = dependency_graph_.find(cmo)->second;
  IMP_FOREACH(kernel::ModelObject * in, ni.get_inputs()) {
    // for teardown
    if (dependency_graph_.find(in) == dependency_graph_.end()) continue;
    base::Vector<ModelObject *> &cur =
        dependency_graph_.find(in)->second.access_readers();
    IMP_INTERNAL_CHECK(std::find(cur.begin(), cur.end(), mo) != cur.end(),
                       "Not in list");
    std::swap(*std::find(cur.begin(), cur.end(), mo), cur.back());
    cur.pop_back();
    IMP_INTERNAL_CHECK(std::find(cur.begin(), cur.end(), mo) == cur.end(),
                       "in list");
  }
  ni.set_inputs(Edges());

  IMP_FOREACH(kernel::ModelObject * out, ni.get_outputs()) {
    if (dependency_graph_.find(out) == dependency_graph_.end()) continue;
    base::Vector<ModelObject *> &cur =
        dependency_graph_.find(out)->second.access_writers();
    IMP_INTERNAL_CHECK(std::find(cur.begin(), cur.end(), mo) != cur.end(),
                       "Not in list");
    std::swap(*std::find(cur.begin(), cur.end(), mo), cur.back());
    cur.pop_back();
    IMP_INTERNAL_CHECK(std::find(cur.begin(), cur.end(), mo) == cur.end(),
                       " in list");
  }
  ni.set_outputs(Edges());
  ni.set_input_outputs(Edges());
  no_dependencies_.insert(mo);
  {  // down stream might be affected (eg SetContainers)
    Edges readers = ni.get_readers();
    IMP_FOREACH(kernel::ModelObject * reader, readers) {
      do_clear_dependencies(reader);
    }
  }
}

ModelObjectsTemp Model::get_dependency_graph_inputs(
    const ModelObject *mo) const {
  IMP_USAGE_CHECK(do_get_has_dependencies(mo),
                  "Object " << mo->get_name()
                            << " does not have dependencies.");
  IMP_INTERNAL_CHECK(dependency_graph_.find(mo) != dependency_graph_.end(),
                     "Node not in dependency_graph.");
  return dependency_graph_.find(mo)->second.get_inputs() +
         dependency_graph_.find(mo)->second.get_writers();
}

ModelObjectsTemp Model::get_dependency_graph_outputs(
    const ModelObject *mo) const {
  IMP_USAGE_CHECK(mo->get_has_dependencies(),
                  "Object " << mo->get_name()
                            << " does not have dependencies.");
  IMP_INTERNAL_CHECK(dependency_graph_.find(mo) != dependency_graph_.end(),
                     "Node not in dependency_graph.");
  return dependency_graph_.find(mo)->second.get_outputs() +
         dependency_graph_.find(mo)->second.get_readers();
}

bool Model::do_get_has_required_score_states(const ModelObject *mo) const {
  return required_score_states_.find(mo) != required_score_states_.end();
}

void Model::do_set_has_required_score_states(kernel::ModelObject *mo, bool tf) {
  static base::set<const ModelObject *> computed;
  if (computed.find(mo) != computed.end()) {
    IMP_THROW("Loop in dependencies at " << mo->get_name(), ModelException);
  }
  if (tf) {
    // make sure everything is there as we invalidate lazily
    do_set_has_all_dependencies(true);
  }
  if (tf == do_get_has_required_score_states(mo)) return;
  computed.insert(mo);
  IMP_OBJECT_LOG;
  if (tf) {
    ScoreStates all;
    IMP_FOREACH(kernel::ModelObject * input,
                  dependency_graph_.find(mo)->second.get_inputs()) {
      do_set_has_required_score_states(input, true);
      all += required_score_states_.find(input)->second;
      ScoreState *ss = dynamic_cast<ScoreState *>(input);
      if (ss) all.push_back(ss);
    }
    IMP_FOREACH(kernel::ModelObject * input,
                  dependency_graph_.find(mo)->second.get_writers()) {
      do_set_has_required_score_states(input, true);
      all += required_score_states_.find(input)->second;
      ScoreState *ss = dynamic_cast<ScoreState *>(input);
      if (ss) all.push_back(ss);
    }
    required_score_states_[mo] = get_update_order(all);
    IMP_LOG_VERBOSE("Score states for " << get_name() << " are "
                                        << required_score_states_.find(mo)
                                               ->second << std::endl);
  } else {
    required_score_states_.erase(mo);
  }
  mo->handle_set_has_required_score_states(tf);
  computed.erase(mo);
}

void Model::do_add_model_object(kernel::ModelObject *mo) {
  IMP_LOG_VERBOSE("Adding " << mo->get_name() << " to model." << std::endl);
  if (dependency_graph_.find(mo) == dependency_graph_.end()) {
    dependency_graph_[mo] = NodeInfo();
  }
  no_dependencies_.insert(mo);
}

void Model::do_remove_model_object(kernel::ModelObject *mo) {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(dependency_graph_.find(mo) != dependency_graph_.end(),
                  "Already removed from model.");
  if (no_dependencies_.find(mo) == no_dependencies_.end()) {
    do_clear_dependencies(mo);
  }
  // clean up stray references
  {
    Edges readers = dependency_graph_.find(mo)->second.get_readers();
    IMP_FOREACH(kernel::ModelObject * r, readers) {
      do_clear_dependencies(r);
    }
  }
  IMP_INTERNAL_CHECK(dependency_graph_.find(mo) != dependency_graph_.end(),
                     "Not in dependency graph");
  {
    NodeInfo &ni = dependency_graph_.find(mo)->second;
    while (!ni.get_readers().empty()) {
      do_clear_dependencies(ni.get_readers().back());
    }
    while (!ni.get_writers().empty()) {
      do_clear_dependencies(ni.get_writers().back());
    }
  }
  no_dependencies_.erase(mo);
  dependency_graph_.erase(mo);
}

IMPKERNEL_END_NAMESPACE
