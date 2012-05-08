/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
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
#include <IMP/algebra/geometric_alignment.h>

IMPRMF_BEGIN_NAMESPACE

namespace {
atom::Bonded get_bonded(Particle *p) {
  if (atom::Bonded::particle_is_instance(p)) {
    return atom::Bonded(p);
  } else {
    return atom::Bonded::setup_particle(p);
  }
}
  void fix_rigid_body(core::RigidBody rb, core::RigidMembers rms) {
    algebra::Vector3Ds local(rms.size());
    algebra::Vector3Ds global(rms.size());
    for (unsigned int i=0; i< rms.size(); ++i) {
      local[i]= rms[i].get_internal_coordinates();
      global[i]= rms[i].get_coordinates();
    }
    algebra::Transformation3D t3
      = algebra::get_transformation_aligning_first_to_second(local, global);
    rb.set_reference_frame(algebra::ReferenceFrame3D(t3));
    core::RigidBody cur= rb;
    while (core::RigidMember::particle_is_instance(cur)) {
      core::RigidMember rm(cur);
      core::RigidBody parent= rm.get_rigid_body();
      // t0= tp*tl -> tp= t0*tl-1
      algebra::Transformation3D t3
        = cur.get_reference_frame().get_transformation_to()
        *rm.get_internal_transformation().get_inverse();
      cur=parent;
    }
  }

void create_bonds(RMF::FileConstHandle fhc, const RMF::NodeIDs &nhs,
                  const ParticlesTemp &ps) {
  compatibility::map<RMF::NodeConstHandle, Particle*> map;
  for (unsigned int i=0; i< nhs.size(); ++i) {
    map[fhc.get_node_from_id(nhs[i])]= ps[i];
  }
  RMF::NodePairConstHandles hs= fhc.get_node_sets<2>();
  for (unsigned int i=0; i< hs.size(); ++i) {
    if (hs[i].get_type()== RMF::BOND) {
      RMF::NodeConstHandle h0= hs[i].get_node(0);
      RMF::NodeConstHandle h1= hs[i].get_node(1);
      if (map.find(h0) != map.end()
          && map.find(h1) != map.end()) {
        // ignore type and things
        atom::create_bond(get_bonded(map.find(h0)->second),
                          get_bonded(map.find(h1)->second),
                          atom::Bond::SINGLE);
      }
    }
  }
}
}
void HierarchyLoadLink::do_load_one_particle(RMF::NodeConstHandle nh,
                                             Particle *o,
                                             unsigned int frame) {
  if (intermediate_particle_factory_.get_is(nh, frame)) {
    RMF::Floats cs= intermediate_particle_factory_.get(nh, frame)
        .get_coordinates();
    core::XYZR(o).set_coordinates(algebra::Vector3D(cs.begin(),
                                                    cs.end()));
  }
  if (rigid_factory_.get_is(nh, frame)) {
    RMF::RigidParticleConst p=rigid_factory_.get(nh, frame);
    RMF::Floats orient= p.get_orientation();
    algebra::Transformation3D
        tr(algebra::Rotation3D(algebra
                               ::Vector4D(orient.begin(),
                                          orient.end())),
           core::XYZ(o).get_coordinates());
    algebra::ReferenceFrame3D rf(tr);
    core::RigidBody(o).set_reference_frame(rf);
  } else if (core::RigidBody::particle_is_instance(o)) {
    core::RigidBody rb(o);
    core::RigidMembers rbs=rb.get_members();
    if (rbs.size()<3) {
      IMP_WARN("Too few particles to update rigid body: "
               << Showable(o));
    } else {
      fix_rigid_body(rb, rbs);
    }
  }
  if (colored_factory_.get_is(nh, frame)) {
    RMF::Floats c= colored_factory_.get(nh, frame).get_rgb_color();
    display::Colored(o).set_color(display::Color(c[0], c[1], c[2]));
  }
}

