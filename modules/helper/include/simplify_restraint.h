/**
 *  \file helper/simplify_restraint.h
 *  \brief Helper functions for restraints
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPHELPER_SIMPLIFY_RESTRAINT_H
#define IMPHELPER_SIMPLIFY_RESTRAINT_H

#include "config.h"
#include <IMP/SingletonContainer.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include <IMP/misc.h>
#include <IMP/em/FitRestraint.h>
#include <IMP/em/DensityMap.h>
#include <IMP/core/rigid_bodies.h>

IMPHELPER_BEGIN_NAMESPACE

class SimpleCollision;
class SimpleConnectivity;
class SimpleDistance;
class SimpleDiameter;
class SimpleExcludedVolume;
class SimpleEMFit;

/** Creates collision detection on rigid bodies using
    SphereDistancePairScore and ClosePairsScoreState.
    The nonbonded list defined for the score state is refined using
    RigidClosePairsFinder.

    \see SphereDistancePairScore
    \see HarmonicLowerBound
    \see ClosePairsScoreState
    \see ListSingletonContainer
    \see RigidClosePairsFinder
    \relates SimpleCollision
  */
IMPHELPEREXPORT SimpleCollision create_simple_collision_on_rigid_bodies(
                core::RigidBodies *rbs);

/** Creates ConnectivityRestraint on rigid bodies using
    RigidBodyDistancePairScore and RigidMembersRefiner.

    \see RigidBodyDistancePairScore
    \see SphereDistancePairScore
    \see HarmonicUpperBound
    \see core::RigidMembersRefiner
    \relates SimpleConnectivity
  */
IMPHELPEREXPORT SimpleConnectivity create_simple_connectivity_on_rigid_bodies(
                core::RigidBodies *rbs);


/** Creates ConnectivityRestraint on molecules using LowestRefinedPairScore
    and LeavesRefiner.

    \see LowestRefinedPairScore
    \see SphereDistancePairScore
    \see HarmonicUpperBound
    \see LeavesRefiner
    \relates SimpleConnectivity
  */
IMPHELPEREXPORT SimpleConnectivity create_simple_connectivity_on_molecules(
                Particles *ps);


/** Creates DistanceRestraint using HarmonicUpperBound scoring function
    as default.
    \param[in] ps Pointer to two particles in distance restraint.
    \relates SimpleDistance
*/
IMPHELPEREXPORT SimpleDistance create_simple_distance(Particles *ps);



/** Creates DiameterRestraint using HarmonicUpperBound scoring function
    as default.
    \param[in] ps Pointer to particles in diameter restraint.
    \param[in] diameter Diameter.
    \see ListSingletonContainer
    \relates SimpleDiameter
  */
IMPHELPEREXPORT SimpleDiameter create_simple_diameter(
                Particles *ps, Float diameter);



/** Creates ExcludedVolumeRestraint using LeavesRefiner.
    \see ListSingletonContainer
    \relates SimpleExcludedVolume
*/
IMPHELPEREXPORT SimpleExcludedVolume
                create_simple_excluded_volume_on_rigid_bodies(
                core::RigidBodies *rbs);

IMPHELPEREXPORT SimpleEMFit create_simple_em_fit(
                atom::Hierarchies const &mhs, em::DensityMap *dmap);


/** Load MRC file. */
IMPHELPEREXPORT em::DensityMap *load_mrc_density_map(
                char const *map_fn, float spacing, float resolution);

/** Load density files in EM format. */
IMPHELPEREXPORT em::DensityMap *load_erw_density_map(
                char const *map_fn, float spacing, float resolution);

IMPHELPEREXPORT Particles set_rigid_bodies(atom::Hierarchies const &mhs);


//! Simple collision detection.
/**
  \note SimpleCollision stores pointers to PairsRestraint, HarmonicLowerBound,
        SphereDistancePairScore, and ClosePairsScoreState.
  \note It provides convenient methods to change mean, k, standard deviation,
        slack and distance.
  \see PairsRestraint
 */
