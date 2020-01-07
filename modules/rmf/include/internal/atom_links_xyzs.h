/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_XYZS_H
#define IMPRMF_ATOM_LINKS_XYZS_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorators.h>
#include <IMP/core/rigid_bodies.h>
IMPRMF_BEGIN_INTERNAL_NAMESPACE

class IMPRMFEXPORT HierarchyLoadXYZs {
  RMF::decorator::ReferenceFrameFactory reference_frame_factory_;
  RMF::decorator::IntermediateParticleFactory ip_factory_;
  typedef std::pair<RMF::NodeID, ParticleIndex> Pair;
  typedef Vector<Pair> NodeParticlePairs;

  NodeParticlePairs global_, local_;

  // backwards compat
  RMF::IntKey rb_index_key_;

 public:
  HierarchyLoadXYZs(RMF::FileConstHandle f);
  void setup_particle(RMF::NodeConstHandle n, Model *m,
                      ParticleIndex p,
                      const ParticleIndexes &rigid_bodies);
  void link_particle(RMF::NodeConstHandle n, Model *m,
                     ParticleIndex p,
                     const ParticleIndexes &rigid_bodies);
  void load(RMF::FileConstHandle fh, Model *m);
};

class IMPRMFEXPORT HierarchySaveXYZs {
  RMF::decorator::IntermediateParticleFactory ip_factory_;
  typedef std::pair<RMF::NodeID, ParticleIndex> Pair;
  typedef std::pair<RMF::NodeID, ParticleIndexPair> Triplet;
  typedef Vector<Pair> NodeParticlePairs;
  typedef Vector<Triplet> NodeParticleTriplets;

  NodeParticlePairs global_, local_;
  NodeParticleTriplets rigid_nonmember_;

  void handle_xyz(Model *m, ParticleIndex p, RMF::NodeHandle n,
                  ParticleIndexes &rigid_bodies);

 public:
  HierarchySaveXYZs(RMF::FileHandle f);
  void setup_node(Model *m, ParticleIndex p, RMF::NodeHandle n,
                  const ParticleIndexes &rigid_bodies);
  void save(Model *m, RMF::FileHandle fh);
};

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_XYZS_H */
