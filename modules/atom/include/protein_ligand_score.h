/**
 *  \file atom/protein_ligand_score.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-9 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPATOM_PROTEIN_LIGAND_SCORE_H
#define IMPATOM_PROTEIN_LIGAND_SCORE_H

#include "atom_config.h"
#include <IMP/core/internal/PMFTable.h>
#include "Hierarchy.h"
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/Restraint.h>
#include <IMP/PairScore.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/file.h>
#include <limits>

IMPATOM_BEGIN_NAMESPACE

/** \name Protein-ligand scoring

    \imp provides a statistical scoring function for scoring
    protein-ligand complexes. Papers will be forthcoming.

    Neither of the scoring methods provide derivatives.

    As will more documentation and examples.
    @{
 */
class ProteinLigandRestraint;

/** add_protein_ligand_score_data() must be called on the molecules
    containing the atoms before the PairScore is used in order
    to properly initialize the particles.
*/
class IMPATOMEXPORT ProteinLigandAtomPairScore: public PairScore {
  friend class ProteinLigandRestraint;
  core::internal::PMFTable<true> table_;
  double threshold_;
  inline double evaluate(const algebra::VectorD<3> &protein_v,
                         int ptype,
                         const algebra::VectorD<3> &ligand_v,
                         int ltype,
                         core::XYZ pxyz, core::XYZ lxyz,
                         DerivativeAccumulator *da) const;
 public:
  ProteinLigandAtomPairScore(double threshold
                             = std::numeric_limits<double>::max());
  ProteinLigandAtomPairScore(double threshold,
                             TextInput data_file);
  double get_maximum_distance() const;
  IMP_SIMPLE_PAIR_SCORE(ProteinLigandAtomPairScore);
};

IMP_OBJECTS(ProteinLigandAtomPairScore,ProteinLigandAtomPairScores);

/** Score a pair of molecules. See ProteinLigandAtomPairScore for
    simply scoring the atom pairs.
*/
class IMPATOMEXPORT ProteinLigandRestraint: public Restraint {
  IMP::internal::OwnerPointer<ProteinLigandAtomPairScore> score_;
  RefCountingDecorator<Hierarchy> protein_, ligand_;
  void initialize(Hierarchy protein,
                  Hierarchy ligand);
 public:
  ProteinLigandRestraint(Hierarchy protein, Hierarchy ligand,
                         double threshold= std::numeric_limits<double>::max());
  ProteinLigandRestraint(Hierarchy protein, Hierarchy ligand,
                         double threshold,
                         TextInput data_file);
  IMP_RESTRAINT(ProteinLigandRestraint);
};

IMP_OBJECTS(ProteinLigandRestraint,ProteinLigandRestraints);


IMPATOMEXPORT void add_protein_ligand_score_data(Hierarchy h);

/** @} */
IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PROTEIN_LIGAND_SCORE_H */
