/**
 *  \file IMP/rmf/HierarchyLoadLink.h
 *  \brief Handle read of atom hierarchy from rmf files.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/HierarchyLoadLink.h>
#include <IMP/rmf/internal/hierarchy_links_helpers.h>
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


void HierarchyLoadLink::do_load_one(RMF::NodeConstHandle nh,
                                    Particle *o) {
  data_.find(o->get_index())
      ->second->load_rigid_bodies.load(nh.get_file(), o->get_model());
  data_.find(o->get_index())
      ->second->load_xyzs.load(nh.get_file(), o->get_model());
  data_.find(o->get_index())
      ->second->load_gaussians.load(nh.get_file(), o->get_model());
  data_.find(o->get_index())->second->load_rigid_bodies.update_rigid_bodies(
      nh.get_file(), o->get_model());
  do_load_hierarchy(nh, o->get_model(), o->get_index());
}

void HierarchyLoadLink::create_recursive(Model *m,
                                         ParticleIndex root,
                                         ParticleIndex cur,
                                         RMF::NodeConstHandle name,
                                         ParticleIndexes rigid_bodies,
                                         Data &data) {
  set_association(name, m->get_particle(cur));
  ParticleIndexes breps, greps;
  Floats bresols, gresols;
  std::map<RMF::NodeConstHandle, ParticleIndex> rep_map;
  if (af_.get_is(name)) {
    IMP_FOREACH(RMF::NodeConstHandle alt,
                af_.get(name).get_alternatives(RMF::PARTICLE)) {
      if (alt == name) continue;
      ParticleIndex cur_rep = m->add_particle(alt.get_name());
      rep_map[alt] = cur_rep;
      create_recursive(m, root, cur_rep, alt, rigid_bodies, data);
      breps.push_back(cur_rep);
      if (explicit_resolution_factory_.get_is(alt)) {
        bresols.push_back(explicit_resolution_factory_.get(alt)
                                    .get_static_explicit_resolution());
      } else {
        bresols.push_back(-1);
      }
      IMP_LOG_TERSE("Found particle alternative " << alt << std::endl);
    }
    IMP_FOREACH(RMF::NodeConstHandle alt,
                af_.get(name).get_alternatives(RMF::GAUSSIAN_PARTICLE)) {
      if (alt == name) continue;
      ParticleIndex cur_rep = m->add_particle(alt.get_name());
      rep_map[alt] = cur_rep;
      create_recursive(m, root, cur_rep, alt, rigid_bodies, data);
      greps.push_back(cur_rep);
      if (explicit_resolution_factory_.get_is(alt)) {
        gresols.push_back(explicit_resolution_factory_.get(alt)
                                    .get_static_explicit_resolution());
      } else {
        gresols.push_back(-1);
      }
      IMP_LOG_TERSE("Found Gaussian alternative " << alt << std::endl);
    }
    // for each of them, add particle
    // recurse
  }

  data.load_static.setup_particle(name, m, cur, rigid_bodies);
  data.load_rigid_bodies.setup_particle(name, m, cur, rigid_bodies);
  data.load_xyzs.setup_particle(name, m, cur, rigid_bodies);
  data.load_gaussians.setup_particle(name, m, cur, rigid_bodies);

  IMP_FOREACH(RMF::NodeConstHandle ch, name.get_children()) {
    if (ch.get_type() == RMF::REPRESENTATION) {
      if (rep_map.find(ch) == rep_map.end()) {
        ParticleIndex child = m->add_particle(ch.get_name());
        atom::Hierarchy(m, cur)
            .add_child(atom::Hierarchy::setup_particle(m, child));
        create_recursive(m, root, child, ch, rigid_bodies, data);
      } else {
        /* Handle case where the child particle is also a representation */
        ParticleIndex child = rep_map[ch];
        atom::Hierarchy(m, cur)
            .add_child(atom::Hierarchy::setup_particle(m, child));
      }
    } else if (ch.get_type() == RMF::PROVENANCE) {
      // Note that at most only one such node should be encountered. If more
      // than one is found, this is an error (and Provenanced::setup_particle()
      // will throw)
      create_provenance(m, ch, cur);
    }
  }
  do_setup_particle(m, root, cur, name);
  if (!breps.empty() || !greps.empty()) {
    double resolution = -1;
    if (explicit_resolution_factory_.get_is(name)) {
      resolution = explicit_resolution_factory_.get(name)
                               .get_static_explicit_resolution();
    }
    atom::Representation::setup_particle(m, cur, resolution);
  }

  int i = 0;
  RMF_FOREACH(ParticleIndex r, breps) {
    atom::Representation(m, cur).add_representation(r, atom::BALLS,
                                                    bresols[i++]);
  }

  i = 0;
  RMF_FOREACH(ParticleIndex r, greps) {
    atom::Representation(m, cur).add_representation(r, atom::DENSITIES,
                                                    gresols[i++]);
  }
}

