/**
 *  \file ScoreState.h   \brief Shared score state.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_SCORE_STATE_H
#define __IMP_SCORE_STATE_H

#include "IMP_config.h"
#include "internal/Object.h"
#include "internal/ObjectPointer.h"
#include "Model.h"
#include "utility.h"

#include <vector>
#include <iostream>

namespace IMP
{

class Model;

//! Shared score state.
class IMPDLLEXPORT ScoreState : public internal::Object
{
  friend class Model;
  void set_model(Model* model);

public:
  ScoreState(std::string name=std::string());
  virtual ~ScoreState();

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
    IMP_assert(model_,
               "Must call set_model before get_model on state");
    return model_.get();
  }
protected:
  // all of the particle data
  internal::ObjectPointer<Model, false> model_;

  std::string name_;
};

IMP_OUTPUT_OPERATOR(ScoreState);

} // namespace IMP

#endif  /* __IMP_SCORE_STATE_H */
