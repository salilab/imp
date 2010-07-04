/**
 *  \file WriteModelOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_WRITE_MODEL_OPTIMIZER_STATE_H
#define IMPCORE_WRITE_MODEL_OPTIMIZER_STATE_H

#include "core_config.h"
#include <IMP/OptimizerState.h>
#include <IMP/internal/utility.h>
#include <IMP/io.h>

IMPCORE_BEGIN_NAMESPACE

//! Write the model to a file each n time steps.
/**
    \ingroup logging
 */
class IMPCOREEXPORT WriteModelOptimizerState: public OptimizerState
{
  Pointer<Model> model_;
  std::string name_template_;
  Pointer<Particle> p_;
  FloatKey k_;
public:
  //! Write files using name_template as a template (must have a %1% in it)
  WriteModelOptimizerState(Model *m, std::string name_template);
  //! label the files witha  value
  void set_file_value(Particle *p, FloatKey k) {
    p_=p;
    k_=k;
  }

  IMP_PERIODIC_OPTIMIZER_STATE(WriteModelOptimizerState);
};


IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_WRITE_MODEL_OPTIMIZER_STATE_H */
