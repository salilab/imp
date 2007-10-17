/**
 *  \file ConjugateGradients.h    \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONJUGATE_GRADIENTS_H
#define __IMP_CONJUGATE_GRADIENTS_H

#include "../IMP_config.h"
#include "Optimizer.h"

namespace IMP
{

//! Simple conjugate gradients optimizer.
/** Algorithm is as per Shanno and Phua, ACM Transactions On Mathematical
    Software 6 (December 1980), 618-622
 */ 
class IMPDLLEXPORT ConjugateGradients : public Optimizer
{
public:
  ConjugateGradients();
  virtual ~ConjugateGradients();

  //! Optimize the model.
  /** \param[in] model     Model that is being optimized.
      \param[in] max_steps Maximum number of iterations before aborting.
      \param[in] threshold Largest acceptable gradient-squared value
                           for convergence.
      \return score of the final state of the model.
   */
  virtual Float optimize(Model& model, int max_steps, Float threshold=0.0);

  virtual std::string version(void) const {
    return "0.1";
  }
  virtual std::string last_modified_by(void) const {
    return "Ben Webb";
  }
};

} // namespace IMP

#endif  /* __IMP_CONJUGATE_GRADIENTS_H */