// Make *Provenance IMP particles corresponding to those in the RMF
void HierarchyLoadLink::create_provenance(Model *m, RMF::NodeConstHandle node,
                                          ParticleIndex cur) {
  core::Provenance prov = create_one_provenance(m, node);
  core::Provenanced provd = core::Provenanced::setup_particle(m, cur, prov);

  while ((node = internal::get_previous_rmf_provenance(node)) != RMF::NodeConstHandle()) {
    core::Provenance thisprov = create_one_provenance(m, node);
    prov.set_previous(thisprov);
    prov = thisprov;
  }
}

core::Provenance HierarchyLoadLink::create_one_provenance(Model *m,
                                              RMF::NodeConstHandle node) {
  if (strucpf_.get_is(node)) {
    RMF::decorator::StructureProvenanceConst rp = strucpf_.get(node);
    ParticleIndex ip = m->add_particle(node.get_name());
    return core::StructureProvenance::setup_particle(m, ip, rp.get_filename(),
                                rp.get_chain(), rp.get_residue_offset());
  } else if (samppf_.get_is(node)) {
    RMF::decorator::SampleProvenanceConst rp = samppf_.get(node);
    ParticleIndex ip = m->add_particle(node.get_name());
    return core::SampleProvenance::setup_particle(m, ip, rp.get_method(),
                                    rp.get_frames(), rp.get_iterations(),
                                    rp.get_replicas());
  } else if (combpf_.get_is(node)) {
    RMF::decorator::CombineProvenanceConst rp = combpf_.get(node);
    ParticleIndex ip = m->add_particle(node.get_name());
    return core::CombineProvenance::setup_particle(m, ip, rp.get_runs(),
                                                   rp.get_frames());
  } else if (filtpf_.get_is(node)) {
    RMF::decorator::FilterProvenanceConst rp = filtpf_.get(node);
    ParticleIndex ip = m->add_particle(node.get_name());
    return core::FilterProvenance::setup_particle(m, ip, rp.get_method(),
                                                  rp.get_threshold(),
                                                  rp.get_frames());
  } else if (clustpf_.get_is(node)) {
    RMF::decorator::ClusterProvenanceConst rp = clustpf_.get(node);
    ParticleIndex ip = m->add_particle(node.get_name());
    return core::ClusterProvenance::setup_particle(m, ip, rp.get_members());
  } else if (scriptpf_.get_is(node)) {
    RMF::decorator::ScriptProvenanceConst rp = scriptpf_.get(node);
    ParticleIndex ip = m->add_particle(node.get_name());
    return core::ScriptProvenance::setup_particle(m, ip, rp.get_filename());
  } else if (softpf_.get_is(node)) {
    RMF::decorator::SoftwareProvenanceConst rp = softpf_.get(node);
    ParticleIndex ip = m->add_particle(node.get_name());
    return core::SoftwareProvenance::setup_particle(m, ip,
                 rp.get_name(), rp.get_version(), rp.get_location());
  } else {
    IMP_THROW("Unhandled provenance type " << node, IOException);
  }
}

Particle *HierarchyLoadLink::do_create(RMF::NodeConstHandle node,
                                       Model *m) {
  IMP_FUNCTION_LOG;
  ParticleIndex ret = m->add_particle(node.get_name());
  data_.insert(std::make_pair(ret, boost::make_shared<Data>(node.get_file())));
  create_recursive(m, ret, ret, node, ParticleIndexes(), *data_[ret]);
  data_.find(ret)->second->load_bonds.setup_bonds(node, m, ret);
  if (!atom::Hierarchy(m, ret).get_is_valid(true)) {
    IMP_WARN("Invalid hierarchy created.");
  }
  return m->get_particle(ret);
}

