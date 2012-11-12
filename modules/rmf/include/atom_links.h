/**
 *  \file IMP/rmf/atom_links.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_ATOM_LINKS_H
#define IMPRMF_ATOM_LINKS_H

#include "rmf_config.h"
#include "simple_links.h"
#include <IMP/base/object_macros.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/rmf/links.h>
#include <IMP/rmf/link_macros.h>
#include <IMP/base/tuple_macros.h>
#include <RMF/decorators.h>
#include <IMP/compatibility/map.h>
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

  compatibility::map<Particle*, ConstData> contents_;
  compatibility::map<unsigned int, ParticlesTemp> rigid_bodies_;
  void do_load_one_particle(RMF::NodeConstHandle nh,
                            Particle *o);

  void do_load_one( RMF::NodeConstHandle nh,
                    Particle *o);
  bool get_is(RMF::NodeConstHandle nh) const {
    return nh.get_type()==RMF::REPRESENTATION;
  }
  bool setup_particle(Particle *root,
                      RMF::NodeConstHandle nh,
                      Particle *p,
                      Particle *rbp);

  Particle* do_create_recursive(Particle *root,
                                RMF::NodeConstHandle name,
                                Particle *rbp=nullptr);

  Particle* do_create(RMF::NodeConstHandle name);
  void do_add_link_recursive(Particle *root,
                             Particle *o, RMF::NodeConstHandle node);

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

  // ones in this set have their internal coordinates saved
  compatibility::set<Particle*> internal_;
  compatibility::map<Particle*, Data> contents_;
  compatibility::map<Particle*, unsigned int> rigid_bodies_;
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
};


IMPRMF_END_NAMESPACE

#endif /* IMPRMF_ATOM_LINKS_H */
