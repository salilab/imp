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

using namespace IMP;
using namespace IMP::membrane;

IMPMEMBRANE_BEGIN_NAMESPACE

void spb_assemble_restraints
(Model *m, SPBParameters mydata, atom::Hierarchies& all_mol,
 container::ListSingletonContainer *bCP_ps,
 container::ListSingletonContainer *CP_ps,
 container::ListSingletonContainer *IL2_ps)
{
//
// Excluded volume
//
add_SPBexcluded_volume(m,all_mol,mydata.kappa);
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
if(mydata.add_fret){
// intra-CP
 add_fret_restraint(m,all_mol[0], "Spc29p",   "C",
                      all_mol,     "Cmd1p",   "C", 1.69,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc29p",   "N",
                      all_mol,     "Cmd1p",   "C", 1.75,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0],  "Spc29p",  "C",
                      all_mol,    "Spc110p",  "C", 1.37,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc29p",   "C",
                      all_mol,    "Spc42p",   "N", 2.05,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0],  "Cmd1p",   "C",
                      all_mol,    "Spc42p",   "N", 2.07,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0],   "Cmd1p",  "C",
                      all_mol,    "Spc110p",  "C", 2.15,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0],  "Spc42p",  "N",
                      all_mol,    "Spc110p",  "C", 2.02,
                      mydata.kappa, mydata.add_GFP);
// inter CP-IL2

 add_fret_restraint(m,all_mol[0],  "Spc42p",  "C",
                      all_mol,    "Spc110p",  "C", 1.07,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Cnm67p_c", "C",
                      all_mol,       "Cmd1p", "C", 1.09,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc42p",   "C",
                      all_mol,     "Cmd1p",   "C", 1.1,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Cnm67p_c", "C",
                      all_mol,      "Spc29p", "C", 1.1,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Cnm67p_c", "C",
                      all_mol,      "Spc42p", "N", 1.13,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc42p",   "C",
                      all_mol,    "Spc29p",   "C", 1.17,
                      mydata.kappa, mydata.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc42p",   "C",
                      all_mol,    "Spc42p",   "N", 1.27,
                      mydata.kappa, mydata.add_GFP);

// intra-IL2
 add_fret_restraint(m,all_mol[0],   "Spc42p", "C",
                      all_mol,    "Cnm67p_c", "C", 2.29,
                      mydata.kappa, mydata.add_GFP);
}
//
// TWO-HYBRID SCREENING
//
if(mydata.add_y2h){
// CP
 // substitued by link restraint
 //add_y2h_restraint(m,all_mol, "Cmd1p",      "ALL",
 //                    all_mol, "Spc110p",    IntRange(900,927), mydata.kappa);
 add_y2h_restraint(m,all_mol[0],  "Spc42p",               "N",
                     all_mol,    "Spc110p",               "C", mydata.kappa);
 add_y2h_restraint(m,all_mol[0],  "Spc29p",             "ALL",
                     all_mol,    "Spc110p", IntRange(811,944), mydata.kappa);
 add_y2h_restraint(m,all_mol[0], "Spc110p",               "C",
                     all_mol,    "Spc110p",               "C", mydata.kappa);
 add_y2h_restraint(m,all_mol[0],  "Spc42p",   IntRange(1,138),
                     all_mol,     "Spc29p",             "ALL", mydata.kappa);
// Having a rigid coiled-coil, this is always satisfied
// add_y2h_restraint(m,all_mol[0], "Spc42p",     IntRange(1,138),
//                     all_mol, "Spc42p",     IntRange(1,138), mydata.kappa);
// IL2
 add_y2h_restraint(m,all_mol[0], "Cnm67p_c",             "C",
                     all_mol,      "Spc42p",             "C", mydata.kappa);
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
  add_link(m,all_mol[0],     "Cnm67p_c", "C",
             all_mol,  "Cnm67p_c-C-GFP", "N", mydata.kappa);
}

}

IMPMEMBRANE_END_NAMESPACE
