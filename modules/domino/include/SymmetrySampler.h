/**
 * \file  SymmetrySampler.h
 * \brief Sample transformations of particles while preseving N-symmetry.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_SYMMETRY_SAMPLER_H
#define IMPDOMINO_SYMMETRY_SAMPLER_H

#include "IMP/Particle.h"
#include <map>
#include  <sstream>
#include "IMP/base_types.h"
#include "config.h"
#include "DiscreteSampler.h"
#include "CombState.h"
#include "TransformationDiscreteSet.h"
#include <IMP/algebra/Transformation3D.h>
#include <IMP/algebra/Cylinder3D.h>

IMPDOMINO_BEGIN_NAMESPACE
//! SymmetrySampler
/**
The class symmetrically sample particles.
/note We currently assume that the particles are MolecularHierarchyDecorators,
      it should be changes to general Hierarchy.
 */
class IMPDOMINOEXPORT SymmetrySampler : public DiscreteSampler
{
public:
  //! Constructor
  /**
  /param[in] ps the particles that should obey N-symmetry ( N is the
                number of particles).
  /param[in] ts a set of transformations for the first particle
  /param[in] c  the cylinder represent the symmery axis of the particles
  /note The sampler assumes that the order of the particles in the ring
        is the order of the input particles.
  /note The sampled set of transformations for particle number i is the set of
        input transformations rotated by (360/N)*i around the symmetry axis.
  /todo consider calculating the cylinder in construction
   */
  SymmetrySampler(Particles *ps, TransformationDiscreteSet *ts,
                  const algebra::Cylinder3D &c);
  void set_ref(Particle *p){ref_=p;}
  void move2state(const CombState *cs);
  void populate_states_of_particles(Particles *particles,
               std::map<std::string, CombState *> *states) const;
 protected:

  void reset_placement(const CombState *cs);

  algebra::Cylinder3D cyl_;
  std::map<Particle*,int> symm_deg_;
  Particles *ps_;
  TransformationDiscreteSet *ts_;
  Particle *ref_;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_SYMMETRY_SAMPLER_H */
