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
// check which particles are members of a rigid body
  std::vector<unsigned> irigid,inotrigid;
  for(unsigned int k=0;k<ps.size();++k){
   if(core::RigidMember::particle_is_instance(ps[k])){
    irigid.push_back(k);
   } else {
    inotrigid.push_back(k);
   }
  }
// if this protein is not a single rigid body
  if(inotrigid.size()>0){
  // choose a special particle
   IMP_NEW(membrane::PbcBoxedMover,mv,
          (ps[inotrigid[0]],mc_dx_,myparam.CP_centers,myparam.trs));
   mvs.push_back(mv);
  //for the other particles standard BallMover
   for(unsigned int k=1;k<inotrigid.size();++k) {
    Particles pps;
    pps.push_back(ps[inotrigid[k]]);
    IMP_NEW(core::BallMover,bmv,(pps,mc_dx_));
    mvs.push_back(bmv);
   }
  // rigidbody mover if needed
   if(irigid.size()>0){
    core::RigidBody prb=core::RigidMember(ps[irigid[0]]).get_rigid_body();
    IMP_NEW(core::RigidBodyMover,rbmv,(prb,mc_dx_,mc_dang_));
    mvs.push_back(rbmv);
   }
  } else {
// this protein is a single rigid body
   core::RigidBody prb=core::RigidMember(ps[irigid[0]]).get_rigid_body();
   IMP_NEW(membrane::PbcBoxedRigidBodyMover,prbmv,
    (prb,mc_dx_,mc_dang_,myparam.CP_centers,myparam.trs));
   mvs.push_back(prbmv);
  }
 }

// second hierarchy is IL2
// TO DO

 IMP_NEW(core::SerialMover,mvmv,(mvs));
 mc->add_mover(mvmv);
 return mc.release();
}

IMPMEMBRANE_END_NAMESPACE
