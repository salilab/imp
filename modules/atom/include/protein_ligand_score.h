/**
 *  \file IMP/atom/protein_ligand_score.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_PROTEIN_LIGAND_SCORE_H
#define IMPATOM_PROTEIN_LIGAND_SCORE_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include <IMP/core/StatisticalPairScore.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/Restraint.h>
#include <IMP/PairScore.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/file.h>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

/** \name Protein-ligand scoring
    \anchor protein_ligand

    \imp provides a statistical scoring function for scoring
    protein-ligand complexes. See the
    \ref application_ligand_score "ligand scoring"
    application for more information.
    @{
 */
class ProteinLigandRestraint;

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
IMP_DECLARE_CONTROLLED_KEY_TYPE(ProteinLigandType, 783462);
#elif defined(SWIG)
class ProteinLigandType;
#endif

/** add_protein_ligand_score_data() must be called on the molecules
    containing the atoms before the PairScore is used in order
    to properly initialize the particles.

    The default library to use is \c protein_ligand_rank_score.lib.
    \imp also provides \c protein_ligand_pose_score.lib which can be
    found at IMP::atom::get_data_path("protein_ligand_pose_score.lib").
*/
class IMPATOMEXPORT ProteinLigandAtomPairScore:
  public core::StatisticalPairScore<ProteinLigandType, true, false> {
  friend class ProteinLigandRestraint;
  typedef core::StatisticalPairScore<ProteinLigandType, true, false>  P;
  double threshold_;
 public:
  ProteinLigandAtomPairScore(double threshold
                             = std::numeric_limits<double>::max());
  ProteinLigandAtomPairScore(double threshold,
                             base::TextInput data_file);
  double get_maximum_distance() const {
    return threshold_;
  }
};

IMP_OBJECTS(ProteinLigandAtomPairScore,ProteinLigandAtomPairScores);

/** Score a pair of molecules. See ProteinLigandAtomPairScore for
    simply scoring the atom pairs.

    \see ProteinLigandAtomPairScore
*/
class IMPATOMEXPORT ProteinLigandRestraint: public container::PairsRestraint {
  void initialize(Hierarchy protein,
                  Hierarchy ligand);
 public:
  ProteinLigandRestraint(Hierarchy protein, Hierarchy ligand,
                         double threshold= std::numeric_limits<double>::max());
  ProteinLigandRestraint(Hierarchy protein, Hierarchy ligand,
                         double threshold,
                         base::TextInput data_file);
};

IMP_OBJECTS(ProteinLigandRestraint,ProteinLigandRestraints);


IMPATOMEXPORT void add_protein_ligand_score_data(Hierarchy h);

/** @} */
IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PROTEIN_LIGAND_SCORE_H */
