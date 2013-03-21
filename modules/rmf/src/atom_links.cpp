/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
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
#include <IMP/core/Typed.h>
#include <IMP/display/Colored.h>
#include <algorithm>
#include <RMF/utility.h>
#include <RMF/decorators.h>

IMPRMF_BEGIN_NAMESPACE

namespace {

std::string get_good_name(Particle * h) {
  if (atom::Atom::particle_is_instance(h)) {
    return atom::Atom(h).get_atom_type().get_string();
  } else if (atom::Residue::particle_is_instance(h)) {
    std::ostringstream oss;
    oss << atom::Residue(h).get_index();
    return oss.str();
  } else if (atom::Chain::particle_is_instance(h)) {
    return std::string(1, atom::Chain(h).get_id());
  } else {
    return h->get_name();
  }
}
atom::Bonded get_bonded(Particle *p) {
  if (atom::Bonded::particle_is_instance(p)) {
    return atom::Bonded(p);
  } else {
    return atom::Bonded::setup_particle(p);
  }
}


void create_bonds(RMF::NodeConstHandle fhc,
                  RMF::AliasConstFactory af,
                  const base::map<RMF::NodeConstHandle,
                                           Particle*> &map) {
    RMF::NodeConstHandles children= fhc.get_children();
    if (fhc.get_type()== RMF::BOND && children.size()==2) {
      RMF::NodeConstHandle bd0, bd1;
      if (af.get_is(children[0])) {
        bd0= af.get(children[0]).get_aliased();
        bd1= af.get(children[1]).get_aliased();
      } else {
        bd0= children[0];
        bd1= children[1];
      }
      if (map.find(bd0) != map.end() && map.find(bd1) != map.end()) {
        // ignore type and things
        atom::create_bond(get_bonded(map.find(bd0)->second),
                          get_bonded(map.find(bd1)->second),
                          atom::Bond::SINGLE);
      }
    } else {
      for (unsigned int i=0; i< children.size(); ++i) {
        create_bonds(children[i], af, map);
      }
    }
}

void create_bonds(RMF::FileConstHandle fhc, const RMF::NodeIDs &nhs,
                  const ParticlesTemp &ps) {
  base::map<RMF::NodeConstHandle, Particle*> map;
  for (unsigned int i=0; i< nhs.size(); ++i) {
    map[fhc.get_node_from_id(nhs[i])]= ps[i];
  }
  RMF::AliasConstFactory af(fhc);
  create_bonds(fhc.get_root_node(), af, map);
}

void create_rigid_bodies(Model *m,
                         const base::map<unsigned int, ParticlesTemp>
                         &rbs) {
  IMP_FUNCTION_LOG;
  for (base::map<unsigned int, ParticlesTemp>::const_iterator
           it= rbs.begin(); it != rbs.end(); ++it) {
    // skip already created rigid bodies eg when there are multiple hierarchies
    // so we get here twice
    if (core::RigidMember::particle_is_instance(it->second.front())) continue;
    IMP_LOG_TERSE( "Creating rigid body " << it->first << " on "
            << it->second << std::endl);
    std::ostringstream oss;
    oss << "rigid body " << it->first;
    IMP_NEW(Particle, rbp, (m, oss.str()));
    core::RigidBody::setup_particle(rbp, it->second);
  }
}

  void fix_internal_coordinates(core::RigidBody rb,
                                algebra::ReferenceFrame3D rf,
                                ParticleIndex pi) {
    // Make sure the internal coordinates of the particles match
    // This is needed to handle scripts that change them during optimation
    // and save the result out to RMF.
    core::RigidMember rm(rb.get_model(), pi);
    if (core::RigidBody::particle_is_instance(rb.get_model(), pi)) {
      core::RigidBody crb(rb.get_model(), pi);
      algebra::ReferenceFrame3D crf= crb.get_reference_frame();
      algebra::ReferenceFrame3D lcrf= rf.get_local_reference_frame(crf);
      rm.set_internal_transformation(lcrf.get_transformation_to());
    } else {
      algebra::Vector3D crf= rm.get_coordinates();
      algebra::Vector3D lcrf= rf.get_local_coordinates(crf);
      if ((lcrf- rm.get_internal_coordinates()).get_squared_magnitude()
          > .0001) {
        // try to avoid resetting caches
        rm.set_internal_coordinates(lcrf);
      }
    }
  }

