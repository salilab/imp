/**
 *  \file SaveModelOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_SAVE_MODEL_OPTIMIZER_STATE_H
#define IMPCORE_SAVE_MODEL_OPTIMIZER_STATE_H

#include "core_config.h"
#include <IMP/OptimizerState.h>
#include <IMP/internal/utility.h>
#include <IMP/io.h>

IMPCORE_BEGIN_NAMESPACE

//! Save the model to a file each n time steps.
/**
    \ingroup logging
 */
class IMPCOREEXPORT SaveModelOptimizerState: public OptimizerState
{
  Pointer<Model> model_;
  std::string name_template_;

public:
  //! Write files using name_template as a template (must have a %d in it)
  SaveModelOptimizerState(Model *m, std::string name_template);

  IMP_PERIODIC_OPTIMIZER_STATE(SaveModelOptimizerState);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_SAVE_MODEL_OPTIMIZER_STATE_H */
