/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/internal/atom_links_static.h>
#include <IMP/rmf/associations.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Copy.h>
#include <IMP/atom/Diffusion.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Fragment.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Representation.h>
#include <IMP/atom/State.h>
#include <IMP/core/Typed.h>
#include <IMP/core/symmetry.h>
#include <IMP/display/Colored.h>
#include <RMF/SetCurrentFrame.h>
#include <RMF/NodeHandle.h>
#include <RMF/show_hierarchy.h>
#include <RMF/config.h>
#include <boost/range/iterator_range.hpp>

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
      fragment_factory_(fh),
      backwards_fragment_factory_(fh),
      state_factory_(fh),
      reference_factory_(fh),
      molecule_(fh) {
  RMF::Category phy = fh.get_category("physics");
  radius_key_ = fh.get_key(phy, "radius", RMF::FloatTraits());
  mass_key_ = fh.get_key(phy, "mass", RMF::FloatTraits());
}
void HierarchyLoadStatic::setup_particle(RMF::NodeConstHandle nh,
                                         Model *m,
                                         ParticleIndex p,
                                         const ParticleIndexes &) {
  IMP_FUNCTION_LOG;
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
    RMF::decorator::ResidueConst residue = residue_factory_.get(nh);
    int b = residue.get_residue_index();
    atom::Residue::setup_particle(
        m, p, atom::ResidueType(residue.get_residue_type())).set_index(b);
    IMP_INTERNAL_CHECK(atom::Residue::get_is_setup(m, p),
                       "Setup failed for residue");
  }
  if (domain_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("domain " << std::endl);
    RMF::IntRange be = domain_factory_.get(nh).get_residue_indexes();
    if (be[1] == be[0] + 1) {
    } else {
      atom::Domain::setup_particle(m, p, IntRange(be[0], be[1]));
    }
  }
  if (molecule_.get_is(nh)) {
    atom::Molecule::setup_particle(m, p);
  }
  if (fragment_factory_.get_is_static(nh)) {
    RMF::Ints idx = fragment_factory_.get(nh).get_residue_indexes();
    atom::Fragment::setup_particle(m, p, Ints(idx.begin(), idx.end()));
  }
  if (backwards_fragment_factory_.get_is_static(nh)) {
    RMF::Ints idx = backwards_fragment_factory_.get(nh).get_indexes();
    atom::Fragment::setup_particle(m, p, Ints(idx.begin(), idx.end()));
  }
  if (colored_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("colored " << std::endl);
    RMF::Vector3 c = colored_factory_.get(nh).get_rgb_color();
    display::Colored::setup_particle(m, p, display::Color(c[0], c[1], c[2]));
  }
  if (chain_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("chain " << std::endl);
    RMF::decorator::ChainConst chain = chain_factory_.get(nh);
    atom::Chain imp_chain = atom::Chain::setup_particle(m, p,
                                                        chain.get_chain_id());
    imp_chain.set_sequence(chain.get_sequence());
    imp_chain.set_chain_type(atom::ChainType(chain.get_chain_type()));
#if RMF_VERSION >= 100600
    imp_chain.set_sequence_offset(chain.get_sequence_offset());
    imp_chain.set_uniprot_accession(chain.get_uniprot_accession());
#endif
#if RMF_VERSION >= 100700
    imp_chain.set_label_asym_id(chain.get_label_asym_id());
#endif
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
  if (state_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("state " << std::endl);
    int dv = state_factory_.get(nh).get_state_index();
    atom::State::setup_particle(m, p, dv);
  }
  if (reference_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("reference " << std::endl);
    RMF::NodeConstHandle refn = reference_factory_.get(nh).get_reference();
    Particle *refp = get_association<Particle>(refn);
    core::Reference::setup_particle(m, p, refp);
  }
}

