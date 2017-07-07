/**
 *  \file  pt_stuff.h
 *  \brief Useful stuff for parallel tempering
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSPB_PT_STUFF_H
#define IMPSPB_PT_STUFF_H
#include <IMP.h>
#include "spb_config.h"

IMPSPB_BEGIN_NAMESPACE

IMPSPBEXPORT double* create_temperatures(double tmin, double tmax,
                                              int nrep);

IMPSPBEXPORT int* create_indexes(int nrep);

IMPSPBEXPORT int get_friend(int* index, int myrank, int step, int nrep);

IMPSPBEXPORT bool get_acceptance(double score0, double score1,
                                      double delta_wte, double T0, double T1);

IMPSPB_END_NAMESPACE

#endif /* IMPSPB_PT_STUFF_H */
