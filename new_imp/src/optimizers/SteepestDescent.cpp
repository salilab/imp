/**
 *  \file SteepestDescent.cpp \brief Simple steepest descent optimizer.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "IMP/log.h"
#include "IMP/optimizers/SteepestDescent.h"

namespace IMP
{

//! Constructor
SteepestDescent::SteepestDescent()
{
}


//! Destructor
SteepestDescent::~SteepestDescent()
{
}


//! Optimize the model.
/** Follow the gradient based on the partial derivatives. Multiply by the
   current step size. If the score gets worse, reduce the step size.
   If the score gets better, increase the step size if we are sufficiently
   far from a score of zero. If the score reaches the threshold, quit.

   \param[in] max_steps The maximum steps that should be take before giving up.
   \return score of the final state of the model.
 */
Float SteepestDescent::optimize(unsigned int max_steps)
{
  std::vector<Float> temp_vals;
  std::vector<Float> temp_derivs;
  std::vector<FloatIndex> float_indexes;
  Float last_score, new_score = 0.0;
  ModelData* model_data;

  model_data = get_model()->get_model_data();

  // set up the indexes
  int opt_var_cnt = 0;

  OptFloatIndexIterator opt_float_iter;

  opt_float_iter.reset(model_data);
  while (opt_float_iter.next()) {
    float_indexes.push_back(opt_float_iter.get());
    opt_var_cnt++;
  }

  // ... and space for the old values
  step_size_ = 0.01;
  temp_vals.resize(opt_var_cnt);
  temp_derivs.resize(opt_var_cnt);

  for (unsigned int step = 0; step < max_steps; step++) {
    IMP_LOG(VERBOSE, "=== Step " << step << " ===");
    // model.show(std::cout);
    int cnt = 0;

    // evaluate the last model state
    last_score = get_model()->evaluate(true);
    IMP_LOG(VERBOSE, "start score: " << last_score);

    // store the old values
    for (int i = 0; i < opt_var_cnt; i++) {
      FloatIndex fi = float_indexes[0];

      temp_vals[i] = model_data->get_value(float_indexes[i]);
      temp_derivs[i] = model_data->get_deriv(float_indexes[i]);
    }

    bool done = false;
    bool not_changing = false;
    // try to increase step_size if we are moving to slowly down gradient
    bool move_bigger;
    while (!done && (cnt < 200)) {
      cnt++;
      move_bigger = true;

      // try new values based on moving down the gradient at the current
      // step size
      for (int i = 0; i < opt_var_cnt; i++) {
        IMP_LOG(VERBOSE, i << " move: " << temp_vals[i] << " new: "
                << temp_vals[i] - temp_derivs[i] * step_size_ << "  "
                << temp_derivs[i]);

        model_data->set_value(float_indexes[i],
                              temp_vals[i] - temp_derivs[i] * step_size_);
      }

      // check the new model
      new_score = get_model()->evaluate(false);
      IMP_LOG(VERBOSE, "last score: " << last_score << "  new score: "
              << new_score << "  step size: " << step_size_);

      // if the score is less than the threshold, we're done
      if (new_score <= threshold_)
        return new_score;

      // if the score got better, we'll take it
      if (new_score < last_score) {
        done = true;
        if (move_bigger)
          step_size_ *= 1.4;
      }

      // if the score is the same, keep going one more time
      else if (new_score == last_score) {
        if (not_changing)
          done = true;

        step_size_ *= 0.9;
        not_changing = true;
      }

      // if the score got worse, our step size was too big
      // ... make it smaller, and try again
      else {
        not_changing = false;
        move_bigger = false;
        step_size_ *= 0.71;
      }

      if (step_size_ == 0.0)
        return new_score;
    }
  }

  return new_score;
}


}  // namespace IMP
