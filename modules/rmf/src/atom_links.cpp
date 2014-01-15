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
#include <IMP/atom/State.h>
#include <IMP/atom/Representation.h>
#include <IMP/atom/Fragment.h>
#include <IMP/core/Typed.h>
#include <IMP/display/Colored.h>
#include <IMP/rmf/internal/atom_links_coordinate_helpers.h>
#include <RMF/decorator/alternatives.h>
#include <algorithm>
#include <RMF/utility.h>
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
  kernel::ParticleIndexes reps;
  if (af_.get_is(name)) {
    IMP_FOREACH(RMF::NodeConstHandle alt,
                af_.get(name).get_alternatives(RMF::decorator::PARTICLE)) {
      if (alt == name) continue;
      kernel::ParticleIndex cur_rep = m->add_particle(alt.get_name());
      create_recursive(m, root, cur_rep, alt, rigid_bodies, data);
      reps.push_back(cur_rep);
    }
    // for each of them, add particle
    // recurse
  }

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
  if (!reps.empty()) {
    atom::Representation rep= atom::Representation::setup_particle(m, cur);
    RMF_FOREACH(kernel::ParticleIndex r, reps) {
      rep.add_representation(r, atom::BALLS);
    }
  }
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
                  "Names don't match: " << get_good_name(m, cur) << " vs "
                                        << node.get_name());
  set_association(node, m->get_particle(cur), true);
  if (af_.get_is(node)) {
    RMF::decorator::AlternativesConst ad = af_.get(node);
    atom::Representation rd(m, cur);
    RMF::NodeConstHandles alts = ad.get_alternatives(RMF::decorator::PARTICLE);
    atom::Hierarchies reps = rd.get_representations(atom::BALLS);
    if (alts.size() != reps.size()) {
      IMP_THROW("Number of alternate representations doesn't match: "
                << alts.size() << " vs " << reps.size(), IOException);
    }
    IMP_INTERNAL_CHECK(reps.back().get_particle_index() == cur,
                       "Not at the back");
    IMP_INTERNAL_CHECK(alts[0] == node, "Not at front of RMF");
    for (unsigned int i = 0; i < reps.size() - 1; ++i) {
      std::cout << "Linking reps " << reps[i]->get_name() << " and "
                << alts[i + 1].get_name() << std::endl;
      add_link_recursive(m, root, reps[i].get_particle_index(), alts[i + 1],
                         rigid_bodies, data);
    }
  }
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
      reference_frame_factory_(fh),
      af_(fh) {
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
  RMF::NodeHandles rep_nodes;
  if (IMP::atom::Representation::get_is_setup(m, p)) {
    atom::Hierarchies reps =
        IMP::atom::Representation(m, p).get_representations(atom::BALLS);
    IMP_FOREACH(atom::Hierarchy cr, reps) {
      if (cr.get_particle_index() == p) continue;
      RMF::NodeHandle cn = cur.get_file().add_node(
          get_good_name(m, cr.get_particle_index()), RMF::REPRESENTATION);
      rep_nodes.push_back(cn);
      add_recursive(m, root, cr.get_particle_index(), rigid_bodies, cn, data);
    }
  }

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

  if (!rep_nodes.empty()) {
    RMF::decorator::Alternatives ad = af_.get(cur);
    IMP_FOREACH(RMF::NodeHandle nh, rep_nodes) {
      ad.add_alternative(nh, RMF::decorator::PARTICLE);
    }
  }
}

HierarchySaveLink::HierarchySaveLink(RMF::FileHandle f)
  : P("HierarchySaveLink%1%"), af_(f) {
  RMF::Category imp_cat = f.get_category("IMP");
  external_rigid_body_key_ =
      f.get_key(imp_cat, "external frame", RMF::IntTraits());
}

IMPRMF_END_NAMESPACE
