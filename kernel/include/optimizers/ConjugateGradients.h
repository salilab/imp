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
#include "../internal/kernel_version_info.h"

namespace IMP
{

//! Simple conjugate gradients optimizer.
/** Algorithm is as per Shanno and Phua, ACM Transactions On Mathematical
    Software 6 (December 1980), 618-622
    \ingroup optimizer
 */
class IMPDLLEXPORT ConjugateGradients : public Optimizer
{
public:
  ConjugateGradients();
  virtual ~ConjugateGradients();

  IMP_OPTIMIZER(internal::kernel_version_info)

  //! Set the threshold for the minimum gradient
  void set_threshold(Float t){ threshold_=t;}

  //! Limit how far anything can change each time step
  void set_max_change(Float t) { max_change_ = t; }
private:

  Float get_score(std::vector<FloatIndex> float_indices,
                  std::vector<Float> &x, std::vector<Float> &dscore);
  bool line_search(std::vector<Float> &x, std::vector<Float> &dx,
                   float &alpha, const std::vector<FloatIndex> &float_indices,
                   int &ifun, float &f, float &dg, float &dg1,
                   int max_steps, const std::vector<Float> &search,
                   const std::vector<Float> &estimate);
  Float threshold_;
  Float max_change_;
};

} // namespace IMP

#endif  /* __IMP_CONJUGATE_GRADIENTS_H */
