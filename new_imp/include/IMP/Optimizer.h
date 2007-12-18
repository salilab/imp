/**
 *  \file Optimizer.h     \brief Base class for all optimizers.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_OPTIMIZER_H
#define __IMP_OPTIMIZER_H

#include <limits>

#include "IMP_config.h"
#include "base_types.h"
#include "Object.h"
#include "utility.h"

namespace IMP
{

class Model;

//! Base class for all optimizers
class IMPDLLEXPORT Optimizer: public Object
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
    IMP_assert(model_ != NULL, 
               "get_model() called before set_model() "
               << " this can crash python");
    return model_;
  }

  //! Set the model being optimized
  /**
     \note The model is not owned by the optimizer and so is not
     deleted when the optimizer is deleted. Further, the Optimizer
     does not prevent the model from being deleted when all python
     references go away. 
   */
  void set_model(Model *m) {model_=m;}

private:
  Model *model_;
};

} // namespace IMP

#endif  /* __IMP_OPTIMIZER_H */
