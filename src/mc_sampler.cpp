/**
 *  \file mc_sampler.cpp
 *  \brief Membrane MonteCarlo
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

core::MonteCarlo* setup_MonteCarlo(Model *m,
 atom::Hierarchy protein,double temp,Parameters *myparam)
{
 MCParameters *MC=&(myparam->MC);
 double mc_dx_=MC->dx;
 double mc_dang_=MC->dang;

 Pointer<core::MonteCarlo> mc;
 if (MC->do_wte){
  double w0=MC->wte_w0*temp/MC->tmin;
  mc= new membrane::MonteCarloWithWte(m,MC->wte_emin,MC->wte_emax,
                                      MC->wte_sigma,MC->wte_gamma,w0);
 }else{
  mc= new core::MonteCarlo(m);
 }
 mc->set_return_best(false);
 core::Movers mvs;
 for(int i=0;i<myparam->TM.num;++i){
  atom::Selection s=atom::Selection(protein);
  s.set_molecule(myparam->TM.name[i]);
  core::RigidBody rb=
  core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
  if(i==0){
   IMP_NEW(membrane::RigidBodyNewMover,mv,(rb,0.0,0.0,mc_dx_,mc_dang_));
   mvs.push_back(mv);
  }
  if(i==1){
   IMP_NEW(membrane::RigidBodyNewMover,mv,(rb,mc_dx_,0.0,mc_dx_,mc_dang_));
   mvs.push_back(mv);
  }
  if(i>1){
   IMP_NEW(membrane::RigidBodyNewMover,mv,(rb,mc_dx_,mc_dx_,mc_dx_,mc_dang_));
   mvs.push_back(mv);
  }
 }
 IMP_NEW(membrane::MoversMover,mvmv,(mvs));
 mc->add_mover(mvmv);
 return mc.release();
}

IMPMEMBRANE_END_NAMESPACE
