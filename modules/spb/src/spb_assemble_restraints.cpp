/**
 *  \file spb_create_restraints.cpp
 *  \brief Assemble the restraints
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/algebra.h>
#include <IMP/atom.h>
#include <IMP/container.h>
#include <IMP/core.h>
#include <IMP/spb.h>
#include <fstream>
#include <map>

using namespace IMP;
using namespace IMP::spb;

IMPSPB_BEGIN_NAMESPACE

std::map<std::string, IMP::Pointer<RestraintSet> > spb_assemble_restraints(
    Model *m, RestraintSet *allrs, SPBParameters &mydata,
    atom::Hierarchies &all_mol, container::ListSingletonContainer *CP_ps,
    container::ListSingletonContainer *IL2_ps,
    std::map<std::string, IMP::Pointer<Particle> > ISD_ps) {

  CP_ps->set_was_used(true);
  IL2_ps->set_was_used(true);

  // prepare the map of RestraintSet
  std::map<std::string, IMP::Pointer<RestraintSet> > rst_map;
  //
  // Excluded volume
  //
  add_SPBexcluded_volume(m, allrs, all_mol, mydata.GFP_exc_volume,
                         mydata.kappa_vol);
  //
  // Symmetry
  add_symmetry_restraint(m, all_mol, mydata.trs, ISD_ps["SideXY"],
                         ISD_ps["SideZ"]);
  //
  // Layer restraints
  //
  // inside CP
  add_bayesian_layer_restraint(allrs, CP_ps, ISD_ps["CP_A"], ISD_ps["CP_B"]);
  // inside IL2
  if (mydata.add_IL2_layer) {
    add_bayesian_layer_restraint(allrs, IL2_ps, ISD_ps["GAP_A"],
                                 ISD_ps["GAP_B"]);
  }
  //
  // TILT restraint
  //

  if (mydata.add_tilt) {
    double kappa_tilt = 100.0 * mydata.kappa;
    if (mydata.protein_list["Spc110p"]) {
      add_tilt(m, allrs, all_mol[0], "Spc110p", IntRange(700, 705), mydata.tilt,
               kappa_tilt);
    }
    if (mydata.protein_list["Cnm67p"]) {
      add_tilt(m, allrs, all_mol[0], "Cnm67p", IntRange(1, 573), mydata.tilt,
               kappa_tilt);
    }
    if (mydata.protein_list["Spc42p"]) {
      // the maximum tilt angle is determined by the length of coiled coil
      // domain
      // (111Ang) and the minimum gap misured (108Ang-2*sigma)=79Ang ->
      // tilt_max=45deg
      add_tilt(m, allrs, all_mol[0], "Spc42p", IntRange(60, 130),
               mydata.tilt_Spc42, kappa_tilt);
    }
  }

  //
  // Add stay on plane for Spc42
  //
  if (mydata.protein_list["Spc42p"]) {
    // last bead before CC
    add_stay_on_plane_restraint(m, allrs, all_mol[0], "Spc42p", 50,
                                mydata.kappa);
    // first bead after CC
    // add_stay_on_plane_restraint(m,all_mol[0],"Spc42p",150, mydata.kappa);
    // need more after CC
    // add_stay_on_plane_restraint(m,all_mol[0],"Spc42p",250, mydata.kappa);
    // add_stay_on_plane_restraint(m,all_mol[0],"Spc42p",350, mydata.kappa);

    // if adding to single terminus bead
    add_stay_on_plane_restraint(m, allrs, all_mol[0], "Spc42p", 360,
                                mydata.kappa);
  }
  //
  // FRET
  //
  if (mydata.add_fret) {
    // prepare the restraint set
    IMP_NEW(RestraintSet, fret, (m, "FRET_R"));
    // temporary variables
    std::string name_d, ter_d, name_a, ter_a;
    double fexp, sexp;
    // open fret file
    std::ifstream fretfile;
    fretfile.open(mydata.Fret.filename.c_str());
    while (fretfile >> name_d >> ter_d >> name_a >> ter_a >> fexp >> sexp) {
      fret->add_restraint(fret_restraint(
          all_mol, name_d, ter_d, name_a, ter_a, fexp, mydata.Fret,
          mydata.cell_type, mydata.add_GFP, ISD_ps["Kda"], ISD_ps["Ida"],
          ISD_ps["R0"], ISD_ps["Sigma0"], ISD_ps["pBl"]));
    }
    // close file
    fretfile.close();
    // add new fret
    if (mydata.add_new_fret) {
      // temporary variables
      std::string name_d, ter_d, name_a, ter_a;
      double fexp, sexp;
      // open new_fret file
      std::ifstream newfretfile;
      newfretfile.open(mydata.Fret.filename_new.c_str());
      while (newfretfile >> name_d >> ter_d >> name_a >> ter_a >> fexp >>
             sexp) {
        // try conversion to integer
        int int_d = atoi(ter_d.c_str());
        int int_a = atoi(ter_a.c_str());
        // both are strings
        if (int_d == 0 && int_a == 0) {
          fret->add_restraint(fret_restraint(
              all_mol, name_d, ter_d, name_a, ter_a, fexp, mydata.Fret,
              mydata.cell_type, mydata.add_GFP, ISD_ps["Kda"], ISD_ps["Ida"],
              ISD_ps["R0"], ISD_ps["Sigma0"], ISD_ps["pBl"]));
        }
        // donor is string, acceptor is residue index
        if (int_d == 0 && int_a != 0) {
          fret->add_restraint(fret_restraint(
              all_mol, name_d, ter_d, name_a, int_a, fexp, mydata.Fret,
              mydata.cell_type, mydata.add_GFP, ISD_ps["Kda"], ISD_ps["Ida"],
              ISD_ps["R0"], ISD_ps["Sigma0"], ISD_ps["pBl"]));
        }
        // donor is residue index, acceptor is string
        if (int_d != 0 && int_a == 0) {
          fret->add_restraint(fret_restraint(
              all_mol, name_d, int_d, name_a, ter_a, fexp, mydata.Fret,
              mydata.cell_type, mydata.add_GFP, ISD_ps["Kda"], ISD_ps["Ida"],
              ISD_ps["R0"], ISD_ps["Sigma0"], ISD_ps["pBl"]));
        }
      }
      // close file
      newfretfile.close();
    }
    // add the RestraintSet tp model
    // m->add_restraint(fret);
    allrs->add_restraint(fret);
    // add the RestraintSet to map
    rst_map["FRET_R"] = fret;
  }

  //
  // TWO-HYBRID SCREENING
  //
  if (mydata.add_y2h) {
    // prepare the restraint set
    IMP_NEW(RestraintSet, y2h, (m, "Y2H"));
    // CP
    y2h->add_restraint(y2h_restraint(m, all_mol[0], "Spc42p", IntRange(1, 141),
                                     all_mol, "Spc110p", IntRange(781, 944),
                                     mydata.kappa));
    // We are keeping those for validation
    // y2h->add_restraint(y2h_restraint(m,
    //                     all_mol[0],  "Spc29p",             "ALL",
    //                     all_mol,    "Spc110p", IntRange(811,898),
    //                     mydata.kappa));
    // y2h->add_restraint(y2h_restraint(m,
    //                     all_mol[0], "Spc110p", IntRange(823,944),
    //                     all_mol,    "Spc110p", IntRange(823,944),
    //                     mydata.kappa));
    y2h->add_restraint(y2h_restraint(m, all_mol[0], "Spc42p", IntRange(1, 138),
                                     all_mol, "Spc29p", "ALL", mydata.kappa));
    // IL2
    y2h->add_restraint(y2h_restraint(m, all_mol[0], "Cnm67p",
                                     IntRange(442, 573), all_mol, "Spc42p",
                                     IntRange(49, 363), mydata.kappa));
    // Adding restraint from Klenchin 2011 JBC paper
    // Not technically a Y2H restraint but similar
    y2h->add_restraint(y2h_restraint(m, all_mol[0], "Cnm67p",
                                     IntRange(502, 507), all_mol, "Spc42p",
                                     "ALL", mydata.kappa));

    // add the RestraintSet to model
    // m->add_restraint(y2h);
    allrs->add_restraint(y2h);
    // add the RestraintSet to map
    rst_map["Y2H"] = y2h;
  }
  //
  // Add Spc110 stay on plane to enforce some symmetry from SAXS
  //
  if (mydata.protein_list["Spc110p"]) {
    add_stay_on_plane_restraint(m, allrs, all_mol[0], "Spc110p", 900,
                                mydata.kappa);
    add_stay_on_plane_restraint(m, allrs, all_mol[0], "Spc110p", 940,
                                mydata.kappa);
  }
  //
  // Two cmd1 should stay close according to SAXS shape
  //
  if (mydata.protein_list["Cmd1p"]) {
    add_stay_close_restraint(m, allrs, all_mol[0], "Cmd1p", mydata.kappa);
  }
  // Add restraint to fix distance between termini for Spc29
  if (mydata.restrain_distance_Spc29_termini) {
    add_restrain_protein_length(allrs, all_mol[0], "Spc29p",
                                ISD_ps["Spc29TermDist"], mydata.sigma0_dist);
  }

  // Add restraint to fix distance between Cterm of Spc42 and coiled coil
  if (mydata.restrain_distance_Spc42_Cterm) {
    add_restrain_coiledcoil_to_cterm(m, allrs, all_mol[0], "Spc42p",
                                     ISD_ps["Spc42CtermDist"],
                                     mydata.sigma0_dist_spc42c);
  }

  //
  // Add restraint to diameter and rgyr of Spc29 from SAXS
  //
  // if(mydata.protein_list["Spc29p"]){
  // add_diameter_rgyr_restraint(RestraintSet *allrs,all_mol[0],"Spc29p",
  //          mydata.Spc29_diameter,mydata.Spc29_rgyr,mydata.kappa);
  //}
  //
  // Add link with GFPs
  //
  if (mydata.add_GFP) {
    add_link(m, allrs, all_mol[0], "Spc110p", "C", all_mol, "Spc110p-C-GFP",
             "N", mydata.kappa);
    add_link(m, allrs, all_mol[0], "Cmd1p", "N", all_mol, "Cmd1p-N-GFP", "C",
             mydata.kappa);
    add_link(m, allrs, all_mol[0], "Cmd1p", "C", all_mol, "Cmd1p-C-GFP", "N",
             mydata.kappa);
    add_link(m, allrs, all_mol[0], "Spc42p", "N", all_mol, "Spc42p-N-GFP", "C",
             mydata.kappa);
    add_link(m, allrs, all_mol[0], "Spc42p", "C", all_mol, "Spc42p-C-GFP", "N",
             mydata.kappa);
    add_link(m, allrs, all_mol[0], "Spc29p", "N", all_mol, "Spc29p-N-GFP", "C",
             mydata.kappa);
    add_link(m, allrs, all_mol[0], "Spc29p", "C", all_mol, "Spc29p-C-GFP", "N",
             mydata.kappa);
    add_link(m, allrs, all_mol[0], "Cnm67p", "C", all_mol, "Cnm67p-C-GFP", "N",
             mydata.kappa);
  }
  //
  // RESTRAINT GFPs POSITION for refinement
  //
  if (mydata.add_GFP && mydata.restraint_GFP) {
    add_GFP_restraint(m, allrs, all_mol[0], mydata.kappa);
  }

  return rst_map;
}

IMPSPB_END_NAMESPACE
