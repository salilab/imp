/**
 *  \file IMP/rmf/HierarchySaveLink.cpp
 *  \brief Handle writing of atom hierarchy to rmf files.
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/HierarchySaveLink.h>
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


void HierarchySaveLink::do_add(Particle *p, RMF::NodeHandle cur) {
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

void HierarchySaveLink::do_save_one(Particle *o, RMF::NodeHandle nh) {
  RMF::FileHandle fh = nh.get_file();
  DM::iterator it = data_.find(o->get_index());
  IMP_USAGE_CHECK(it != data_.end(), "I don't know that node");
  it->second->save_rigid_bodies.save(o->get_model(), nh.get_file());
  it->second->save_xyzs.save(o->get_model(), nh.get_file());
  it->second->save_gaussians.save(o->get_model(), nh.get_file());
  do_save_hierarchy(o->get_model(), o->get_index(), nh);
}

void HierarchySaveLink::add_recursive(Model *m, ParticleIndex root,
                                      ParticleIndex p,
                                      ParticleIndexes rigid_bodies,
                                      RMF::NodeHandle cur, Data &data) {
  IMP_LOG_VERBOSE("Adding " << atom::Hierarchy(m, p) << std::endl);
  // make sure not to double add
  if (p != root) set_association(cur, m->get_particle(p));
  RMF::NodeHandles prep_nodes, grep_nodes;
  std::map<ParticleIndex, RMF::NodeHandle> rep_map;
  if (IMP::atom::Representation::get_is_setup(m, p)) {
    IMP::atom::Representation rep(m, p);
    Floats bresols = rep.get_resolutions(atom::BALLS);
    /* First "BALLS" resolution is the default */
    explicit_resolution_factory_.get(cur).set_static_explicit_resolution(
                                               bresols[0]);
    {
      atom::Hierarchies reps = rep.get_representations(atom::BALLS);
      int i = 0; /* Skip first (default) resolution */
      IMP_FOREACH(atom::Hierarchy cr, reps) {
        i++;
        if (cr.get_particle_index() == p) continue;
        RMF::NodeHandle cn = cur.get_file().add_node
          ( internal::get_good_name_to_atom_node(m, cr.get_particle_index()),
            RMF::REPRESENTATION);
        explicit_resolution_factory_.get(cn).set_static_explicit_resolution(
                                                      bresols[i]);
        prep_nodes.push_back(cn);
        rep_map[cr.get_particle_index()] = cn;
        add_recursive(m, root, cr.get_particle_index(), rigid_bodies, cn, data);
      }
    }
    {
      atom::Hierarchies reps = rep.get_representations(atom::DENSITIES);
      Floats gresols = rep.get_resolutions(atom::DENSITIES);
      int i = -1;
      IMP_FOREACH(atom::Hierarchy cr, reps) {
        i++;
        if (cr.get_particle_index() == p) continue;
        RMF::NodeHandle cn = cur.get_file().add_node
          ( internal::get_good_name_to_atom_node(m, cr.get_particle_index()),
            RMF::REPRESENTATION);
        explicit_resolution_factory_.get(cn).set_static_explicit_resolution(
                                                      gresols[i]);
        grep_nodes.push_back(cn);
        rep_map[cr.get_particle_index()] = cn;
        add_recursive(m, root, cr.get_particle_index(), rigid_bodies, cn, data);
      }
    }
  }

  data.save_static.setup_node(m, p, cur);
  data.save_rigid_bodies.setup_node(m, p, cur, rigid_bodies);
  data.save_xyzs.setup_node(m, p, cur, rigid_bodies);
  data.save_gaussians.setup_node(m, p, cur, rigid_bodies);

  do_setup_node(m, root, p, cur);

  if (core::RigidBody::get_is_setup(m, p)) {
    rigid_bodies.push_back(p);
  }
  for (unsigned int i = 0; i < atom::Hierarchy(m, p).get_number_of_children();
       ++i) {
    ParticleIndex pc = atom::Hierarchy(m, p).get_child_index(i);
    if (rep_map.find(pc) == rep_map.end()) {
      RMF::NodeHandle curc =
        cur.add_child( internal::get_good_name_to_atom_node(m, pc),
                       RMF::REPRESENTATION);
      add_recursive(m, root, pc, rigid_bodies, curc, data);
    } else {
      /* Handle case where the child particle is also a representation */
      cur.add_child(rep_map[pc]);
    }
  }

  if (core::Provenanced::get_is_setup(m, p)) {
    add_provenance(m, p, cur);
  }

  if (!prep_nodes.empty() || !grep_nodes.empty()) {
    RMF::decorator::Alternatives ad = af_.get(cur);
    IMP_FOREACH(RMF::NodeHandle nh, prep_nodes) {
      ad.add_alternative(nh, RMF::PARTICLE);
    }
    IMP_FOREACH(RMF::NodeHandle nh, grep_nodes) {
      ad.add_alternative(nh, RMF::GAUSSIAN_PARTICLE);
    }
  }
}

