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
 (Model *m,atom::Hierarchies hs,double temp,MCParameters myparam)
{
 double mc_dx_=myparam.dx;
 double mc_dang_=myparam.dang;

 Pointer<core::MonteCarlo> mc;
 if (myparam.do_wte){
  double w0=myparam.wte_w0*temp/myparam.tmin;
  mc= new membrane::MonteCarloWithWte(m,myparam.wte_emin,myparam.wte_emax,
                                      myparam.wte_sigma,myparam.wte_gamma,w0);
 }else{
  mc= new core::MonteCarlo(m);
 }
 mc->set_return_best(false);

// create movers
 IMP_NEW(container::ListSingletonContainer,lsc,(m));
 for(int i=0;i<hs.size();++i){
  Particles ps=hs[i].get_leaves();
  lsc->add_particles(ps);
 }

 FloatKeys keys;
 keys.push_back(FloatKey("x"));
 keys.push_back(FloatKey("y"));
 keys.push_back(FloatKey("z"));

 IMP_NEW(core::NormalMover,mv,(lsc,keys,mc_dx_));
 mc->add_mover(mv);

 return mc.release();
}

IMPMEMBRANE_END_NAMESPACE