  void fix_rigid_body(const std::pair<core::RigidBody,
                      ParticleIndexes> &in) {
    //core::RigidMembers rms=in.second;
    core::RigidBody rb= in.first;
    rb.set_reference_frame_from_members(in.second);
    algebra::ReferenceFrame3D rf= rb.get_reference_frame();
    // fix rigid bodies that aren't rigid
    for (unsigned int i=0; i< in.second.size(); ++i) {
      fix_internal_coordinates(rb, rf, in.second[i]);
    }
  }

}
void HierarchyLoadLink::do_load_node(RMF::NodeConstHandle nh,
                                     Particle *o) {
  if (rigid_factory_.get_is(nh)) {
    RMF::RigidParticleConst p=rigid_factory_.get(nh);
    RMF::Floats cs= p.get_coordinates();
    algebra::Vector3D v(cs.begin(),
                        cs.end());
    RMF::Floats orient= p.get_orientation();
    algebra::Transformation3D
        tr(algebra::Rotation3D(algebra
                               ::Vector4D(orient.begin(),
                                          orient.end())),
           v);
    algebra::ReferenceFrame3D rf(tr);
    core::RigidBody(o).set_reference_frame(rf);
  } else if (reference_frame_factory_.get_is(nh)) {
    RMF::ReferenceFrameConst p=reference_frame_factory_.get(nh);
    RMF::Floats cs= p.get_translation();
    algebra::Vector3D v(cs.begin(),
                        cs.end());
    RMF::Floats orient= p.get_rotation();
    algebra::Transformation3D
        tr(algebra::Rotation3D(algebra
                               ::Vector4D(orient.begin(),
                                          orient.end())),
           v);
    algebra::ReferenceFrame3D rf(tr);
    core::RigidBody(o).set_reference_frame(rf);
  } else if (intermediate_particle_factory_.get_is(nh)) {
    RMF::Floats cs= intermediate_particle_factory_.get(nh)
        .get_coordinates();
    algebra::Vector3D v(cs.begin(),
                        cs.end());
    core::XYZR(o).set_coordinates(v);
  }
  if (colored_factory_.get_is(nh)) {
    RMF::Floats c= colored_factory_.get(nh).get_rgb_color();
    display::Colored(o).set_color(display::Color(c[0], c[1], c[2]));
  }
  // needed since atom requires XYZ
  if (atom_factory_.get_is(nh)) {
    if (!atom::Atom::particle_is_instance(o)) {
      IMP_LOG_VERBOSE( "atomic " << std::endl);
      if (!atom::get_atom_type_exists(nh.get_name())) {
        atom::add_atom_type(nh.get_name(),
                            atom::Element(atom_factory_.get(nh)
                                          .get_element()));
      }
      atom::Atom::setup_particle(o, atom::AtomType(nh.get_name()));
    }
  }
}

void HierarchyLoadLink::do_load_one( RMF::NodeConstHandle nh,
                                     Particle *o) {
  RMF::FileConstHandle fh= nh.get_file();
  const ConstData&d= contents_.find(o)->second;
  IMP_LOG_VERBOSE( "Loading hierarchy " << atom::Hierarchy(o)
          << " with contents " << atom::Hierarchies(d.get_particles())
          << std::endl);
  base::map<core::RigidBody, ParticleIndexes> rbs;
  for (unsigned int i=0; i< d.get_nodes().size(); ++i) {
    do_load_node(fh.get_node_from_id(d.get_nodes()[i]),
                         d.get_particles()[i]);
    if (core::RigidMember::particle_is_instance(d.get_particles()[i])) {
      rbs[core::RigidMember(d.get_particles()[i]).get_rigid_body()].
        push_back(d.get_particles()[i]->get_index());
    }
  }
  std::for_each(rbs.begin(), rbs.end(), fix_rigid_body);

  //IMP::atom::show(atom::Hierarchy(o));
  IMP_INTERNAL_CHECK(atom::Hierarchy(o).get_is_valid(true),
                     "Invalid hierarchy loaded");
}

