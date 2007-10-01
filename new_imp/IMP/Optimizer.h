/*
 *  Optimizer.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#if !defined(__optimizer_h)
#define __optimizer_h 1

#include "IMP_config.h"

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

#endif
