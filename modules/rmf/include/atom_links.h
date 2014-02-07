/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_H
#define IMPRMF_ATOM_LINKS_H

#include <IMP/rmf/rmf_config.h>
#include "simple_links.h"
#include "internal/atom_links_static.h"
#include "internal/atom_links_rigid_bodies.h"
#include "internal/atom_links_xyzs.h"
#include "internal/atom_links_gaussians.h"
#include <RMF/decorator/alternatives.h>
#include <IMP/base/object_macros.h>
#include <IMP/base/utility_macros.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/rmf/links.h>
#include <IMP/rmf/link_macros.h>
#include <IMP/base/tuple_macros.h>
#include <RMF/decorators.h>
#include <boost/unordered_map.hpp>
IMPRMF_BEGIN_NAMESPACE

/** Manage the loading link between an RMF file and an atom::Hierarchy.

    \unstable{HierarchyLoadLink}
 */
class IMPRMFEXPORT HierarchyLoadLink : public SimpleLoadLink<kernel::Particle> {
  typedef SimpleLoadLink<kernel::Particle> P;
  RMF::decorator::IntermediateParticleFactory intermediate_particle_factory_;
  RMF::decorator::ReferenceFrameFactory reference_frame_factory_;
  RMF::decorator::AlternativesFactory af_;
  RMF::IntKey external_rigid_body_key_;
  struct Data {
    internal::HierarchyLoadStatic load_static;
    internal::HierarchyLoadBonds load_bonds;
    internal::HierarchyLoadXYZs load_xyzs;
    internal::HierarchyLoadGaussians load_gaussians;
    internal::HierarchyLoadRigidBodies load_rigid_bodies;
    Data(RMF::FileConstHandle h)
        : load_static(h),
          load_bonds(h),
          load_xyzs(h),
          load_gaussians(h),
          load_rigid_bodies(h) {}
  };
  typedef boost::unordered_map<kernel::ParticleIndex, boost::shared_ptr<Data> >
      DM;
  DM data_;
  virtual bool get_is(RMF::NodeConstHandle nh) const IMP_OVERRIDE {
    return nh.get_type() == RMF::REPRESENTATION;
  }
  virtual void add_link_recursive(kernel::Model *m, kernel::ParticleIndex root,
                                  kernel::ParticleIndex o,
                                  RMF::NodeConstHandle node,
                                  kernel::ParticleIndexes rigid_bodies,
                                  Data &data);

  virtual void do_add_link(kernel::Particle *o,
                           RMF::NodeConstHandle node) IMP_FINAL IMP_OVERRIDE;
  using P::do_create;
  virtual kernel::Particle *do_create(RMF::NodeConstHandle name,
                                      kernel::Model *m) IMP_FINAL IMP_OVERRIDE;
  void create_recursive(kernel::Model *m, kernel::ParticleIndex root,
                        kernel::ParticleIndex cur, RMF::NodeConstHandle name,
                        kernel::ParticleIndexes rigid_bodies, Data &data);
  virtual void do_load_one(RMF::NodeConstHandle nh,
                           kernel::Particle *o) IMP_FINAL IMP_OVERRIDE;

 protected:
  /** This method is called for the hierarchy.*/
  virtual void do_load_hierarchy(RMF::NodeConstHandle root_node,
                                 kernel::Model *m, kernel::ParticleIndex root) {
    IMP_UNUSED(root_node);
    IMP_UNUSED(m);
    IMP_UNUSED(root);
  }
  /** Overload this to take specific action on creating
      a member of the hierarchy.
      \unstable{do_create_recursive} */
  virtual void do_setup_particle(kernel::Model *m, kernel::ParticleIndex root,
                                 kernel::ParticleIndex cur,
                                 RMF::NodeConstHandle node) {
    IMP_UNUSED(m);
    IMP_UNUSED(root);
    IMP_UNUSED(cur);
    IMP_UNUSED(node);
  }

  /** Overload this to take specific action on linking
     a member of the hierarchy.
     \unstable{do_add_link_recursive} */
  virtual void do_link_particle(kernel::Model *m, kernel::ParticleIndex root,
                                kernel::ParticleIndex cur,
                                RMF::NodeConstHandle node) {
    IMP_UNUSED(root);
    IMP_UNUSED(m);
    IMP_UNUSED(cur);
    IMP_UNUSED(node);
  }

 public:
  HierarchyLoadLink(RMF::FileConstHandle fh);

  static const char *get_name() { return "atom load"; }

  IMP_OBJECT_METHODS(HierarchyLoadLink);
};

/** Manage the save link between an RMF file and an atom::Hierarchy.

    \unstable{HierarchySaveLink}
 */
class IMPRMFEXPORT HierarchySaveLink : public SimpleSaveLink<kernel::Particle> {
  typedef SimpleSaveLink<kernel::Particle> P;
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
  typedef boost::unordered_map<kernel::ParticleIndex, boost::shared_ptr<Data> >
      DM;
  DM data_;
  RMF::decorator::AlternativesFactory af_;
  RMF::IntKey external_rigid_body_key_;

  void add_recursive(Model *m, kernel::ParticleIndex root,
                     kernel::ParticleIndex p,
                     kernel::ParticleIndexes rigid_bodies, RMF::NodeHandle cur,
                     Data &data);
  virtual void do_add(kernel::Particle *p, RMF::NodeHandle cur) IMP_OVERRIDE;
  virtual void do_save_one(kernel::Particle *o,
                           RMF::NodeHandle nh) IMP_OVERRIDE;
  RMF::NodeType get_type(kernel::Particle *) const {
    return RMF::REPRESENTATION;
  }

 protected:
  /** Override to add extra data for nodes. */
  virtual void do_setup_node(Model *m, kernel::ParticleIndex root,
                             kernel::ParticleIndex p, RMF::NodeHandle n) {
    IMP_UNUSED(m);
    IMP_UNUSED(root);
    IMP_UNUSED(p);
    IMP_UNUSED(n);
  }
  /** Override to add extra data for nodes. */
  virtual void do_save_hierarchy(Model *m, kernel::ParticleIndex root,
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

#endif /* IMPRMF_ATOM_LINKS_H */
