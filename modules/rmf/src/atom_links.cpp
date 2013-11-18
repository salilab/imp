/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_links.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Diffusion.h>
#include <IMP/atom/Copy.h>
#include <IMP/atom/Fragment.h>
#include <IMP/core/Typed.h>
#include <IMP/display/Colored.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <algorithm>
#include <RMF/utility.h>
#include <RMF/decorators.h>
#include <boost/make_shared.hpp>
#include <RMF/RestoreCurrentFrame.h>
#include <RMF/SetCurrentFrame.h>

IMPRMF_BEGIN_NAMESPACE

namespace {

std::string get_good_name(kernel::Model *m, kernel::ParticleIndex h) {
  if (atom::Atom::get_is_setup(m, h)) {
    return atom::Atom(m, h).get_atom_type().get_string();
  } else if (atom::Residue::get_is_setup(m, h)) {
    std::ostringstream oss;
    oss << atom::Residue(m, h).get_index();
    return oss.str();
  } else if (atom::Chain::get_is_setup(m, h)) {
    return atom::Chain(m, h).get_id();
  } else {
    return m->get_particle_name(h);
  }
}

unsigned int get_coords_state(RMF::NodeConstHandle nh,
                              RMF::IntermediateParticleConstFactory ipcf,
                              RMF::ReferenceFrameConstFactory rfcf) {
  unsigned int ret = 0;
  {
    if (ipcf.get_is_static(nh)) ret |= internal::STATIC_XYZ;
    if (rfcf.get_is_static(nh)) ret |= internal::STATIC_RB;
  }
  {
    RMF::SetCurrentFrame fa(nh.get_file(), RMF::FrameID(0));
    if (!(ret & internal::STATIC_XYZ) && ipcf.get_is(nh))
      ret |= internal::FRAME_XYZ;
    if (!(ret & internal::STATIC_RB) && rfcf.get_is(nh))
      ret |= internal::FRAME_RB;
  }
  return ret;
}
}

void HierarchyLoadLink::do_load_one(RMF::NodeConstHandle nh,
                                    kernel::Particle *o) {
  data_.find(o->get_index())
      ->second->load_global_coordinates.load(nh.get_file(), o->get_model());
  data_.find(o->get_index())
      ->second->load_local_coordinates.load(nh.get_file(), o->get_model());
  do_load_hierarchy(nh, o->get_model(), o->get_index());
}

void HierarchyLoadLink::create_recursive(kernel::Model *m,
                                         kernel::ParticleIndex root,
                                         kernel::ParticleIndex cur,
                                         RMF::NodeConstHandle name,
                                         kernel::ParticleIndexes rigid_bodies,
                                         Data &data) {
  set_association(name, m->get_particle(cur));
  unsigned int state = get_coords_state(name, intermediate_particle_factory_,
                                        reference_frame_factory_);
  data.load_static.setup_particle(name, m, cur, rigid_bodies);
  data.load_local_coordinates.setup_particle(name, state, m, cur, rigid_bodies);
  data.load_static_coordinates.setup_particle(name, state, m, cur,
                                              rigid_bodies);
  data.load_global_coordinates.setup_particle(name, state, m, cur,
                                              rigid_bodies);

  if (core::RigidBody::get_is_setup(m, cur)) {
    rigid_bodies.push_back(cur);
  }
  RMF::NodeConstHandles ch = name.get_children();
  for (unsigned int i = 0; i < ch.size(); ++i) {
    if (ch[i].get_type() == RMF::REPRESENTATION) {
      kernel::ParticleIndex child = m->add_particle(ch[i].get_name());
      atom::Hierarchy(m, cur)
          .add_child(atom::Hierarchy::setup_particle(m, child));
      create_recursive(m, root, child, ch[i], rigid_bodies, data);
    }
  }
  do_setup_particle(m, root, cur, name);
}

Particle *HierarchyLoadLink::do_create(RMF::NodeConstHandle node,
                                       kernel::Model *m) {
  IMP_FUNCTION_LOG;
  kernel::ParticleIndex ret = m->add_particle(node.get_name());
  data_.insert(std::make_pair(ret, boost::make_shared<Data>(node.get_file())));
  create_recursive(m, ret, ret, node, kernel::ParticleIndexes(), *data_[ret]);
  IMP_INTERNAL_CHECK(atom::Hierarchy(m, ret).get_is_valid(true),
                     "Invalid hierarchy created");
  data_.find(ret)->second->load_bonds.setup_bonds(node, m, ret);
  return m->get_particle(ret);
}

