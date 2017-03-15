/**
 *  \file  pt_stuff.h
 *  \brief Useful stuff for parallel tempering
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMEMBRANE_PT_STUFF_H
#define IMPMEMBRANE_PT_STUFF_H
#include "membrane_config.h"
#include <IMP.h>

IMPMEMBRANE_BEGIN_NAMESPACE

IMPMEMBRANEEXPORT double*
 create_temperatures(double tmin,double tmax,int nrep);

IMPMEMBRANEEXPORT int* create_indexes(int nrep);

IMPMEMBRANEEXPORT int get_friend(int* index,int myrank,int step,int nrep);

IMPMEMBRANEEXPORT bool get_acceptance(double score0,double score1,
                       double delta_wte,double T0,double T1);

IMPMEMBRANE_END_NAMESPACE

#endif  /* IMPMEMBRANE_PT_STUFF_H */
