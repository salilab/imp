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
 (Model *m,atom::Hierarchies hs, double temp, SPBParameters myparam)
{
 double mc_dx_=myparam.MC.dx;
 double mc_dang_=myparam.MC.dang;

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

// create movers
 core::Movers mvs;
// first hierarchy hs[0] is CP
 atom::Hierarchies hhs=hs[0].get_children();
 for(unsigned int j=0;j<hhs.size();++j){
  Particles ps=atom::get_leaves(hhs[j]);
// particle 0 is special
  IMP_NEW(membrane::PbcBoxedMover,mv,
         (ps[0],mc_dx_,myparam.CP_centers,myparam.trs));
  mvs.push_back(mv);
 //for the other particles standard BallMover
  for(unsigned int k=1;k<ps.size();++k) {
   Particles pps;
   pps.push_back(ps[k]);
   IMP_NEW(core::BallMover,bmv,(pps,mc_dx_));
   mvs.push_back(bmv);
  }
 }

// second hierarchy is IL2
// TO DO

 IMP_NEW(membrane::MoversMover,mvmv,(mvs));
 mc->add_mover(mvmv);
 return mc.release();
}

IMPMEMBRANE_END_NAMESPACE
