/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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
  RMF::decorator::IntermediateParticleConstFactory ip_factory_;
  typedef std::pair<RMF::NodeID, kernel::ParticleIndex> Pair;
  typedef base::Vector<Pair> NodeParticlePairs;

  NodeParticlePairs global_, local_;

 public:
  HierarchyLoadXYZs(RMF::FileConstHandle f);
  void setup_particle(RMF::NodeConstHandle n,
                      kernel::Model *m, kernel::ParticleIndex p,
                      const kernel::ParticleIndexes &rigid_bodies);
  void link_particle(RMF::NodeConstHandle n,
                     kernel::Model *m, kernel::ParticleIndex p,
                     const kernel::ParticleIndexes &rigid_bodies);
  void load(RMF::FileConstHandle fh, Model *m);
};

class IMPRMFEXPORT HierarchySaveXYZs {
  RMF::decorator::IntermediateParticleFactory ip_factory_;
  typedef std::pair<RMF::NodeID, kernel::ParticleIndex> Pair;
  typedef base::Vector<Pair> NodeParticlePairs;
  NodeParticlePairs global_, local_;

  void handle_xyz(kernel::Model *m, kernel::ParticleIndex p,
                         RMF::NodeHandle n,
                         kernel::ParticleIndexes &rigid_bodies);

 public:
  HierarchySaveXYZs(RMF::FileHandle f);
  void setup_node(kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n,
                  const kernel::ParticleIndexes& rigid_bodies);
  void save(Model *m, RMF::FileHandle fh);
};

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_XYZS_H */
