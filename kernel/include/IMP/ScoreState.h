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
#include "VersionInfo.h"
#include "utility.h"

#include <vector>
#include <iostream>

namespace IMP
{

class Model;

//! Shared score state.
/** ScoreStates should be used to generate state that needs to be
    updated every time Particle attributes change. 

    ScoreStates can change the state of particles and restraints.
    However, optimizers may not pick up new particles or changes
    to whether particular attributes are optimized or not.

 */
class IMPDLLEXPORT ScoreState : public internal::Object
{
  friend class Model;
  void set_model(Model* model);

public:
  ScoreState(std::string name=std::string());
  virtual ~ScoreState();

  // Update the state given the current state of the model
  virtual void update() = 0;

  //! Show the ScoreState
  /** The output of show may take several lines and should end in a newline.
   */
  virtual void show(std::ostream& out = std::cout) const;

  //! \return version and authorship information.
  virtual VersionInfo get_version_info() const {
    return VersionInfo("unknown", "unknown");
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
