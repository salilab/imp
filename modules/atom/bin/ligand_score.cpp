/**
 *  \file ligand_score.cpp   \brief A class for reading mol2 files
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/protein_ligand_score.h>
#include <IMP/atom/pdb.h>
#include <IMP/atom/mol2.h>
#include <IMP/core/GridClosePairsFinder.h>
#include <IMP/particle_index.h>
#include <IMP/kernel/Model.h>
#include <IMP/base/flags.h>

namespace {
  bool rank_score = false, pose_score = false;
  IMP::base::AddBoolFlag abf1("rank",
                              "Use the RankScore (default)", &rank_score);
  IMP::base::AddBoolFlag abf2("pose",
                              "Use the PoseScore", &pose_score);

  IMP::atom::ProteinLigandAtomPairScore *
  get_pair_score(IMP::base::TextInput &lib)
  {
    if (lib) {
      return new IMP::atom::ProteinLigandAtomPairScore(100000, lib);
    } else if (pose_score) {
      return new IMP::atom::ProteinLigandAtomPairScore(100000,
                     IMP::atom::get_data_path("protein_ligand_pose_score.lib"));
    } else {
      return new IMP::atom::ProteinLigandAtomPairScore();
    }
  }
}

int main(int argc, char *argv[]) {
  IMP::Strings args = IMP::base::setup_from_argv(argc, argv,
                              "Score a protein-ligand complex",
                              "file.mol2 file.pdb [libfile]", -1);

  IMP::base::set_log_level(IMP::base::SILENT);
  std::string mol2name, pdbname;
  for (size_t i = 0; i < args.size(); ++i) {
    std::string nm(args[i]);
    if (nm.rfind(".mol2") == nm.size() - 5) {
      mol2name = nm;
    } else if (nm.rfind(".pdb") == nm.size() - 4) {
      pdbname = nm;
    } else {
      break;
    }
  }
  if (mol2name.empty() || pdbname.empty()) {
    std::cerr << "Usage: " << argv[0] << " file.mol2 file.pdb [libfile]"
              << std::endl;
    return EXIT_FAILURE;
  }
  IMP::base::TextInput lib;
  if (args.size() == 3) {
    lib = IMP::base::TextInput(args[2]);
  }
  {
    int lib_requested = 0;
    if (lib) lib_requested++;
    if (pose_score) lib_requested++;
    if (rank_score) lib_requested++;
    if (lib_requested > 1) {
      std::cerr << "Can only specify one of --pose, --rank, "
                << "or a library name" << std::endl;
      return EXIT_FAILURE;
    }
  }


  IMP_NEW(IMP::kernel::Model, m, ());
  IMP::atom::Hierarchy p, l;
  {
    IMP::base::SetLogState ss(IMP::base::SILENT);
    p = IMP::atom::read_pdb(pdbname, m, new IMP::atom::ATOMPDBSelector());
    IMP::atom::add_protein_ligand_score_data(p);
    l = IMP::atom::read_mol2(mol2name, m);
    IMP::atom::add_protein_ligand_score_data(l);
  }
  IMP::atom::Hierarchies mols =
      IMP::atom::get_by_type(l, IMP::atom::RESIDUE_TYPE);
  IMP::base::Pointer<IMP::atom::ProteinLigandAtomPairScore> ps
          = get_pair_score(lib);
  double d = ps->get_maximum_distance();
  IMP_NEW(IMP::core::GridClosePairsFinder, gcpf, ());
  gcpf->set_distance(d);

  IMP::kernel::ParticlesTemp patoms = IMP::atom::get_leaves(p);
  IMP::kernel::ParticleIndexes ipatoms = IMP::get_indexes(patoms);
  for (unsigned int i = 0; i < mols.size(); ++i) {
    // IMP::SetLogState ss(i==0? TERSE: IMP::SILENT);
    IMP::kernel::ParticlesTemp latoms = IMP::atom::get_leaves(mols[i]);
    IMP::kernel::ParticleIndexes ilatoms = IMP::get_indexes(latoms);
    IMP::kernel::ParticleIndexPairs ppt =
        gcpf->get_close_pairs(m, ipatoms, ilatoms);
    double score = ps->evaluate_indexes(m, ppt, NULL, 0, ppt.size());
    std::cout << "Score for " << mols[i]->get_name() << " is " << score
              << std::endl;
  }
  ps->set_was_used(true);
  return EXIT_SUCCESS;
}
