/**
 *  \file isd/HybridMonteCarlo.cpp
 *  \brief The hybrid monte carlo algorithm
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/HybridMonteCarlo.h>
#include <math.h>

IMPISD_BEGIN_NAMESPACE

HybridMonteCarlo::HybridMonteCarlo(atom::MolecularDynamics *md, 
        unsigned steps) : MonteCarlo(md->get_model()), num_md_steps_(steps),
        md_(md) {}





IMPISD_END_NAMESPACE
