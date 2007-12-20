/**
 *  \file State.h   \brief Shared state.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_STATE_H
#define __IMP_STATE_H

#include <iostream>

#include "IMP_config.h"
#include "Object.h"

namespace IMP
{

class Model;

//! Shared state.
class IMPDLLEXPORT State : public Object
{
  friend class Model;
  void set_model(Model* model);

public:
  State(std::string name=std::string());
  virtual ~State();

  // Update the state given the current state of the model
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

  //! return the stored model data
  Model *get_model() const {
    IMP_assert(model_ != NULL,
               "Must call set_model before get_model on state");
    return model_;
  }
protected:
  // all of the particle data
  Model* model_;

  std::string name_;
};


inline std::ostream &operator<<(std::ostream &out, const State &s)
{
  s.show(out);
  return out;
}

} // namespace IMP

#endif  /* __IMP_STATE_H */
