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

base::Pointer<core::MonteCarlo> setup_SPBMonteCarlo
 (Model *m, core::MonteCarloMovers& mvs, double temp, SPBParameters myparam)
{
 base::Pointer<core::MonteCarlo> mc;
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
 mc->add_mover(new core::SerialMover(get_as<core::MonteCarloMoversTemp>(mvs)));
 return mc.release();
}

void add_BallMover(Particles ps, double dx, core::MonteCarloMovers& mvs)
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
 algebra::Transformation3Ds trs, core::MonteCarloMovers& mvs,
 Particle *SideXY, Particle *SideZ)
{
 IMP_NEW(membrane::PbcBoxedMover,mv,
         (ps[0],ps,dx,centers,trs,SideXY,SideXY,SideZ));
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
 algebra::Transformation3Ds trs, core::MonteCarloMovers& mvs,
 Particle *SideXY, Particle *SideZ)
{
 Particles fake;
 core::RigidBody rb=core::RigidMember(ps[0]).get_rigid_body();
 IMP_NEW(membrane::PbcBoxedRigidBodyMover,rbmv,
         (rb,fake,dx,dang,centers,trs,SideXY,SideXY,SideZ));
 mvs.push_back(rbmv);
}

void add_NuisanceMover(Particle *p, double dp, core::MonteCarloMovers& mvs)
{
 // put particle in a list
 Particles ps;
 ps.push_back(p);
 // put nuisance floatkey in a list
 FloatKeys fks;
 fks.push_back(FloatKey("nuisance"));
 IMP_NEW(core::NormalMover,mv,(ps,fks,dp));
 mvs.push_back(mv);
}

IMPMEMBRANE_END_NAMESPACE
