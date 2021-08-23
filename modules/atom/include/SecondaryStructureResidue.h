/**
 *  \file IMP/atom/SecondaryStructureResidue.h
 *  \brief A decorator for storing secondary structure probabilities.
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_SECONDARY_STRUCTURE_RESIDUE_H
#define IMPATOM_SECONDARY_STRUCTURE_RESIDUE_H

#include <IMP/atom/atom_config.h>
#include <IMP/atom/atom_macros.h>
#include <IMP/atom/Hierarchy.h>

#include <IMP/base_types.h>
#include <IMP/Particle.h>
#include <IMP/Model.h>
#include <IMP/Decorator.h>

IMPATOM_BEGIN_NAMESPACE

//! A decorator for a residue with probability of secondary structure
/**
   Contains probabilities for each sse type (helix, strand, coil)
 */
class IMPATOMEXPORT SecondaryStructureResidue : public Decorator {
  static void do_setup_particle(Model *m, ParticleIndex pi,
                                Float prob_helix, Float prob_strand,
                                Float prob_coil) {
    m->add_attribute(get_prob_helix_key(), pi, prob_helix);
    m->add_attribute(get_prob_strand_key(), pi, prob_strand);
    m->add_attribute(get_prob_coil_key(), pi, prob_coil);
    if (!Hierarchy::get_is_setup(m, pi)) {
      Hierarchy::setup_particle(m, pi);
    }
    // seems redundant
    SecondaryStructureResidue ssr(m, pi);
    ssr.set_prob_helix(prob_helix);
    ssr.set_prob_strand(prob_strand);
    ssr.set_prob_coil(prob_coil);
  }

 public:
  IMP_DECORATOR_METHODS(SecondaryStructureResidue, Decorator);
  IMP_DECORATOR_SETUP_3(SecondaryStructureResidue, Float, prob_helix, Float,
                        prob_strand, Float, prob_coil);

  //! Set up SecondaryStructureResidue with default probabilities
  static SecondaryStructureResidue setup_particle(Particle *res_p) {
    Float prob_helix = 1.0 / 3.0, prob_strand = 1.0 / 3.0,
          prob_coil = 1.0 / 3.0;
    SecondaryStructureResidue ssr =
        setup_particle(res_p, prob_helix, prob_strand, prob_coil);
    return ssr;
  }

  //! Return true if the particle is a secondary structure residue
  static bool get_is_setup(Model *m, ParticleIndex pi) {
    if (m->get_has_attribute(get_prob_helix_key(), pi) &&
        (m->get_has_attribute(get_prob_strand_key(), pi)) &&
        (m->get_has_attribute(get_prob_coil_key(), pi)))
      return true;
    return false;
  }

  Particle *get_particle() const { return Decorator::get_particle(); }

  //! Return all probabilities in one vector
  Floats get_all_probabilities() {
    Floats res;
    res.push_back(get_prob_helix());
    res.push_back(get_prob_strand());
    res.push_back(get_prob_coil());
    return res;
  }

  IMP_DECORATOR_GET_SET_OPT(prob_helix, get_prob_helix_key(), Float, Float,
                            0.333);
  IMP_DECORATOR_GET_SET_OPT(prob_strand, get_prob_strand_key(), Float, Float,
                            0.333);
  IMP_DECORATOR_GET_SET_OPT(prob_coil, get_prob_coil_key(), Float, Float,
                            0.333);

  static FloatKey get_prob_helix_key();
  static FloatKey get_prob_strand_key();
  static FloatKey get_prob_coil_key();
};
IMP_DECORATORS(SecondaryStructureResidue, SecondaryStructureResidues,
               ParticlesTemp);

//! Coarsen some SecondaryStructureResidues.
/** \returns a SecondaryStructureResidue whose probabilities reflect those
    of the underlying residues. Useful if you want to represent the secondary
    structure contents at a coarser level.
    \param[in] ssr_ps The SSR-decorated particles to be combined
    \param[in] mdl The IMP Model
    \param[in] winner_takes_all_per_res Whether to set prob=1.0 for top
               scoring secondary structure type
 */
IMPATOMEXPORT SecondaryStructureResidue
    setup_coarse_secondary_structure_residue(const Particles &ssr_ps,
                                             Model *mdl,
                                             bool winner_takes_all_per_res =
                                                 false);

/** Groups SecondaryStructureResidues into segments and then coarsens them.
    Useful if you have a long sequence and want to make several coarse nodes.
    \param[in] ssr_ps The SSR-decorated particles to be combined
    \param[in] mdl The IMP Model
    \param[in] coarse_factor Group size
    \param[in] start_res_num Starting residue number for the provided sequence
    \param[in] winner_takes_all_per_res Whether to set prob=1.0 for top
               scoring secondary structure type
 */
IMPATOMEXPORT SecondaryStructureResidues
    setup_coarse_secondary_structure_residues(
        const Particles &ssr_ps, Model *mdl, int coarse_factor,
        int start_res_num, bool winner_takes_all_per_res = false);

//! Compares the secondary structure probabilities of two residues.
/** \returns the RMSD of the three probabilities (lower is better match).
 */
IMPATOMEXPORT Float
    get_secondary_structure_match_score(SecondaryStructureResidue ssr1,
                                        SecondaryStructureResidue ssr2);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SECONDARY_STRUCTURE_RESIDUE_H */
