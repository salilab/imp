/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_STATIC_COORDINATES_H
#define IMPRMF_ATOM_LINKS_STATIC_COORDINATES_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>
#include <IMP/kernel/Model.h>
IMPRMF_BEGIN_INTERNAL_NAMESPACE

/** Load coordinates that don't change between frames. */
class IMPRMFEXPORT HierarchyLoadStaticCoordinates {
  RMF::decorator::IntermediateParticleConstFactory
      intermediate_particle_factory_;
  RMF::decorator::ReferenceFrameConstFactory reference_frame_factory_;

 public:
  HierarchyLoadStaticCoordinates(RMF::FileConstHandle f);
  void setup_particle(RMF::NodeConstHandle n, unsigned int node_state,
                      kernel::Model *m, kernel::ParticleIndex p,
                      const kernel::ParticleIndexes &rigid_bodies);
};

/** Save coordinates that don't change between frames. */
class IMPRMFEXPORT HierarchySaveStaticCoordinates {
  RMF::decorator::IntermediateParticleFactory intermediate_particle_factory_;
  RMF::decorator::ReferenceFrameFactory reference_frame_factory_;

 public:
  HierarchySaveStaticCoordinates(RMF::FileHandle f);
  bool setup_node(kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n,
                  kernel::ParticleIndexes rigid_bodies);
};

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_STATIC_COORDINATES_H */
