/**
 *  \file bin/protein_ligand_score.cpp   \brief A class for reading mol2 files
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/protein_ligand_score.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/mol2.h>
#include <IMP/Model.h>

int main(int argc, char *argv[]) {
  std::string mol2name, pdbname;
  for (unsigned int i=1; i < argc; ++i) {
    std::string nm(argv[i]);
    if (nm.rfind(".mol2") == nm.size()-5) {
      mol2name= nm;
    } else if (nm.rfind(".pdb") == nm.size()-4) {
      pdbname= nm;
    } else {
      break;
    }
  }
  if (mol2name.empty() || pdbname.empty()) {
    std::cerr << "Usage: " << argv[0] << " file.mol2 file.pdb" << std::endl;
    return EXIT_FAILURE;
  }

  IMP_NEW(IMP::Model, m, ());
  IMP::atom::Hierarchy p= IMP::atom::read_pdb(pdbname, m);
  IMP::atom::Hierarchy l= IMP::atom::read_mol2(mol2name, m);
  IMP::atom::HierarchiesTemp mols
    = IMP::atom::get_by_type(l, IMP::atom::RESIDUE_TYPE);
  for (unsigned int i=0; i< mols.size(); ++i) {
    IMP_NEW(IMP::atom::ProteinLigandRestraint, r, (p, mols[i]));
    m->add_restraint(r);
    double s= m->evaluate(false);
    std::cout << "Molecule " << mols[i]->get_name()
              << " has score " << s << std::endl;
    m->remove_restraint(r);
  }
  return EXIT_SUCCESS;
}
