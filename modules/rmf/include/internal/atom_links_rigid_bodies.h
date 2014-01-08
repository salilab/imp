/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_RIGID_BODIES_H
#define IMPRMF_ATOM_LINKS_RIGID_BODIES_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>
#include <IMP/core/rigid_bodies.h>
IMPRMF_BEGIN_INTERNAL_NAMESPACE

class IMPRMFEXPORT HierarchyLoadRigidBodies {
  RMF::decorator::ReferenceFrameFactory reference_frame_factory_;
  RMF::decorator::IntermediateParticleFactory ip_factory_;
  typedef std::pair<RMF::NodeID, kernel::ParticleIndex> Pair;
  typedef base::Vector<Pair> NodeParticlePairs;

  NodeParticlePairs global_, local_;
  RMF::IntKey external_rigid_body_index_;
  boost::unordered_map<int, kernel::ParticleIndex>
      external_rigid_body_index_map_;
  boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndex>
      external_rigid_bodies_;

  // begin backwards compat
  struct RB {
    kernel::ParticleIndex rb;
    kernel::ParticleIndexes members;
    bool initialized;
    RB() : initialized(true) {}
  };
  RMF::IntKey rb_index_key_;
  base::map<int, RB> rigid_body_compositions_;
  boost::unordered_map<RMF::NodeID, kernel::ParticleIndex> external_rbs_;
  void fix_internal_coordinates(core::RigidBody rb,
                                algebra::ReferenceFrame3D rf,
                                core::RigidBodyMember rm) const;
  void fix_rigid_body(Model *m, const RB &in) const;
  void initialize_rigid_body(Model *m, RB &in) const;
  // end backwards compat

  kernel::ParticleIndex find_rigid_body(kernel::Model *m,
                                        kernel::ParticleIndex p);
  void link_rigid_body(RMF::NodeConstHandle n, kernel::Model *m,
                       kernel::ParticleIndex p,
                       kernel::ParticleIndexes &rigid_bodies);

 public:
  HierarchyLoadRigidBodies(RMF::FileConstHandle f);
  void setup_particle(RMF::NodeConstHandle n,
                      kernel::Model *m, kernel::ParticleIndex p,
                      kernel::ParticleIndexes &rigid_bodies);
  void link_particle(RMF::NodeConstHandle n,
                     kernel::Model *m, kernel::ParticleIndex p,
                     kernel::ParticleIndexes &rigid_bodies);
  void load(RMF::FileConstHandle fh, Model *m);
  // backwards compat
  void update_rigid_bodies(RMF::FileConstHandle fh, Model *m);
};

class IMPRMFEXPORT HierarchySaveRigidBodies {
  RMF::decorator::ReferenceFrameFactory reference_frame_factory_;
  typedef std::pair<RMF::NodeID, kernel::ParticleIndex> Pair;
  typedef base::Vector<Pair> NodeParticlePairs;
  NodeParticlePairs global_, local_;
  RMF::IntKey external_rigid_body_index_;

  boost::unordered_map<kernel::ParticleIndex, int> external_index_;
  boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndex> externals_;
  boost::unordered_set<kernel::ParticleIndex> not_externals_;

  kernel::ParticleIndex fill_external(kernel::Model *m,
                                      kernel::ParticleIndex p);

  void handle_rigid_body(kernel::Model *m, kernel::ParticleIndex p,
                         RMF::NodeHandle n,
                         kernel::ParticleIndexes &rigid_bodies);

 public:
  HierarchySaveRigidBodies(RMF::FileHandle f);
  void setup_node(kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n,
                  kernel::ParticleIndexes& rigid_bodies);
  void save(Model *m, RMF::FileHandle fh);
};

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_RIGID_BODIES_H */