bool HierarchyLoadLink::setup_particle(Particle *root,
                                       RMF::NodeConstHandle nh,
                                       Particle *p,
                                       Particle *rbp) {
  contents_[root].access_particles().push_back(p);
  contents_[root].access_nodes().push_back(nh.get_id());
  atom::Hierarchy hp=atom::Hierarchy::setup_particle(p);
  IMP_LOG_VERBOSE( "Particle " << hp << " is " << std::endl);
  bool crbp=false;
  if (rigid_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "rigid " << std::endl);
    crbp=true;
    core::RigidBody::setup_particle(p, algebra::ReferenceFrame3D());
    if (rbp) {
      core::RigidBody(rbp).add_member(p);
    }
  }
  if (intermediate_particle_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "xyzr " << std::endl);
    double r= intermediate_particle_factory_.get(nh).get_radius();
    core::XYZR::setup_particle(p).set_radius(r);
    if (rbp) {
      core::RigidBody(rbp).add_member(p);
    }
  }
  if (particle_factory_.get_is(nh)) {
    RMF::ParticleConst m= particle_factory_.get(nh);
    IMP_LOG_VERBOSE( "massive " << std::endl);
    atom::Mass::setup_particle(p, m.get_mass());
  }

  if (atom_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "atomic " << std::endl);
    if (!atom::get_atom_type_exists(nh.get_name())) {
      atom::add_atom_type(nh.get_name(),
                          atom::Element(atom_factory_.get(nh)
                                        .get_element()));
    }
    atom::Atom::setup_particle(p, atom::AtomType(nh.get_name()));
  }
  if (residue_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "residue " << std::endl);
    RMF::ResidueConst residue=residue_factory_.get(nh);
    int b= residue.get_index();
    atom::Residue::setup_particle(p,
                                  atom::ResidueType(residue.get_type()))
        .set_index(b);
    IMP_INTERNAL_CHECK(atom::Residue::particle_is_instance(p),
                       "Setup failed for residue");
  }
  if (domain_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "domian " << std::endl);
    int b,e;
    boost::tie(b,e)= domain_factory_.get(nh).get_indexes();
    if (e==b+1) {
    } else {
      atom::Domain::setup_particle(p, b, e);
    }
  }
  if (colored_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "colored " << std::endl);
    RMF::Floats c= colored_factory_.get(nh).get_rgb_color();
    display::Colored::setup_particle(p, display::Color(c[0], c[1], c[2]));
  }
  if (chain_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "chain " << std::endl);
    int cci= chain_factory_.get(nh).get_chain_id();
    atom::Chain::setup_particle(p, cci+'A');
  }
  if (typed_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "typed " << std::endl);
    std::string t= typed_factory_.get(nh).get_type_name();
    core::ParticleType pt(t);
    core::Typed::setup_particle(p, pt);
  }
  if (diffuser_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "diffuser " << std::endl);
    double dv= diffuser_factory_.get(nh)
        .get_diffusion_coefficient();
    atom::Diffusion::setup_particle(p, dv);
  }
  if (copy_factory_.get_is(nh)) {
    IMP_LOG_VERBOSE( "copy " << std::endl);
    int dv= copy_factory_.get(nh).get_copy_index();
    atom::Copy::setup_particle(p, dv);
  }
  if (nh.get_has_value(rigid_body_key_) && !rbp) {
    int v= nh.get_value(rigid_body_key_);
    rigid_bodies_[v].push_back(p);
    // load coordinates for the particle so rb is set up right
    RMF::Floats cs= intermediate_particle_factory_.get(nh)
        .get_coordinates();
    algebra::Vector3D vv(cs.begin(),
                        cs.end());
    core::XYZ(p).set_coordinates(vv);
  }
  return crbp;
}

