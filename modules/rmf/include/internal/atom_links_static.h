/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_STATIC_H
#define IMPRMF_ATOM_LINKS_STATIC_H

#include <IMP/rmf/rmf_config.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/decorator/physics.h>
#include <RMF/decorator/sequence.h>
#include <RMF/decorator/shape.h>
#include <RMF/decorator/labels.h>
#include <RMF/decorator/bond.h>
#include <IMP/kernel/Model.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

/** Load things that are assumed to be unchanging, excluding coordinates. */
class IMPRMFEXPORT HierarchyLoadStatic {
  RMF::decorator::ParticleFactory particle_factory_;
  RMF::decorator::IntermediateParticleFactory intermediate_particle_factory_;
  RMF::decorator::AtomFactory atom_factory_;
  RMF::decorator::ResidueFactory residue_factory_;
  RMF::decorator::ChainFactory chain_factory_;
  RMF::decorator::ColoredFactory colored_factory_;
  RMF::decorator::CopyFactory copy_factory_;
  RMF::decorator::DiffuserFactory diffuser_factory_;
  RMF::decorator::TypedFactory typed_factory_;
  RMF::decorator::DomainFactory domain_factory_;
  RMF::decorator::FragmentFactory fragment_factory_;
  RMF::decorator::BackwardsCompatibilityFragmentFactory
      backwards_fragment_factory_;
  RMF::decorator::StateFactory state_factory_;
  RMF::decorator::Molecule molecule_;
  RMF::FloatKey radius_key_;
  RMF::FloatKey mass_key_;

 public:
  HierarchyLoadStatic(RMF::FileConstHandle f);
  void setup_particle(RMF::NodeConstHandle n, kernel::Model *m,
                      kernel::ParticleIndex p,
                      const kernel::ParticleIndexes &rigid_bodies);
  void link_particle(RMF::NodeConstHandle n, kernel::Model *m,
                     kernel::ParticleIndex p,
                     const kernel::ParticleIndexes &rigid_bodies);
};

/** Save things that are assumed to be unchanging, excluding coordinates. */
class IMPRMFEXPORT HierarchySaveStatic {
  RMF::decorator::ParticleFactory particle_factory_;
  RMF::decorator::IntermediateParticleFactory intermediate_particle_factory_;
  RMF::decorator::AtomFactory atom_factory_;
  RMF::decorator::ResidueFactory residue_factory_;
  RMF::decorator::ChainFactory chain_factory_;
  RMF::decorator::ColoredFactory colored_factory_;
  RMF::decorator::CopyFactory copy_factory_;
  RMF::decorator::DiffuserFactory diffuser_factory_;
  RMF::decorator::TypedFactory typed_factory_;
  RMF::decorator::DomainFactory domain_factory_;
  RMF::decorator::FragmentFactory fragment_factory_;
  RMF::decorator::StateFactory state_factory_;
  RMF::decorator::Molecule molecule_;

 public:
  HierarchySaveStatic(RMF::FileHandle f);
  void setup_node(kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n);
};

class IMPRMFEXPORT HierarchyLoadBonds {
  RMF::decorator::BondFactory af_;

 public:
  HierarchyLoadBonds(RMF::FileConstHandle f) : af_(f) {}
  void setup_bonds(RMF::NodeConstHandle n, kernel::Model *m,
                   kernel::ParticleIndex p);
};

class IMPRMFEXPORT HierarchySaveBonds {
  RMF::decorator::BondFactory af_;

 public:
  HierarchySaveBonds(RMF::FileHandle f) : af_(f) {}
  void setup_bonds(kernel::Model *m, kernel::ParticleIndex p,
                   RMF::NodeHandle n);
};

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_STATIC_H */