class IMPHELPEREXPORT SimpleCollision
{
  IMP_NO_SWIG(friend SimpleCollision create_simple_collision_on_rigid_bodies(
                                                   core::RigidBodies *rbs));
public:
  core::PairsRestraint *restraint()
  {
    return pairs_restraint_;
  }

  core::HarmonicLowerBound *harmonic_lower_bound()
  {
    return harmonic_lower_bound_;
  }

  core::SphereDistancePairScore *sphere_distance_pair_score()
  {
    return sphere_distance_pair_score_;
  }

  core::ClosePairsScoreState *close_pairs_score_state()
  {
    return close_pairs_score_state_;
  }

  //! Set the mean for the HarmonicLowerBound.
  /** The default mean is 0.
   */
  void set_mean(Float mean)
  {
     harmonic_lower_bound_->set_mean(mean);
  }

  //! Set the standard deviation for the HarmonicLowerBound.
  void set_stddev(Float sd)
  {
     static Float k = harmonic_lower_bound_->k_from_standard_deviation(sd);
     harmonic_lower_bound_->set_k(k);
  }

  //! Set the spring constant for the HarmonicLowerBound.
  /** The default k is 1.
   */
  void set_k(Float k)
  {
     harmonic_lower_bound_->set_k(k);
  }

  //! Set the amount of slack for the ClosePairsScoreState.
  /** The default slack is 2.
   */
  void set_slack(Float slack)
  {
     close_pairs_score_state_->set_slack(slack);
  }

  //! Set the distance threshold for the ClosePairsScoreState.
  /** Uses the default distance in the ClosePairsScoreState.
   */
  void set_distance(Float distance)
  {
     close_pairs_score_state_->set_distance(distance);
  }

  VersionInfo get_version_info() const
  {
    return IMP::get_module_version_info();
  }

  void show(std::ostream &out = std::cout) const
  {
    out << "SimpleCollision(";
    if ( pairs_restraint_ )
      pairs_restraint_->show(out);
    out << ")";
  }

private:
  // prevent unauthorized creation
  SimpleCollision(core::PairsRestraint *pairs_restraint,
      core::HarmonicLowerBound *harmonic_lower_bound,
      core::SphereDistancePairScore *sphere_distance_pair_score,
      core::ClosePairsScoreState *close_pairs_score_state)
    : pairs_restraint_(pairs_restraint)
    , harmonic_lower_bound_(harmonic_lower_bound)
    , sphere_distance_pair_score_(sphere_distance_pair_score)
    , close_pairs_score_state_(close_pairs_score_state)
  {}

  IMP::Pointer<core::PairsRestraint> pairs_restraint_;
  IMP::Pointer<core::HarmonicLowerBound> harmonic_lower_bound_;
  IMP::Pointer<core::SphereDistancePairScore> sphere_distance_pair_score_;
  IMP::Pointer<core::ClosePairsScoreState> close_pairs_score_state_;
};

//! Simple connectivity restraint.
/**
  \note SimpleConnectivity stores pointers to ConnectivityRestraint,
        HarmonicUpperBound, and SphereDistancePairScore.
  \note It provides convenient methods to change mean, k,
        and standard deviation.
  \see ConnectivityRestraint
 */
class IMPHELPEREXPORT SimpleConnectivity
{
  IMP_NO_SWIG(
  friend SimpleConnectivity create_simple_connectivity_on_rigid_bodies(
         core::RigidBodies *rbs);
  friend SimpleConnectivity create_simple_connectivity_on_molecules(
         Particles *ps);
              )
 public:
  core::ConnectivityRestraint *restraint()
  {
    return connectivity_restraint_;
  }

  core::HarmonicUpperBound *harmonic_upper_bound()
  {
    return harmonic_upper_bound_;
  }

  core::SphereDistancePairScore *sphere_distance_pair_score()
  {
    return sphere_distance_pair_score_;
  }

