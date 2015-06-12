/**
 *  \file IMP/multi_state/EnsembleGenerator.cpp
 *
 * \brief A class for storing and generation of ensembles
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multi_state/EnsembleGenerator.h>
#include <IMP/multi_state/stat_helpers.h>
#include <IMP/multi_state/SAXSMultiStateModelScore.h>

#include <IMP/exception.h>

#include <algorithm>
#include <boost/tuple/tuple.hpp>
#include <queue>
#include <fstream>
#include <map>

IMPMULTISTATE_BEGIN_NAMESPACE

EnsembleGenerator::EnsembleGenerator(unsigned int population_size,
                                     unsigned int best_k,
                                     Vector<MultiStateModelScore*>& scorers,
                                     double min_weight_threshold = 0.05) :
  N_(population_size), K_(best_k), scorers_(scorers),
  min_weight_threshold_(min_weight_threshold) {
}

namespace {
  struct Comparator {
    bool operator()(const boost::tuple<double, int, int>& p1,
                    const boost::tuple<double, int, int>& p2) {
      return boost::get<0>(p1) < boost::get<0>(p2);
    }
  };
}

void EnsembleGenerator::init() {
  // reserve space
  ensembles_.insert(ensembles_.begin(), 100, Ensemble());
  ensembles_[0].reserve(N_);

  // generate & score N MultiStateModels of size 1 (=SingleStateModels)
  for(unsigned int i=0; i<N_; i++) {
    MultiStateModel m(1);
    m.add_state(i);
    //m.set_score(get_score(e));

    double min_score = std::numeric_limits<double>::max();
    for (unsigned int j=0; j<scorers_.size(); j++) {
      double score = scorers_[j]->get_score(m);
      if(score < min_score) min_score = score;
    }
    m.set_score(min_score);

    ensembles_[0].push_back(m);
  }

  std::sort(ensembles_[0].begin(), ensembles_[0].end(), CompareMultiStateModels());

  // erase low scoring MultiStateModels
  if(ensembles_[0].size() > K_) {
    ensembles_[0].erase(ensembles_[0].begin()+K_, ensembles_[0].end());
    std::cout << "Number of MultiStateModels of size 1: " << ensembles_[0].size() << std::endl;
  }

  Ensemble rescored_ensemble;
  Vector<Vector<saxs::WeightedFitParameters> > rescored_fps;
  rescore(ensembles_[0], rescored_ensemble, rescored_fps);
  output(rescored_ensemble, rescored_fps);
}

void EnsembleGenerator::generate(unsigned int multi_state_model_size) {
  init();
  if(ensembles_.size() < multi_state_model_size) {
    std::cerr << "Only MultiStateModels up to " << multi_state_model_size
              << " states are supported" << std::endl;
    return;
  }
  for(unsigned int i=1; i<multi_state_model_size; i++) {
    if(ensembles_[i-1].size() > 0) {
      add_one_state(ensembles_[i-1], ensembles_[i]);

      Ensemble rescored_ensemble;
      Vector<Vector<saxs::WeightedFitParameters> > rescored_fps;
      rescore(ensembles_[i], rescored_ensemble, rescored_fps);
      std::cout << "initial size " << ensembles_[i].size() << " after rescoring "
                << rescored_ensemble.size() << std::endl;
      output(rescored_ensemble, rescored_fps);

      if(rescored_ensemble.size() > 0) {
        for(unsigned int j = 0; j < scorers_.size(); j++) {
          SAXSMultiStateModelScore<saxs::ChiScore> *s =
            dynamic_cast<SAXSMultiStateModelScore<saxs::ChiScore>*>(scorers_[j]);
          if(s != NULL) s->set_average_c1_c2(rescored_fps[j]);
        }
      }
    }
  }
}

void EnsembleGenerator::rescore(Ensemble& ensemble,
                                Ensemble& rescored_ensemble,
                     Vector<Vector<saxs::WeightedFitParameters> >& rescored_fps)
  const {

  unsigned int print_num = std::min((unsigned int)ensemble.size(), K_);

  Vector<Vector<saxs::WeightedFitParameters> > fps(scorers_.size()),
    sorted_fps(scorers_.size());
  std::multimap<double, unsigned int> scores;
  unsigned int counter = 0;

  // re-score
  for(unsigned int i = 0; i < ensemble.size(); i++) {
    if(i>0 && i%100==0) {
      std::cerr << "Rescoring ensemble " << i << " out of " << ensemble.size() << std::endl;
    }
    // iterate scorers and record max weight for each state
    Vector<double> max_weights(ensemble[i].size(), 0.0);
    double score = 0;
    for(unsigned int k = 0; k < scorers_.size(); k++) {
      saxs::WeightedFitParameters p = scorers_[k]->get_fit_parameters(ensemble[i]);
      score += p.get_score();
      // find the max weight contribution of each state
      for(unsigned int wi = 0; wi < p.get_weights().size(); wi++) {
        if(p.get_weights()[wi] > max_weights[wi])
          max_weights[wi] = p.get_weights()[wi];
      }
      fps[k].push_back(p);
    }
    ensemble[i].set_score(score);

    // check max weights for threshold
    for(unsigned int wi=0; wi<max_weights.size(); wi++) {
      if(max_weights[wi] < min_weight_threshold_) {
        ensemble[i].set_score(-1);
        break;
      }
    }

    // do not output MultiStateModels with one of the weights below threshold
    if(ensemble[i].get_score() < 0.0) continue;
    scores.insert(std::make_pair(ensemble[i].get_score(), i));
    counter++;

    if(counter >= K_) break;
  }

  // sort
  Ensemble sorted_ensemble;
  sorted_ensemble.reserve(print_num);
  std::multimap<double, unsigned int>::iterator it, end_it = scores.end();
  for(it = scores.begin(); it != end_it; it++) {
    //std::cerr << "score = " << it->first << std::endl;
    sorted_ensemble.push_back(ensemble[it->second]);
    for(unsigned int k=0; k<scorers_.size(); k++) {
      sorted_fps[k].push_back(fps[k][it->second]);
    }
  }
  rescored_ensemble = sorted_ensemble;
  rescored_fps = sorted_fps;
}

void EnsembleGenerator::output(Ensemble& ensemble,
       const Vector<Vector<saxs::WeightedFitParameters> >& fps) const {

  if(ensemble.size() == 0) return;

  // calculate z-score
  Vector<double> scores(ensemble.size());
  for(unsigned int i=0; i<ensemble.size(); i++) scores[i] = ensemble[i].get_score();
  std::pair<double, double> average_and_std = get_average_and_stdev(scores);
  for(unsigned int i=0; i<ensemble.size(); i++) {
    double zscore = (ensemble[i].get_score()-average_and_std.first) /
      average_and_std.second;
    ensemble[i].set_zscore(zscore);
  }

  // calculate frequency of each state
  Vector<double> state_prob;
  get_state_probabilities(ensemble, state_prob);

  // calculate weights average and variance
  Vector<Vector<double> > weights_average(scorers_.size()),
    weights_variance(scorers_.size());
  for(unsigned int i=0; i<scorers_.size(); i++) {
    get_weights_average_and_std(ensemble, fps[i], weights_average[i],
                                weights_variance[i]);
  }

  // output file
  unsigned int number_of_states = ensemble[0].size();
  std::string out_file_name = "ensembles_size_" +
    std::string(boost::lexical_cast<std::string>(number_of_states)) + ".txt";
  std::ofstream s(out_file_name.c_str());
  std::cout << "multi_state_model_size " << ensemble.size ()
            << " number_of_states " << number_of_states << std::endl;

  for(unsigned int i=0; i<ensemble.size(); i++) {
    // output ensemble scores
    s.setf(std::ios::fixed, std::ios::floatfield);
    s << i+1 << " | " << std::setw(5) << std::setprecision(2)
      << ensemble[i].get_score(); // << " | " << ensemble[i].get_zscore();

    // output scores for each scorer
    for(unsigned int j=0; j<scorers_.size(); j++) {
      const saxs::WeightedFitParameters& p = fps[j][i];
      s << " | x" << std::string(boost::lexical_cast<std::string>(j+1))
        //scorers_[j]->get_dataset_name() << ": "
        << " " << std::setprecision(2) << p.get_chi()
        << " (" << p.get_c1() << ", " << p.get_c2() << ")";
    }
    s << std::endl;

    // output states and their probabilities
    const Vector<unsigned int>& states = ensemble[i].get_states();
    for(unsigned int k=0; k<states.size(); k++) {
      s << std::setw(5) << states[k];

      // output weights
      for(unsigned int j=0; j<scorers_.size(); j++) {
        const saxs::WeightedFitParameters& p = fps[j][i];
        if(p.get_weights().size() > k) {
          s << std::setw(5) << std::setprecision(3) << " | "
            << p.get_weights()[k] << " ("
            << weights_average[j][states[k]] << ", "
            << weights_variance[j][states[k]] << ")";
        }
      }
      s << " | "  << scorers_[0]->get_state_name(states[k])
        << " (" << state_prob[states[k]] << ")" << std::endl;
    }

    // output fit file
    if(i<10) { // TODO: add parameter
      for(unsigned int j=0; j<scorers_.size(); j++) {
        std::string fit_file_name = "multi_state_model_" +
          std::string(boost::lexical_cast<std::string>(number_of_states)) + "_" +
          std::string(boost::lexical_cast<std::string>(i+1));
        if(scorers_.size() > 0) {
          fit_file_name +=  "_" + std::string(boost::lexical_cast<std::string>(j+1));
        }
        fit_file_name += ".dat";
        scorers_[j]->write_fit_file(ensemble[i], fps[j][i], fit_file_name);
      }
    }
  }
  s.close();
}

void EnsembleGenerator::add_one_state(const Ensemble& init_ensemble,
                                      Ensemble& new_ensemble) {

  std::priority_queue<boost::tuple<double, int, int>,
                      Vector<boost::tuple<double, int, int> >,
                      Comparator> bestK;

  // iterate over all init MultiStateModels and try to add a new state to each
  for(unsigned int i=0; i<init_ensemble.size(); i++) {
    unsigned int first_to_search = init_ensemble[i].get_last_state()+1;
    if(first_to_search<N_) {

      if(i>0 && i%100==0 && !bestK.empty()) {
        double curr_bestK_score = boost::get<0>(bestK.top());
        std::cout << "Extending ensemble: " << i << " out of "
                  << init_ensemble.size() << " last best "
                  << curr_bestK_score << std::endl;
      }

      MultiStateModel new_model(init_ensemble[i]);
      new_model.add_state(first_to_search);

      // try all possible additions of a new state
      for(unsigned int j=first_to_search; j<N_; j++) {
        new_model.replace_last_state(j);
        double curr_score = get_score(new_model);
        if(curr_score < 0.0) continue; // invalid model
        // add to bestK
        if(bestK.size() <= K_ || curr_score < boost::get<0>(bestK.top())) {
          bestK.push(boost::make_tuple(curr_score, i, j));
          if(bestK.size() > K_) bestK.pop();
        }
      }
    }
  }

  // save best scoring
  new_ensemble.assign(bestK.size(), MultiStateModel(0));
  int index = bestK.size()-1;
  // generate bestK new MultiStateModels
  while(!bestK.empty()) {
    double score;
    int ensemble_index, new_state_index;
    boost::tie(score, ensemble_index, new_state_index) = bestK.top();
    MultiStateModel new_model(init_ensemble[ensemble_index]);
    new_model.add_state(new_state_index);
    new_model.set_score(score);
    new_ensemble[index] = new_model;
    index--;
    bestK.pop();
  }
}

void EnsembleGenerator::get_state_probabilities(const Ensemble& ensemble,
                                         Vector<double>& state_prob) const {

  Vector<unsigned int> states_counters(N_, 0);
  unsigned int total_state_num = 0;

  // count the number of occurences of each state in MultiStateModels
  // in the entire Ensemble (states_counters)
  for(unsigned int i=0; i<ensemble.size(); i++) {
    const Vector<unsigned int>& states = ensemble[i].get_states();
    for(unsigned int k=0; k<states.size(); k++) {
      states_counters[states[k]]++;
      total_state_num++;
    }
  }

  // compute state probs and weight variance
  state_prob.insert(state_prob.begin(), N_, 0.0);
  // compute the probability of each state to appear in the MultiStateModels
  // (state_prob), it's average weight across models (weight_average)
  // and variance (weight_variance)
  for(unsigned int i=0; i<N_; i++) {
    if(states_counters[i] > 0) {
      if(states_counters[i] == 1) {
        state_prob[i] = 1.0/total_state_num;
      } else {
        state_prob[i] = states_counters[i]/(double)ensemble.size();
      }
    }
  }
}

void EnsembleGenerator::get_weights_average_and_std(const Ensemble& ensemble,
                    const Vector<saxs::WeightedFitParameters>& fps,
                    Vector<double>& weights_average,
                    Vector<double>& weights_variance) const {

  Vector<unsigned int> states_counters(N_, 0);
  Vector<Vector<double> > states_weights(N_);

  // count the number of occurences of each state in MultiStateModels
  // (states_counters) and store the weights (states_weights)
  for(unsigned int i=0; i<ensemble.size(); i++) {
    const Vector<unsigned int>& states = ensemble[i].get_states();
    const Vector<double>& weights = fps[i].get_weights();
    for(unsigned int k=0; k<states.size(); k++) {
      states_counters[states[k]]++;
      states_weights[states[k]].push_back(weights[k]);
    }
  }

  // compute weights average and variance for each state
  weights_average.insert(weights_average.begin(), N_, 0.0);
  weights_variance.insert(weights_variance.begin(), N_, 0.0);

  for(unsigned int i=0; i < N_; i++) {
    if(states_counters[i] > 0) {
      if(states_counters[i] == 1) {
        weights_average[i] = states_weights[i][0];
        weights_variance[i] = 1.0;
      } else {
        std::pair<double, double> av_std = get_average_and_stdev(states_weights[i]);
        weights_average[i] = av_std.first;
        weights_variance[i] = av_std.second;
      }
    }
  }
}

IMPMULTISTATE_END_NAMESPACE
