/**
 *  \file LogOptimizerState.h
 *  \brief Write geometry to a file during optimization
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMPDISPLAY_LOG_OPTIMIZER_STATE_H
#define IMPDISPLAY_LOG_OPTIMIZER_STATE_H

#include "display_config.h"
#include "Writer.h"
#include <IMP/OptimizerState.h>
#include <IMP/SingletonContainer.h>
#include <IMP/display/geometry.h>
#include <IMP/Pointer.h>
#include <IMP/FailureHandler.h>
#include <IMP/internal/utility.h>
#include <vector>

IMPDISPLAY_BEGIN_NAMESPACE


IMP_MODEL_SAVE(Write, (Writer *writer, std::string file_name),
               mutable IMP::internal::OwnerPointer<Writer> writer_;
               Geometries gdata_;,
                writer_=writer;,
                void add_geometry(Geometry* g) {
                  gdata_.push_back(g);
                  g->set_was_used(true);
                }
                void add_geometry(const Geometries& g) {
                  for (unsigned int i=0; i< g.size(); ++i) {
                    add_geometry(g);
                  }
                },
                {
                  IMP_LOG(TERSE, "Writing log file " << file_name << std::endl);
                  writer_->set_output(file_name);
                  for (unsigned int i=0; i < gdata_.size(); ++i) {
                    writer_->add_geometry(gdata_[i]);
                  }
                  writer_->close();
                });

#ifndef IMP_DOXYGEN
typedef WriteOptimizerState LogOptimizerState;
typedef WriteFailureHandler DisplayModelOnFailure;
#endif

IMPDISPLAY_END_NAMESPACE

#endif  /* IMPDISPLAY_LOG_OPTIMIZER_STATE_H */