void HierarchyLoadStatic::link_particle(RMF::NodeConstHandle nh,
                                        Model *m,
                                        ParticleIndex p,
                                        const ParticleIndexes &) {
  IMP_FUNCTION_LOG;
  atom::Hierarchy hp(m, p);

  IMP_LOG_VERBOSE("Linking hierarchy particle " << hp
                  << " with " << hp.get_number_of_children() << " children"
                  << " to an RMF node with " << nh.get_children().size()
                  << " children, with decorations: " << std::endl);
  IMP_IF_LOG(IMP::VERBOSE) {
    if (hp.get_number_of_children() >= 8) {
      RMF::show_hierarchy_with_decorators(nh);
    }
  }
  if (nh.get_has_value(radius_key_)) {
    IMP_LOG_VERBOSE("xyzr " << std::endl);
    double r = nh.get_value(radius_key_);
    core::XYZR(m, p).set_radius(r);
  }
  if (particle_factory_.get_is(nh)) {
    RMF::decorator::ParticleConst mn = particle_factory_.get(nh);
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
    RMF::decorator::ResidueConst residue = residue_factory_.get(nh);
    int b = residue.get_residue_index();
    atom::Residue(m, p)
        .set_residue_type(atom::ResidueType(residue.get_residue_type()));
    atom::Residue(m, p).set_index(b);
    IMP_INTERNAL_CHECK(atom::Residue::get_is_setup(m, p),
                       "Setup failed for residue");
  }
  if (domain_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("domain " << std::endl);
    RMF::IntRange be = domain_factory_.get(nh).get_residue_indexes();
    if (be[1] == be[0] + 1) {
    } else {
      atom::Domain(m, p).set_index_range(IntRange(be[0], be[1]));
    }
  }
  if (fragment_factory_.get_is_static(nh)) {
    RMF::Ints idx = fragment_factory_.get(nh).get_residue_indexes();
    atom::Fragment(m, p).set_residue_indexes(Ints(idx.begin(), idx.end()));
  }
  if (colored_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("colored " << std::endl);
    RMF::Vector3 c = colored_factory_.get(nh).get_rgb_color();
    if(display::Colored::get_is_setup(m, p)){
      display::Colored(m, p).set_color(display::Color(c[0], c[1], c[2]));
    }else{
      IMP_WARN("Colored RMF node cannot be linked to a non-colored particle"
               << std::endl); // Color is not sufficiently important to abort
    }
  }
  if (chain_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("chain " << std::endl);
    RMF::decorator::ChainConst chain = chain_factory_.get(nh);
    atom::Chain imp_chain(m, p);
    imp_chain.set_id(chain.get_chain_id());
    imp_chain.set_sequence(chain.get_sequence());
    imp_chain.set_chain_type(atom::ChainType(chain.get_chain_type()));
#if RMF_VERSION >= 100600
    imp_chain.set_sequence_offset(chain.get_sequence_offset());
    imp_chain.set_uniprot_accession(chain.get_uniprot_accession());
#endif
#if RMF_VERSION >= 100700
    imp_chain.set_label_asym_id(chain.get_label_asym_id());
#endif
  }
  IMP_CHECK_CODE
    (
     if (typed_factory_.get_is_static(nh)) {
       // can't set
       IMP_LOG_VERBOSE("typed " << std::endl);
       std::string t = typed_factory_.get(nh).get_type_name();
       core::ParticleType pt(t);
       IMP_USAGE_CHECK(core::Typed(m, p).get_type() == pt,
                       "Particle types don't match");
       // core::Typed(m, p).set_particle_type(pt);
     }
     );
  if (diffuser_factory_.get_is_static(nh)) {
    IMP_LOG_VERBOSE("diffuser " << std::endl);
    double dv = diffuser_factory_.get(nh).get_diffusion_coefficient();
    atom::Diffusion(m, p).set_diffusion_coefficient(dv);
  }
  IMP_CHECK_CODE
    (
     IMP_IF_CHECK(USAGE) {
       if (copy_factory_.get_is_static(nh)) {
         IMP_LOG_VERBOSE("copy " << std::endl);
         int dv = copy_factory_.get(nh).get_copy_index();
         IMP_USAGE_CHECK(atom::Copy(m, p).get_copy_index() == dv,
                         "Copy indexes don't match");
       }
     }
     if (state_factory_.get_is_static(nh)) {
       IMP_LOG_VERBOSE("state " << std::endl);
       unsigned int dv = state_factory_.get(nh).get_state_index();
       IMP_USAGE_CHECK(atom::State(m, p).get_state_index() == dv,
                       "State indexes don't match");
     }
     if (reference_factory_.get_is_static(nh)) {
       IMP_LOG_VERBOSE("reference " << std::endl);
       RMF::NodeConstHandle refn = reference_factory_.get(nh).get_reference();
       Particle *refp = get_association<Particle>(refn);
       IMP_USAGE_CHECK(core::Reference(m, p).get_reference_particle() == refp,
                       "Reference particles don't match");
     }
     );
}

