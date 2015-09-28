/**
 * \file IMP/kinematics/helpers.cpp
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/kinematics/helpers.h>

#include <IMP/saxs/utility.h>

IMPKINEMATICS_BEGIN_NAMESPACE

IMP::atom::Atom find_atom(const IMP::ParticlesTemp& atoms, int input_index) {
  for(unsigned int i=0; i<atoms.size(); i++) {
    IMP::atom::Atom ad = IMP::atom::Atom(atoms[i]);
    if(ad.get_input_index() == input_index) return ad;
  }
  std::cerr << "Atom not found " << input_index << std::endl;
  exit(0);
}

IMP::atom::Residue find_residue(const IMP::ParticlesTemp& residues,
                                int res_index, std::string chain) {
  for(unsigned int i=0; i<residues.size(); i++) {
    IMP::atom::Residue rd = IMP::atom::Residue(residues[i]);
    if(rd.get_index() == res_index &&
       IMP::atom::get_chain(rd).get_id() == chain) return rd;
  }
  std::cerr << "Residue not found " << res_index << chain << std::endl;
  exit(0);
}

IMP::atom::Atom get_ca_atom(const IMP::ParticlesTemp& atoms,
                            int residue_index, std::string chain_id) {
  for (unsigned int i = 0; i < atoms.size(); i++) {
    IMP::atom::Residue r = IMP::atom::get_residue(IMP::atom::Atom(atoms[i]));
    int curr_residue_index = r.get_index();
    std::string curr_chain_id =
        IMP::atom::get_chain_id(IMP::atom::Atom(atoms[i]));
    IMP::atom::AtomType at = IMP::atom::Atom(atoms[i]).get_atom_type();
    if (curr_residue_index == residue_index &&
        curr_chain_id == chain_id &&
        at == IMP::atom::AT_CA) {
      return IMP::atom::Atom(atoms[i]);
    }
  }
  std::cerr << "Residue not found " << residue_index << " " << chain_id
            << std::endl;
  exit(1);
}

void read_connect_chains_file(const std::string& file_name,
                              const IMP::ParticlesTemp& atoms,
                              std::vector<IMP::atom::Atoms>& connect_atoms) {

  std::ifstream in_file(file_name.c_str());
  if(!in_file) {
    std::cerr << "Can't find rotatable bonds file " << file_name << std::endl;
    exit(1);
  }

  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line); // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if(split_results.size() != 2 && split_results.size() != 4) continue;

    IMP::atom::Atoms connecting_atoms;
    if(split_results.size() == 2) {
      for(int i=0; i<2; i++) {
        int atom_index = atoi(split_results[i].c_str());
        connecting_atoms.push_back(find_atom(atoms, atom_index));
      }
    } else { // 4
      int res_number1 = atoi(split_results[0].c_str());
      std::string chain_id1 = split_results[1];
      int res_number2 = atoi(split_results[2].c_str());
      std::string chain_id2 = split_results[3];
      connecting_atoms.push_back(get_ca_atom(atoms, res_number1, chain_id1));
      connecting_atoms.push_back(get_ca_atom(atoms, res_number2, chain_id2));
    }
    connect_atoms.push_back(connecting_atoms);
  }
  std::cerr << connect_atoms.size() << " chain connecting atoms/residues "
            << " were read from file "  << file_name << std::endl;
}

void read_angle_file(const std::string& file_name,
                     const IMP::ParticlesTemp& residues,
                     const IMP::ParticlesTemp& atoms,
                     IMP::atom::Residues& flexible_residues,
                     std::vector<IMP::atom::Atoms>& dihedral_angles) {

  std::ifstream in_file(file_name.c_str());
  if(!in_file) {
    std::cerr << "Can't find rotatable bonds file " << file_name << std::endl;
    exit(1);
  }

  std::string line;
  while (!in_file.eof()) {
    getline(in_file, line);
    boost::trim(line); // remove all spaces
    // skip comments
    if (line[0] == '#' || line[0] == '\0' || !isdigit(line[0])) continue;
    std::vector<std::string> split_results;
    boost::split(split_results, line, boost::is_any_of("\t "),
                 boost::token_compress_on);
    if(split_results.size() <= 2) {
      int res_number = atoi(split_results[0].c_str());
      std::string chain_id = " ";
      if(split_results.size() == 2) {
        chain_id = split_results[1];
      }
      flexible_residues.push_back(find_residue(residues, res_number, chain_id));
    }
    if(split_results.size() == 4) {
      IMP::atom::Atoms angle_atoms;
      for(int i=0; i<4; i++) {
        int atom_index = atoi(split_results[i].c_str());
        angle_atoms.push_back(find_atom(atoms, atom_index));
      }
      dihedral_angles.push_back(angle_atoms);
    }
  }
  std::cerr << flexible_residues.size() << " residues "
            << dihedral_angles.size() << " bonds "
            << " were read from file "  << file_name << std::endl;

}

IMP::atom::Bond create_bond(IMP::atom::Atoms& as) {
  IMP::atom::Bonded b[2];
  for(unsigned int i = 0; i < 2; ++i) {
    if(IMP::atom::Bonded::get_is_setup(as[i]))
      b[i] = IMP::atom::Bonded(as[i]);
    else
      b[i] = IMP::atom::Bonded::setup_particle(as[i]);
  }
  IMP::atom::Bond bd = IMP::atom::get_bond(b[0], b[1]);
  if(bd == IMP::atom::Bond()) {
    bd = IMP::atom::create_bond(b[0], b[1], IMP::atom::Bond::SINGLE);
    return bd;
  } else return IMP::atom::Bond();
}

void add_missing_bonds(IMP::ParticlesTemp& atoms, IMP::ParticlesTemp& bonds) {
  float thr2 = 1.65*1.65;
  IMP::Vector<IMP::algebra::Vector3D> coordinates;
  IMP::saxs::get_coordinates(atoms, coordinates);
  int counter = 0;
  for(unsigned int i=0; i<atoms.size(); i++) {
    for(unsigned int j=i+1; j<atoms.size(); j++) {
      float dist2 = IMP::algebra::get_squared_distance(coordinates[i], coordinates[j]);
      if(dist2 < thr2) { // add bond
        IMP::atom::Atoms as;
        IMP::atom::Atom ai = IMP::atom::Atom(atoms[i]);
        IMP::atom::Atom aj = IMP::atom::Atom(atoms[j]);
        as.push_back(ai);
        as.push_back(aj);

        IMP::atom::Bonded b[2];
        for(unsigned int i = 0; i < 2; ++i) {
          if(IMP::atom::Bonded::get_is_setup(as[i]))
            b[i] = IMP::atom::Bonded(as[i]);
          else
            b[i] = IMP::atom::Bonded::setup_particle(as[i]);
        }
        IMP::atom::Bond bd = IMP::atom::get_bond(b[0], b[1]);
        if(bd == IMP::atom::Bond()) {
          bd = IMP::atom::create_bond(b[0], b[1], IMP::atom::Bond::SINGLE);
          bonds.push_back(bd);
          counter++;
        }
      }
    }
  }
  std::cerr << counter << " bonds were added" << std::endl;
}

IMPKINEMATICS_END_NAMESPACE
