/**
 *  \file spb_sampler.cpp
 *  \brief SPB MonteCarlo
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom.h>
#include <IMP/core.h>
#include <IMP/spb.h>
#include <iostream>

using namespace IMP;
using namespace IMP::spb;

IMPSPB_BEGIN_NAMESPACE

IMP::Pointer<core::MonteCarlo> setup_SPBMonteCarlo(Model *m,
                                                   core::MonteCarloMovers &mvs,
                                                   double temp,
                                                   SPBParameters myparam) {
  IMP::Pointer<core::MonteCarlo> mc;
  if (myparam.MC.do_wte) {
    double w0 = myparam.MC.wte_w0 * temp / myparam.MC.tmin;
    mc = new spb::MonteCarloWithWte(
        m, myparam.MC.wte_emin, myparam.MC.wte_emax, myparam.MC.wte_sigma,
        myparam.MC.wte_gamma, w0);
  } else {
    mc = new core::MonteCarlo(m);
  }
  mc->set_return_best(false);
  mc->set_kt(temp);
  mc->add_mover(new core::SerialMover(get_as<core::MonteCarloMoversTemp>(mvs)));
  return mc.release();
}

void add_BallMover(Particles ps, double dx, core::MonteCarloMovers &mvs) {
  for (unsigned int k = 0; k < ps.size(); ++k) {
    IMP_NEW(core::BallMover, bmv, (ps[k]->get_model(), ps[k]->get_index(), dx));
    mvs.push_back(bmv);
  }
}

void add_PbcBoxedMover(Particles ps, double dx, algebra::Vector3Ds centers,
                       algebra::Transformation3Ds trs,
                       core::MonteCarloMovers &mvs, Particle *SideXY,
                       Particle *SideZ) {
  IMP_NEW(spb::PbcBoxedMover, mv,
          (ps[0], ps, dx, centers, trs, SideXY, SideXY, SideZ));
  mvs.push_back(mv);
  for (unsigned int k = 1; k < ps.size(); ++k) {
    IMP_NEW(core::BallMover, bmv, (ps[k]->get_model(), ps[k]->get_index(), dx));
    mvs.push_back(bmv);
  }
}

void add_PbcBoxedRigidBodyMover(Particles ps, double dx, double dang,
                                algebra::Vector3Ds centers,
                                algebra::Transformation3Ds trs,
                                core::MonteCarloMovers &mvs, Particle *SideXY,
                                Particle *SideZ) {
  Particles fake;
  core::RigidBody rb = core::RigidMember(ps[0]).get_rigid_body();
  rb.set_coordinates_are_optimized(true);
  IMP_NEW(spb::PbcBoxedRigidBodyMover, rbmv,
          (rb, fake, dx, dang, centers, trs, SideXY, SideXY, SideZ));
  mvs.push_back(rbmv);
}

void add_NuisanceMover(Particle *p, double dp, core::MonteCarloMovers &mvs) {
  // put particle in a list
  Particles ps;
  ps.push_back(p);
  // put nuisance floatkey in a list
  FloatKeys fks;
  fks.push_back(FloatKey("nuisance"));
  IMP_NEW(core::NormalMover, mv, (ps, fks, dp));
  mvs.push_back(mv);
}

IMPSPB_END_NAMESPACE
