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
(Model *m, atom::Hierarchies& all_mol,
 container::ListSingletonContainer *bCP_ps,
 container::ListSingletonContainer *CP_ps,
 container::ListSingletonContainer *IL2_ps,
 SPBParameters myparam)
{
//
// Excluded volume
//
add_SPBexcluded_volume(m,all_mol,myparam.kappa);
//
// Symmetry
//
add_symmetry_restraint(m,all_mol,myparam.trs);
//
// Layer restraint
//
// CP and below
add_layer_restraint(m, bCP_ps,
 FloatRange(-1.0e+34,myparam.CP_thickness/2.0),myparam.kappa);
// inside CP
add_layer_restraint(m, CP_ps,
 FloatRange(-myparam.CP_thickness/2.0,myparam.CP_thickness/2.0),
 myparam.kappa);
// inside IL2
double dz=myparam.IL2_centers[0][2];
add_layer_restraint(m, IL2_ps,
 FloatRange(-myparam.IL2_thickness/2.0+dz,myparam.IL2_thickness/2.0+dz),
 myparam.kappa);
//
// TILT restraint
//
if(myparam.add_tilt && myparam.protein_list["Spc110p"]){
 add_tilt(m,all_mol[0],"Spc110p",myparam.tilt,myparam.kappa);
}
//
// FRET
//
if(myparam.add_fret){
// intra-CP
 add_fret_restraint(m,all_mol[0], "Spc29p",   "C",
                      all_mol,     "Cmd1p",   "C", 1.69,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc29p",   "N",
                      all_mol,     "Cmd1p",   "C", 1.75,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0],  "Spc29p",  "C",
                      all_mol,    "Spc110p",  "C", 1.37,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc29p",   "C",
                      all_mol,    "Spc42p",   "N", 2.05,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0],  "Cmd1p",   "C",
                      all_mol,    "Spc42p",   "N", 2.07,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0],   "Cmd1p",  "C",
                      all_mol,    "Spc110p",  "C", 2.15,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0],  "Spc42p",  "N",
                      all_mol,    "Spc110p",  "C", 2.02,
                      myparam.kappa, myparam.add_GFP);
// inter CP-IL2

 add_fret_restraint(m,all_mol[0],  "Spc42p",  "C",
                      all_mol,    "Spc110p",  "C", 1.07,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0], "Cnm67p_c", "C",
                      all_mol,       "Cmd1p", "C", 1.09,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc42p",   "C",
                      all_mol,     "Cmd1p",   "C", 1.1,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0], "Cnm67p_c", "C",
                      all_mol,      "Spc29p", "C", 1.1,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0], "Cnm67p_c", "C",
                      all_mol,      "Spc42p", "N", 1.13,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc42p",   "C",
                      all_mol,    "Spc29p",   "C", 1.17,
                      myparam.kappa, myparam.add_GFP);
 add_fret_restraint(m,all_mol[0], "Spc42p",   "C",
                      all_mol,    "Spc42p",   "N", 1.27,
                      myparam.kappa, myparam.add_GFP);

// intra-IL2
 add_fret_restraint(m,all_mol[0],   "Spc42p", "C",
                      all_mol,    "Cnm67p_c", "C", 2.29,
                      myparam.kappa, myparam.add_GFP);
}
//
// TWO-HYBRID SCREENING
//
if(myparam.add_y2h){
// CP
 // substitued by link restraint
 //add_y2h_restraint(m,all_mol, "Cmd1p",      "ALL",
 //                    all_mol, "Spc110p",    IntRange(900,927), myparam.kappa);
 add_y2h_restraint(m,all_mol[0],  "Spc42p",               "N",
                     all_mol,    "Spc110p",               "C", myparam.kappa);
 add_y2h_restraint(m,all_mol[0],  "Spc29p",             "ALL",
                     all_mol,    "Spc110p", IntRange(811,944), myparam.kappa);
 add_y2h_restraint(m,all_mol[0], "Spc110p",               "C",
                     all_mol,    "Spc110p",               "C", myparam.kappa);
 add_y2h_restraint(m,all_mol[0],  "Spc42p",   IntRange(1,138),
                     all_mol,     "Spc29p",             "ALL", myparam.kappa);
// Having a rigid coiled-coil, this is always satisfied
// add_y2h_restraint(m,all_mol[0], "Spc42p",     IntRange(1,138),
//                     all_mol, "Spc42p",     IntRange(1,138), myparam.kappa);
// IL2
 add_y2h_restraint(m,all_mol[0], "Cnm67p_c",             "C",
                     all_mol,      "Spc42p",             "C", myparam.kappa);
}
//
// Add link between Spc110p_C and Cmd1p
//
add_link(m,all_mol[0],   "Cmd1p",             "ALL",
             all_mol,    "Spc110p", IntRange(900,927),myparam.kappa);
//
// Add link with GFPs
//
if(myparam.add_GFP){
  add_link(m,all_mol[0],      "Spc110p", "C",
             all_mol,   "Spc110p-C-GFP", "N", myparam.kappa);
  add_link(m,all_mol[0],        "Cmd1p", "N",
             all_mol,     "Cmd1p-N-GFP", "C", myparam.kappa);
  add_link(m,all_mol[0],        "Cmd1p", "C",
             all_mol,     "Cmd1p-C-GFP", "N", myparam.kappa);
  add_link(m,all_mol[0],       "Spc42p", "N",
             all_mol,    "Spc42p-N-GFP", "C", myparam.kappa);
  add_link(m,all_mol[0],       "Spc42p", "C",
             all_mol,    "Spc42p-C-GFP", "N", myparam.kappa);
  add_link(m,all_mol[0],       "Spc29p", "N",
             all_mol,    "Spc29p-N-GFP", "C", myparam.kappa);
  add_link(m,all_mol[0],       "Spc29p", "C",
             all_mol,    "Spc29p-C-GFP", "N", myparam.kappa);
  add_link(m,all_mol[0],     "Cnm67p_c", "C",
             all_mol,  "Cnm67p_c-C-GFP", "N", myparam.kappa);
}
}

IMPMEMBRANE_END_NAMESPACE
