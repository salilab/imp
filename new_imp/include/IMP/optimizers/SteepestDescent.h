/**
 *  \file SteepestDescent.h   \brief Simple steepest descent optimizer.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_STEEPEST_DESCENT_H
#define __IMP_STEEPEST_DESCENT_H

#include "../IMP_config.h"
#include "Optimizer.h"

namespace IMP
{

//! A simple steepest descent optimizer
class IMPDLLEXPORT SteepestDescent : public Optimizer
{
public:
  SteepestDescent();
  virtual ~SteepestDescent();

  //! Optimize the model.
  /** Follow the gradient based on the partial derivatives. Multiply by the
      current step size. If the score gets worse, reduce the step size.
      If the score gets better, increase the step size if we are sufficiently
      far from a score of zero. If the score reaches the threshold, quit.

      \param[in] model Model that is being optimized.
      \param[in] max_steps The maximum steps that should be take before
                           giving up.
      \param[in] threshold Terminate optimization when score drops to
                           this value.
      \return score of the final state of the model.
   */
  virtual Float optimize(Model& model, int max_steps, Float threshold=0.0);

  virtual std::string version() {
    return "0.5.0";
  }
  virtual std::string last_modified_by() {
    return "Bret Peterson";
  }

protected:
  Float step_size;
};

} // namespace IMP

#endif  /* __IMP_STEEPEST_DESCENT_H */