Particle* HierarchyLoadLink::do_create_recursive(Particle *root,
                                                 RMF::NodeConstHandle name,
                                                 Particle *rb) {
  IMP_NEW(Particle, p, (m_, name.get_name()));
  if (!root) {
    root=p;
  } else {
    set_association(name, p);
  }
  bool is_rb=setup_particle(root, name, p, rb);
  if (is_rb) {
    rb=p;
  }
  RMF::NodeConstHandles ch= name.get_children();
  for (unsigned int i=0; i< ch.size(); ++i) {
    if (ch[i].get_type()== RMF::REPRESENTATION) {
      Particle*c= do_create_recursive(root, ch[i], rb);
      atom::Hierarchy(p).add_child(atom::Hierarchy(c));
    }
  }
  return p.release();
}

Particle* HierarchyLoadLink::do_create(RMF::NodeConstHandle name) {
  Particle *ret= do_create_recursive(nullptr, name, nullptr);
  create_bonds(name.get_file(),contents_[ret].get_nodes(),
               contents_[ret].get_particles());
  create_rigid_bodies(ret->get_model(), rigid_bodies_);
  IMP_USAGE_CHECK(name.get_file().get_current_frame().get_id() ==
                  RMF::FrameID(0),
                  "Bad frame in create: "
                  << name.get_file().get_current_frame());
  //IMP::atom::show(atom::Hierarchy(ret));
  IMP_INTERNAL_CHECK(atom::Hierarchy(ret).get_is_valid(true),
                     "Invalid hierarchy created");
  return ret;
}

void HierarchyLoadLink::do_add_link_recursive(Particle *root,
                                              Particle *o,
                                              RMF::NodeConstHandle node) {
  IMP_LOG_VERBOSE( "Linking " << Showable(o) << " and " << node << std::endl);
  contents_[root].access_particles().push_back(o);
  contents_[root].access_nodes().push_back(node.get_id());
  set_association(node, o, true);
  RMF::NodeConstHandles ch= node.get_children();
  int cur=0;
  for (unsigned int i=0; i< ch.size(); ++i) {
    if (ch[i].get_type()== RMF::REPRESENTATION) {
      do_add_link_recursive(root,
                            atom::Hierarchy(o).get_child(cur),
                            ch[i]);
      ++cur;
    }
  }
}

void HierarchyLoadLink::do_add_link(Particle *o, RMF::NodeConstHandle node) {
  do_add_link_recursive(o, o, node);
}
HierarchyLoadLink::HierarchyLoadLink(RMF::FileConstHandle fh, Model *m):
    P("HierarchyLoadLink%1%"), m_(m),
    particle_factory_(fh),
    intermediate_particle_factory_(fh),
    rigid_factory_(fh),
    atom_factory_(fh),
    residue_factory_(fh),
    chain_factory_(fh),
    colored_factory_(fh),
    copy_factory_(fh),
    diffuser_factory_(fh),
    typed_factory_(fh),
    domain_factory_(fh),
    reference_frame_factory_(fh)
{
  RMF::Category cat= fh.get_category("IMP");
  rigid_body_key_=fh.get_key<RMF::IndexTraits>(cat, "rigid body");
}

namespace {
void copy_bonds(Particle *root,
                RMF::NodeHandle fhc) {
  IMP_FUNCTION_LOG;
  atom::Bonds bds= atom::get_internal_bonds(atom::Hierarchy(root));
  if (bds.empty()) return;
  // could do this better, but...
  RMF::NodeHandle bonds= fhc.add_child("bonds",
                                       RMF::ORGANIZATIONAL);
  RMF::AliasFactory af(fhc.get_file());
  for (unsigned int i=0; i< bds.size(); ++i) {
    Particle *p0= bds[i].get_bonded(0);
    Particle *p1= bds[i].get_bonded(1);
    IMP_LOG_VERBOSE( "Adding bond for pair " << Showable(p0)
            << " and " << Showable(p1) << std::endl);
    RMF::NodeHandle n0= get_node_from_association(fhc.get_file(), p0);
    RMF::NodeHandle n1= get_node_from_association(fhc.get_file(), p1);
    RMF::NodeHandle bd= bonds.add_child("bond", RMF::BOND);
    RMF::add_child_alias(af, bd, n0);
    RMF::add_child_alias(af, bd, n1);
  }
}
}

