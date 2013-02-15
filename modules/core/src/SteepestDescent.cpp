/**
 *  \file SteepestDescent.cpp \brief Simple steepest descent optimizer.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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

  Float current_step_size = step_size_;

  // ... and space for the old values
  algebra::VectorKD temp_derivs(Floats(float_indexes.size(), 0));
  algebra::VectorKD temp_values(Floats(float_indexes.size(), 0));

  for (unsigned int step = 0; step < max_steps; step++) {
    // model.show(std::cout);
    int cnt = 0;

    // evaluate the last model state
    last_score = get_scoring_function()->evaluate(true);
    IMP_LOG_TERSE( "start score: " << last_score << std::endl);

    // store the old values
    for (unsigned int i = 0; i < temp_derivs.get_dimension(); i++) {
      temp_derivs[i] = get_derivative(float_indexes[i]);
      temp_values[i] = get_value(float_indexes[i]);
    }

    bool constant_score=false;
    while (true) {
      cnt++;
      // try new values based on moving down the gradient at the current
      // step size

      IMP_LOG_VERBOSE( "step: "
              << temp_derivs * current_step_size << std::endl);
      for (unsigned int i = 0; i < float_indexes.size(); i++) {
        set_value(float_indexes[i],
                  temp_values[i] - temp_derivs[i]
                  * current_step_size);
      }

      // check the new model
      new_score = get_scoring_function()->evaluate(false);
      IMP_LOG_TERSE( "last score: " << last_score << "  new score: "
              << new_score << "  step size: " << current_step_size
              << std::endl);

      // if the score got better, we'll take it
      if (new_score < last_score) {
        IMP_LOG_TERSE( "Accepting step of size "
                << current_step_size);
        update_states();
        if (new_score <= threshold_) {
           IMP_LOG_TERSE( "Below threshold, returning." << std::endl);
           return new_score;
        }
        current_step_size= std::min(current_step_size* 1.4,
                                      max_step_size_);
        break;
      }

      // if the score is the same, keep going one more time
      if (std::abs(new_score- last_score) < .0000001) {
        if (constant_score) {
            break;
        }

        current_step_size *= 0.9;
        constant_score = true;
      } else {
        constant_score=false;
        current_step_size*=.7;
      }
      if (cnt>200) {
        // stuck
        for (unsigned int i = 0; i < float_indexes.size(); i++) {
          set_value(float_indexes[i],
                    temp_values[i]);
        }
        IMP_LOG_TERSE( "Unable to find a good step. Returning" << std::endl);
        return  last_score;
      }
      if (current_step_size <.00000001) {
        // here is as good as any place we found
        update_states();
        IMP_LOG_TERSE( "Unable to make progress, returning." << std::endl);
        return new_score;
      }
    }
  }

  return new_score;
}

IMPCORE_END_NAMESPACE
