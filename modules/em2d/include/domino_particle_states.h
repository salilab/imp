/**
 *  \file ProjectionStates.h
 *  \brief Particles states for a rigid body that is going to be projected
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPEM2D_DOMINO_PARTICLE_STATES_H
#define IMPEM2D_DOMINO_PARTICLE_STATES_H

#include "IMP/em2d/Image.h"
#include "IMP/domino/domino_macros.h"
#include "IMP/domino/particle_states.h"
#include "IMP/core/rigid_bodies.h"
#include "IMP/algebra/Vector3D.h"
#include "IMP/algebra/Rotation3D.h"
#include "IMP/Pointer.h"
#include "IMP/base_types.h"
#include "IMP/Particle.h"

IMPEM2D_BEGIN_NAMESPACE

/*! States that a set of Particles can have according to their positions
    and orientations in a grid. The orientations also determine the orientation
    of the projections of the particles.
  \param[in]
*/
class IMPEM2DEXPORT ProjectionStates: public domino::ParticleStates {

protected:

  algebra::Vector3Ds positions_;
  algebra::Rotation3Ds orientations_;
  Images projections_;
  unsigned int number_of_states_;

public:


  /*!
    \param[in] positions Set of positions that the set of particles
               can have in the grid (points of the grid).
    \param[in] orientations Orientations of the particles in in the grid
    \param[in] Projections of the particles in the given orientations.
               There must be correspondence between the orientations and the
               projections.
    Note: The number of states considered is positions * orientations, as
          each position can have all orientations.
    \
  */
  ProjectionStates(const algebra::Vector3Ds &positions,
                            const algebra::Rotation3Ds &orientations,
                            const Images &projections,
                            const String &name="ProjectionStates") :
                            ParticleStates(name),
                           positions_(positions),
                           orientations_(orientations),
                           projections_(projections) {
    number_of_states_ = positions_.size()*orientations_.size();
  };

  /*! Returns the projection corresponding to the state i;
    \param[in] i
  */
  Image* get_projection(unsigned int  i) const;

  /*! Returns the position corresponding to the state i
    \param[in] i
  */
  algebra::Vector3D get_position(unsigned int  i) const;

  IMP_PARTICLE_STATES(ProjectionStates);

 };


IMPEM2D_END_NAMESPACE

#endif  /* IMPEM2D_DOMINO_PARTICLE_STATES_H */
