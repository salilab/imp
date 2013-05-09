/**
 *  \file IMP/atom/LennardJonesPairScore.h
 *  \brief Lennard-Jones score between a pair of particles.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_LENNARD_JONES_PAIR_SCORE_H
#define IMPATOM_LENNARD_JONES_PAIR_SCORE_H

#include <IMP/atom/atom_config.h>
#include <IMP/generic.h>
#include <IMP/PairScore.h>
#include <IMP/Pointer.h>
#include <IMP/atom/LennardJones.h>
#include <IMP/atom/smoothing_functions.h>

IMPATOM_BEGIN_NAMESPACE

//! Lennard-Jones score between a pair of particles.
/** The two particles in the pair must be LennardJones particles.
    The form of the potential is \f[
       -\epsilon \left[ w_{rep} \left(\frac{r_{min}}{r}\right)^{12}
                       - 2 w_{att} \left(\frac{r_{min}}{r}\right)^{6}\right]
    \f] where \f$\epsilon\f$ is the depth of the well between the
    two particles, \f$r_{min}\f$ the sum of the particles' radii, \f$r\f$
    the interparticle distance, and \f$w_{rep}\f$ and \f$w_{att}\f$ the
    weights on the repulsive and attractive parts of the potential respectively;
    both weights are 1.0 by default.

    The well depth is the geometric mean of the individual particles' well
    depths (as extracted by LennardJones::get_well_depth).

    Note that because this score uses radii and well depths set in the particles
    themselves, the strength of the interaction cannot be changed for a
    particular pair of atoms (as is done in the CHARMM forcefield with the
    rarely-used NBFIX directive, for example). If the well depth or radius of
    a single particle is modifed, that will affect its interaction with all
    particles.
 */
class IMPATOMEXPORT LennardJonesPairScore : public PairScore {
  IMP::OwnerPointer<SmoothingFunction> smoothing_function_;
  double repulsive_weight_, attractive_weight_;

  // Calculate A, B factors from particle well depths and radii
  // It may be appropriate to cache these for speed since the particle
  // attributes rarely change and square roots are expensive
  inline void get_factors(const LennardJones &lj0, const LennardJones &lj1,
                          double &A, double &B) const {
    double well_depth = std::sqrt(lj0.get_well_depth() * lj1.get_well_depth());
    double rmin = lj0.get_radius() + lj1.get_radius();
    // probably faster than pow(rmin, 6) on systems that don't
    // have pow(double, int)
    double rmin6 = rmin * rmin * rmin * rmin * rmin * rmin;
    double rmin12 = rmin6 * rmin6;

    A = well_depth * rmin12 * repulsive_weight_;
    B = 2.0 * well_depth * rmin6 * attractive_weight_;
  }

 public:
  LennardJonesPairScore(SmoothingFunction *f)
      : smoothing_function_(f),
        repulsive_weight_(1.0),
        attractive_weight_(1.0) {}

  void set_repulsive_weight(double repulsive_weight) {
    repulsive_weight_ = repulsive_weight;
  }

  double get_repulsive_weight() const { return repulsive_weight_; }

  void set_attractive_weight(double attractive_weight) {
    attractive_weight_ = attractive_weight;
  }

  double get_attractive_weight() const { return attractive_weight_; }

  IMP_SIMPLE_PAIR_SCORE(LennardJonesPairScore);
};

IMP_OBJECTS(LennardJonesPairScore, LennardJonesPairScores);

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_LENNARD_JONES_PAIR_SCORE_H */
