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
  IMP_NAMED_TUPLE_2(ConstData, ConstDatas, RMF::NodeIDs, nodes,
                    kernel::ParticlesTemp, particles, );
  typedef SimpleLoadLink<kernel::Particle> P;
  RMF::ParticleConstFactory particle_factory_;
  RMF::IntermediateParticleConstFactory intermediate_particle_factory_;
  RMF::RigidParticleConstFactory rigid_factory_;
  RMF::AtomConstFactory atom_factory_;
  RMF::ResidueConstFactory residue_factory_;
  RMF::ChainConstFactory chain_factory_;
  RMF::ColoredConstFactory colored_factory_;
  RMF::CopyConstFactory copy_factory_;
  RMF::DiffuserConstFactory diffuser_factory_;
  RMF::TypedConstFactory typed_factory_;
  RMF::DomainConstFactory domain_factory_;
  RMF::FragmentConstFactory fragment_factory_;
  RMF::ReferenceFrameConstFactory reference_frame_factory_;
  RMF::IndexKey rigid_body_key_;

  base::map<kernel::Particle *, ConstData> contents_;
  base::map<unsigned int, kernel::ParticlesTemp> rigid_bodies_;
  bool get_is(RMF::NodeConstHandle nh) const {
    return nh.get_type() == RMF::REPRESENTATION;
  }
  bool setup_particle(kernel::Particle *root, RMF::NodeConstHandle nh,
                      kernel::Particle *p, kernel::Particle *rbp);

 protected:
  /** This method is called for each particle in the hierarchy.*/
  virtual void do_load_node(RMF::NodeConstHandle nh, kernel::Particle *o);
  /** Overload this method to take specific action on loading a hierarchy.
   */
  virtual void do_load_one(RMF::NodeConstHandle nh, kernel::Particle *o);
  /** Overload this to take specific action on creating
      a member of the hierarchy.
      \unstable{do_create_recursive} */
  virtual kernel::Particle *do_create_recursive(
      kernel::Particle *root, kernel::Model *m, RMF::NodeConstHandle name,
      kernel::Particle *rbp = nullptr);

  kernel::Particle *do_create(RMF::NodeConstHandle name, kernel::Model *m);
  /** Overload this to take specific action on linking
      a member of the hierarchy.
      \unstable{do_add_link_recursive} */
  virtual void do_add_link_recursive(kernel::Particle *root,
                                     kernel::Particle *o,
                                     RMF::NodeConstHandle node);

  void do_add_link(kernel::Particle *o, RMF::NodeConstHandle node);

 public:
  HierarchyLoadLink(RMF::FileConstHandle fh);

  static const char *get_name() {return "atom load";}

  IMP_OBJECT_METHODS(HierarchyLoadLink);
};

/** Manage the save link between an RMF file and an atom::Hierarchy.

    \unstable{HierarchySaveLink}
 */
class IMPRMFEXPORT HierarchySaveLink : public SimpleSaveLink<kernel::Particle> {
  typedef SimpleSaveLink<kernel::Particle> P;
  IMP_NAMED_TUPLE_2(Data, Datas, RMF::NodeIDs, nodes, kernel::ParticlesTemp,
                    particles, );

  RMF::ParticleFactory particle_factory_;
  RMF::IntermediateParticleFactory intermediate_particle_factory_;
  RMF::RigidParticleFactory rigid_factory_;
  RMF::AtomFactory atom_factory_;
  RMF::ResidueFactory residue_factory_;
  RMF::ChainFactory chain_factory_;
  RMF::ColoredFactory colored_factory_;
  RMF::CopyFactory copy_factory_;
  RMF::DiffuserFactory diffuser_factory_;
  RMF::TypedFactory typed_factory_;
  RMF::DomainFactory domain_factory_;
  RMF::FragmentFactory fragment_factory_;
  RMF::ReferenceFrameFactory reference_frame_factory_;
  RMF::IndexKey rigid_body_key_;

  bool forces_;
  RMF::ForceFactory force_factory_;
  RMF::TorqueFactory torque_factory_;

  // ones in this set have their internal coordinates saved
  base::set<kernel::Particle *> internal_;
  base::map<kernel::Particle *, Data> contents_;
  base::map<kernel::Particle *, unsigned int> rigid_bodies_;
  void setup_node(kernel::Particle *p, RMF::NodeHandle n);
  void do_add(kernel::Particle *p, RMF::NodeHandle cur);
  void do_save_one(kernel::Particle *o, RMF::NodeHandle nh);
  RMF::NodeType get_type(kernel::Particle *) const {
    return RMF::REPRESENTATION;
  }

 protected:
  virtual void do_add_recursive(kernel::Particle *root, kernel::Particle *p,
                                RMF::NodeHandle cur);
  virtual void do_save_node(kernel::Particle *p, RMF::NodeHandle n);

 public:
  HierarchySaveLink(RMF::FileHandle fh);
  void set_save_forces(bool tf) { forces_ = tf; }

  static const char *get_name() {return "atom save";}
  IMP_OBJECT_METHODS(HierarchySaveLink);
};

IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_H */
