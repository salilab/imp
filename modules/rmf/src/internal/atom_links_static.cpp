/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of kernel::Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/atom_links_static.h>
#include <IMP/rmf/associations.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Diffusion.h>
#include <IMP/atom/Copy.h>
#include <IMP/atom/Fragment.h>
#include <IMP/core/Typed.h>
#include <IMP/display/Colored.h>
#include <RMF/SetCurrentFrame.h>
#include <RMF/NodeHandle.h>

IMPRMF_BEGIN_INTERNAL_NAMESPACE

HierarchyLoadStatic::HierarchyLoadStatic(RMF::FileConstHandle fh)
    : particle_factory_(fh),
      intermediate_particle_factory_(fh),
      atom_factory_(fh),
      residue_factory_(fh),
      chain_factory_(fh),
      colored_factory_(fh),
      copy_factory_(fh),
      diffuser_factory_(fh),
      typed_factory_(fh),
      domain_factory_(fh),
      fragment_factory_(fh) {
  RMF::Category phy = fh.get_category("physics");
  radius_key_ = fh.get_key(phy, "radius", RMF::FloatTraits());
  mass_key_ = fh.get_key(phy, "mass", RMF::FloatTraits());
}
void HierarchyLoadStatic::setup_particle(RMF::NodeConstHandle nh,
                                         kernel::Model *m,
                                         kernel::ParticleIndex p,
                                         const kernel::ParticleIndexes &) {
  atom::Hierarchy hp = atom::Hierarchy::setup_particle(m, p);
  IMP_LOG_VERBOSE("Particle " << hp << " is " << std::endl);
  if (nh.get_has_value(radius_key_)) {
    IMP_LOG_VERBOSE("xyzr " << std::endl);
    double r = nh.get_value(radius_key_);
    m->add_attribute(core::XYZR::get_radius_key(), p, r);
  }
  if (nh.get_has_value(mass_key_)) {
    IMP_LOG_VERBOSE("massive " << std::endl);
    atom::Mass::setup_particle(m, p, nh.get_value(mass_key_));
  }

  if (atom_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("atomic " << std::endl);
    if (!atom::get_atom_type_exists(nh.get_name())) {
      atom::add_atom_type(nh.get_name(),
                          atom::Element(atom_factory_.get(nh).get_element()));
    }
    atom::Atom::setup_particle(m, p, atom::AtomType(nh.get_name()));
  }
  if (residue_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("residue " << std::endl);
    RMF::ResidueConst residue = residue_factory_.get(nh);
    int b = residue.get_index();
    atom::Residue::setup_particle(m, p, atom::ResidueType(residue.get_type()))
        .set_index(b);
    IMP_INTERNAL_CHECK(atom::Residue::get_is_setup(m, p),
                       "Setup failed for residue");
  }
  if (domain_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("domain " << std::endl);
    int b, e;
    boost::tie(b, e) = domain_factory_.get(nh).get_indexes();
    if (e == b + 1) {
    } else {
      atom::Domain::setup_particle(m, p, IntRange(b, e));
    }
  }
  if (fragment_factory_.get_is_static(nh)) {
    RMF::Indexes idx = fragment_factory_.get(nh).get_indexes();
    atom::Fragment::setup_particle(m, p, Ints(idx.begin(), idx.end()));
  }
  if (colored_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("colored " << std::endl);
    RMF::Vector3 c = colored_factory_.get(nh).get_rgb_color();
    display::Colored::setup_particle(m, p, display::Color(c[0], c[1], c[2]));
  }
  if (chain_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("chain " << std::endl);
    std::string cci = chain_factory_.get(nh).get_chain_id();
    atom::Chain::setup_particle(m, p, cci);
  }
  if (typed_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("typed " << std::endl);
    std::string t = typed_factory_.get(nh).get_type_name();
    core::ParticleType pt(t);
    core::Typed::setup_particle(m, p, pt);
  }
  if (diffuser_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("diffuser " << std::endl);
    double dv = diffuser_factory_.get(nh).get_diffusion_coefficient();
    core::XYZ::setup_particle(m, p);
    atom::Diffusion::setup_particle(m, p, dv);
  }
  if (copy_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("copy " << std::endl);
    int dv = copy_factory_.get(nh).get_copy_index();
    atom::Copy::setup_particle(m, p, dv);
  }
}

