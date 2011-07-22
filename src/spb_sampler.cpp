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

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

core::MonteCarlo* setup_SPBMonteCarlo
 (Model *m,atom::Hierarchy h,SPBParameters myparam)
{
 MCParameters MC=myparam.MC;
 double mc_dx_=MC.dx;
 double mc_dang_=MC.dang;

 Pointer<core::MonteCarlo> mc;
 if (MC.do_wte){
  double w0=MC.wte_w0*temp/MC.tmin;
  mc= new membrane::MonteCarloWithWte(m,MC.wte_emin,MC.wte_emax,
                                      MC.wte_sigma,MC.wte_gamma,w0);
 }else{
  mc= new core::MonteCarlo(m);
 }
 mc->set_return_best(false);

// create movers
 Particles ps=h.get_leaves();
 IMP_NEW(container::ListSingletonContainer,lsc,(ps));

 FloatKeys keys;
 keys.push_back(FloatKey("x"));
 keys.push_back(FloatKey("y"));
 keys.push_back(FloatKey("z"));

 IMP_NEW(core::NormalMover,mv,(lsc,keys,mc_dx_));
 mc->add_mover(mv);

 return mc.release();
}

IMPMEMBRANE_END_NAMESPACE