void HierarchyLoadLink::do_load_one( RMF::NodeConstHandle nh,
                                     Particle *o,
                                     unsigned int frame) {
  RMF::FileConstHandle fh= nh.get_file();
  const ConstData&d= contents_.find(o)->second;
  IMP_LOG(VERBOSE, "Loading hierarchy " << atom::Hierarchy(o)
          << " with contents " << atom::Hierarchies(d.particles)
          << std::endl);
  for (unsigned int i=0; i< d.nodes.size(); ++i) {
    do_load_one_particle(fh.get_node_from_id(d.nodes[i]),
                         d.particles[i], frame);
  }
}

void HierarchyLoadLink::setup_particle(Particle *root,
                                       RMF::NodeConstHandle nh,
                                       Particle *p) {
  contents_[root].particles.push_back(p);
  contents_[root].nodes.push_back(nh.get_id());
  atom::Hierarchy hp=atom::Hierarchy::setup_particle(p);
  IMP_LOG(VERBOSE, "Particle " << hp << " is ");
  if (intermediate_particle_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "xyzr ");
    double r= intermediate_particle_factory_.get(nh).get_radius();
    core::XYZR::setup_particle(p).set_radius(r);
  }
  if (particle_factory_.get_is(nh)) {
    RMF::ParticleConst m= particle_factory_.get(nh);
    IMP_LOG(VERBOSE, "massive ");
    atom::Mass::setup_particle(p, m.get_mass());
  }
  if (rigid_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "rigid ");
    core::RigidBody::setup_particle(p, algebra::ReferenceFrame3D());
  }
  if (atom_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "atomic ");
    if (!atom::get_atom_type_exists(nh.get_name())) {
      atom::add_atom_type(nh.get_name(),
                          atom::Element(atom_factory_.get(nh)
                                        .get_element()));
    }
    atom::Atom::setup_particle(p, atom::AtomType(nh.get_name()));
  }
  if (residue_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "residue ");
    RMF::ResidueConst residue=residue_factory_.get(nh);
    int b= residue.get_index();
    atom::Residue::setup_particle(p,
                                  atom::ResidueType(residue.get_type()))
        .set_index(b);
  }
  if (domain_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "domian ");
    int b,e;
    boost::tie(b,e)= domain_factory_.get(nh).get_indexes();
    if (e==b+1) {
    } else {
      atom::Domain::setup_particle(p, b, e);
    }
  }
  if (colored_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "colored ");
    RMF::Floats c= colored_factory_.get(nh).get_rgb_color();
    display::Colored::setup_particle(p, display::Color(c[0], c[1], c[2]));
  }
  if (chain_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "chain ");
    int cci= chain_factory_.get(nh).get_chain_id();
    atom::Chain::setup_particle(p, cci+'A');
  }
  if (typed_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "typed ");
    std::string t= typed_factory_.get(nh).get_type_name();
    core::ParticleType pt(t);
    core::Typed::setup_particle(p, pt);
  }
  if (diffuser_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "diffuser ");
    double dv= diffuser_factory_.get(nh)
        .get_diffusion_coefficient();
    atom::Diffusion::setup_particle(p, dv);
  }
  if (copy_factory_.get_is(nh)) {
    IMP_LOG(VERBOSE, "copy ");
    int dv= copy_factory_.get(nh).get_copy_index();
    atom::Copy::setup_particle(p, dv);
  }
  IMP_LOG(VERBOSE, std::endl);
}

Particle* HierarchyLoadLink::do_create_recursive(Particle *root,
                                                 RMF::NodeConstHandle name) {
  IMP_NEW(Particle, p, (m_, name.get_name()));
  if (!root) {
    root=p;
  } else {
    set_association(name, p);
  }
  setup_particle(root, name, p);
  RMF::NodeConstHandles ch= name.get_children();
  for (unsigned int i=0; i< ch.size(); ++i) {
    if (ch[i].get_type()== RMF::REPRESENTATION) {
      Particle*c= do_create_recursive(root, ch[i]);
      atom::Hierarchy(p).add_child(atom::Hierarchy(c));
    }
  }
  return p.release();
}

Particle* HierarchyLoadLink::do_create(RMF::NodeConstHandle name) {
  Particle *ret= do_create_recursive(nullptr, name);
  create_bonds(name.get_file(),contents_[ret].nodes,
               contents_[ret].particles);
  return ret;
}