// Make RMF PROVENANCE nodes corresponding to those in IMP
void HierarchySaveLink::add_provenance(Model *m, ParticleIndex p,
                                       RMF::NodeHandle cur) {
  core::Provenanced provd(m, p);
  for (core::Provenance prov = provd.get_provenance(); prov;
       prov = prov.get_previous()) {
    if (core::StructureProvenance::get_is_setup(prov)) {
      core::StructureProvenance ip(prov);
      cur = cur.add_child(m->get_particle_name(prov.get_particle_index()),
                          RMF::PROVENANCE);
      RMF::decorator::StructureProvenance rp = strucpf_.get(cur);
      rp.set_filename(ip.get_filename());
      rp.set_chain(ip.get_chain_id());
      rp.set_residue_offset(ip.get_residue_offset());
    } else if (core::SampleProvenance::get_is_setup(prov)) {
      core::SampleProvenance ip(prov);
      cur = cur.add_child(m->get_particle_name(prov.get_particle_index()),
                          RMF::PROVENANCE);
      RMF::decorator::SampleProvenance rp = samppf_.get(cur);
      rp.set_method(ip.get_method());
      rp.set_frames(ip.get_number_of_frames());
      rp.set_iterations(ip.get_number_of_iterations());
      rp.set_replicas(ip.get_number_of_replicas());
    } else if (core::CombineProvenance::get_is_setup(prov)) {
      core::CombineProvenance ip(prov);
      cur = cur.add_child(m->get_particle_name(prov.get_particle_index()),
                          RMF::PROVENANCE);
      RMF::decorator::CombineProvenance rp = combpf_.get(cur);
      rp.set_runs(ip.get_number_of_runs());
      rp.set_frames(ip.get_number_of_frames());
    } else if (core::FilterProvenance::get_is_setup(prov)) {
      core::FilterProvenance ip(prov);
      cur = cur.add_child(m->get_particle_name(prov.get_particle_index()),
                          RMF::PROVENANCE);
      RMF::decorator::FilterProvenance rp = filtpf_.get(cur);
      rp.set_method(ip.get_method());
      rp.set_threshold(ip.get_threshold());
      rp.set_frames(ip.get_number_of_frames());
    } else if (core::ClusterProvenance::get_is_setup(prov)) {
      core::ClusterProvenance ip(prov);
      cur = cur.add_child(m->get_particle_name(prov.get_particle_index()),
                          RMF::PROVENANCE);
      RMF::decorator::ClusterProvenance rp = clustpf_.get(cur);
      rp.set_members(ip.get_number_of_members());
    } else if (core::ScriptProvenance::get_is_setup(prov)) {
      core::ScriptProvenance ip(prov);
      cur = cur.add_child(m->get_particle_name(prov.get_particle_index()),
                          RMF::PROVENANCE);
      RMF::decorator::ScriptProvenance rp = scriptpf_.get(cur);
      rp.set_filename(ip.get_filename());
    } else if (core::SoftwareProvenance::get_is_setup(prov)) {
      core::SoftwareProvenance ip(prov);
      cur = cur.add_child(m->get_particle_name(prov.get_particle_index()),
                          RMF::PROVENANCE);
      RMF::decorator::SoftwareProvenance rp = softpf_.get(cur);
      rp.set_name(ip.get_software_name());
      rp.set_version(ip.get_version());
      rp.set_location(ip.get_location());
    } else {
      IMP_THROW("Unhandled provenance type "
                << m->get_particle_name(prov.get_particle_index()),
                IOException);
    }
  }
}

HierarchySaveLink::HierarchySaveLink(RMF::FileHandle f)
    : P("HierarchySaveLink%1%"), af_(f), strucpf_(f), samppf_(f),
      combpf_(f), filtpf_(f), clustpf_(f), scriptpf_(f), softpf_(f),
      explicit_resolution_factory_(f) {
  RMF::Category imp_cat = f.get_category("IMP");
  external_rigid_body_key_ =
      f.get_key(imp_cat, "external frame", RMF::IntTraits());
}

IMPRMF_END_NAMESPACE
