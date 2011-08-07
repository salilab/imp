/**
 *  \file membrane_sampler.cpp
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

Pointer<core::MonteCarlo> setup_MonteCarlo(Model *m,
 atom::Hierarchy protein,double temp,Parameters *myparam)
{
 MCParameters* MCp=&(myparam->MC);
 Pointer<core::MonteCarlo> mc;

 if (MCp->do_wte){
  double w0=MCp->wte_w0*temp/MCp->tmin;
  mc= new membrane::MonteCarloWithWte(m,MCp->wte_emin,MCp->wte_emax,
                                      MCp->wte_sigma,MCp->wte_gamma,w0);
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
  double dx_,dy_;
  if(i==0){
   dx_=0.0;
   dy_=0.0;
  } else if (i==1) {
   dx_=MCp->dx;
   dy_=0.0;
  } else {
   dx_=MCp->dx;
   dy_=MCp->dx;
  }
  IMP_NEW(membrane::RigidBodyNewMover,mv,(rb,dx_,dy_,MCp->dx,MCp->dang));
  mvs.push_back(mv);
 }
 IMP_NEW(core::SerialMover,mvmv,(mvs));
 mc->add_mover(mvmv);
 return mc.release();
}

IMPMEMBRANE_END_NAMESPACE