  //! Set the mean for the HarmonicUpperBound.
  /** The default mean is 0.
   */
  void set_mean(Float mean)
  {
     harmonic_upper_bound_->set_mean(mean);
  }

  //! Set the standard deviation for the HarmonicUpperBound.
  void set_stddev(Float sd)
  {
     static Float k = harmonic_upper_bound_->k_from_standard_deviation(sd);
     harmonic_upper_bound_->set_k(k);
  }

  //! Set the spring constant for the HarmonicUpperBound.
  /** The default k is 1.
   */
  void set_k(Float k)
  {
     harmonic_upper_bound_->set_k(k);
  }

  VersionInfo get_version_info() const
  {
    return IMP::get_module_version_info();
  }

  void show(std::ostream &out = std::cout) const
  {
    out << "SimpleConnectivity(";
    if ( connectivity_restraint_ )
      connectivity_restraint_->show(out);
    out << ")";
  }

private:
  // prevent unauthorized creation
  SimpleConnectivity(
      core::ConnectivityRestraint *connectivity_restraint,
      core::HarmonicUpperBound *harmonic_upper_bound,
      core::SphereDistancePairScore *sphere_distance_pair_score)
    : connectivity_restraint_(connectivity_restraint)
    , harmonic_upper_bound_(harmonic_upper_bound)
    , sphere_distance_pair_score_(sphere_distance_pair_score)
  {}

  IMP::Pointer<core::ConnectivityRestraint> connectivity_restraint_;
  IMP::Pointer<core::HarmonicUpperBound> harmonic_upper_bound_;
  IMP::Pointer<core::SphereDistancePairScore> sphere_distance_pair_score_;
};

//! Simple distance restraint between two particles.
/**
   \note SimpleDistance stores pointers to DistanceRestraint, and
         HarmonicUpperBound.
   \note It provides convenient methods to change mean, k, and
         standard deviation.

   \see DistanceRestraint
 */
class IMPHELPEREXPORT SimpleDistance
{
  IMP_NO_SWIG(friend SimpleDistance create_simple_distance(Particles *ps));
 public:
  core::DistanceRestraint *restraint()
  {
    return distance_restraint_;
  }

  core::HarmonicUpperBound *harmonic_upper_bound()
  {
    return harmonic_upper_bound_;
  }

  //! Set the mean for the HarmonicUpperBound.
  /** The default mean is 0.
   */
  void set_mean(Float mean)
  {
     harmonic_upper_bound_->set_mean(mean);
  }

  //! Set the standard deviation for the HarmonicUpperBound.
  void set_stddev(Float sd)
  {
     static Float k = harmonic_upper_bound_->k_from_standard_deviation(sd);
     harmonic_upper_bound_->set_k(k);
  }

  //! Set the spring constant for the HarmonicUpperBound.
  /** The default k is 1.
   */
  void set_k(Float k)
  {
     harmonic_upper_bound_->set_k(k);
  }

  VersionInfo get_version_info() const
  {
    return IMP::get_module_version_info();
  }

  void show(std::ostream &out = std::cout) const
  {
    out << "SimpleDistance(";
    if ( distance_restraint_ )
      distance_restraint_->show(out);
    out << ")";
  }

private:
  // prevent unauthorized creation
  SimpleDistance(
      core::DistanceRestraint *distance_restraint,
      core::HarmonicUpperBound *harmonic_upper_bound)
    : distance_restraint_(distance_restraint)
    , harmonic_upper_bound_(harmonic_upper_bound)
  {}

  IMP::Pointer<core::DistanceRestraint> distance_restraint_;
  IMP::Pointer<core::HarmonicUpperBound> harmonic_upper_bound_;
};

//! Simple diameter restraint.
/**
   \note SimpleDiameter stores pointers to DiameterRestraint, and
         HarmonicUpperBound.
   \note It provides convenient methods to change mean, k, and
         standard deviation.
   \see DiameterRestraint
 */
class IMPHELPEREXPORT SimpleDiameter
{
  IMP_NO_SWIG(friend SimpleDiameter create_simple_diameter(Particles *ps,
                                                           Float diameter));
 public:

