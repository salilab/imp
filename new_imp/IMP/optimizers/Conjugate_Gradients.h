/**
 *  \file Conjugate_Gradients.h    Simple conjugate gradients optimizer.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONJUGATE_GRADIENTS_H
#define __IMP_CONJUGATE_GRADIENTS_H

#include "../IMP_config.h"
#include "Optimizer.h"

namespace imp
{

/** Simple conjugate gradients optimizer, as per Shanno and Phua,
    ACM Transactions On Mathematical Software 6 (December 1980), 618-622
 */ 
class IMPDLLEXPORT Conjugate_Gradients : public Optimizer
{
public:
  Conjugate_Gradients();
  virtual ~Conjugate_Gradients();

  virtual Float optimize(Model& model, int max_steps, Float threshold=0.0);
  virtual std::string version(void) const {
    return "0.1";
  }
  virtual std::string last_modified_by(void) const {
    return "Ben Webb";
  }
};

} // namespace imp

#endif  /* __IMP_CONJUGATE_GRADIENTS_H */
