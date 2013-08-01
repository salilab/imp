/**
 *  \file IMP/restrainer/simplify_restraint.h
 *  \brief Restrainer functions for restraints
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPRESTRAINER_SIMPLIFY_RESTRAINT_H
#define IMPRESTRAINER_SIMPLIFY_RESTRAINT_H

#include <IMP/restrainer/restrainer_config.h>
#include <IMP/SingletonContainer.h>
#include <IMP/atom.h>
#include <IMP/core.h>
#include <IMP/em/FitRestraint.h>
#include <IMP/em/DensityMap.h>
#include <IMP/core/rigid_bodies.h>

IMPRESTRAINER_BEGIN_NAMESPACE

class SimpleConnectivity;
class SimpleDistance;
class SimpleDiameter;
class SimpleExcludedVolume;
class SimpleEMFit;


/* Having the default be RigidMembersRefiner is most likely a bad
   idea, but it is needed to make things work. The refiner should
   probably be exposed, otherwise this method cannot be used with the
   rigid bodies created from molecular hierarchies, which is kind of
   an unintuitive result. A better solution may be to ensure that
   everything is an atom.Hierarchy and use the LeavesRefiner
   implicitly.

 */
IMPRESTRAINEREXPORT SimpleConnectivity
create_simple_connectivity_on_rigid_bodies(const core::RigidBodies &rbs,
                                           Refiner
 *ref=IMP::core::internal::get_rigid_members_refiner());


/** Creates ConnectivityRestraint on molecules using LowestRefinedPairScore
    and LeavesRefiner.

    \see LowestRefinedPairScore
    \see SphereDistancePairScore
    \see HarmonicUpperBound
    \see LeavesRefiner
    See SimpleConnectivity
  */
IMPRESTRAINEREXPORT SimpleConnectivity create_simple_connectivity_on_molecules(
                const atom::Hierarchies &mhs);


/** Creates DistanceRestraint using HarmonicUpperBound scoring function
    as default.
    \param[in] ps Pointer to two particles in distance restraint.
    See SimpleDistance
*/
IMPRESTRAINEREXPORT SimpleDistance create_simple_distance(const Particles &ps);



/** Creates DiameterRestraint using HarmonicUpperBound scoring function
    as default.
    \param[in] ps Pointer to particles in diameter restraint.
    \param[in] diameter Diameter.
    \see ListSingletonContainer
    See SimpleDiameter
  */
IMPRESTRAINEREXPORT SimpleDiameter create_simple_diameter(
                const Particles &ps, Float diameter);


/* Having the default be RigidMembersRefiner is most likely a bad
   idea, but it is needed to make things work. The refiner should
   probably be exposed, otherwise this method cannot be used with the
   rigid bodies created from molecular hierarchies, which is kind of
   an unintuitive result. A better solution may be to ensure that
   everything is an atom.Hierarchy and use the LeavesRefiner
   implicitly (as the docs say, but not as it was doing before).
 */

/** Creates ExcludedVolumeRestraint using LeavesRefiner.
    \see ListSingletonContainer
    See SimpleExcludedVolume
*/
IMPRESTRAINEREXPORT SimpleExcludedVolume
                create_simple_excluded_volume_on_rigid_bodies(
                               const core::RigidBodies &rbs,
                Refiner *ref= IMP::core::internal::get_rigid_members_refiner());



/** Creates ExcludedVolumeRestraint using RigidMembersRefiner.
    \see RigidMembersRefiner
    See SimpleExcludedVolume
*/
IMPRESTRAINEREXPORT SimpleExcludedVolume
                create_simple_excluded_volume_on_molecules(
                const atom::Hierarchies &mhs);



/** Creates EM FitRestraint.
    \see FitRestraint
    \see DensityMap
    See SimpleEMFit
*/
IMPRESTRAINEREXPORT SimpleEMFit create_simple_em_fit(
                atom::Hierarchies const &mhs, em::DensityMap *dmap);
//! Creates EM FitRestraint.
/**
   \param[in] mh a molecule to fit in the density
   \param[in] dmap the density map
    \see FitRestraint
    \see DensityMap
    See SimpleEMFit
*/
IMPRESTRAINEREXPORT SimpleEMFit create_simple_em_fit(
    atom::Hierarchy const &mh, em::DensityMap *dmap);

/** Load EM density file (.mrc or .em file). */
IMPRESTRAINEREXPORT em::DensityMap *load_em_density_map(
                char const *map_fn, float spacing, float resolution);

/** Set each of the given hierachies to be a rigid body.
    \deprecated_at{2.1} Use atom::create_rigid_body() instead. */
IMPRESTRAINER_DEPRECATED_FUNCTION_DECL(2.1)
IMPRESTRAINEREXPORT core::RigidBodies set_rigid_bodies(
                atom::Hierarchies const &mhs);

//! Simple connectivity restraint.
/**
  \note SimpleConnectivity stores pointers to ConnectivityRestraint,
        HarmonicUpperBound, and SphereDistancePairScore.
  \note It provides convenient methods to change mean, k,
        and standard deviation.
  \see ConnectivityRestraint
 */
