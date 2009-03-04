/**
 *  \file PDBParser.h   \brief A class for reading PDB files
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include <IMP/atom/PDBReader.h>

#include <IMP/core/AtomDecorator.h>
#include <IMP/core/ResidueDecorator.h>
#include <IMP/core/NameDecorator.h>

#include <boost/algorithm/string.hpp>

#include <fstream>

IMPATOM_BEGIN_NAMESPACE

core::MolecularHierarchyDecorator PDBReader::read_PDB_file(
                             String pdb_file_name, Model *model,
                             const Selector& selector,
                             bool select_first_model,
                             bool ignore_alternatives)
{
  std::ifstream pdb_file(pdb_file_name.c_str());
  if (!pdb_file) {
    std::cerr << "no such PDB file " << pdb_file_name << std::endl;
    return 0;
  }

  // create root particle
  Particle* root_p = root_particle(model, pdb_file_name);
  core::MolecularHierarchyDecorator root_d =
    core::MolecularHierarchyDecorator::cast(root_p);

  Particle* cp = NULL;
  Particle* rp = NULL;
  core::MolecularHierarchyDecorator hcd, hrd;

  char curr_residue_icode = '-';
  char curr_chain = '-';
  bool chain_type_set = false;

  String line;
  while (!pdb_file.eof()) {
    getline(pdb_file, line);
    // check that line is an HETATM or ATOM rec and that selector accepts line.
    // if this is the case construct a new Particle using line and add the
    // Particle to the Model
    if ((PDBParser::is_ATOM_rec(line) || PDBParser::is_HETATM_rec(line))
        && selector(line)) {

      int residue_index = PDBParser::atom_residue_number(line);
      char residue_icode = PDBParser::atom_residue_icode(line);
      char chain = PDBParser::atom_chain_id(line);

      // check if new chain
      if (cp == NULL || chain != curr_chain) {
        curr_chain = chain;
        // create new chain particle
        cp = chain_particle(model, chain);
        chain_type_set = false;
        hcd = core::MolecularHierarchyDecorator::cast(cp);
        root_d.add_child(hcd);
      }

      // check if new residue
      if (rp == NULL ||
          residue_index != core::ResidueDecorator::cast(rp).get_index() ||
          residue_icode != curr_residue_icode) {
        curr_residue_icode = residue_icode;
        // create new residue particle
        rp = residue_particle(model, line);
        hrd = core::MolecularHierarchyDecorator::cast(rp);
        hcd.add_child(hrd);
      }

      // set chain type (protein/nucleotide/other) according to residue type
      if (!chain_type_set) {
        set_chain_type(hrd, hcd);
        chain_type_set = true;
      }

      // check if alternatives should be skipped
      IgnoreAlternativesSelector sel;
      if(ignore_alternatives && !sel(line)) continue;

      // create atom particle
      Particle* ap = atom_particle(model, line);
      core::MolecularHierarchyDecorator had =
        core::MolecularHierarchyDecorator::cast(ap);
        hrd.add_child(had);
    }
  }
  return root_d;
}

Particle* PDBReader::atom_particle(Model *m, const String& pdb_line)
{
  Particle* p = new Particle(m);

  algebra::Vector3D v(PDBParser::atom_xcoord(pdb_line),
                      PDBParser::atom_ycoord(pdb_line),
                      PDBParser::atom_zcoord(pdb_line));

  String atom_name = PDBParser::atom_type(pdb_line);
  boost::trim(atom_name);
  core::AtomType atom_type = core::AtomType(atom_name.c_str());

  // atom decorator
  core::AtomDecorator d = core::AtomDecorator::create(p, atom_type, v);

  d.set_coordinates_are_optimized(true);
  d.set_input_index(PDBParser::atom_number(pdb_line));

  // hierarchy decorator
  core::MolecularHierarchyDecorator hd =
    core::MolecularHierarchyDecorator::create(p,
               core::MolecularHierarchyDecorator::ATOM);
  return p;
}

Particle* PDBReader::residue_particle(Model *m, const String& pdb_line)
{
  Particle* p = new Particle(m);

  int residue_index = PDBParser::atom_residue_number(pdb_line);
  char residue_icode = PDBParser::atom_residue_icode(pdb_line);
  core::ResidueType residue_type =
    core::ResidueType(PDBParser::atom_residue_name(pdb_line).c_str());

  // residue decorator
  core::ResidueDecorator rd =
    core::ResidueDecorator::create(p, residue_type,
                                   residue_index, (int)residue_icode);

  // hierarchy decorator
  core::MolecularHierarchyDecorator hd =
    core::MolecularHierarchyDecorator::create(p,
              core::MolecularHierarchyDecorator::RESIDUE);

  // check if amino acid or nucleic acid or something else
  if (rd.get_is_amino_acid())
    hd.set_type(core::MolecularHierarchyDecorator::RESIDUE);
  else if (rd.get_is_nucleic_acid())
    hd.set_type(core::MolecularHierarchyDecorator::NUCLEICACID);
  else
    hd.set_type(core::MolecularHierarchyDecorator::MOLECULE);

  // name decorator
  core::NameDecorator::create(p, String("residue " + residue_index));

  return p;
}

Particle* PDBReader::root_particle(Model *m, const String& pdb_file_name)
{
  Particle* p = new Particle(m);

  // name decorator
  core::NameDecorator::create(p, pdb_file_name);

  // hierarchy decorator
  core::MolecularHierarchyDecorator hd =
    core::MolecularHierarchyDecorator::create(p,
               core::MolecularHierarchyDecorator::PROTEIN);
  return p;
}

Particle* PDBReader::chain_particle(Model *m, char chain_id)
{
  Particle* p = new Particle(m);

  // name decorator
  String chain;
  chain += chain_id;
  core::NameDecorator::create(p, chain);

  // hierarchy decorator
  core::MolecularHierarchyDecorator hd =
    core::MolecularHierarchyDecorator::create(p,
               core::MolecularHierarchyDecorator::FRAGMENT);
  return p;
}

void PDBReader::set_chain_type(const core::MolecularHierarchyDecorator& hrd,
                               core::MolecularHierarchyDecorator& hcd) {

  if (hrd.get_type() == core::MolecularHierarchyDecorator::RESIDUE)
    hcd.set_type(core::MolecularHierarchyDecorator::CHAIN);
  else if (hrd.get_type() == core::MolecularHierarchyDecorator::NUCLEICACID)
    hcd.set_type(core::MolecularHierarchyDecorator::NUCLEOTIDE);
  else
    hcd.set_type(core::MolecularHierarchyDecorator::MOLECULE);
}

void PDBReader::write_PDB_file(String pdb_file_name, Model *model) {
  std::ofstream out_file(pdb_file_name.c_str());
  for (Model::ParticleIterator it= model->particles_begin();
       it != model->particles_end(); ++it) {
    if (core::AtomDecorator::is_instance_of(*it)) {
      core::AtomDecorator ad = core::AtomDecorator::cast(*it);
      out_file << ad.get_pdb_string();
    }
  }
  out_file.close();
}

IMPATOM_END_NAMESPACE
