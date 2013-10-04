/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_LOCAL_COORDINATES_H
#define IMPRMF_ATOM_LINKS_LOCAL_COORDINATES_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>
#include <IMP/kernel/particle_index.h>
#include <IMP/kernel/Model.h>
IMPRMF_BEGIN_INTERNAL_NAMESPACE

class IMPRMFEXPORT HierarchyLoadLocalCoordinates {
  RMF::IntermediateParticleConstFactory intermediate_particle_factory_;
  RMF::ReferenceFrameConstFactory reference_frame_factory_;
  typedef std::pair<RMF::NodeID, kernel::ParticleIndex> Pair;
  typedef base::Vector<Pair> NodeParticlePairs;
  NodeParticlePairs xyzs_;
  NodeParticlePairs rigid_bodies_;

 public:
  HierarchyLoadLocalCoordinates(RMF::FileConstHandle f);
  bool setup_particle(RMF::NodeConstHandle n, unsigned int node_state,
                      kernel::Model *m,
                      kernel::ParticleIndex p,
                      const kernel::ParticleIndexes &rigid_bodies);
  enum Type {
    NONE = 0,
    PARTICLE,
    RIGID_BODY
  };
  Type link_particle(RMF::NodeConstHandle n, unsigned int node_state,
                     kernel::Model *m,
                     kernel::ParticleIndex p,
                     const kernel::ParticleIndexes &rigid_bodies);
  void load(RMF::FileConstHandle fh, kernel::Model *m);
};

class IMPRMFEXPORT HierarchySaveLocalCoordinates {
  RMF::IntermediateParticleFactory intermediate_particle_factory_;
  RMF::ReferenceFrameFactory reference_frame_factory_;
  typedef std::pair<RMF::NodeID, kernel::ParticleIndex> Pair;
  typedef base::Vector<Pair> NodeParticlePairs;
  NodeParticlePairs xyzs_;
  NodeParticlePairs rigid_bodies_;

 public:
  HierarchySaveLocalCoordinates(RMF::FileHandle f);
  bool setup_node(kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n,
                  kernel::ParticleIndexes rigid_bodies);
  void save(kernel::Model *m, RMF::FileHandle fh);
};

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_LOCAL_COORDINATES_H */