void HierarchyLoadLink::add_link_recursive(Model *m,
                                           ParticleIndex root,
                                           ParticleIndex cur,
                                           RMF::NodeConstHandle node,
                                           ParticleIndexes rigid_bodies,
                                           Data &data) {
  IMP_USAGE_CHECK(internal::get_good_name_to_atom_node(m, cur) == node.get_name(),
                  "Names don't match: " << internal::get_good_name_to_atom_node(m, cur)
                  << " vs " << node.get_name());
  set_association(node, m->get_particle(cur), true);
  if (af_.get_is(node)) { // alternative factory
    RMF::decorator::AlternativesConst ad = af_.get(node);
    atom::Representation rd(m, cur);
    {
      RMF::NodeConstHandles alts = ad.get_alternatives(RMF::PARTICLE);
      atom::Hierarchies reps = rd.get_representations(atom::BALLS);
      if (alts.size() != reps.size()) {
        IMP_THROW("Number of alternate representations doesn't match: "
                      << alts.size() << " vs " << reps.size(),
                  IOException);
      }
      IMP_INTERNAL_CHECK(reps.back().get_particle_index() == cur,
                         "Not at the back");
      IMP_INTERNAL_CHECK(alts[0] == node, "Not at front of RMF");
      for (unsigned int i = 0; i < reps.size() - 1; ++i) {
        IMP_LOG_VERBOSE("Linking reps " << reps[i]->get_name() << " and "
                        << alts[i + 1].get_name() << std::endl);
        add_link_recursive(m, root, reps[i].get_particle_index(), alts[i + 1],
                           rigid_bodies, data);
      }
    }
    {
      RMF::NodeConstHandles alts = ad.get_alternatives(RMF::GAUSSIAN_PARTICLE);
      atom::Hierarchies reps = rd.get_representations(atom::DENSITIES);
      if (alts.size() != reps.size()) {
        IMP_THROW("Number of alternate representations doesn't match: "
                      << alts.size() << " vs " << reps.size(),
                  IOException);
      }
      for (unsigned int i = 0; i < reps.size(); ++i) {
        IMP_LOG_VERBOSE("Linking reps " << reps[i]->get_name() << " and "
                        << alts[i].get_name() << std::endl);
        add_link_recursive(m, root, reps[i].get_particle_index(), alts[i],
                           rigid_bodies, data);
      }
    }
  } // alternative factory
  data.load_static.link_particle(node, m, cur, rigid_bodies);
  data.load_rigid_bodies.link_particle(node, m, cur, rigid_bodies);
  data.load_xyzs.link_particle(node, m, cur, rigid_bodies);
  data.load_gaussians.link_particle(node, m, cur, rigid_bodies);

  do_link_particle(m, root, cur, node);

  RMF::NodeConstHandles nchs;
  IMP_FOREACH(RMF::NodeConstHandle ch, node.get_children()) {
    if (ch.get_type() == RMF::REPRESENTATION) {
      nchs.push_back(ch);
    }
  }
  ParticleIndexes pchs = atom::Hierarchy(m, cur).get_children_indexes();
  if (nchs.size() != pchs.size()) {
    IMP_THROW(
        "Number of children doesn't match the number of representation nodes "
        "at "
            << m->get_particle_name(cur) << ". "
            << "They are " << pchs.size() << " and " << nchs.size()
            << " respectively. " << get_particles(m, pchs) << " vs " << nchs,
        ValueException);
  }
  for (unsigned int i = 0; i < nchs.size(); ++i) {
    if (nchs[i].get_type() == RMF::REPRESENTATION) {
      add_link_recursive(m, root, pchs[i], nchs[i], rigid_bodies, data);
    }
  }
}

void HierarchyLoadLink::do_add_link(Particle *o,
                                    RMF::NodeConstHandle node) {
  IMP_FUNCTION_LOG;
  data_.insert(std::make_pair(o->get_index(),
                              boost::make_shared<Data>(node.get_file())));
  add_link_recursive(o->get_model(), o->get_index(), o->get_index(), node,
                     ParticleIndexes(),
                     *data_.find(o->get_index())->second);
}

HierarchyLoadLink::HierarchyLoadLink(RMF::FileConstHandle fh)
    : P("HierarchyLoadLink%1%"),
      intermediate_particle_factory_(fh),
      reference_frame_factory_(fh),
      af_(fh), strucpf_(fh), samppf_(fh),
      combpf_(fh), filtpf_(fh), clustpf_(fh),
      scriptpf_(fh), softpf_(fh), explicit_resolution_factory_(fh) {
  RMF::Category imp_cat = fh.get_category("IMP");
  external_rigid_body_key_ =
      fh.get_key(imp_cat, "external frame", RMF::IntTraits());
}

IMPRMF_END_NAMESPACE
