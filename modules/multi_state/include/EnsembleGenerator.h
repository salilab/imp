/**
 * \file IMP/multi_state/EnsembleGenerator.h
 * \brief implements branch&bound enumeration of an ensemble of good scoring MultiStateModels
 *
 * \authors Dina Schneidman
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTI_STATE_ENSEMBLE_GENERATOR_H
#define IMPMULTI_STATE_ENSEMBLE_GENERATOR_H

#include "MultiStateModel.h"
#include "MultiStateModelScore.h"

IMPMULTISTATE_BEGIN_NAMESPACE

// Ensemble is a set of multi-state models
typedef std::vector<MultiStateModel> Ensemble;

class IMPMULTISTATEEXPORT EnsembleGenerator {
public:
  EnsembleGenerator(unsigned int population_size,
                    unsigned int best_k,
                    std::vector<MultiStateModelScore*>& scorers,
                    float min_weight_threshold);

  void generate(unsigned int multi_state_model_size);

private:
  void init();

  // enumerate MultiStateModels of size N from MultiStateModels of size N-1
  void add_one_state(const Ensemble& init_ensemble,
                      Ensemble& new_ensemble);

  void rescore(Ensemble& ensemble,
               Ensemble& rescored_ensemble,
               std::vector<std::vector<IMP::saxs::WeightedFitParameters> >& rfps) const;

  void output(Ensemble& ensemble,
              const std::vector<std::vector<IMP::saxs::WeightedFitParameters> >& fps) const;

  double get_score(const MultiStateModel& e) const {
    double ret = 0;
    for(unsigned int i=0; i<scorers_.size(); i++) {
      ret += scorers_[i]->get_score(e);
    }
    return ret;
  }

  void get_state_probabilities(const Ensemble& ensemble,
                                std::vector<double>& state_prob) const;

  void get_weights_average_and_std(const Ensemble& ensemble,
                                   const std::vector<IMP::saxs::WeightedFitParameters>& fps,
                                   std::vector<double>& weights_average,
                                   std::vector<double>& weights_variance) const;

private:
  unsigned int N_;
  unsigned int K_;
  std::vector<MultiStateModelScore*> scorers_;
  std::vector<Ensemble > ensembles_;
  float min_weight_threshold_; // default=5%
};

IMPMULTISTATE_END_NAMESPACE

#endif /* IMPMULTI_STATE_ENSEMBLE_GENERATOR_H */
