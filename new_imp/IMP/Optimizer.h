/*
 *  Optimizer.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OPTIMIZER_H
#define __IMP_OPTIMIZER_H

#include "IMP_config.h"
#include "Base_Types.h"
#include "Model.h"

namespace imp
{

class IMPDLLEXPORT Optimizer
{
public:
  Optimizer();
  virtual ~Optimizer();

  virtual Float optimize(Model& model, int max_steps, Float threshold=0.0) = 0;
  virtual std::string version(void) {
    return "unknown";
  }
  virtual std::string last_modified_by(void) {
    return "unknown";
  }
};

class IMPDLLEXPORT Steepest_Descent : public Optimizer
{
public:
  Steepest_Descent();
  virtual ~Steepest_Descent();

  virtual Float optimize(Model& model, int max_steps, Float threshold=0.0);
  virtual std::string version(void) {
    return "0.5.0";
  }
  virtual std::string last_modified_by(void) {
    return "Bret Peterson";
  }

protected:
  Float step_size;
};

} // namespace imp

#endif  /* __IMP_OPTIMIZER_H */
