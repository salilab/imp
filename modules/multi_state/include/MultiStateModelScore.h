/**
 * \file IMP/multi_state/MultiStateModelScore.h
 * \brief base class for MultiStateModel scoring classes
 *
 * \authors Dina Schneidman
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTI_STATE_MULTI_STATE_MODEL_SCORE_H
#define IMPMULTI_STATE_MULTI_STATE_MODEL_SCORE_H

#include <IMP/multi_state/multi_state_config.h>
#include <IMP/multi_state/MultiStateModel.h>
#include <IMP/saxs/WeightedFitParameters.h>

IMPMULTISTATE_BEGIN_NAMESPACE

//! Base class for MultiStateModel scoring classes
class MultiStateModelScore {
public:
  // multi_state_model score
  virtual double get_score(const MultiStateModel& e) const = 0;

  // multi_state_model score and weights
  virtual double get_score(const MultiStateModel& e,
                           Vector<double>& weights) const = 0;

  // multi_state_model score, weights and other fitting params
  virtual saxs::WeightedFitParameters
                   get_fit_parameters(MultiStateModel& e) const = 0;

  // get score and weights
  virtual saxs::WeightedFitParameters get_fit_parameters() const = 0;

  // write fit file
  virtual void write_fit_file(MultiStateModel& e,
                              const saxs::WeightedFitParameters& fp,
                              const std::string fit_file_name) const = 0;

  // get name
  virtual std::string get_state_name(unsigned int id) const = 0;

  // get data name
  virtual std::string get_dataset_name() const = 0;

  virtual ~MultiStateModelScore() {}
};

IMPMULTISTATE_END_NAMESPACE

#endif /* IMPMULTI_STATE_MULTI_STATE_MODEL_SCORE_H */
