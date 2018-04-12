/**
 *  \file IMP/saxs/utility.cpp
 *  \brief Functions to deal with very common saxs operations
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
*/

#include <IMP/saxs/utility.h>
#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/atom/pdb.h>

IMPSAXS_BEGIN_NAMESPACE

Profile* compute_profile(Particles particles, double min_q,
                         double max_q, double delta_q, FormFactorTable* ft,
                         FormFactorType ff_type, bool hydration_layer, bool fit,
                         bool reciprocal, bool ab_initio, bool vacuum,
                         std::string beam_profile_file) {
  IMP_NEW(Profile, profile, (min_q, max_q, delta_q));
  if (reciprocal) profile->set_ff_table(ft);
  if (beam_profile_file.size() > 0) profile->set_beam_profile(beam_profile_file);

  // compute surface accessibility and average radius
  Vector<double> surface_area;
  SolventAccessibleSurface s;
  double average_radius = 0.0;
  if (hydration_layer) {
    // add radius
    for (unsigned int i = 0; i < particles.size(); i++) {
      double radius = ft->get_radius(particles[i], ff_type);
      core::XYZR::setup_particle(particles[i], radius);
      average_radius += radius;
    }
    surface_area = s.get_solvent_accessibility(core::XYZRs(particles));
    average_radius /= particles.size();
    profile->set_average_radius(average_radius);
  }

  // pick profile calculation based on input parameters
  if (!fit) {         // regular profile, no c1/c2 fitting
    if (ab_initio) {  // bead model, constant form factor
      profile->calculate_profile_constant_form_factor(particles);
    } else if (vacuum) {
      profile->calculate_profile_partial(particles, surface_area, ff_type);
      profile->sum_partial_profiles(0.0, 0.0);  // c1 = 0;
    } else {
      profile->calculate_profile(particles, ff_type, reciprocal);
    }
  } else {  // c1/c2 fitting
    if (reciprocal)
      profile->calculate_profile_reciprocal_partial(particles, surface_area,
                                                    ff_type);
    else
      profile->calculate_profile_partial(particles, surface_area, ff_type);
  }
  return profile.release();
}

void read_pdb(Model *model, const std::string file,
              std::vector<std::string>& pdb_file_names,
              std::vector<IMP::Particles>& particles_vec,
              bool residue_level, bool heavy_atoms_only, int multi_model_pdb,
              bool explicit_water) {
  IMP::atom::Hierarchies mhds;
  IMP::atom::PDBSelector* selector;
  if (residue_level)  // read CA only
    selector = new IMP::atom::CAlphaPDBSelector();
  else if (heavy_atoms_only) {  // read without hydrogens
    if (explicit_water)
      selector = new IMP::atom::NonHydrogenPDBSelector();
    else
      selector = new IMP::atom::NonWaterNonHydrogenPDBSelector();
  } else { // read with hydrogens
    if (explicit_water)
      selector = new IMP::atom::NonAlternativePDBSelector();
    else
      selector = new IMP::atom::NonWaterPDBSelector();
  }

  if (multi_model_pdb == 2) {
    mhds = read_multimodel_pdb(file, model, selector, true);
  } else {
    if (multi_model_pdb == 3) {
      IMP::atom::Hierarchy mhd =
          IMP::atom::read_pdb(file, model, selector, false, true);
      mhds.push_back(mhd);
    } else {
      IMP::atom::Hierarchy mhd =
          IMP::atom::read_pdb(file, model, selector, true, true);
      mhds.push_back(mhd);
    }
  }

  for (unsigned int h_index = 0; h_index < mhds.size(); h_index++) {
    IMP::ParticlesTemp ps =
        get_by_type(mhds[h_index], IMP::atom::ATOM_TYPE);
    if (ps.size() > 0) {  // pdb file
      std::string pdb_id = file;
      if (mhds.size() > 1) {
        pdb_id = trim_extension(file) + "_m" +
                 std::string(boost::lexical_cast<std::string>(h_index + 1)) +
                 ".pdb";
      }
      pdb_file_names.push_back(pdb_id);
      particles_vec.push_back(IMP::get_as<IMP::Particles>(ps));
      if (mhds.size() > 1) {
        IMP_LOG_TERSE(ps.size() << " atoms were read from PDB file " << file
                      << " MODEL " << h_index + 1 << std::endl);
      } else {
        IMP_LOG_TERSE(ps.size() << " atoms were read from PDB file " << file
                      << std::endl);
      }
    }
  }
}

void read_files(Model *m, const std::vector<std::string>& files,
                std::vector<std::string>& pdb_file_names,
                std::vector<std::string>& dat_files,
                std::vector<IMP::Particles>& particles_vec,
                Profiles& exp_profiles, bool residue_level,
                bool heavy_atoms_only, int multi_model_pdb,
                bool explicit_water, float max_q, int units) {

  for (unsigned int i = 0; i < files.size(); i++) {
    // check if file exists
    std::ifstream in_file(files[i].c_str());
    if (!in_file) {
      IMP_WARN("Can't open file " << files[i] << std::endl);
      return;
    }
    // 1. try as pdb
    try {
      read_pdb(m, files[i], pdb_file_names, particles_vec, residue_level,
               heavy_atoms_only, multi_model_pdb, explicit_water);
    }
    catch (IMP::ValueException e) {  // not a pdb file
      // 2. try as a dat profile file
      IMP_NEW(Profile, profile, (files[i], false, max_q, units));
      if (profile->size() == 0) {
        IMP_WARN("can't parse input file " << files[i] << std::endl);
        return;
      } else {
        dat_files.push_back(files[i]);
        exp_profiles.push_back(profile);
        IMP_LOG_TERSE("Profile read from file " << files[i]
                      << " size = " << profile->size() << std::endl);
      }
    }
  }
}

std::string trim_extension(const std::string file_name) {
  if (file_name[file_name.size() - 4] == '.')
    return file_name.substr(0, file_name.size() - 4);
  return file_name;
}

IMPSAXS_END_NAMESPACE
