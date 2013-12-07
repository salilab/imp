/**
 *  \file cn_rmsd.h  \brief High-level functions for RMSD calculation.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCNMULTIFIT_CN_RMSD_H
#define IMPCNMULTIFIT_CN_RMSD_H

#include <IMP/cnmultifit/cnmultifit_config.h>
#include <IMP/base/types.h>
#include <string>

IMPCNMULTIFIT_BEGIN_NAMESPACE

Floats IMPCNMULTIFITEXPORT get_rmsd_for_models(const std::string param_filename,
                                               const std::string trans_filename,
                                               const std::string ref_filename,
                                               int start_model = 0,
                                               int end_model = -1);

IMPCNMULTIFIT_END_NAMESPACE

#endif /* IMPCNMULTIFIT_CN_RMSD_H */