void HierarchyLoadLink::add_link_recursive(kernel::Model *m,
                                           kernel::ParticleIndex root,
                                           kernel::ParticleIndex cur,
                                           RMF::NodeConstHandle node,
                                           kernel::ParticleIndexes rigid_bodies,
                                           Data &data) {
  IMP_USAGE_CHECK(get_good_name(m, cur) == node.get_name(),
                  "Names don't match");
  set_association(node, m->get_particle(cur), true);
  RMF::NodeConstHandles ch = node.get_children();
  unsigned int state = get_coords_state(node, intermediate_particle_factory_,
                                        reference_frame_factory_);
  data.load_static.link_particle(node, m, cur, rigid_bodies);
  data.load_global_coordinates.link_particle(node, state, m, cur, rigid_bodies);
  // data.load_static_coordinates.link_particle(node, m, cur, rigid_bodies);
  data.load_local_coordinates.link_particle(node, state, m, cur, rigid_bodies);

  do_link_particle(m, root, cur, node);

  if (core::RigidBody::get_is_setup(m, cur)) {
    rigid_bodies.push_back(cur);
  }
  int child = 0;
  for (unsigned int i = 0; i < ch.size(); ++i) {
    if (ch[i].get_type() == RMF::REPRESENTATION) {
      add_link_recursive(m, root,
                         atom::Hierarchy(m, cur).get_child_index(child), ch[i],
                         rigid_bodies, data);
      ++child;
    }
  }
}

void HierarchyLoadLink::do_add_link(kernel::Particle *o,
                                    RMF::NodeConstHandle node) {
  IMP_FUNCTION_LOG;
  data_.insert(std::make_pair(o->get_index(),
                              boost::make_shared<Data>(node.get_file())));
  add_link_recursive(o->get_model(), o->get_index(), o->get_index(), node,
                     kernel::ParticleIndexes(),
                     *data_.find(o->get_index())->second);
}

HierarchyLoadLink::HierarchyLoadLink(RMF::FileConstHandle fh)
    : P("HierarchyLoadLink%1%"),
      intermediate_particle_factory_(fh),
      reference_frame_factory_(fh) {}

void HierarchySaveLink::do_add(kernel::Particle *p, RMF::NodeHandle cur) {
  IMP_USAGE_CHECK(atom::Hierarchy(p).get_is_valid(true),
                  "Invalid hierarchy passed.");

  RMF::RestoreCurrentFrame scf(cur.get_file());
  data_.insert(
      std::make_pair(p->get_index(), boost::make_shared<Data>(cur.get_file())));
  add_recursive(p->get_model(), p->get_index(), p->get_index(),
                ParticleIndexes(), cur, *data_[p->get_index()]);
  P::add_link(p, cur);
  data_[p->get_index()]
      ->save_bonds.setup_bonds(p->get_model(), p->get_index(), cur);
}

void HierarchySaveLink::do_save_one(kernel::Particle *o, RMF::NodeHandle nh) {
  RMF::FileHandle fh = nh.get_file();
  DM::iterator it = data_.find(o->get_index());
  IMP_USAGE_CHECK(it != data_.end(), "I don't know that node");
  it->second->save_local_coordinates.save(o->get_model(), nh.get_file());
  it->second->save_global_coordinates.save(o->get_model(), nh.get_file());
  do_save_hierarchy(o->get_model(), o->get_index(), nh);
}

void HierarchySaveLink::add_recursive(Model *m, kernel::ParticleIndex root,
                                      kernel::ParticleIndex p,
                                      kernel::ParticleIndexes rigid_bodies,
                                      RMF::NodeHandle cur, Data &data) {
  IMP_LOG_VERBOSE("Adding " << atom::Hierarchy(m, p) << std::endl);
  // make sure not to double add
  if (p != root) set_association(cur, m->get_particle(p));
  data.save_static.setup_node(m, p, cur);
  bool local_coords =
      data.save_local_coordinates.setup_node(m, p, cur, rigid_bodies);
  bool global_coords =
      data.save_global_coordinates.setup_node(m, p, cur, rigid_bodies);
  /*bool static_coords =
    data.save_static_coordinates.setup_node(m, p, cur, rigid_bodies);*/
  IMP_INTERNAL_CHECK(!local_coords || !global_coords,
                     "A particle can't have saved local and global coords");

  do_setup_node(m, root, p, cur);

  if (core::RigidBody::get_is_setup(m, p)) {
    rigid_bodies.push_back(p);
  }
  for (unsigned int i = 0; i < atom::Hierarchy(m, p).get_number_of_children();
       ++i) {
    kernel::ParticleIndex pc = atom::Hierarchy(m, p).get_child_index(i);
    RMF::NodeHandle curc =
        cur.add_child(get_good_name(m, pc), RMF::REPRESENTATION);
    add_recursive(m, root, pc, rigid_bodies, curc, data);
  }
}

HierarchySaveLink::HierarchySaveLink(RMF::FileHandle)
    : P("HierarchySaveLink%1%") {}

IMPRMF_END_NAMESPACE
