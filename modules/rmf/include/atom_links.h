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
#include "internal/atom_links_global_coordinates.h"
#include "internal/atom_links_static_coordinates.h"
#include "internal/atom_links_local_coordinates.h"
#include <IMP/base/object_macros.h>
#include <IMP/base/utility_macros.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/rmf/links.h>
#include <IMP/rmf/link_macros.h>
#include <IMP/base/tuple_macros.h>
#include <RMF/decorators.h>
#include <IMP/base/map.h>
IMPRMF_BEGIN_NAMESPACE

/** Manage the loading link between an RMF file and an atom::Hierarchy.

    \unstable{HierarchyLoadLink}
 */
class IMPRMFEXPORT HierarchyLoadLink : public SimpleLoadLink<kernel::Particle> {
  typedef SimpleLoadLink<kernel::Particle> P;
  RMF::IntermediateParticleConstFactory intermediate_particle_factory_;
  RMF::ReferenceFrameConstFactory reference_frame_factory_;
  struct Data {
    internal::HierarchyLoadStatic load_static;
    internal::HierarchyLoadBonds load_bonds;
    internal::HierarchyLoadLocalCoordinates load_local_coordinates;
    internal::HierarchyLoadStaticCoordinates load_static_coordinates;
    internal::HierarchyLoadGlobalCoordinates load_global_coordinates;
    Data(RMF::FileConstHandle h)
        : load_static(h),
          load_bonds(h),
          load_local_coordinates(h),
          load_static_coordinates(h),
          load_global_coordinates(h) {}
  };
  typedef base::map<kernel::ParticleIndex, boost::shared_ptr<Data> > DM;
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
                           RMF::NodeConstHandle node) IMP_FINAL
      IMP_OVERRIDE;
  virtual kernel::Particle *do_create(RMF::NodeConstHandle name,
                                      kernel::Model *m) IMP_FINAL
      IMP_OVERRIDE;
  void create_recursive(kernel::Model *m, kernel::ParticleIndex root,
                        kernel::ParticleIndex cur, RMF::NodeConstHandle name,
                        kernel::ParticleIndexes rigid_bodies, Data &data);
  virtual void do_load_one(RMF::NodeConstHandle nh,
                           kernel::Particle *o) IMP_FINAL
      IMP_OVERRIDE;

 protected:
  /** This method is called for the hierarchy.*/
  virtual void do_load_hierarchy(RMF::NodeConstHandle root_node,
                                 kernel::Model *m, kernel::ParticleIndex root) {
  }
  /** Overload this to take specific action on creating
      a member of the hierarchy.
      \unstable{do_create_recursive} */
  virtual void do_setup_particle(kernel::Model *m, kernel::ParticleIndex root,
                                 kernel::ParticleIndex cur,
                                 RMF::NodeConstHandle node) {}

  /** Overload this to take specific action on linking
     a member of the hierarchy.
     \unstable{do_add_link_recursive} */
  virtual void do_link_particle(kernel::Model *m, kernel::ParticleIndex root,
                                kernel::ParticleIndex cur,
                                RMF::NodeConstHandle node) {}

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
    internal::HierarchySaveLocalCoordinates save_local_coordinates;
    internal::HierarchySaveStaticCoordinates save_static_coordinates;
    internal::HierarchySaveGlobalCoordinates save_global_coordinates;
    Data(RMF::FileHandle h)
        : save_static(h),
          save_bonds(h),
          save_local_coordinates(h),
          save_static_coordinates(h),
          save_global_coordinates(h) {}
  };
  typedef base::map<kernel::ParticleIndex, boost::shared_ptr<Data> > DM;
  DM data_;

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
                             kernel::ParticleIndex p, RMF::NodeHandle n) {}
  /** Override to add extra data for nodes. */
  virtual void do_save_hierarchy(Model *m, kernel::ParticleIndex root,
                                 RMF::NodeHandle root_node) {}

 public:
  HierarchySaveLink(RMF::FileHandle fh);
  static const char *get_name() { return "atom save"; }
  IMP_OBJECT_METHODS(HierarchySaveLink);
};

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_H */
