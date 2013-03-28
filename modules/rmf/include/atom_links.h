/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of Model data from/to files.
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

class IMPRMFEXPORT HierarchyLoadLink: public SimpleLoadLink<Particle> {
  IMP_NAMED_TUPLE_2(ConstData, ConstDatas, RMF::NodeIDs, nodes,
                    ParticlesTemp, particles,);
  typedef SimpleLoadLink<Particle> P;
  Pointer<Model> m_;
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
  RMF::ReferenceFrameConstFactory reference_frame_factory_;
  RMF::IndexKey rigid_body_key_;

  base::map<Particle*, ConstData> contents_;
  base::map<unsigned int, ParticlesTemp> rigid_bodies_;
  /** This method is called for each particle in the hierarchy.*/
  IMP_PROTECTED_METHOD(virtual void,
                       do_load_node,
                       (RMF::NodeConstHandle nh,
                        Particle *o),,);
  /** Overload this method to take specific action on loading a hierarchy.
   */
  IMP_PROTECTED_METHOD(virtual void, do_load_one,
                       ( RMF::NodeConstHandle nh,
                         Particle *o),,);
  bool get_is(RMF::NodeConstHandle nh) const {
    return nh.get_type()==RMF::REPRESENTATION;
  }
  bool setup_particle(Particle *root,
                      RMF::NodeConstHandle nh,
                      Particle *p,
                      Particle *rbp);
  /** Overload this to take specific action on creating
      a member of the hierarchy.
      \unstable{do_create_recursive} */
  IMP_PROTECTED_METHOD(virtual Particle*, do_create_recursive, (Particle *root,
                                RMF::NodeConstHandle name,
                                Particle *rbp=nullptr),,);

  Particle* do_create(RMF::NodeConstHandle name);
  /** Overload this to take specific action on linking
      a member of the hierarchy.
      \unstable{do_add_link_recursive} */
  IMP_PROTECTED_METHOD(virtual void, do_add_link_recursive, (Particle *root,
                                     Particle *o, RMF::NodeConstHandle node),,);

  void do_add_link(Particle *o, RMF::NodeConstHandle node);
public:
  HierarchyLoadLink(RMF::FileConstHandle fh, Model *m);

  IMP_OBJECT_INLINE(HierarchyLoadLink,IMP_UNUSED(out),);
};


class IMPRMFEXPORT HierarchySaveLink: public SimpleSaveLink<Particle> {
  typedef SimpleSaveLink<Particle> P;
  IMP_NAMED_TUPLE_2(Data, Datas, RMF::NodeIDs, nodes,
                    ParticlesTemp, particles,);

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
  RMF::ReferenceFrameFactory reference_frame_factory_;
  RMF::IndexKey rigid_body_key_;

  bool forces_;
  RMF::ForceFactory force_factory_;
  RMF::TorqueFactory torque_factory_;

  // ones in this set have their internal coordinates saved
  base::set<Particle*> internal_;
  base::map<Particle*, Data> contents_;
  base::map<Particle*, unsigned int> rigid_bodies_;
  void setup_node(Particle *p, RMF::NodeHandle n) ;
  void do_add(Particle *p, RMF::NodeHandle cur);
  void do_save_one(Particle *o,
                   RMF::NodeHandle nh);
  RMF::NodeType get_type(Particle*) const {
    return RMF::REPRESENTATION;
  }
  IMP_PROTECTED_METHOD(virtual void, do_add_recursive,
                       (Particle *root, Particle *p,
                        RMF::NodeHandle cur),,);
  IMP_PROTECTED_METHOD(virtual void, do_save_node, (Particle *p,
                                                    RMF::NodeHandle n),,);
public:
  HierarchySaveLink(RMF::FileHandle fh);
  IMP_OBJECT_INLINE(HierarchySaveLink,IMP_UNUSED(out),);
  void set_save_forces(bool tf) {
    forces_ = tf;
  }
};


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_H */
