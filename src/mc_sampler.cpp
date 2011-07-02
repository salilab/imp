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
 atom::Hierarchy protein, Parameters *myparam)
{
 IMP_NEW(core::MonteCarlo,mc,(m));
 mc->set_return_best(false);
 for(int i=0;i<myparam->TM.num;++i){
  atom::Selection s=atom::Selection(protein);
  s.set_molecule(myparam->TM.name[i]);
  core::RigidBody rb=
  core::RigidMember(s.get_selected_particles()[0]).get_rigid_body();
  if(i==0){
   IMP_NEW(membrane::RigidBodyNewMover,mv,(rb,0.0,0.0,mc_dx_,mc_dang_));
   mc->add_mover(mv);
  }
  if(i==1){
   IMP_NEW(membrane::RigidBodyNewMover,mv,(rb,mc_dx_,0.0,mc_dx_,mc_dang_));
   mc->add_mover(mv);
  }
  if(i>1){
   IMP_NEW(membrane::RigidBodyNewMover,mv,(rb,mc_dx_,mc_dx_,mc_dx_,mc_dang_));
   mc->add_mover(mv);
  }
 }
 return mc.release();
}

IMPMEMBRANE_END_NAMESPACE
