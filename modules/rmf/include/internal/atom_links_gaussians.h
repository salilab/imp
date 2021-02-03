/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_GAUSSIANS_H
#define IMPRMF_ATOM_LINKS_GAUSSIANS_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorator/physics.h>
#include <IMP/core/rigid_bodies.h>
IMPRMF_BEGIN_INTERNAL_NAMESPACE

class IMPRMFEXPORT HierarchyLoadGaussians {
  RMF::decorator::GaussianParticleFactory gaussian_factory_;
  typedef std::pair<RMF::NodeID, ParticleIndex> Pair;
  typedef Vector<Pair> NodeParticlePairs;

  NodeParticlePairs gaussians_;

 public:
  HierarchyLoadGaussians(RMF::FileConstHandle f);
  void setup_particle(RMF::NodeConstHandle n, Model *m,
                      ParticleIndex p,
                      const ParticleIndexes &rigid_bodies);
  void link_particle(RMF::NodeConstHandle n, Model *m,
                     ParticleIndex p,
                     const ParticleIndexes &rigid_bodies);
  void load(RMF::FileConstHandle fh, Model *m);
};

class IMPRMFEXPORT HierarchySaveGaussians {
  RMF::decorator::GaussianParticleFactory gaussian_factory_;
  typedef std::pair<RMF::NodeID, ParticleIndex> Pair;
  typedef Vector<Pair> NodeParticlePairs;
  NodeParticlePairs gaussians_;

  void handle_xyz(Model *m, ParticleIndex p, RMF::NodeHandle n,
                  ParticleIndexes &rigid_bodies);

 public:
  HierarchySaveGaussians(RMF::FileHandle f);
  void setup_node(Model *m, ParticleIndex p, RMF::NodeHandle n,
                  const ParticleIndexes &rigid_bodies);
  void save(Model *m, RMF::FileHandle fh);
};

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_GAUSSIANS_H */
