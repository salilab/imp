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

Pointer<core::MonteCarlo> setup_SPBMonteCarlo
 (Model *m, const core::MoversTemp& mvs, double temp, SPBParameters myparam)
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

void add_BallMover(Particles ps, double dx, core::Movers& mvs)
{
 for(unsigned int k=0;k<ps.size();++k){
  Particles pps;
  pps.push_back(ps[k]);
  IMP_NEW(core::BallMover,bmv,(pps,dx));
  mvs.push_back(bmv);
 }
}

void add_PbcBoxedMover
(Particles ps, double dx, algebra::Vector3Ds centers,
 algebra::Transformation3Ds trs, core::Movers& mvs)
{
 IMP_NEW(membrane::PbcBoxedMover,mv,(ps[0],ps,dx,centers,trs));
 mvs.push_back(mv);
 for(unsigned int k=1;k<ps.size();++k){
  Particles pps;
  pps.push_back(ps[k]);
  IMP_NEW(core::BallMover,bmv,(pps,dx));
  mvs.push_back(bmv);
 }
}

void add_PbcBoxedRigidBodyMover
(Particles ps,double dx,double dang,algebra::Vector3Ds centers,
 algebra::Transformation3Ds trs, core::Movers& mvs)
{
 Particles fake;
 core::RigidBody prb=core::RigidMember(ps[0]).get_rigid_body();
 IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,(prb,fake,dx,dang,centers,trs));
 mvs.push_back(rbmv);
}

IMPMEMBRANE_END_NAMESPACE