class IMPRESTRAINEREXPORT SimpleConnectivity
{
  IMP_NO_SWIG(
  friend IMPRESTRAINEREXPORT SimpleConnectivity
  create_simple_connectivity_on_rigid_bodies(
                        const core::RigidBodies &rbs, Refiner *ref);
  friend IMPRESTRAINEREXPORT SimpleConnectivity
  create_simple_connectivity_on_molecules(
         atom::Hierarchies const &mhs);
              )
 public:
  core::ConnectivityRestraint *get_restraint()
  {
    return connectivity_restraint_;
  }

  core::HarmonicUpperBound *get_harmonic_upper_bound()
  {
    return harmonic_upper_bound_;
  }

  core::SphereDistancePairScore *get_sphere_distance_pair_score()
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
  void set_standard_deviation(Float sd)
  {
     static Float k = harmonic_upper_bound_->get_k_from_standard_deviation(sd);
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
    return IMP::VersionInfo("restrainer",
                            get_module_version());
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

  base::Pointer<core::ConnectivityRestraint> connectivity_restraint_;
  base::Pointer<core::HarmonicUpperBound> harmonic_upper_bound_;
  base::Pointer<core::SphereDistancePairScore> sphere_distance_pair_score_;
};
IMP_VALUES(SimpleConnectivity, SimpleConnectivities);

//! Simple distance restraint between two particles.
/**
   \note SimpleDistance stores pointers to DistanceRestraint, and
         HarmonicUpperBound.
   \note It provides convenient methods to change mean, k, and
         standard deviation.

   \see DistanceRestraint
 */
class IMPRESTRAINEREXPORT SimpleDistance
{
  IMP_NO_SWIG(friend IMPRESTRAINEREXPORT SimpleDistance
  create_simple_distance(const Particles &ps));
 public:
  core::DistanceRestraint *get_restraint()
  {
    return distance_restraint_;
  }

  core::HarmonicUpperBound *get_harmonic_upper_bound()
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
  void set_standard_deviation(Float sd)
  {
     static Float k = harmonic_upper_bound_->get_k_from_standard_deviation(sd);
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
    return VersionInfo("restrainer",
                       get_module_version());
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

  base::Pointer<core::DistanceRestraint> distance_restraint_;
  base::Pointer<core::HarmonicUpperBound> harmonic_upper_bound_;
};
IMP_VALUES(SimpleDistance, SimpleDistances);

//! Simple diameter restraint.
/**
   \note SimpleDiameter stores pointers to DiameterRestraint, and
         HarmonicUpperBound.
   \note It provides convenient methods to change mean, k, and
         standard deviation.
   \see DiameterRestraint
 */
class IMPRESTRAINEREXPORT SimpleDiameter
{
  IMP_NO_SWIG(friend IMPRESTRAINEREXPORT SimpleDiameter
  create_simple_diameter(const Particles &ps,
                                                           Float diameter));
 public:

  core::DiameterRestraint *get_restraint()
  {
    return diameter_restraint_;
  }

  core::HarmonicUpperBound *get_harmonic_upper_bound()
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
  void set_standard_deviation(Float sd)
  {
     static Float k = harmonic_upper_bound_->get_k_from_standard_deviation(sd);
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
    return VersionInfo("restrainer",
                       get_module_version());
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

  base::Pointer<core::DiameterRestraint> diameter_restraint_;
  base::Pointer<core::HarmonicUpperBound> harmonic_upper_bound_;
};
IMP_VALUES(SimpleDiameter, SimpleDiameters);

//! Simple excluded volume restraint.
/**
   \see ExcludedVolumeRestraint
 */
class IMPRESTRAINEREXPORT SimpleExcludedVolume
{
  IMP_NO_SWIG(friend IMPRESTRAINEREXPORT SimpleExcludedVolume
              create_simple_excluded_volume_on_rigid_bodies(
                    const core::RigidBodies &rbs, Refiner*ref)
              );
  IMP_NO_SWIG(friend IMPRESTRAINEREXPORT SimpleExcludedVolume
              create_simple_excluded_volume_on_molecules(
              atom::Hierarchies const &mhs)
              );
 public:

  core::ExcludedVolumeRestraint *get_restraint()
  {
    return excluded_volume_restraint_;
  }

  VersionInfo get_version_info() const
  {
    return VersionInfo("restrainer",
                       get_module_version());
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

  base::Pointer<core::ExcludedVolumeRestraint> excluded_volume_restraint_;
};
IMP_VALUES(SimpleExcludedVolume, SimpleExcludedVolumes);

//! Simple EM fit restraint.
/**
  \note SimpleEMFit stores pointer to FitRestraint.
  \see FitRestraint
 */
class IMPRESTRAINEREXPORT SimpleEMFit
{
  IMP_NO_SWIG(friend IMPRESTRAINEREXPORT SimpleEMFit create_simple_em_fit(
              atom::Hierarchies const &mhs, em::DensityMap *dmap));
public:
  em::FitRestraint *get_restraint()
  {
    return fit_restraint_;
  }

  VersionInfo get_version_info() const
  {
    return VersionInfo("restrainer",
                       get_module_version());
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

  base::Pointer<em::FitRestraint> fit_restraint_;
};
IMP_VALUES(SimpleEMFit, SimpleEMFits);

IMPRESTRAINER_END_NAMESPACE

#endif /* IMPRESTRAINER_SIMPLIFY_RESTRAINT_H */
