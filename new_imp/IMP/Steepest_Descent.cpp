/*
 *  Steepest_Descent.cpp
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#include "Steepest_Descent.h"
#include "log.h"

namespace imp
{

//######### Steepest Descent #########
// A simple steepest descent optimizer


/**
  Constructor
 */

Steepest_Descent::Steepest_Descent()
{
}


/**
  Destructor
 */

Steepest_Descent::~Steepest_Descent ()
{
}


/**
Follow the gradient based on the partial derivatives. Multiply by the current step size.
If the score gets worse, reduce the step size. If the score gets better, increase the step size
if we are sufficiently far from a score of zero. If a score of zero is reached, quit.

\param[in] model Model that is being optimized.
\param[in] variables Variables that may be optimizable.
\param[in] max_steps The maximum steps that should be take before giving up.

\return score of the final state of the model.
*/

Float Steepest_Descent::optimize (Model& model, int max_steps, Float threshold)
{
  std::vector<Float> temp_vals;
  std::vector<Float> temp_derivs;
  std::vector<Float_Index> float_indexes;
  Float last_score, new_score = 0.0;
  Model_Data* model_data;

  model_data = model.get_model_data();

  // set up the indexes
  int opt_var_cnt = 0;

  Opt_Float_Index_Iterator opt_float_iter;

  opt_float_iter.reset(model_data);
  while (opt_float_iter.next()) {
    float_indexes.push_back(opt_float_iter.get());
    opt_var_cnt++;
  }

  // ... and space for the old values
  step_size = 0.01;
  temp_vals.resize(opt_var_cnt);
  temp_derivs.resize(opt_var_cnt);

  for (int step = 0; step < max_steps; step++) {
    IMP_LOG(VERBOSE, "=== Step " << step << " ===");
    // model.show(std::cout);
    int cnt = 0;

    // evaluate the last model state
    last_score = model.evaluate(true);
    IMP_LOG(VERBOSE, "start score: " << last_score);

    // store the old values
    for (int i = 0; i < opt_var_cnt; i++) {
      Float_Index fi = float_indexes[0];

      temp_vals[i] = model_data->get_float(float_indexes[i]);
      temp_derivs[i] = model_data->get_deriv(float_indexes[i]);
    }

    bool done = false;
    bool not_changing = false;
    bool move_bigger; // try to increase step_size if we are moving to slowly down gradient
    while (!done && (cnt < 200)) {
      cnt++;
      move_bigger = true;

      // try new values based on moving down the gradient at the current step size
      for (int i = 0; i < opt_var_cnt; i++) {
        IMP_LOG(VERBOSE, i << " move: " << temp_vals[i] << " new: " << temp_vals[i] - temp_derivs[i] * step_size << "  " << temp_derivs[i]);

        model_data->set_float(float_indexes[i], temp_vals[i] - temp_derivs[i] * step_size);
      }

      // check the new model
      new_score = model.evaluate(false);
      IMP_LOG(VERBOSE, "last score: " << last_score << "  new score: " << new_score << "  step size: " << step_size);

      // if the score is less than the threshold, we're done
      if (new_score <= threshold)
        return new_score;

      // if the score got better, we'll take it
      if (new_score < last_score) {
        done = true;
        if (move_bigger)
          step_size *= 1.4;
      }

      // if the score is the same, keep going one more time
      else if (new_score == last_score) {
        if (not_changing)
          done = true;

        step_size *= 0.9;
        not_changing = true;
      }

      // if the score got worse, our step size was too big
      // ... make it smaller, and try again
      else {
        not_changing = false;
        move_bigger = false;
        step_size *= 0.71;
      }

      if (step_size == 0.0)
        return new_score;
    }
  }

  return new_score;
}


}  // namespace imp
