/**
 *  \file bin/protein_ligand_score.cpp   \brief A class for reading mol2 files
 *
 *  Copyright 2007-9 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/protein_ligand_score.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/mol2.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <IMP/Model.h>

int main(int argc, char *argv[]) {
  std::string mol2name, pdbname;
  for (int i=1; i < argc; ++i) {
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
  IMP::atom::Hierarchy p, l;
  {
    IMP::SetLogState ss(IMP::SILENT);
    p= IMP::atom::read_pdb(pdbname, m, IMP::atom::ATOMPDBSelector());
    IMP::atom::add_protein_ligand_score_data(p);
    l= IMP::atom::read_mol2(mol2name, m);
    IMP::atom::add_protein_ligand_score_data(l);
  }
  IMP::atom::HierarchiesTemp mols
    = IMP::atom::get_by_type(l, IMP::atom::RESIDUE_TYPE);
  IMP_NEW(IMP::atom::ProteinLigandAtomPairScore, ps, ());
  double d= ps->get_maximum_distance();
  IMP_NEW(IMP::core::GridClosePairsFinder, gcpf, ());
  gcpf->set_distance(d);

  IMP::ParticlesTemp patoms= IMP::atom::get_leaves(p);

  for (unsigned int i=0; i< mols.size(); ++i) {
    IMP::SetLogState ss(i==0? IMP::TERSE: IMP::SILENT);
    IMP::ParticlesTemp latoms= IMP::atom::get_leaves(mols[i]);
    IMP::ParticlePairsTemp ppt= gcpf->get_close_pairs(patoms, latoms);
    double score=0;
    for (unsigned int j=0; j< ppt.size(); ++j) {
      score+= ps->evaluate(ppt[j], false);
    }
    std::cout << "Score for " << mols[i]->get_name() << " is "
              << score << std::endl;
  }
  ps->set_was_used(true);
  return EXIT_SUCCESS;
}
