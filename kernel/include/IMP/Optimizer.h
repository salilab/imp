/**
 *  \file Optimizer.h     \brief Base class for all optimizers.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OPTIMIZER_H
#define __IMP_OPTIMIZER_H

#include <limits>

#include "IMP_config.h"
#include "base_types.h"
#include "internal/Object.h"
#include "utility.h"
#include "Model.h"
#include "internal/ObjectPointer.h"

namespace IMP
{

class OptimizerState;

typedef std::vector<OptimizerState*> OptimizerStates;

/** \defgroup optimizer Optimizers
    Optimizers of various sorts.
  */

//! Base class for all optimizers
class IMPDLLEXPORT Optimizer: public internal::Object
{
public:
  Optimizer();
  virtual ~Optimizer();

  /** Optimize the model
      \param[in] max_steps The maximum number of steps to take.
      \return The final score.
   */       
  virtual Float optimize(unsigned int max_steps) = 0;
  /** \return the current version*/
  virtual std::string version() const =0;
  /** \return the last person to modify this restraint */
  virtual std::string last_modified_by() const=0;

  //! Get the model being optimized
  Model *get_model() const {
    return model_.get();
  }

  //! Set the model being optimized
  /**
     \note The model is not owned by the optimizer and so is not
     deleted when the optimizer is deleted. Further, the Optimizer
     does not prevent the model from being deleted when all python
     references go away. 
   */
  void set_model(Model *m) {model_=m;}

  IMP_CONTAINER(OptimizerState, optimizer_state, OptimizerStateIndex);

protected:
  //! Update optimizer state, should be called at each successful step
  void update_states();

private:
  internal::ObjectPointer<Model, false> model_;

};

} // namespace IMP

#endif  /* __IMP_OPTIMIZER_H */
