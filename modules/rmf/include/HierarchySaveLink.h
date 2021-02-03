/**
 *  \file IMP/rmf/HierarchySaveLink.h
 *  \brief Manage saving links to an RMF file from an atom::Hierarchy.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_HIERARCHY_SAVE_LINK_H
#define IMPRMF_HIERARCHY_SAVE_LINK_H

#include <IMP/rmf/rmf_config.h>
#include "simple_links.h"
#include "internal/atom_links_static.h"
#include "internal/atom_links_rigid_bodies.h"
#include "internal/atom_links_xyzs.h"
#include "internal/atom_links_gaussians.h"
#include <RMF/decorator/alternatives.h>
#include <RMF/decorator/provenance.h>
#include <IMP/object_macros.h>
#include <IMP/utility_macros.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/core/provenance.h>
#include <IMP/rmf/links.h>
#include <IMP/rmf/link_macros.h>
#include <IMP/tuple_macros.h>
#include <RMF/decorators.h>
#include <boost/unordered_map.hpp>
IMPRMF_BEGIN_NAMESPACE

/** Manage the save link between an RMF file and an atom::Hierarchy.

    \unstable{HierarchySaveLink}
 */
class IMPRMFEXPORT HierarchySaveLink : public SimpleSaveLink<Particle> {
  typedef SimpleSaveLink<Particle> P;
  struct Data {
    internal::HierarchySaveStatic save_static;
    internal::HierarchySaveBonds save_bonds;
    internal::HierarchySaveXYZs save_xyzs;
    internal::HierarchySaveGaussians save_gaussians;
    internal::HierarchySaveRigidBodies save_rigid_bodies;
    Data(RMF::FileHandle h)
        : save_static(h),
          save_bonds(h),
          save_xyzs(h),
          save_gaussians(h),
          save_rigid_bodies(h) {}
  };
  typedef boost::unordered_map<ParticleIndex, boost::shared_ptr<Data> >
      DM;
  DM data_;
  RMF::decorator::AlternativesFactory af_;
  RMF::decorator::StructureProvenanceFactory strucpf_;
  RMF::decorator::SampleProvenanceFactory samppf_;
  RMF::decorator::CombineProvenanceFactory combpf_;
  RMF::decorator::FilterProvenanceFactory filtpf_;
  RMF::decorator::ClusterProvenanceFactory clustpf_;
  RMF::decorator::ScriptProvenanceFactory scriptpf_;
  RMF::decorator::SoftwareProvenanceFactory softpf_;
  RMF::decorator::ExplicitResolutionFactory explicit_resolution_factory_;
  RMF::IntKey external_rigid_body_key_;

  void add_recursive(Model *m, ParticleIndex root,
                     ParticleIndex p,
                     ParticleIndexes rigid_bodies, RMF::NodeHandle cur,
                     Data &data);

  // Make RMF PROVENANCE nodes corresponding to those in IMP
  void add_provenance(Model *m, ParticleIndex p, RMF::NodeHandle cur);

  virtual void do_add(Particle *p, RMF::NodeHandle cur) IMP_OVERRIDE;
  virtual void do_save_one(Particle *o,
                           RMF::NodeHandle nh) IMP_OVERRIDE;
  RMF::NodeType get_type(Particle *) const {
    return RMF::REPRESENTATION;
  }

 protected:
  /** Override to add extra data for nodes. */
  virtual void do_setup_node(Model *m, ParticleIndex root,
                             ParticleIndex p, RMF::NodeHandle n) {
    IMP_UNUSED(m);
    IMP_UNUSED(root);
    IMP_UNUSED(p);
    IMP_UNUSED(n);
  }
  /** Override to add extra data for nodes. */
  virtual void do_save_hierarchy(Model *m, ParticleIndex root,
                                 RMF::NodeHandle root_node) {
    IMP_UNUSED(m);
    IMP_UNUSED(root);
    IMP_UNUSED(root_node);
  }

 public:
  HierarchySaveLink(RMF::FileHandle fh);
  static const char *get_name() { return "atom save"; }
  IMP_OBJECT_METHODS(HierarchySaveLink);
};

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_HIERARCHY_SAVE_LINK_H */
