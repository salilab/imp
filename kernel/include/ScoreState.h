/**
 *  \file ScoreState.h   \brief Shared score state.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_SCORE_STATE_H
#define __IMP_SCORE_STATE_H

#include "IMP_config.h"
#include "RefCountedObject.h"
#include "Pointer.h"
#include "Model.h"
#include "DerivativeAccumulator.h"
#include "VersionInfo.h"
#include "utility.h"

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

    The ScoreState base class has an iteration counter built in
    so that a given score state will only be updated once per
    Model::evaluate call, even if many other ScoreStates ask that
    it be updated. To use this projection mechanism, inherit from
    ScoreState and provide implementations of do_before_evaluate()
    and do_after_evaluate(). Or, better yet, use the IMP_SCORESTATE
    macro.

    \note When logging is VERBOSE, state should print enough information
    in evaluate to reproduce the the entire flow of data in update. When
    logging is TERSE the restraint should print out only a constant number
    of lines per update call.

 */
class IMPDLLEXPORT ScoreState : public RefCountedObject
{
  friend class Model;
  void set_model(Model* model);

public:
  ScoreState(std::string name=std::string());
  virtual ~ScoreState();

  //! Update if needed
  /** The protected do_before_evaluate method will be called if the iteration
      count has not yet been seen.
   */
  void before_evaluate(unsigned int iteration);

  //! Do post evaluation work if needed
  /** The protected do_after_evaluate method will be called if needed.
   */
  void after_evaluate(unsigned int iteration,
                      DerivativeAccumulator *accpt);


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
  //! Update the state given the current state of the model.
  /** This is also called prior to every calculation of the model score.
      It should be implemented by ScoreStates in order to provide functionality.

      \note This can't have the same name as the public function due to the
      way C++ handles overloading and name lookups--if only one is implemented
      in the child class it will only find that one.
   */
  virtual void do_before_evaluate() = 0;

  //! Do any necessary updates after the model score is calculated.
  /** \param[in] accpt The object used to scale derivatives in the score
                       calculation, or NULL if derivatives were not requested.
   */
  virtual void do_after_evaluate(DerivativeAccumulator *accpt) {}

  //! Get the current iteration count value.
  /** The value is updated before update() is called
   */
  unsigned int get_before_evaluate_iteration() const {
    return update_iteration_;
  }

  //! Get the current after_evaluate iteration count value.
  /** The value is updated before after_evaluate() is called
   */
  unsigned int get_after_evaluate_iteration() const {
    return after_iteration_;
  }

 private:

  unsigned int update_iteration_;
  unsigned int after_iteration_;
  // all of the particle data
  Pointer<Model> model_;
  std::string name_;
};

IMP_OUTPUT_OPERATOR(ScoreState);

} // namespace IMP

#endif  /* __IMP_SCORE_STATE_H */