void HierarchyLoadLink::do_add_link_recursive(Particle *root,
                                              Particle *o,
                                              RMF::NodeConstHandle node) {
  IMP_LOG(VERBOSE, "Linking " << Showable(o) << " and " << node << std::endl);
  contents_[root].particles.push_back(o);
  contents_[root].nodes.push_back(node.get_id());
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
    domain_factory_(fh)
{
}

namespace {
void copy_bonds(Particle *root,
                RMF::FileHandle fhc) {
  IMP_FUNCTION_LOG;
  atom::Bonds bds= atom::get_internal_bonds(atom::Hierarchy(root));
  for (unsigned int i=0; i< bds.size(); ++i) {
    Particle *p0= bds[i].get_bonded(0);
    Particle *p1= bds[i].get_bonded(1);
    IMP_LOG(VERBOSE, "Adding bond for pair " << Showable(p0)
            << " and " << Showable(p1) << std::endl);
    RMF::NodeHandle n0= get_node_from_association(fhc, p0);
    RMF::NodeHandle n1= get_node_from_association(fhc, p1);
    RMF::NodeHandles nphs(2);
    nphs[0]=n0;
    nphs[1]=n1;
    RMF::NodePairHandle np=fhc.add_node_set<2>(nphs, RMF::BOND);
  }
}
}

void HierarchySaveLink::setup_node(Particle *p, RMF::NodeHandle n) {
  if (core::XYZR::particle_is_instance(p)) {
    core::XYZR d(p);
    intermediate_particle_factory_.get(n).set_radius(d.get_radius());
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
    diffuser_factory_.get(n).set_diffusion_coefficient(d.get_d());
  }
  if (atom::Copy::particle_is_instance(p)) {
    atom::Copy d(p);
    copy_factory_.get(n).set_copy_index(d.get_copy_index());
  }
}
void HierarchySaveLink::do_add_recursive(Particle *root, Particle *p,
                                         RMF::NodeHandle cur) {
  IMP_LOG(VERBOSE, "Adding " << atom::Hierarchy(p) << std::endl);
  contents_[root].particles.push_back(p);
  contents_[root].nodes.push_back(cur.get_id());
  // make sure not to double add
  if (p != root) set_association(cur, p);
  setup_node(p, cur);
  for (unsigned int i=0;
       i < atom::Hierarchy(p).get_number_of_children();++i) {
    Particle *pc=atom::Hierarchy(p).get_child(i);
    RMF::NodeHandle curc=cur.add_child(pc->get_name(), RMF::REPRESENTATION);
    do_add_recursive(root, pc, curc);
  }
}
void HierarchySaveLink::do_add(Particle *p, RMF::NodeHandle cur) {
  IMP_USAGE_CHECK(atom::Hierarchy(p).get_is_valid(true),
                  "Invalid hierarchy passed.");
  do_add_recursive(p,p, cur);
  copy_bonds(p, cur.get_file());
}
void HierarchySaveLink::do_save_node(Particle *p,
                                     RMF::NodeHandle n,
                                     unsigned int frame) {
  if (core::XYZ::particle_is_instance(p)) {
    core::XYZ d(p);
    RMF::IntermediateParticle p
        = intermediate_particle_factory_.get(n, frame);
    algebra::Vector3D v= d.get_coordinates();
    p.set_coordinates(RMF::Floats(v.coordinates_begin(),
                                  v.coordinates_end()));
  }
  if (core::RigidBody::particle_is_instance(p)) {
    core::RigidBody bd(p);
    RMF::RigidParticle p= rigid_factory_.get(n, frame);
    algebra::Vector4D q= bd.get_reference_frame().
        get_transformation_to().get_rotation().get_quaternion();
    p.set_orientation(RMF::Floats(q.coordinates_begin(),
                                  q.coordinates_end()));
  }
}
void HierarchySaveLink::do_save_one(Particle *o,
                                    RMF::NodeHandle nh,
                                    unsigned int frame) {
  RMF::FileHandle fh= nh.get_file();
  const Data &d= contents_.find(o)->second;
  for (unsigned int i=0; i< d.nodes.size(); ++i) {
    do_save_node(d.particles[i], fh.get_node_from_id(d.nodes[i]), frame);
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
    domain_factory_(fh) {
}

IMPRMF_END_NAMESPACE