void HierarchyLoadStatic::link_particle(RMF::NodeConstHandle nh,
                                        kernel::Model *m,
                                        kernel::ParticleIndex p,
                                        const kernel::ParticleIndexes &) {
  atom::Hierarchy hp(m, p);
  IMP_LOG_VERBOSE("Particle " << hp << " is " << std::endl);
  if (nh.get_has_value(radius_key_)) {
    IMP_LOG_VERBOSE("xyzr " << std::endl);
    double r = nh.get_value(radius_key_);
    core::XYZR(m, p).set_radius(r);
  }
  if (particle_factory_.get_is(nh)) {
    RMF::ParticleConst mn = particle_factory_.get(nh);
    IMP_LOG_VERBOSE("massive " << std::endl);
    atom::Mass(m, p).set_mass(mn.get_mass());
  }

  if (atom_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("atomic " << std::endl);
    if (!atom::get_atom_type_exists(nh.get_name())) {
      atom::add_atom_type(nh.get_name(),
                          atom::Element(atom_factory_.get(nh).get_element()));
    }
    atom::Atom(m, p).set_atom_type(atom::AtomType(nh.get_name()));
  }
  if (residue_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("residue " << std::endl);
    RMF::ResidueConst residue = residue_factory_.get(nh);
    int b = residue.get_index();
    atom::Residue(m, p).set_residue_type(atom::ResidueType(residue.get_type()));
    atom::Residue(m, p).set_index(b);
    IMP_INTERNAL_CHECK(atom::Residue::get_is_setup(m, p),
                       "Setup failed for residue");
  }
  if (domain_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("domain " << std::endl);
    int b, e;
    boost::tie(b, e) = domain_factory_.get(nh).get_indexes();
    if (e == b + 1) {
    } else {
      atom::Domain(m, p).set_index_range(IntRange(b, e));
    }
  }
  if (fragment_factory_.get_is_static(nh)) {
    RMF::Indexes idx = fragment_factory_.get(nh).get_indexes();
    atom::Fragment(m, p).set_residue_indexes(Ints(idx.begin(), idx.end()));
  }
  if (colored_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("colored " << std::endl);
    RMF::Vector3 c = colored_factory_.get(nh).get_rgb_color();
    display::Colored(m, p).set_color(display::Color(c[0], c[1], c[2]));
  }
  if (chain_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("chain " << std::endl);
    std::string cci = chain_factory_.get(nh).get_chain_id();
    atom::Chain(m, p).set_id(cci);
  }
  if (typed_factory_.get_is_static(nh)) {
    // can't set
    IMP_LOG_VERBOSE("typed " << std::endl);
    std::string t = typed_factory_.get(nh).get_type_name();
    core::ParticleType pt(t);
    IMP_USAGE_CHECK(core::Typed(m, p).get_type() == pt,
                    "Particle types don't match");
    // core::Typed(m, p).set_particle_type(pt);
  }
  if (diffuser_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("diffuser " << std::endl);
    double dv = diffuser_factory_.get(nh).get_diffusion_coefficient();
    atom::Diffusion(m, p).set_diffusion_coefficient(dv);
  }
  if (copy_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("copy " << std::endl);
    int dv = copy_factory_.get(nh).get_copy_index();
    IMP_USAGE_CHECK(atom::Copy(m, p).get_copy_index() == dv,
                    "Copy indexes don't match");
  }
}