void HierarchySaveStatic::setup_node(Model *m, ParticleIndex p,
                                     RMF::NodeHandle n) {
  IMP_FUNCTION_LOG;
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
    RMF::decorator::Residue r = residue_factory_.get(n);
    r.set_residue_index(d.get_index());
    r.set_residue_type(d.get_residue_type().get_string());
  }
  if (atom::Molecule::get_is_setup(m, p)) {
    molecule_.set_is(n);
  }
  if (atom::Domain::get_is_setup(m, p)) {
    atom::Domain d(m, p);
    domain_factory_.get(n).set_residue_indexes(d.get_index_range().first,
                                               d.get_index_range().second);
  }
  if (atom::Fragment::get_is_setup(m, p)) {
    atom::Fragment d(m, p);
    Ints idx = d.get_residue_indexes();
    if (!idx.empty()) {
      fragment_factory_.get(n)
          .set_residue_indexes(RMF::Ints(idx.begin(), idx.end()));
    }
  }
  if (display::Colored::get_is_setup(m, p)) {
    display::Colored d(m, p);
    RMF::Vector3 color;
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
    RMF::decorator::Chain chain = chain_factory_.get(n);
    chain.set_chain_id(d.get_id());
    chain.set_sequence(d.get_sequence());
    chain.set_chain_type(d.get_chain_type().get_string());
#if RMF_VERSION >= 100600
    chain.set_sequence_offset(d.get_sequence_offset());
    chain.set_uniprot_accession(d.get_uniprot_accession());
#endif
#if RMF_VERSION >= 100700
    chain.set_label_asym_id(d.get_label_asym_id());
#endif
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
  if (atom::State::get_is_setup(m, p)) {
    atom::State d(m, p);
    state_factory_.get(n).set_state_index(d.get_state_index());
  }
  if (core::Reference::get_is_setup(m, p)) {
    core::Reference d(m, p);
    Particle *refp = d.get_reference_particle();
    RMF::NodeHandle refn = get_node_from_association(n.get_file(), refp);
    reference_factory_.get(n).set_reference(refn);
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
      fragment_factory_(fh),
      state_factory_(fh),
      reference_factory_(fh),
      molecule_(fh) {}

namespace {
atom::Bonded get_bonded(Particle *p) {
  if (atom::Bonded::get_is_setup(p)) {
    return atom::Bonded(p);
  } else {
    return atom::Bonded::setup_particle(p);
  }
}
}

void HierarchyLoadBonds::setup_bonds(RMF::NodeConstHandle n, Model *m,
                                     ParticleIndex p) {
  IMP_FUNCTION_LOG;
  if (af_.get_is(n)) {
    RMF::decorator::BondConst bd = af_.get(n);
    RMF::NodeConstHandle bd0 = bd.get_bonded_0();
    RMF::NodeConstHandle bd1 = bd.get_bonded_1();
    Particle *p0 = get_association<Particle>(bd0);
    Particle *p1 = get_association<Particle>(bd1);
    if (p0 && p1) {
      // ignore type and things
      atom::create_bond(get_bonded(p0), get_bonded(p1), atom::Bond::SINGLE);
    }
  } else {
    for(RMF::NodeConstHandle c : n.get_children()) {
      setup_bonds(c, m, p);
    }
  }
}

namespace {
atom::Bonds get_rep_bonds(atom::Hierarchy h) {
  IMP_FUNCTION_LOG;
  atom::Bonds ret;
  if (atom::Representation::get_is_setup(h)) {
    for(atom::Hierarchy r :
                atom::Representation(h).get_representations(atom::BALLS)) {
      if (r != h) {
        ret += atom::get_internal_bonds(r);
      }
    }
    IMP_LOG_VERBOSE("Found " << ret.size() << " alt bonds" << std::endl);
  } else {
    for(atom::Hierarchy ch : h.get_children()) {
      ret += get_rep_bonds(ch);
    }
  }
  return ret;
}
atom::Bonds get_rmf_bonds(atom::Hierarchy h) {
  atom::Bonds ret = atom::get_internal_bonds(h);
  return ret + get_rep_bonds(h);
}
}

void HierarchySaveBonds::setup_bonds(Model *m, ParticleIndex p,
                                     RMF::NodeHandle n) {
  IMP_FUNCTION_LOG;
  // kind of stupid. Would be nice to put alt bonds in the alt tree too.
  atom::Bonds bds = get_rmf_bonds(IMP::atom::Hierarchy(m, p));
  if (bds.empty()) return;
  // could do this better, but...
  RMF::NodeHandle bonds = n.add_child("bonds", RMF::ORGANIZATIONAL);
  RMF_FOREACH(atom::Bond bd, bds) {
    Particle *p0 = bd.get_bonded(0);
    Particle *p1 = bd.get_bonded(1);
    IMP_LOG_VERBOSE("Adding bond for pair " << Showable(p0) << " and "
                                            << Showable(p1) << std::endl);
    RMF::NodeHandle n0 = get_node_from_association(n.get_file(), p0);
    RMF::NodeHandle n1 = get_node_from_association(n.get_file(), p1);
    RMF::NodeHandle cbd = bonds.add_child("bond", RMF::BOND);
    RMF::decorator::Bond b = af_.get(cbd);
    b.set_bonded_0(n0.get_id().get_index());
    b.set_bonded_1(n1.get_id().get_index());
  }
}

IMPRMF_END_INTERNAL_NAMESPACE
