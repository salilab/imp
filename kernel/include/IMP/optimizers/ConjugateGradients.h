/**
 *  \file ConjugateGradients.h    \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONJUGATE_GRADIENTS_H
#define __IMP_CONJUGATE_GRADIENTS_H

#include "../IMP_config.h"
#include "../Optimizer.h"

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

  IMP_OPTIMIZER("0.1", "Ben Webb");
  //! Set the threshold for the minimum gradient
  void set_threshold(Float t){ threshold_=t;}
private:
  Float threshold_;
};

} // namespace IMP

#endif  /* __IMP_CONJUGATE_GRADIENTS_H */
