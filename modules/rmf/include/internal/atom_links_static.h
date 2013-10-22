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
#include <RMF/decorators.h>
#include <IMP/kernel/Model.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

/** Load things that are assumed to be unchanging, excluding coordinates. */
class IMPRMFEXPORT HierarchyLoadStatic {
  RMF::ParticleConstFactory particle_factory_;
  RMF::IntermediateParticleConstFactory intermediate_particle_factory_;
  RMF::AtomConstFactory atom_factory_;
  RMF::ResidueConstFactory residue_factory_;
  RMF::ChainConstFactory chain_factory_;
  RMF::ColoredConstFactory colored_factory_;
  RMF::CopyConstFactory copy_factory_;
  RMF::DiffuserConstFactory diffuser_factory_;
  RMF::TypedConstFactory typed_factory_;
  RMF::DomainConstFactory domain_factory_;
  RMF::FragmentConstFactory fragment_factory_;
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
  RMF::ParticleFactory particle_factory_;
  RMF::IntermediateParticleFactory intermediate_particle_factory_;
  RMF::AtomFactory atom_factory_;
  RMF::ResidueFactory residue_factory_;
  RMF::ChainFactory chain_factory_;
  RMF::ColoredFactory colored_factory_;
  RMF::CopyFactory copy_factory_;
  RMF::DiffuserFactory diffuser_factory_;
  RMF::TypedFactory typed_factory_;
  RMF::DomainFactory domain_factory_;
  RMF::FragmentFactory fragment_factory_;

 public:
  HierarchySaveStatic(RMF::FileHandle f);
  void setup_node(kernel::Model *m, kernel::ParticleIndex p, RMF::NodeHandle n);
};

class IMPRMFEXPORT HierarchyLoadBonds {
  RMF::AliasConstFactory af_;

 public:
  HierarchyLoadBonds(RMF::FileConstHandle f) : af_(f) {}
  void setup_bonds(RMF::NodeConstHandle n, kernel::Model *m,
                   kernel::ParticleIndex p);
};

class IMPRMFEXPORT HierarchySaveBonds {
  RMF::AliasFactory af_;

 public:
  HierarchySaveBonds(RMF::FileHandle f) : af_(f) {}
  void setup_bonds(kernel::Model *m, kernel::ParticleIndex p,
                   RMF::NodeHandle n);
};

IMPRMF_END_INTERNAL_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_STATIC_H */