void HierarchySaveLink::setup_node(Particle *p, RMF::NodeHandle n) {
  if (core::XYZR::particle_is_instance(p)) {
    core::XYZR d(p);
    intermediate_particle_factory_.get(n).set_radius(d.get_radius());
  }
  if (core::RigidBody::particle_is_instance(p)
      && atom::Hierarchy(p).get_number_of_children()==0
      && core::XYZR::particle_is_instance(p)) {
    // center the particle's ball
    RMF::Floats zeros(3);
    std::fill(zeros.begin(), zeros.end(), 0.0);
    intermediate_particle_factory_.get(n).set_coordinates(zeros);
  }
  if (atom::Mass::particle_is_instance(p)) {
    atom::Mass d(p);
    particle_factory_.get(n).set_mass(d.get_mass());
  }
  if (atom::Atom::particle_is_instance(p)) {
    atom::Atom d(p);
    atom_factory_.get(n).set_element(d.get_element());
  }
  if (atom::Residue::particle_is_instance(p)) {
    atom::Residue d(p);
    RMF::Residue r= residue_factory_.get(n);
    r.set_index(d.get_index());
    r.set_type(d.get_residue_type().get_string());
  }
  if (atom::Domain::particle_is_instance(p)) {
    atom::Domain d(p);
    domain_factory_.get(n).set_indexes( d.get_index_range().first,
                                        d.get_index_range().second);
  }
  if (display::Colored::particle_is_instance(p)) {
    display::Colored d(p);
    RMF::Floats color(3);
    color[0]=d.get_color().get_red();
    color[1]=d.get_color().get_green();
    color[2]=d.get_color().get_blue();
    colored_factory_.get(n).set_rgb_color(color);
  }
  if (core::Typed::particle_is_instance(p)) {
    core::Typed d(p);
    typed_factory_.get(n).set_type_name(d.get_type().get_string());
  }
  if (atom::Chain::particle_is_instance(p)) {
    atom::Chain d(p);
    chain_factory_.get(n).set_chain_id(d.get_id()-'A');
  }
  if (atom::Diffusion::particle_is_instance(p)) {
    atom::Diffusion d(p);
    IMP_USAGE_CHECK(d.get_diffusion_coefficient() >0,
                    "Zero diffusion coefficient");
    diffuser_factory_.get(n)
        .set_diffusion_coefficient(d.get_diffusion_coefficient());
  }
  if (atom::Copy::particle_is_instance(p)) {
    atom::Copy d(p);
    copy_factory_.get(n).set_copy_index(d.get_copy_index());
  }
  if (core::RigidMember::particle_is_instance(p)) {
    Particle *rb= core::RigidMember(p).get_rigid_body();
    if (rigid_bodies_.find(rb) == rigid_bodies_.end()) {
      int index=rigid_bodies_.size();
      rigid_bodies_[rb]= index;
    }
    unsigned int index= rigid_bodies_.find(rb)->second;
    n.set_value(rigid_body_key_, index);
  }
}
void HierarchySaveLink::do_add_recursive(Particle *root, Particle *p,
                                         RMF::NodeHandle cur) {
  IMP_LOG_VERBOSE( "Adding " << atom::Hierarchy(p) << std::endl);
  contents_[root].access_particles().push_back(p);
  contents_[root].access_nodes().push_back(cur.get_id());
  // make sure not to double add
  if (p != root) set_association(cur, p);
  setup_node(p, cur);
  for (unsigned int i=0;
       i < atom::Hierarchy(p).get_number_of_children();++i) {
    Particle *pc=atom::Hierarchy(p).get_child(i);
    RMF::NodeHandle curc=cur.add_child(get_good_name(pc),
                                       RMF::REPRESENTATION);
    do_add_recursive(root, pc, curc);
  }
}
void HierarchySaveLink::do_add(Particle *p, RMF::NodeHandle cur) {
  IMP_USAGE_CHECK(atom::Hierarchy(p).get_is_valid(true),
                  "Invalid hierarchy passed.");
  do_add_recursive(p,p, cur);
  P::add_link(p, cur);
  copy_bonds(p, cur);
}
void HierarchySaveLink::do_save_node(Particle *p,
                                     RMF::NodeHandle n) {
  if (core::RigidBody::particle_is_instance(p)) {
    if (atom::Hierarchy(p).get_number_of_children()==0) {
      // evil special case for now
      core::RigidBody bd(p);
      RMF::ReferenceFrame p= reference_frame_factory_.get(n);
      algebra::Vector4D q= bd.get_reference_frame().
          get_transformation_to().get_rotation().get_quaternion();
      p.set_rotation(RMF::Floats(q.coordinates_begin(),
                                    q.coordinates_end()));
      algebra::Vector3D t= bd.get_reference_frame().
        get_transformation_to().get_translation();
      p.set_translation(RMF::Floats(t.coordinates_begin(),
                                    t.coordinates_end()));
    } else {
      core::RigidBody bd(p);
      RMF::RigidParticle p= rigid_factory_.get(n);
      algebra::Vector4D q= bd.get_reference_frame().
          get_transformation_to().get_rotation().get_quaternion();
      p.set_orientation(RMF::Floats(q.coordinates_begin(),
                                    q.coordinates_end()));
    }
    if (forces_) {
      core::RigidBody bd(p);
      algebra::Vector3D tv = bd.get_torque();
      algebra::Vector3D fv = -bd.get_derivatives();
      force_factory_.get(n).set_force(RMF::Floats(fv.coordinates_begin(),
                                                  fv.coordinates_end()));
      torque_factory_.get(n).set_torque(RMF::Floats(tv.coordinates_begin(),
                                                    tv.coordinates_end()));
    }
  } else if (core::XYZ::particle_is_instance(p)) {
    core::XYZ d(p);
    RMF::IntermediateParticle ip
        = intermediate_particle_factory_.get(n);
    algebra::Vector3D v= d.get_coordinates();
    ip.set_coordinates(RMF::Floats(v.coordinates_begin(),
                                  v.coordinates_end()));
    if (forces_) {
      core::XYZ bd(p);
      algebra::Vector3D fv = -bd.get_derivatives();
      force_factory_.get(n).set_force(RMF::Floats(fv.coordinates_begin(),
                                                  fv.coordinates_end()));
    }
  }

}
void HierarchySaveLink::do_save_one(Particle *o,
                                    RMF::NodeHandle nh) {
  RMF::FileHandle fh= nh.get_file();
  const Data &d= contents_.find(o)->second;
  for (unsigned int i=0; i< d.get_nodes().size(); ++i) {
    do_save_node(d.get_particles()[i], fh.get_node_from_id(d.get_nodes()[i]));
  }
}

HierarchySaveLink::HierarchySaveLink(RMF::FileHandle fh):
    P("HierarchySaveLink%1%"),
    particle_factory_(fh),
    intermediate_particle_factory_(fh),
    rigid_factory_(fh),
    atom_factory_(fh),
    residue_factory_(fh),
    chain_factory_(fh),
    colored_factory_(fh),
    copy_factory_(fh),
    diffuser_factory_(fh),
    typed_factory_(fh),
    domain_factory_(fh),
    reference_frame_factory_(fh),
    forces_(false),
    force_factory_(fh),
    torque_factory_(fh)
{
  RMF::Category ic= fh.get_category("IMP");
  rigid_body_key_= fh.get_key<RMF::IndexTraits>(ic, "rigid body");
}

IMPRMF_END_NAMESPACE
