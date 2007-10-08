/*
 *  Conjugate_Gradients.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONJUGATE_GRADIENTS_H
#define __IMP_CONJUGATE_GRADIENTS_H

#include "IMP_config.h"
#include "Optimizer.h"

namespace imp
{

class IMPDLLEXPORT Conjugate_Gradients : public Optimizer
{
public:
  Conjugate_Gradients();
  virtual ~Conjugate_Gradients();

  virtual Float optimize(Model& model, int max_steps, Float threshold=0.0);
  virtual std::string version(void) {
    return "0.1";
  }
  virtual std::string last_modified_by(void) {
    return "Ben Webb";
  }
};

} // namespace imp

#endif  /* __IMP_CONJUGATE_GRADIENTS_H */
