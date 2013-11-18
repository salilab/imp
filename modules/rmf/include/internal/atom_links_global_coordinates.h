/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_GLOBAL_COORDINATES_H
#define IMPRMF_ATOM_LINKS_GLOBAL_COORDINATES_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>
#include <IMP/core/rigid_bodies.h>
IMPRMF_BEGIN_INTERNAL_NAMESPACE

class IMPRMFEXPORT HierarchyLoadGlobalCoordinates {
  RMF::IntermediateParticleConstFactory intermediate_particle_factory_;
  RMF::ReferenceFrameConstFactory reference_frame_factory_;
  typedef std::pair<RMF::NodeID, kernel::ParticleIndex> Pair;
  typedef base::Vector<Pair> NodeParticlePairs;

  struct RB {
    kernel::ParticleIndex rb;
    kernel::ParticleIndexes members;
    bool initialized;
    RB() : initialized(true) {}
  };
  NodeParticlePairs xyzs_;
  NodeParticlePairs rigid_bodies_;
  base::map<int, RB> rigid_body_compositions_;

  RMF::IntKey key_, non_rigid_key_;

  void fix_internal_coordinates(core::RigidBody rb,
                                algebra::ReferenceFrame3D rf,
                                core::RigidBodyMember rm) const;
  void fix_rigid_body(Model *m, const RB &in) const;
  void initialize_rigid_body(Model *m, RB &in) const;

 public:
  HierarchyLoadGlobalCoordinates(RMF::FileConstHandle f);
  bool setup_particle(RMF::NodeConstHandle n, unsigned int node_state,
                      kernel::Model *m, kernel::ParticleIndex p,
                      const kernel::ParticleIndexes &rigid_bodies);
  bool link_particle(RMF::NodeConstHandle n, unsigned int node_state,
                     kernel::Model *m, kernel::ParticleIndex p,
                     const kernel::ParticleIndexes &rigid_bodies);
  void load(RMF::FileConstHandle fh, Model *m);
};

class IMPRMFEXPORT HierarchySaveGlobalCoordinates {
  RMF::IntermediateParticleFactory intermediate_particle_factory_;
  RMF::ReferenceFrameFactory reference_frame_factory_;
  typedef std::pair<RMF::NodeID, kernel::ParticleIndex> Pair;
  typedef base::Vector<Pair> NodeParticlePairs;
  NodeParticlePairs xyzs_;
  NodeParticlePairs rigid_bodies_;
  base::map<kernel::ParticleIndex, int> bodies_;
  RMF::IntKey key_, non_rigid_key_;

 public:
  HierarchySaveGlobalCoordinates(RMF::FileHandle f);
  bool setup_node(kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n,
                  kernel::ParticleIndexes rigid_bodies);
  void save(Model *m, RMF::FileHandle fh);
};

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_GLOBAL_COORDINATES_H */
