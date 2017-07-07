/**
 *  \file spb_analysis.cpp
 *  \brief SPB Analysis Tool
 *
 *  Copyright 2011 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/Pointer.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include <IMP/isd.h>
#include <IMP/spb.h>
#include <IMP/rmf.h>
#include <IMP/statistics.h>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace IMP;
using namespace IMP::spb;

Particles generate_new_particles(Model *m, const atom::Hierarchies &all_mol,
                                 double side, double off_x, double off_y,
                                 double xmin, double xmax, double ymin,
                                 double ymax) {
  // first pick Spc42 particles in hierarchies 0, 1 and 2
  atom::Hierarchies hhs;
  for (unsigned i = 0; i < 3; ++i) {
    hhs.push_back(all_mol[i]);
  }
  // select from them
  atom::Selection s = atom::Selection(hhs);
  s.set_molecule("Spc42p");
  Particles ps0 = s.get_selected_particles();
  // now create new particles
  Particles ps;
  for (int i = -4; i < 5; ++i) {
    for (int j = -4; j < 5; ++j) {
      // cell translations
      double dx = static_cast<double>(i) * 1.5 * side;
      double dy = side * sqrt(3.) *
                  (static_cast<double>(j) + static_cast<double>(i % 2) / 2.);
      for (unsigned k = 0; k < ps0.size(); ++k) {
        // old coordinates
        algebra::Vector3D xyz = core::XYZR(ps0[k]).get_coordinates();
        // radius
        double radius = core::XYZR(ps0[k]).get_radius();
        // and mass
        double mass = atom::Mass(ps0[k]).get_mass();
        // coordinates of the new particle
        double xnew = xyz[0] + dx + off_x;
        double ynew = xyz[1] + dy + off_y;
        double znew = xyz[2];
        // check if within boundaries
        if (xnew > xmin && xnew <= xmax && ynew > ymin && ynew <= ymax) {
          // create new particle
          IMP_NEW(Particle, p, (m));
          // and decorate with old attributes
          core::XYZR d = core::XYZR::setup_particle(p);
          d.set_coordinates(algebra::Vector3D(xnew, ynew, znew));
          d.set_radius(radius);
          atom::Mass ma = atom::Mass::setup_particle(p, mass);
          ps.push_back(p);
        }
      }
    }
  }
  return ps;
};

int main(int argc, char *argv[]) {
  // read input
  std::string inputfile = "config.ini";
  int i = 1;
  while (i < argc) {
    if (strcmp(argv[i], "-in") == 0) {
      ++i;
      inputfile = argv[i];
    }
    ++i;
  }

  // prepare output files
  // a) FRET file
  FILE *fretlog;
  fretlog = fopen("fret.dat", "w");
  // b) log file
  FILE *logfile;
  logfile = fopen("log.dat", "w");

  // parsing input
  SPBParameters mydata = get_SPBParameters(inputfile, "0");

  // create a new model
  IMP_NEW(Model, m, ());
  IMP_NEW(RestraintSet, allrs, (m, "All restraints"));

  // List of particles for layer restraint
  IMP_NEW(container::ListSingletonContainer, CP_ps, (m));
  IMP_NEW(container::ListSingletonContainer, IL2_ps, (m));
  // List of MonteCarloMovers for MC, not used here
  core::MonteCarloMovers mvs;

  //
  // ISD PARTICLES
  //
  std::map<std::string, IMP::Pointer<Particle> > ISD_ps =
      add_ISD_particles(m, allrs, mydata, mvs);

  // create list of particles from map
  Particles ISD_ps_list;
  std::map<std::string, IMP::Pointer<Particle> >::iterator itr;
  for (itr = ISD_ps.begin(); itr != ISD_ps.end(); ++itr) {
    ISD_ps_list.push_back((*itr).second);
  }

  //
  // add new particle Ida_new and kda_new if needed
  //
  if (!mydata.add_new_fret) {
    // kda_new particle
    IMP_NEW(Particle, pKda_new, (m));
    // initial value
    isd::Scale Kda_new = isd::Scale::setup_particle(pKda_new, 7.5);
    Kda_new.set_lower(7.5);
    Kda_new.set_upper(7.5);
    Kda_new->set_is_optimized(Kda_new.get_nuisance_key(), false);
    // add particle to map
    ISD_ps["Kda_new"] = pKda_new;

    // Ida particle
    IMP_NEW(Particle, pIda_new, (m));
    isd::Scale Ida_new = isd::Scale::setup_particle(pIda_new, 6.2);
    Ida_new.set_lower(6.2);
    Ida_new.set_upper(6.2);
    Ida_new->set_is_optimized(Ida_new.get_nuisance_key(), false);
    // add particle to map
    ISD_ps["Ida_new"] = pIda_new;
  }

  //
  // PROTEIN REPRESENTATION
  //
  atom::Hierarchies all_mol =
      create_representation(m, allrs, mydata, CP_ps, IL2_ps, mvs,
                            ISD_ps["SideXY"], ISD_ps["SideZ"], 0);
  //
  // create list to link to rmf
  //
  atom::Hierarchies hhs;
  for (unsigned int i = 0; i < all_mol.size(); ++i) {
    atom::Hierarchies hs = all_mol[i].get_children();
    for (unsigned int j = 0; j < hs.size(); ++j) {
      hhs.push_back(hs[j]);
    }
  }

  //
  // Initialize MonteCarloWithWte to deal with bias
  //
  IMP_NEW(spb::MonteCarloWithWte, mcwte,
          (m, mydata.MC.wte_emin, mydata.MC.wte_emax, mydata.MC.wte_sigma,
           mydata.MC.wte_gamma, 1.0));
  //
  // READ BIAS file
  //
  if (mydata.Cluster.weight) {
    Floats val;
    double bias;
    std::ifstream biasfile;
    biasfile.open(mydata.Cluster.biasfile.c_str());
    if (biasfile.is_open()) {
      // read file
      while (biasfile >> bias) {
        val.push_back(bias);
      }
      // find max of bias (only first half of the array, the rest is
      // derivatives)
      Float maxval = 0;
      for (unsigned i = 0; i < val.size() / 2; ++i) {
        if (val[i] > maxval) {
          maxval = val[i];
        }
      }
      // shift bias to set max at zero
      for (unsigned i = 0; i < val.size() / 2; ++i) {
        val[i] -= maxval;
      }
      // set shifted bias into mcwte class
      mcwte->set_bias(val);
      // close file
      biasfile.close();
    }
  }

  //
  // Restraints for backcalculating stuff
  //
  // prepare the map of RestraintSet
  std::map<std::string, IMP::Pointer<RestraintSet> > rst_map;
  // the other restraints are not there

  //
  // 1) FRET
  //
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
        m, all_mol, name_d, ter_d, name_a, ter_a, fexp, mydata.Fret,
        mydata.cell_type, mydata.add_GFP, ISD_ps["Kda"], ISD_ps["Ida"],
        ISD_ps["R0"], ISD_ps["Sigma0"], ISD_ps["pBl"]));
  }
  // close file
  fretfile.close();
  //
  // open new_fret file
  std::ifstream newfretfile;
  newfretfile.open(mydata.Fret.filename_new.c_str());
  while (newfretfile >> name_d >> ter_d >> name_a >> ter_a >> fexp >> sexp) {
    // try conversion to integer
    int int_d = atoi(ter_d.c_str());
    int int_a = atoi(ter_a.c_str());
    // both are strings
    if (int_d == 0 && int_a == 0) {
      fret->add_restraint(fret_restraint(
          m, all_mol, name_d, ter_d, name_a, ter_a, fexp, mydata.Fret,
          mydata.cell_type, mydata.add_GFP, ISD_ps["Kda"], ISD_ps["Ida"],
          ISD_ps["R0"], ISD_ps["Sigma0"], ISD_ps["pBl"]));
    }
    // donor is string, acceptor is residue index
    if (int_d == 0 && int_a != 0) {
      fret->add_restraint(fret_restraint(
          m, all_mol, name_d, ter_d, name_a, int_a, fexp, mydata.Fret,
          mydata.cell_type, mydata.add_GFP, ISD_ps["Kda"], ISD_ps["Ida"],
          ISD_ps["R0"], ISD_ps["Sigma0"], ISD_ps["pBl"]));
    }
    // donor is residue index, acceptor is string
    if (int_d != 0 && int_a == 0) {
      fret->add_restraint(fret_restraint(
          m, all_mol, name_d, int_d, name_a, ter_a, fexp, mydata.Fret,
          mydata.cell_type, mydata.add_GFP, ISD_ps["Kda"], ISD_ps["Ida"],
          ISD_ps["R0"], ISD_ps["Sigma0"], ISD_ps["pBl"]));
    }
  }
  // close file
  newfretfile.close();

  // add the RestraintSet to model
  allrs->add_restraint(fret);
  // m->add_restraint(fret);
  // add the RestraintSet to map
  rst_map["FRET_R"] = fret;

  //
  // 2) EM2D
  //
  // prepare the restraint set
  IMP_NEW(RestraintSet, em2d, (m, "EM2D"));
  // prepare logarithmic grid on sigma
  double s_min = 0.001;
  double s_max = 1000.0;
  int ns = 10000;
  Floats s_grid;
  for (int i = 0; i <= ns; ++i) {
    s_grid.push_back(s_min * exp(static_cast<double>(i) /
                                 static_cast<double>(ns) * log(s_max / s_min)));
  }
  // prepare grid on cross-correlation, linear from 0->1
  int ncross = 1000;
  Floats fmod_grid;
  for (int i = 0; i <= ncross; ++i) {
    fmod_grid.push_back(static_cast<double>(i) / static_cast<double>(ncross));
  }

  // add restraint to set
  em2d->add_restraint(
      em2d_restraint(m, all_mol, "Spc42p", mydata.EM2D, s_grid, fmod_grid));
  // add the RestraintSet to model
  // m->add_restraint(em2d);
  allrs->add_restraint(em2d);

  // add the RestraintSet to map
  rst_map["EM2D"] = em2d;

  // open rmf for coordinates
  RMF::FileConstHandle rh =
      RMF::open_rmf_file_read_only(mydata.Cluster.trajfile);
  // read various info
  RMF::Category my_kc = rh.get_category("my data");
  // RMF::FloatKey my_key0 = rh.get_float_key(my_kc,"my score",true);
  // RMF::IntKey   my_key1 = rh.get_int_key(my_kc,"my index",true);

  RMF::FloatKey my_key0 = rh.get_key<RMF::FloatTag>(my_kc, "my score");
  RMF::IntKey my_key1 = rh.get_key<RMF::IntTag>(my_kc, "my index");

  // linking hierarchies
  rmf::link_hierarchies(rh, hhs);
  // number of frames
  unsigned int nframes = rh.get_number_of_frames();

  // open rmf for ISD particles
  RMF::FileConstHandle rh_ISD =
      RMF::open_rmf_file_read_only(mydata.Cluster.isdtrajfile);
  // linking particles
  rmf::link_particles(rh_ISD, ISD_ps_list);
  // number of frames
  unsigned int nframes_ISD = rh_ISD.get_number_of_frames();

  // check number of frames are the same
  if (nframes != nframes_ISD) {
    exit(1);
  }

  // cycle on frames
  for (unsigned int imc = 0; imc < nframes; ++imc) {
    // load coordinates
    // rh.set_current_frame(RMF::FrameID(imc));
    rmf::load_frame(rh, RMF::FrameID(imc));

    // and ISD particles
    // rh_ISD.set_current_frame(RMF::FrameID(imc));
    rmf::load_frame(rh_ISD, RMF::FrameID(imc));

    // get score and REM index
    Float score = (rh.get_root_node()).get_value(my_key0);
    Int myindex = (rh.get_root_node()).get_value(my_key1);

    // calculate WTE weight
    Float wte_w = 1.0;
    if (mydata.Cluster.weight) {
      Float bias = mcwte->get_bias(score);
      wte_w = exp(bias);
    }

    // calculate cross-correlation
    IMP::Pointer<spb::EM2DRestraint> rst =
        dynamic_cast<spb::EM2DRestraint *>(
            rst_map["EM2D"]->get_restraint(0));

    // side
    Float side = isd::Scale(ISD_ps["SideXY"]).get_scale() * mydata.sideMin;

    // boundaries
    Float xmin = -3.0 * side;
    Float xmax = +3.0 * side;
    Float ymin = -sqrt(3.0) * side;
    Float ymax = +sqrt(3.0) * side;

    // symmetry offsets
    std::vector<std::pair<double, double> > off;
    off.push_back(std::make_pair(0.0, 0.0));
    off.push_back(std::make_pair(side, 0.0));
    off.push_back(std::make_pair(side / 2.0, sqrt(3.) / 2. * side));

    // Find maximum correlation
    Float crosscorr = 0.;
    for (unsigned i = 0; i < off.size(); ++i) {
      // get shifted particles within boundaries
      Particles Spc42 =
          generate_new_particles(m, all_mol, side, off[i].first, off[i].second,
                                 xmin, xmax, ymin, ymax);
      rst->set_particles(Spc42);
      Float cc = rst->get_cross_correlation();
      if (cc > crosscorr) {
        crosscorr = cc;
      }
    }

    // evaluate EM2D weight
    Float em2d_w = rst->get_marginal_element(crosscorr);

    // write output
    fprintf(logfile,
            "TimeStep %10d REM_Index %4d Side %6.3f EM2D-cc %6.3f Score %6.3f  "
            "WTE_w %12.6f EM2D_w %12.6f TOT_w %12.6f\n",
            imc, myindex, side, crosscorr, score, wte_w, em2d_w,
            wte_w * em2d_w);

    // write fret output
    fprintf(fretlog, "TimeStep %10d Kda %12.6f Ida %12.6f Sigma0 %12.6f\n", imc,
            isd::Scale(ISD_ps["Kda"]).get_scale(),
            isd::Scale(ISD_ps["Ida"]).get_scale(),
            isd::Scale(ISD_ps["Sigma0"]).get_scale());
    fprintf(fretlog, "TimeStep %10d R0 %12.6f pBl %12.6f\n", imc,
            isd::Scale(ISD_ps["R0"]).get_scale(),
            isd::Scale(ISD_ps["pBl"]).get_scale());
    // fprintf(fretlog,"TimeStep %10d Kda_new %12.6f Ida_new %12.6f\n",
    //        imc,
    //        isd::Scale(ISD_ps["Kda_new"]).get_scale(),
    //        isd::Scale(ISD_ps["Ida_new"]).get_scale());
    for (unsigned i = 0; i < rst_map["FRET_R"]->get_number_of_restraints();
         ++i) {
      IMP::Pointer<isd::FretRestraint> rst = dynamic_cast<isd::FretRestraint *>(
          rst_map["FRET_R"]->get_restraint(i));
      std::string name = rst->get_name();
      Float fmod = rst->get_model_fretr();
      Float fmod_err = rst->get_standard_error();
      Float fexp = rst->get_experimental_value();
      fprintf(fretlog,
              "TimeStep %10d Name %30s  Model %6.3f  Model_Error %6.3f  Exp "
              "%6.3f\n",
              imc, name.c_str(), fmod, fmod_err, fexp);
    }
  }

  // close RMFs
  rh = RMF::FileHandle();
  rh_ISD = RMF::FileHandle();

  fclose(fretlog);
  fclose(logfile);

  return 0;
}
