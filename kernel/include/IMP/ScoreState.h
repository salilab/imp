/**
 *  \file ScoreState.h   \brief Shared score state.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_SCORE_STATE_H
#define __IMP_SCORE_STATE_H

#include <iostream>

#include "IMP_config.h"
#include "Object.h"

namespace IMP
{

class Model;

//! Shared score state.
class IMPDLLEXPORT ScoreState : public Object
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
    IMP_assert(model_ != NULL,
               "Must call set_model before get_model on state");
    return model_;
  }
protected:
  // all of the particle data
  Model* model_;

  std::string name_;
};


inline std::ostream &operator<<(std::ostream &out, const ScoreState &s)
{
  s.show(out);
  return out;
}

} // namespace IMP

#endif  /* __IMP_SCORE_STATE_H */
