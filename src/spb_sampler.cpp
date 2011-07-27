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

// create movers
// first hierarchy is CP
 Particles ps0=hs[0].get_leaves();
 for(unsigned int j=0;j<ps0.size();++j){
   IMP_NEW(membrane::BoxedMover,mv, (ps0[j],mc_dx_,myparam.side,
                                     myparam.CP_thickness,
                                     algebra::Vector3D(0.0,0.0,0.0),
                                     myparam.cell_type));
   mc->add_mover(mv);
 }

// second hierarchy is IL2
/*
 Particles ps1=hs[1].get_leaves();
 double dz=
  myparam.CP_IL2_gap+myparam.CP_thickness/2.0+myparam.IL2_thickness/2.0;
 for(unsigned int j=0;j<ps1.size();++j){
   IMP_NEW(membrane::BoxedMover,mv, (ps1[j],mc_dx_,myparam.side,
                                     myparam.IL2_thickness,
                                     algebra::Vector3D(0.0,0.0,dz),
                                     myparam.cell_type));
   mc->add_mover(mv);
 }
*/
 return mc.release();
}

IMPMEMBRANE_END_NAMESPACE
