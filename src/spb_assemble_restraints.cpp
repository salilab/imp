/**
 *  \file spb_create_restraints.cpp
 *  \brief Assemble the restraints
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/core.h>
#include <IMP/atom.h>
#include <IMP/membrane.h>
#include <IMP/algebra.h>
#include <IMP/container.h>
#include <map>

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

std::map< std::string, Pointer<RestraintSet> > spb_assemble_restraints
(Model *m, SPBParameters mydata, atom::Hierarchies& all_mol,
 container::ListSingletonContainer *bCP_ps,
 container::ListSingletonContainer *CP_ps,
 container::ListSingletonContainer *IL2_ps)
{
// prepare the map of RestraintSet
std::map< std::string, Pointer<RestraintSet> > rst_map;
//
// Excluded volume
//
add_SPBexcluded_volume(m,all_mol,mydata.GFP_exc_volume,mydata.kappa);
//
// Symmetry
//
add_symmetry_restraint(m,all_mol,mydata.trs);
//
// Layer restraint
//
// CP and below
add_layer_restraint(m, bCP_ps,
 FloatRange(-1.0e+34,mydata.CP_thickness/2.0),mydata.kappa);
// inside CP
add_layer_restraint(m, CP_ps,
 FloatRange(-mydata.CP_thickness/2.0,mydata.CP_thickness/2.0),
 mydata.kappa);
// inside IL2
double dz=mydata.IL2_centers[0][2];
add_layer_restraint(m, IL2_ps,
 FloatRange(-mydata.IL2_thickness/2.0+dz,mydata.IL2_thickness/2.0+dz),
 mydata.kappa);
//
// TILT restraint
//
if(mydata.add_tilt && mydata.protein_list["Spc110p"]){
 add_tilt(m,all_mol[0],"Spc110p",mydata.tilt,mydata.kappa);
}
//
// FRET
//
if(mydata.add_fret && !mydata.use_new_fret_model){
// prepare the restraint set
 IMP_NEW(RestraintSet,fret,("FRET_R"));
// intra-CP
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Spc29p",   "C",
                      all_mol,     "Cmd1p",   "C", 1.69,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Spc29p",   "N",
                      all_mol,     "Cmd1p",   "C", 1.75,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0],  "Spc29p",  "C",
                      all_mol,    "Spc110p",  "C", 1.37,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Spc29p",   "C",
                      all_mol,    "Spc42p",   "N", 2.05,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0],  "Cmd1p",   "C",
                      all_mol,    "Spc42p",   "N", 2.07,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0],   "Cmd1p",  "C",
                      all_mol,    "Spc110p",  "C", 2.15,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0],  "Spc42p",  "N",
                      all_mol,    "Spc110p",  "C", 2.02,
                      mydata.kappa, mydata.add_GFP));
// inter CP-IL2
 fret->add_restraint(fret_restraint(m,
                      all_mol[0],  "Spc42p",  "C",
                      all_mol,    "Spc110p",  "C", 1.07,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Cnm67p", "C",
                      all_mol,     "Cmd1p", "C", 1.09,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Spc42p",   "C",
                      all_mol,     "Cmd1p",   "C", 1.1,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Cnm67p", "C",
                      all_mol,    "Spc29p", "C", 1.1,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Cnm67p", "C",
                      all_mol,    "Spc42p", "N", 1.13,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Spc42p",   "C",
                      all_mol,    "Spc29p",   "C", 1.17,
                      mydata.kappa, mydata.add_GFP));
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Spc42p",   "C",
                      all_mol,    "Spc42p",   "N", 1.27,
                      mydata.kappa, mydata.add_GFP));
// intra-IL2
 fret->add_restraint(fret_restraint(m,
                      all_mol[0], "Spc42p", "C",
                      all_mol,    "Cnm67p", "C", 2.29,
                      mydata.kappa, mydata.add_GFP));
// add the FRET_R restraints
 m->add_restraint(fret);
// add the RestraintSet FRET_R to map
 rst_map["FRET_R"]=fret;
}
if(mydata.add_fret && mydata.use_new_fret_model){
// prepare the restraint set
 IMP_NEW(RestraintSet,fret,("FRET_R"));
// intra-CP
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc29p", "C",
                          "Cmd1p",  "C", 1.7,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cmd1p",  "C",
                          "Spc29p", "C", 1.68,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cmd1p",  "C",
                          "Spc29p", "N", 1.75,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc29p",  "C",
                          "Spc110p", "C", 1.42,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc110p", "C",
                          "Spc29p",  "C", 1.32,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc29p", "C",
                          "Spc42p", "N", 2.24,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "N",
                          "Spc29p", "C", 1.86,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cmd1p",  "C",
                          "Spc42p", "N", 2.04,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "N",
                          "Cmd1p",  "C", 2.1,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cmd1p",   "C",
                          "Spc110p", "C", 1.94,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc110p", "C",
                          "Cmd1p",   "C", 2.37,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p",  "N",
                          "Spc110p", "C", 1.86,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc110p", "C",
                          "Spc42p",  "N", 2.18,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));

// inter CP-IL2
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "C",
                          "Spc110p","C",  1.08,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc110p", "C",
                          "Spc42p",  "C", 1.06,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cnm67p", "C",
                          "Cmd1p",  "C", 1.1,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cmd1p",  "C",
                          "Cnm67p", "C", 1.09,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "C",
                          "Cmd1p",  "C", 1.13,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cmd1p",  "C",
                          "Spc42p", "C", 1.06,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cnm67p", "C",
                          "Spc29p", "C", 1.1,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc29p", "C",
                          "Cnm67p", "C", 1.1,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cnm67p", "C",
                          "Spc29p", "N", 0.99,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cnm67p", "C",
                          "Spc42p",   "N", 1.15,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "N",
                          "Cnm67p", "C", 1.11,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "C",
                          "Spc29p", "C", 1.2,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc29p", "C",
                          "Spc42p", "C", 1.14,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "C",
                          "Spc29p", "N", 0.98,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "C",
                          "Spc42p", "N", 1.29,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "N",
                          "Spc42p", "C", 1.25,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cnm67p",  "C",
                          "Spc110p", "C", 1.02,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc110p", "C",
                          "Cnm67p",  "C", 1.05,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));

// intra-IL2
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Spc42p", "C",
                          "Cnm67p", "C", 2.06,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
 fret->add_restraint(NEW_fret_restraint(m, all_mol,
                          "Cnm67p", "C",
                          "Spc42p", "C", 2.52,
                          mydata.Fret, mydata.kappa, mydata.add_GFP));
// add the FRET_R restraints
 m->add_restraint(fret);
// add the RestraintSet FRET_R to map
 rst_map["FRET_R"]=fret;
}
//
// TWO-HYBRID SCREENING
//
if(mydata.add_y2h){
 // prepare the restraint set
 IMP_NEW(RestraintSet,y2h,("Y2H"));
// CP
 // substitued by link restraint
 //y2h->add_restraint(y2h_restraint(m,all_mol, "Cmd1p",      "ALL",
 //                    all_mol, "Spc110p",    IntRange(900,927), mydata.kappa));
 y2h->add_restraint(y2h_restraint(m,
                     all_mol[0],  "Spc42p",               "N",
                     all_mol,    "Spc110p",               "C", mydata.kappa));
 y2h->add_restraint(y2h_restraint(m,
                     all_mol[0],  "Spc29p",             "ALL",
                     all_mol,    "Spc110p", IntRange(811,944), mydata.kappa));
 y2h->add_restraint(y2h_restraint(m,
                     all_mol[0], "Spc110p",               "C",
                     all_mol,    "Spc110p",               "C", mydata.kappa));
 y2h->add_restraint(y2h_restraint(m,
                     all_mol[0],  "Spc42p",   IntRange(1,138),
                     all_mol,     "Spc29p",             "ALL", mydata.kappa));
// Having a rigid coiled-coil, this is always satisfied
// y2h->add_restraint(y2h_restraint(m,all_mol[0], "Spc42p",     IntRange(1,138),
//                     all_mol, "Spc42p",     IntRange(1,138), mydata.kappa));
// IL2
 y2h->add_restraint(y2h_restraint(m,
                     all_mol[0], "Cnm67p",             "C",
                     all_mol,    "Spc42p",             "C", mydata.kappa));

 // add the FRET_R restraints
 m->add_restraint(y2h);
 // add the RestraintSet Y2H to map
 rst_map["Y2H"]=y2h;
}
//
// Add link between Spc110p_C and Cmd1p
//
add_link(m,all_mol[0],   "Cmd1p",             "ALL",
             all_mol,    "Spc110p", IntRange(900,927),mydata.kappa);
//
// Add link with GFPs
//
if(mydata.add_GFP){
  add_link(m,all_mol[0],      "Spc110p", "C",
             all_mol,   "Spc110p-C-GFP", "N", mydata.kappa);
  add_link(m,all_mol[0],        "Cmd1p", "N",
             all_mol,     "Cmd1p-N-GFP", "C", mydata.kappa);
  add_link(m,all_mol[0],        "Cmd1p", "C",
             all_mol,     "Cmd1p-C-GFP", "N", mydata.kappa);
  add_link(m,all_mol[0],       "Spc42p", "N",
             all_mol,    "Spc42p-N-GFP", "C", mydata.kappa);
  add_link(m,all_mol[0],       "Spc42p", "C",
             all_mol,    "Spc42p-C-GFP", "N", mydata.kappa);
  add_link(m,all_mol[0],       "Spc29p", "N",
             all_mol,    "Spc29p-N-GFP", "C", mydata.kappa);
  add_link(m,all_mol[0],       "Spc29p", "C",
             all_mol,    "Spc29p-C-GFP", "N", mydata.kappa);
  add_link(m,all_mol[0],     "Cnm67p", "C",
             all_mol,  "Cnm67p-C-GFP", "N", mydata.kappa);
}

return rst_map;
}

IMPMEMBRANE_END_NAMESPACE
