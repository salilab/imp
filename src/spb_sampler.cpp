/**
 *  \file spb_sampler.cpp
 *  \brief SPB MonteCarlo
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <iostream>

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

core::MonteCarlo* setup_SPBMonteCarlo
 (Model *m, core::Movers& mvs, double temp, SPBParameters myparam)
{
 Pointer<core::MonteCarlo> mc;
 if (myparam.MC.do_wte){
  double w0=myparam.MC.wte_w0*temp/myparam.MC.tmin;
  mc= new membrane::MonteCarloWithWte(m,myparam.MC.wte_emin,
                                      myparam.MC.wte_emax,
                                      myparam.MC.wte_sigma,
                                      myparam.MC.wte_gamma,w0);
 }else{
  mc= new core::MonteCarlo(m);
 }
 mc->set_return_best(false);
 mc->set_kt(temp);

 IMP_NEW(core::SerialMover,mvmv,(mvs));
 mc->add_mover(mvmv);
 return mc.release();
}

IMPMEMBRANE_END_NAMESPACE
