/**
 *  \file spb_ISD_particles.cpp
 *  \brief SPB ISD particles
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/isd.h>
#include <IMP/membrane.h>
#include <string>
#include <map>

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

std::map<std::string, base::Pointer<Particle> > add_ISD_particles
 (Model *m, SPBParameters mydata, core::MonteCarloMovers& mvs)
{

std::map<std::string, base::Pointer<Particle> > ISD_ps;
IMP_NEW(NuisanceRangeModifier,nrm,());

if(mydata.add_fret){

 // kda particle
 IMP_NEW(Particle,pKda,(m));
 // initial value
 Float Kda_0=(mydata.Fret.KdaMin+mydata.Fret.KdaMax)/2.;
 isd::Scale Kda=isd::Scale::setup_particle(pKda,Kda_0);
 Kda.set_lower(mydata.Fret.KdaMin);
 Kda.set_upper(mydata.Fret.KdaMax);
 // IMP_NEW(core::SingletonConstraint,sc0,(nrm,NULL,Kda)); // deprecated!
  IMP_NEW(core::SingletonConstraint,sc0,(nrm,NULL,m,Kda->get_index()));
 m->add_score_state(sc0);
 Kda->set_is_optimized(Kda.get_nuisance_key(),true);
 // add mover
 add_NuisanceMover(Kda,mydata.MC.dKda,mvs);
 // add particle to map
 ISD_ps["Kda"]=pKda;

 // Ida particle
 IMP_NEW(Particle,pIda,(m));
 isd::Scale Ida=isd::Scale::setup_particle(pIda,mydata.Fret.Ida);
 Ida.set_lower(1.0);
 Ida.set_upper(mydata.Fret.Ida+4.0*mydata.Fret.IdaErr);
 // IMP_NEW(core::SingletonConstraint,sc1,(nrm,NULL,Ida));
 IMP_NEW(core::SingletonConstraint,sc1,(nrm,NULL,m,Ida->get_index()));
 m->add_score_state(sc1);
 Ida->set_is_optimized(Ida.get_nuisance_key(),true);
 // add mover
 add_NuisanceMover(Ida,mydata.MC.dIda,mvs);
 // add Gaussian restraint on Ida
 IMP_NEW(isd::GaussianRestraint,gr,(Ida,mydata.Fret.Ida,mydata.Fret.IdaErr));
 m->add_restraint(gr);
 // add particle to map
 ISD_ps["Ida"]=pIda;

 // Sigma0 particle
 IMP_NEW(Particle,pSigma0,(m));
 // initial value
 Float Sigma0_0=(mydata.Fret.Sigma0Min+mydata.Fret.Sigma0Max)/2.;
 isd::Scale Sigma0=isd::Scale::setup_particle(pSigma0,Sigma0_0);
 Sigma0.set_lower(mydata.Fret.Sigma0Min);
 Sigma0.set_upper(mydata.Fret.Sigma0Max);
// IMP_NEW(core::SingletonConstraint,sc2,(nrm,NULL,Sigma0));
 IMP_NEW(core::SingletonConstraint,sc2,(nrm,NULL,m,Sigma0->get_index()));
 m->add_score_state(sc2);
 Sigma0->set_is_optimized(Sigma0.get_nuisance_key(),true);
 // add mover
 add_NuisanceMover(Sigma0,mydata.MC.dSigma0,mvs);
 // add particle to map
 ISD_ps["Sigma0"]=pSigma0;

 // R0 particle
 IMP_NEW(Particle,pR0,(m));
 // initial value
 Float R0_0=(mydata.Fret.R0Min+mydata.Fret.R0Max)/2.;
 isd::Scale R0=isd::Scale::setup_particle(pR0,R0_0);
 R0.set_lower(mydata.Fret.R0Min);
 R0.set_upper(mydata.Fret.R0Max);
// IMP_NEW(core::SingletonConstraint,sc3,(nrm,NULL,R0));
 IMP_NEW(core::SingletonConstraint,sc3,(nrm,NULL,m,R0->get_index()));
 m->add_score_state(sc3);
 R0->set_is_optimized(R0.get_nuisance_key(),true);
 // add mover
 add_NuisanceMover(R0,mydata.MC.dR0,mvs);
 // add particle to map
 ISD_ps["R0"]=pR0;

 // Pbl particle
 IMP_NEW(Particle,ppBl,(m));
 // initial value
 Float pBl_0=(mydata.Fret.pBlMin+mydata.Fret.pBlMax)/2.;
 isd::Scale pBl=isd::Scale::setup_particle(ppBl, pBl_0);
 pBl.set_lower(mydata.Fret.pBlMin);
 pBl.set_upper(mydata.Fret.pBlMax);
// IMP_NEW(core::SingletonConstraint,sc4,(nrm,NULL,pBl));
 IMP_NEW(core::SingletonConstraint,sc4,(nrm,NULL,m,pBl->get_index()));
 m->add_score_state(sc4);
 pBl->set_is_optimized(pBl.get_nuisance_key(),true);
 // add mover
 add_NuisanceMover(pBl,mydata.MC.dpBl,mvs);
 // add particle to map
 ISD_ps["pBl"]=ppBl;
}

// CP_A particle
IMP_NEW(Particle,pA,(m));
// initial value
isd::Scale A=isd::Scale::setup_particle(pA,-mydata.CP_thicknessMax);
A.set_lower(-mydata.CP_thicknessMax);
A.set_upper(-mydata.CP_thicknessMin);
//IMP_NEW(core::SingletonConstraint,sc5,(nrm,NULL,A));
IMP_NEW(core::SingletonConstraint,sc5,(nrm,NULL,m,A->get_index()));
m->add_score_state(sc5);
A->set_is_optimized(A.get_nuisance_key(),true);
// add mover
add_NuisanceMover(A,mydata.MC.dA,mvs);
// add particle to map
ISD_ps["CP_A"]=pA;

// CP_B particle
IMP_NEW(Particle,pB,(m));
// initial value
isd::Scale B=isd::Scale::setup_particle(pB,0.0);
B.set_lower(0.0);
B.set_upper(0.0);
B->set_is_optimized(B.get_nuisance_key(),false);
// add particle to map
ISD_ps["CP_B"]=pB;

// SideXY particle
IMP_NEW(Particle,pSideXY,(m));
// initial value
isd::Scale SideXY=
 isd::Scale::setup_particle(pSideXY,mydata.sideMax/mydata.sideMin);
SideXY.set_lower(1.0);
SideXY.set_upper(mydata.sideMax/mydata.sideMin);
//IMP_NEW(core::SingletonConstraint,sc6,(nrm,NULL,SideXY));
IMP_NEW(core::SingletonConstraint,sc6,(nrm,NULL,m,SideXY->get_index()));
m->add_score_state(sc6);
SideXY->set_is_optimized(SideXY.get_nuisance_key(),true);
// add particle to map
ISD_ps["SideXY"]=pSideXY;

// SideZ particle
IMP_NEW(Particle,pSideZ,(m));
// initial value
isd::Scale SideZ=isd::Scale::setup_particle(pSideZ,1.0);
SideZ.set_lower(1.0);
SideZ.set_upper(1.0);
SideZ->set_is_optimized(SideZ.get_nuisance_key(),false);
// add particle to map
ISD_ps["SideZ"]=pSideZ;

// GAP_A particle
IMP_NEW(Particle,pC,(m));
// initial value
isd::Scale C=isd::Scale::setup_particle(pC,mydata.CP_IL2_gapMax);
C.set_lower(mydata.CP_IL2_gapMin);
C.set_upper(mydata.CP_IL2_gapMax);
//IMP_NEW(core::SingletonConstraint,sc7,(nrm,NULL,C));
IMP_NEW(core::SingletonConstraint,sc7,(nrm,NULL,m,C->get_index()));
m->add_score_state(sc7);
C->set_is_optimized(C.get_nuisance_key(),true);
// add mover only if we are applying the Bayesian layer
if(mydata.add_IL2_layer){add_NuisanceMover(C,mydata.MC.dA,mvs);}
// add particle to map
ISD_ps["GAP_A"]=pC;

// GAP_B particle
IMP_NEW(Particle,pD,(m));
// initial value
Float IL2_end = mydata.CP_IL2_gapMax + mydata.IL2_thickness;
isd::Scale D=isd::Scale::setup_particle(pD,IL2_end);
D.set_lower(IL2_end);
D.set_upper(IL2_end);
D->set_is_optimized(D.get_nuisance_key(),false);
// add particle to map
ISD_ps["GAP_B"]=pD;

// if using new fret data
if(mydata.add_new_fret){
 // kda_new particle
 IMP_NEW(Particle,pKda_new,(m));
 // initial value
 Float Kda_new_0=(mydata.Fret.KdaMin_new+mydata.Fret.KdaMax_new)/2.;
 isd::Scale Kda_new=isd::Scale::setup_particle(pKda_new,Kda_new_0);
 Kda_new.set_lower(mydata.Fret.KdaMin_new);
 Kda_new.set_upper(mydata.Fret.KdaMax_new);
// IMP_NEW(core::SingletonConstraint,sc8,(nrm,NULL,Kda_new));
 IMP_NEW(core::SingletonConstraint,sc8,(nrm,NULL,m,Kda_new->get_index()));
 m->add_score_state(sc8);
 Kda_new->set_is_optimized(Kda_new.get_nuisance_key(),true);
 // add mover
 //add_NuisanceMover(Kda_new,mydata.MC.dKda,mvs);
 // add particle to map
 ISD_ps["Kda_new"]=pKda_new;

 // Ida particle
 IMP_NEW(Particle,pIda_new,(m));
 isd::Scale Ida_new=isd::Scale::setup_particle(pIda_new,mydata.Fret.Ida_new);
 Ida_new.set_lower(1.0);
 Ida_new.set_upper(mydata.Fret.Ida_new+4.0*mydata.Fret.IdaErr_new);
// IMP_NEW(core::SingletonConstraint,sc9,(nrm,NULL,Ida_new));
 IMP_NEW(core::SingletonConstraint,sc9,(nrm,NULL,m,Ida_new->get_index()));
 m->add_score_state(sc9);
 Ida_new->set_is_optimized(Ida_new.get_nuisance_key(),true);
 // add mover
 //add_NuisanceMover(Ida_new,mydata.MC.dIda,mvs);
 // add Gaussian restraint on Ida_new
 //IMP_NEW(isd::GaussianRestraint,gr,
 // (Ida_new,mydata.Fret.Ida_new,mydata.Fret.IdaErr_new));
 //m->add_restraint(gr);
 // add particle to map
 ISD_ps["Ida_new"]=pIda_new;
}

return ISD_ps;
}

IMPMEMBRANE_END_NAMESPACE
