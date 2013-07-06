/**
 *  \file symmetry/BallMover.h
 *  \brief A mover that keeps a particle in a box
 *
 *  Copyright 2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSYMMETRY_BALL_MOVER_H
#define IMPSYMMETRY_BALL_MOVER_H

#include "symmetry_config.h"
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/MonteCarloMover.h>
#include <IMP/algebra.h>

IMPSYMMETRY_BEGIN_NAMESPACE

//! Move a particle and keep it in the primitive cell of a periodic lattice
class IMPSYMMETRYEXPORT BallMover : public core::MonteCarloMover
{
public:
  /** The particle is moved within a primitive cell of a periodic lattice
      \param[in] p is the master particle
      \param[in] ps are the slave particles
      \param[in] max_tr is the maximum translation during a step
      \param[in] ctrs is a list of all cell centers
      \param[in] trs is the list of transformations from primitive to all cells
   */
  BallMover(Particle *p, Particles ps, Float max_tr,
             algebra::Vector3Ds ctrs, algebra::Transformation3Ds trs);


protected:
  virtual kernel::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  virtual core::MonteCarloMoverResult do_propose() IMP_OVERRIDE;
  virtual void do_reject() IMP_OVERRIDE;
  IMP_OBJECT_METHODS(BallMover);
private:
  //! Master particle
  IMP::base::OwnerPointer<Particle> p_;
  //! List of slave particles
  Particles ps_;
  //! Maximum translation
  Float max_tr_;
  //! List of centers of all (including primitive) cells
  algebra::Vector3Ds ctrs_;
  //! List of transformations from primitive to all (including primitive) cells
  algebra::Transformation3Ds trs_;
  // master particle old coordinates
  algebra::Vector3D  oldcoord_;
  // slave particles old coordinates
  algebra::Vector3Ds oldcoords_;
};

IMPSYMMETRY_END_NAMESPACE

#endif  /* IMPSYMMETRY_BALL_MOVER_H */
