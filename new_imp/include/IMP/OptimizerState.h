/**
 *  \file OptimizerState.h   \brief Shared optimizer state.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OPTIMIZER_STATE_H
#define __IMP_OPTIMIZER_STATE_H

#include <iostream>

#include "IMP_config.h"
#include "Object.h"

namespace IMP
{

class Optimizer;

//! Shared optimizer state.
class IMPDLLEXPORT OptimizerState : public Object
{
  friend class Optimizer;
  void set_optimizer(Optimizer* optimizer);

public:
  OptimizerState(std::string name=std::string());
  virtual ~OptimizerState();

  // Update the state given the current state of the optimizer
  virtual void update() = 0;

  virtual void show(std::ostream& out = std::cout) const;
  virtual std::string version() const {
    return "unknown";
  }
  virtual std::string last_modified_by() const {
    return "unknown";
  }
  //! Get the name of the state
  const std::string& get_name() const {
    return name_;
  }
  //! Set the name of the state
  void set_name(const std::string &name) {
    name_=name;
  }

  //! return the stored optimizer
  Optimizer *get_optimizer() const {
    IMP_assert(optimizer_ != NULL,
               "Must call set_optimizer before get_optimizer on state");
    return optimizer_;
  }
protected:
  //! Stored optimizer
  Optimizer* optimizer_;

  std::string name_;
};


inline std::ostream &operator<<(std::ostream &out, const OptimizerState &s)
{
  s.show(out);
  return out;
}

} // namespace IMP

#endif  /* __IMP_OPTIMIZER_STATE_H */
