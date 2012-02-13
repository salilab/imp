/**
 *  \file SteepestDescent.cpp \brief Simple steepest descent optimizer.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/SteepestDescent.h>

#include <IMP/log.h>
#include <IMP/Model.h>

IMPCORE_BEGIN_NAMESPACE

//! Constructor
SteepestDescent::SteepestDescent(Model *m) :
    step_size_(0.01),
    max_step_size_(std::numeric_limits<double>::max()),
    threshold_(0.)
{
  if (m) set_model(m);
}
void SteepestDescent::do_show(std::ostream &) const {
}


double SteepestDescent::do_optimize(unsigned int max_steps)
{
  IMP_OBJECT_LOG;
  Float last_score, new_score = 0.0;

  // set up the indexes


  FloatIndexes float_indexes=get_optimized_attributes();
  int opt_var_cnt = float_indexes.size();

  Float current_step_size = step_size_;

  // ... and space for the old values
  algebra::VectorKD temp_derivs(Floats(opt_var_cnt, 0));

  for (unsigned int step = 0; step < max_steps; step++) {
    // model.show(std::cout);
    int cnt = 0;

    // evaluate the last model state
    last_score = evaluate(true);
    IMP_LOG(TERSE, "start score: " << last_score << std::endl);

    // store the old values
    for (int i = 0; i < opt_var_cnt; i++) {
      FloatIndex fi = float_indexes[i];

      temp_derivs[i] = get_scaled_derivative(fi);
    }
    temp_derivs= temp_derivs.get_unit_vector();

    bool done = false;
    bool not_changing = false;
    // try to increase step_size if we are moving to slowly down gradient
    bool move_bigger;
    while (!done && (cnt < 200)) {
      cnt++;
      move_bigger = true;

      // try new values based on moving down the gradient at the current
      // step size

      IMP_LOG(VERBOSE, "step: "
              << temp_derivs * current_step_size << std::endl);
      for (int i = 0; i < opt_var_cnt; i++) {

        set_scaled_value(float_indexes[i],
                         get_scaled_value(float_indexes[i]) - temp_derivs[i]
                         * current_step_size);
      }

      // check the new model
      new_score = evaluate(false);
      IMP_LOG(TERSE, "last score: " << last_score << "  new score: "
              << new_score << "  step size: " << current_step_size
              << std::endl);

      // if the score is less than the threshold, we're done
      if (new_score <= threshold_) {
        update_states();
        IMP_LOG(TERSE, "Below threshold, returning." << std::endl);
        return new_score;
      }

      // if the score got better, we'll take it
      if (new_score < last_score) {
        done = true;
        update_states();
        if (move_bigger) {
          current_step_size= std::min(current_step_size* 1.4,
                                      max_step_size_);
        }
      }

      // if the score is the same, keep going one more time
      else if (std::abs(new_score- last_score) < .0000001) {
        if (not_changing) {
          done = true;
        }

        current_step_size *= 0.9;
        not_changing = true;
      }

      // if the score got worse, our step size was too big
      // ... make it smaller, and try again
      else {
        not_changing = false;
        move_bigger = false;
        current_step_size *= 0.71;
      }

      if (current_step_size <.00000001) {
        update_states();
        IMP_LOG(TERSE, "Unable to make progress, returning." << std::endl);
        return new_score;
      }
    }
    update_states();
  }

  return new_score;
}

IMPCORE_END_NAMESPACE
