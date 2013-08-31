/**
 *  \file IMP/multifit/ComplementarityRestraint.h
 *  \brief Calculate weighted excluded volume between rigid bodies
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_COMPLEMENTARITY_RESTRAINT_H
#define IMPMULTIFIT_COMPLEMENTARITY_RESTRAINT_H

#include <IMP/multifit/multifit_config.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/core/DataObject.h>
#include <IMP/algebra/standard_grids.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Compute the complementarity between two molecules.
/**
    The score returned is infinity if the two molecules overlap by more
    than the value set for set_maximum_penetration_score() or if the
    two molecules are separated by more than set_maximum_separation().

    Otherwise, the integral of the set_complementarity_value() value over
    a layer of thickness set_complementarity_thickness() around the molecule
    is returned.

    \note currently each of the particle lists must belong to exactly one
    rigid body. And, currently, each rigid body can only be associated with
    one list of particles (that is, there cannot be multiple restraints with
    different lists of particles that all belong to the same rigid body).

    \note The restraint does not support derivatives.

    \note The restraint pretty much only makes sense for use with Domino as it
    returns infinity for many cases. Monte Carlo might make sense with certain
    parameters.
*/
class IMPMULTIFITEXPORT ComplementarityRestraint : public kernel::Restraint
{
public:
  ComplementarityRestraint(const kernel::ParticlesTemp &a, const kernel::ParticlesTemp &b,
                           std::string name="ComplementarityRestraint %1%");
  /** If the two molecules have a penetration score of more than this,
      infinity is returned as the score. This score is roughly the number of
      cubic angstroms that the two molecules overlap (assuming only one
      internal layer). */
  void set_maximum_penetration_score(double s) {
    maximum_penetration_score_=s;
  }
  /** Alternatively, you can bound the maximum allowed penetration.
   */
  void set_maximum_penetration(double d) {
    maximum_penetration_=d;
  }
  /** If the two molecules' bounding sphere surfaces are separated by more
      than this distance, infinity is returned.
   */
  void set_maximum_separation(double s) {
    maximum_separation_=s;
  }
  /** Set the thickness to use for the external complementarity layer.*/
  void set_complementarity_thickness(double th) {
    complementarity_thickness_=th;
    update_voxel();
  }
  /** Set the value to use for external complementarity voxels.
   */
  void set_complementarity_value(double th) {
    complementarity_value_=th;
  }
  /** Set the thickness of the interior layers. */
  void set_interior_layer_thickness(double th) {
    interior_thickness_=th;
    update_voxel();
  }

  void set_penetration_coefficient(double pc)
  {
    penetration_coef_ = pc;
  }

  void set_complementarity_coefficient(double cc)
  {
    complementarity_coef_ = cc;
  }

  void set_boundary_coefficient(double bc)
  {
    boundary_coef_ = bc;
  }
  float get_voxel_size() const {return voxel_size_;}
  virtual double
  unprotected_evaluate(IMP::kernel::DerivativeAccumulator *accum)
     const IMP_OVERRIDE;
  virtual IMP::kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ComplementarityRestraint);
#ifndef IMP_DOXYGEN
  double unprotected_evaluate_if_good(DerivativeAccumulator *accum,
                                      double max) const;
#endif
 private:
  typedef std::pair<algebra::Transformation3D,
                    algebra::DenseGrid3D<float> > GridPair;
  typedef core::DataObject<GridPair> GridObject;
  GridObject *get_grid_object(core::RigidBody rb,
                              const kernel::ParticlesTemp &a,
                              ObjectKey ok,
                              double thickness,
                              double value,
                              double interior_thickness,
                              double voxel) const;
  IMP::algebra::DenseGrid3D<float>
    get_grid(const kernel::ParticlesTemp &a,
             double thickness,
             double value,
             double interior_thickness,
             double voxel) const;
  void update_voxel();
  kernel::ParticlesTemp a_, b_;
  core::RigidBody rba_, rbb_;
  ObjectKey ok_;
  // parameters
  double maximum_separation_, maximum_penetration_score_;
  double maximum_penetration_;
  double complementarity_thickness_, complementarity_value_;
  double penetration_coef_, complementarity_coef_, boundary_coef_;
  double interior_thickness_, voxel_size_;
};

IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_COMPLEMENTARITY_RESTRAINT_H */