  core::DiameterRestraint *restraint()
  {
    return diameter_restraint_;
  }

  core::HarmonicUpperBound *harmonic_upper_bound()
  {
    return harmonic_upper_bound_;
  }

  //! Set the mean for the HarmonicUpperBound.
  /** The default mean is 0.
   */
  void set_mean(Float mean)
  {
     harmonic_upper_bound_->set_mean(mean);
  }

  //! Set the standard deviation for the HarmonicUpperBound.
  void set_stddev(Float sd)
  {
     static Float k = harmonic_upper_bound_->k_from_standard_deviation(sd);
     harmonic_upper_bound_->set_k(k);
  }

  //! Set the spring constant for the HarmonicUpperBound.
  /** The default k is 1.
   */
  void set_k(Float k)
  {
     harmonic_upper_bound_->set_k(k);
  }

  VersionInfo get_version_info() const
  {
    return IMP::get_module_version_info();
  }

  void show(std::ostream &out = std::cout) const
  {
    out << "SimpleDiameter(";
    if ( diameter_restraint_ )
      diameter_restraint_->show(out);
    out << ")";
  }

private:
  // prevent unauthorized creation
  SimpleDiameter(
      core::DiameterRestraint *diameter_restraint,
      core::HarmonicUpperBound *harmonic_upper_bound)
    : diameter_restraint_(diameter_restraint)
    , harmonic_upper_bound_(harmonic_upper_bound)
  {}

  IMP::Pointer<core::DiameterRestraint> diameter_restraint_;
  IMP::Pointer<core::HarmonicUpperBound> harmonic_upper_bound_;
};

//! Simple excluded volume restraint.
/**
   \see ExcludedVolumeRestraint
 */
class IMPHELPEREXPORT SimpleExcludedVolume
{
  IMP_NO_SWIG(friend SimpleExcludedVolume
              create_simple_excluded_volume_on_rigid_bodies(
         core::RigidBodies *rbs);
              )
 public:

  core::ExcludedVolumeRestraint *restraint()
  {
    return excluded_volume_restraint_;
  }

  VersionInfo get_version_info() const
  {
    return IMP::get_module_version_info();
  }

  void show(std::ostream &out = std::cout) const
  {
    out << "SimpleExcludedVolume(";
    if ( excluded_volume_restraint_ )
      excluded_volume_restraint_->show(out);
    out << ")";
  }

private:
  // prevent unauthorized creation
  SimpleExcludedVolume(
      core::ExcludedVolumeRestraint *excluded_volume_restraint)
    : excluded_volume_restraint_(excluded_volume_restraint)
  {}

  IMP::Pointer<core::ExcludedVolumeRestraint> excluded_volume_restraint_;
};

//! Simple EM fit restraint.
/**
  \note SimpleEMFit stores pointer to FitRestraint.
  \see FitRestraint
 */
class IMPHELPEREXPORT SimpleEMFit
{
public:

  /** Creates EM FitRestraint.
   \see FitRestraint
   \see DensityMap
  */
  friend SimpleEMFit create_simple_em_fit(atom::Hierarchies const &mhs,
                                            em::DensityMap *dmap);

  em::FitRestraint *restraint()
  {
    return fit_restraint_;
  }

  VersionInfo get_version_info() const
  {
    return IMP::get_module_version_info();
  }

  void show(std::ostream &out = std::cout) const
  {
    out << "SimpleEMFit(";
    if ( fit_restraint_ )
      fit_restraint_->show(out);
    out << ")";
  }

private:
  // prevent unauthorized creation
  SimpleEMFit(
      em::FitRestraint *fit_restraint)
    : fit_restraint_(fit_restraint)
  {}

  IMP::Pointer<em::FitRestraint> fit_restraint_;
};

IMPHELPER_END_NAMESPACE

#endif /* IMPHELPER_SIMPLIFY_RESTRAINT_H */
