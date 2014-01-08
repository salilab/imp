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

unsigned int get_coords_state(
    RMF::NodeConstHandle nh,
    RMF::decorator::IntermediateParticleFactory ipcf,
    RMF::decorator::ReferenceFrameFactory rfcf, RMF::IntKey external_key) {
  unsigned int ret = 0;
  if (nh.get_has_value(external_key)) {
    ret |= internal::EXTERNAL_RB;
  }
  {
    if (ipcf.get_is_static(nh)) ret |= internal::STATIC_XYZ;
    if (rfcf.get_is_static(nh)) ret |= internal::STATIC_RB;
  }
  {
    RMF::SetCurrentFrame fa(nh.get_file(), RMF::FrameID(0));
    if (!(ret & internal::STATIC_XYZ) && ipcf.get_is(nh))
      ret |= internal::FRAME_XYZ;
    if (!(ret & internal::STATIC_RB) &&
        rfcf.get_is(nh) & !(ret & internal::EXTERNAL_RB))
      ret |= internal::FRAME_RB;
  }
  return ret;
}
}

void HierarchyLoadLink::do_load_one(RMF::NodeConstHandle nh,
                                    kernel::Particle *o) {
  data_.find(o->get_index())
      ->second->load_rigid_bodies.load(nh.get_file(), o->get_model());
  data_.find(o->get_index())
      ->second->load_xyzs.load(nh.get_file(), o->get_model());
  data_.find(o->get_index())->second->load_rigid_bodies.update_rigid_bodies(
      nh.get_file(), o->get_model());
  do_load_hierarchy(nh, o->get_model(), o->get_index());
}

void HierarchyLoadLink::create_recursive(kernel::Model *m,
                                         kernel::ParticleIndex root,
                                         kernel::ParticleIndex cur,
                                         RMF::NodeConstHandle name,
                                         kernel::ParticleIndexes rigid_bodies,
                                         Data &data) {
  set_association(name, m->get_particle(cur));
  data.load_static.setup_particle(name, m, cur, rigid_bodies);
  data.load_rigid_bodies.setup_particle(name, m, cur, rigid_bodies);
  data.load_xyzs.setup_particle(name, m, cur, rigid_bodies);

  IMP_FOREACH(RMF::NodeConstHandle ch, name.get_children()) {
    if (ch.get_type() == RMF::REPRESENTATION) {
      kernel::ParticleIndex child = m->add_particle(ch.get_name());
      atom::Hierarchy(m, cur)
          .add_child(atom::Hierarchy::setup_particle(m, child));
      create_recursive(m, root, child, ch, rigid_bodies, data);
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
  data.load_static.link_particle(node, m, cur, rigid_bodies);
  data.load_rigid_bodies.link_particle(node, m, cur, rigid_bodies);
  data.load_xyzs.link_particle(node, m, cur, rigid_bodies);

  do_link_particle(m, root, cur, node);

  RMF::NodeConstHandles nchs;
  IMP_FOREACH(RMF::NodeConstHandle ch, node.get_children()) {
    if (ch.get_type() == RMF::REPRESENTATION) {
      nchs.push_back(ch);
    }
  }
  kernel::ParticleIndexes pchs = atom::Hierarchy(m, cur).get_children_indexes();
  if (nchs.size() != pchs.size()) {
    IMP_THROW(
        "Number of children doesn't match the number of representation nodes "
        "at "
            << m->get_particle_name(cur) << ". "
            << "They are " << pchs.size() << " and " << nchs.size()
            << " respectively. " << pchs << " vs " << nchs,
        ValueException);
  }
  for (unsigned int i = 0; i < nchs.size(); ++i) {
    if (nchs[i].get_type() == RMF::REPRESENTATION) {
      add_link_recursive(m, root, pchs[i], nchs[i], rigid_bodies, data);
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
      reference_frame_factory_(fh) {
  RMF::Category imp_cat = fh.get_category("IMP");
  external_rigid_body_key_ =
      fh.get_key(imp_cat, "external frame", RMF::IntTraits());
}

void HierarchySaveLink::do_add(kernel::Particle *p, RMF::NodeHandle cur) {
  IMP_USAGE_CHECK(atom::Hierarchy(p).get_is_valid(true),
                  "Invalid hierarchy passed.");
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
  it->second->save_rigid_bodies.save(o->get_model(), nh.get_file());
  it->second->save_xyzs.save(o->get_model(), nh.get_file());
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
  data.save_rigid_bodies.setup_node(m, p, cur, rigid_bodies);
  data.save_xyzs.setup_node(m, p, cur, rigid_bodies);

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

HierarchySaveLink::HierarchySaveLink(RMF::FileHandle f)
    : P("HierarchySaveLink%1%") {
  RMF::Category imp_cat = f.get_category("IMP");
  external_rigid_body_key_ =
      f.get_key(imp_cat, "external frame", RMF::IntTraits());
}

IMPRMF_END_NAMESPACE
