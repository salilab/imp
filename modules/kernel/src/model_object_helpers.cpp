/**
 *  \file ModelObject.cpp  \brief Single variable function.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 */

#include <IMP/model_object_helpers.h>
#include <IMP/input_output.h>
#include <IMP/Model.h>

IMPKERNEL_BEGIN_NAMESPACE

ModelObjectsTemp ParticleInputs::get_inputs(Model *m,
                                            const ParticleIndexes &pis) const {
  return do_get_inputs(m, pis);
}

ModelObjectsTemp ParticleOutputs::get_outputs(
    Model *m, const ParticleIndexes &pis) const {
  return do_get_outputs(m, pis);
}

ScoreStatesTemp get_required_score_states(const ModelObjectsTemp &mos,
                                          ScoreStatesTemp exclude) {
  if (mos.empty()) return ScoreStatesTemp();
  ScoreStatesTemp ret;
  for (unsigned int i = 0; i < mos.size(); ++i) {
    mos[0]->get_model()->do_set_has_required_score_states(mos[i], true);
    ret += mos[i]->get_required_score_states();
  }
  std::sort(ret.begin(), ret.end());
  std::sort(exclude.begin(), exclude.end());
  ScoreStatesTemp diff;
  std::set_difference(ret.begin(), ret.end(), exclude.begin(), exclude.end(),
                      std::back_inserter(diff));
  return get_update_order(diff);
}

IMPKERNEL_END_NAMESPACE