void HierarchySaveStatic::setup_node(kernel::Model *m, kernel::ParticleIndex p,
                                     RMF::NodeHandle n) {
  if (core::XYZR::get_is_setup(m, p)) {
    core::XYZR d(m, p);
    intermediate_particle_factory_.get(n).set_radius(d.get_radius());
  }
  if (atom::Mass::get_is_setup(m, p)) {
    atom::Mass d(m, p);
    particle_factory_.get(n).set_mass(d.get_mass());
  }
  if (atom::Atom::get_is_setup(m, p)) {
    atom::Atom d(m, p);
    atom_factory_.get(n).set_element(d.get_element());
  }
  if (atom::Residue::get_is_setup(m, p)) {
    atom::Residue d(m, p);
    RMF::Residue r = residue_factory_.get(n);
    r.set_index(d.get_index());
    r.set_type(d.get_residue_type().get_string());
  }
  if (atom::Domain::get_is_setup(m, p)) {
    atom::Domain d(m, p);
    domain_factory_.get(n).set_indexes(
        std::make_pair(d.get_index_range().first, d.get_index_range().second));
  }
  if (atom::Fragment::get_is_setup(m, p)) {
    atom::Fragment d(m, p);
    Ints idx = d.get_residue_indexes();
    if (!idx.empty()) {
      fragment_factory_.get(n)
          .set_indexes(RMF::Indexes(idx.begin(), idx.end()));
    }
  }
  if (display::Colored::get_is_setup(m, p)) {
    display::Colored d(m, p);
    RMF::Floats color(3);
    color[0] = d.get_color().get_red();
    color[1] = d.get_color().get_green();
    color[2] = d.get_color().get_blue();
    colored_factory_.get(n).set_rgb_color(color);
  }
  if (core::Typed::get_is_setup(m, p)) {
    core::Typed d(m, p);
    typed_factory_.get(n).set_type_name(d.get_type().get_string());
  }
  if (atom::Chain::get_is_setup(m, p)) {
    atom::Chain d(m, p);
    chain_factory_.get(n).set_chain_id(d.get_id());
  }
  if (atom::Diffusion::get_is_setup(m, p)) {
    atom::Diffusion d(m, p);
    IMP_USAGE_CHECK(d.get_diffusion_coefficient() > 0,
                    "Zero diffusion coefficient");
    diffuser_factory_.get(n)
        .set_diffusion_coefficient(d.get_diffusion_coefficient());
  }
  if (atom::Copy::get_is_setup(m, p)) {
    atom::Copy d(m, p);
    copy_factory_.get(n).set_copy_index(d.get_copy_index());
  }
}

HierarchySaveStatic::HierarchySaveStatic(RMF::FileHandle fh)
    : particle_factory_(fh),
      intermediate_particle_factory_(fh),
      atom_factory_(fh),
      residue_factory_(fh),
      chain_factory_(fh),
      colored_factory_(fh),
      copy_factory_(fh),
      diffuser_factory_(fh),
      typed_factory_(fh),
      domain_factory_(fh),
      fragment_factory_(fh) {}

namespace {

atom::Bonded get_bonded(kernel::Particle *p) {
  if (atom::Bonded::get_is_setup(p)) {
    return atom::Bonded(p);
  } else {
    return atom::Bonded::setup_particle(p);
  }
}
}

void HierarchyLoadBonds::setup_bonds(RMF::NodeConstHandle n, kernel::Model *m,
                                     kernel::ParticleIndex p) {
  RMF::NodeConstHandles children = n.get_children();
  if (n.get_type() == RMF::BOND && children.size() == 2) {
    RMF::NodeConstHandle bd0, bd1;
    if (af_.get_is(children[0])) {
      bd0 = af_.get(children[0]).get_aliased();
      bd1 = af_.get(children[1]).get_aliased();
    } else {
      bd0 = children[0];
      bd1 = children[1];
    }
    Particle *p0 = get_association<kernel::Particle>(bd0);
    Particle *p1 = get_association<kernel::Particle>(bd1);
    if (p0 && p1) {
      // ignore type and things
      atom::create_bond(get_bonded(p0), get_bonded(p1), atom::Bond::SINGLE);
    }
  } else {
    IMP_FOREACH(RMF::NodeConstHandle c, children) { setup_bonds(c, m, p); }
  }
}

void HierarchySaveBonds::setup_bonds(kernel::Model *m, kernel::ParticleIndex p,
                                     RMF::NodeHandle n) {
  IMP_FUNCTION_LOG;
  atom::Bonds bds = atom::get_internal_bonds(atom::Hierarchy(m, p));
  if (bds.empty()) return;
  // could do this better, but...
  RMF::NodeHandle bonds = n.add_child("bonds", RMF::ORGANIZATIONAL);
  for (unsigned int i = 0; i < bds.size(); ++i) {
    kernel::Particle *p0 = bds[i].get_bonded(0);
    kernel::Particle *p1 = bds[i].get_bonded(1);
    IMP_LOG_VERBOSE("Adding bond for pair " << Showable(p0) << " and "
                                            << Showable(p1) << std::endl);
    RMF::NodeHandle n0 = get_node_from_association(n.get_file(), p0);
    RMF::NodeHandle n1 = get_node_from_association(n.get_file(), p1);
    RMF::NodeHandle bd = bonds.add_child("bond", RMF::BOND);
    RMF::internal::add_child_alias(af_, bd, n0);
    RMF::internal::add_child_alias(af_, bd, n1);
  }
}

IMPRMF_END_INTERNAL_NAMESPACE
