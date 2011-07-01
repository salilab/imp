/**
 *  \file MonteCarloWithWte.cpp
 *  \brief An exotic version of MonteCarlo
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/membrane/MonteCarloWithWte.h>
#include <IMP/core.h>

IMPMEMBRANE_BEGIN_NAMESPACE

MonteCarloWithWte::MonteCarloWithWte(Model *m, double gmin, double gmax,
                                     double sigma, double gamma):
  core::MonteCarlo(m) {
  min_   = gmin;
  max_   = gmax;
  sigma_ = sigma;
  gamma_ = gamma;
  dx_    = gamma / 3.0;
  nbin_  = ceil((gmax-gmin)/dx_)+1;
  bias_  = new double[nbin_];
  for (unsigned int i=0; i<nbin_; ++i) bias_[i] = 0.0;
  }

double MonteCarloWithWte::get_bias(double score)
{
  int index = floor((score-min_)/dx_);
  if (index < 0)       index=0;
  if (index >= nbin_)  index=nbin_-1;
  return bias_[index];
}

void MonteCarloWithWte::update_bias(double score)
{
  if(score < min_ || score > max_) return;
// first update Gaussian height
  double vbias=get_bias(score);
  double ww = exp(-vbias/(get_kt()*(gamma_-1)));
  for (unsigned int i = 0; i < nbin_; ++i){
   double xx = min_ + i * dx_;
   double dp = ( xx - score ) / sigma_;
   bias_[i] += ww * exp ( - 0.5 * dp * dp );
  }
}

void MonteCarloWithWte::do_step() {
  do_move(get_move_probability());
  double energy= evaluate(false);
  bool do_accept=do_accept_or_reject_move(energy+get_bias(energy));
  if(do_accept) update_bias(energy);
}

double MonteCarloWithWte::do_optimize(unsigned int max_steps) {
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  if (get_number_of_movers() ==0) {
    IMP_THROW("Running MonteCarlo without providing any"
              << " movers isn't very useful.",
              ValueException);
  }
  last_energy_  = evaluate(false);
  last_energy_ += get_bias(last_energy_);
  if (return_best_) {
    best_= new Configuration(get_model());
    best_energy_= last_energy_;
  }
  stat_forward_steps_taken_ = 0;
  stat_num_failures_ = 0;
  update_states();

  IMP_LOG(TERSE, "MC Initial energy is " << last_energy_ << std::endl);

  for (unsigned int i=0; i< max_steps; ++i) {
    if (get_stop_on_good_score() && get_model()->get_has_good_score()) {
      break;
    }
    do_step();
  }

  IMP_LOG(TERSE, "MC Final energy is " << last_energy_  << std::endl);
  if (return_best_) {
    //std::cout << "Final score is " << get_model()->evaluate(false)
    //<< std::endl;
    best_->load_configuration();
    IMP_LOG(TERSE, "MC Returning energy " << best_energy_ << std::endl);
    IMP_IF_CHECK(USAGE) {
      IMP_CHECK_CODE(double e= evaluate(false));
      IMP_LOG(TERSE, "MC Got " << e << std::endl);
      IMP_INTERNAL_CHECK(std::abs(best_energy_ - e)
                         < .01+.1* std::abs(best_energy_ +e),
                         "Energies do not match "
                         << best_energy_ << " vs " << e << std::endl);
    }
    return best_energy_;
  } else {
    return last_energy_;
  }
}

IMPMEMBRANE_END_NAMESPACE
