/**
 *  \file IMP/rmf/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/internal/imp_operations.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Diffusion.h>
#include <IMP/atom/Copy.h>
#include <RMF/decorators.h>
#include <IMP/log.h>
#include <IMP/core/Typed.h>
#include <IMP/display/Colored.h>
#include <IMP/compatibility/map.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/geometric_alignment.h>
#include <boost/progress.hpp>
IMPRMF_BEGIN_NAMESPACE

using namespace RMF;

#define IMP_HDF5_CREATE_MOLECULE_FACTORIES(f, CNST)             \
  RMF::Particle##CNST##Factory particle_factory(f);             \
  RMF::IntermediateParticle##CNST##Factory iparticle_factory(f);        \
  RMF::RigidParticle##CNST##Factory rigid_particle_factory(f);  \
  RMF::Atom##CNST##Factory atom_factory(f);                     \
  RMF::Residue##CNST##Factory residue_factory(f);               \
  RMF::Chain##CNST##Factory chain_factory(f);                   \
  RMF::Colored##CNST##Factory colored_factory(f);               \
  RMF::Copy##CNST##Factory copy_factory(f);                     \
  RMF::Diffuser##CNST##Factory diffuser_factory(f);             \
  RMF::Typed##CNST##Factory typed_factory(f);                   \
  RMF::Domain##CNST##Factory domain_factory(f)





#define IMP_HDF5_ACCEPT_MOLECULE_FACTORIES(CNST)                 \
  RMF::Particle##CNST##Factory particle_factory,                 \
    RMF::IntermediateParticle##CNST##Factory iparticle_factory,  \
    RMF::RigidParticle##CNST##Factory rigid_particle_factory,    \
    RMF::Atom##CNST##Factory atom_factory,                       \
    RMF::Residue##CNST##Factory residue_factory,                 \
    RMF::Chain##CNST##Factory chain_factory,                     \
    RMF::Colored##CNST##Factory colored_factory,                 \
    RMF::Copy##CNST##Factory copy_factory,                       \
    RMF::Diffuser##CNST##Factory diffuser_factory,               \
    RMF::Typed##CNST##Factory typed_factory,                     \
    RMF::Domain##CNST##Factory domain_factory



#define IMP_HDF5_PASS_MOLECULE_FACTORIES                        \
  particle_factory,                                             \
    iparticle_factory,                                          \
    rigid_particle_factory,                                     \
    atom_factory,                                               \
    residue_factory,                                            \
    chain_factory,                                              \
    colored_factory,                                            \
    copy_factory,                                               \
    diffuser_factory,                                           \
    typed_factory,                                              \
    domain_factory

namespace {
  RMF::Floats get_rmf_floats(double v0, double v1) {
    RMF::Floats ret(2); ret[0]=v0; ret[1]=v1;
    return ret;
  }
  RMF::Floats get_rmf_floats(double v0, double v1, double v2) {
    RMF::Floats ret(3); ret[0]=v0; ret[1]=v1; ret[2]=v2;
    return ret;
  }
  std::string get_name(atom::Hierarchy h) {
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

  template <class TypeTag, class T>
  void set_one(RMF::NodeHandle n, RMF::Key<TypeTag, 1> k,
               T v, unsigned int frame) {
    n.set_value(k, v, frame);
  }

  void copy_data(atom::Hierarchy h, RMF::NodeHandle n,
                 unsigned int frame,
                 IMP_HDF5_ACCEPT_MOLECULE_FACTORIES()) {
    if (core::XYZ::particle_is_instance(h)) {
      core::XYZ d(h);
      RMF::IntermediateParticle p= iparticle_factory.get(n, frame);
      algebra::Vector3D v= d.get_coordinates();
      p.set_coordinates(RMF::Floats(v.coordinates_begin(),
                                    v.coordinates_end()));
    }
    if (core::RigidBody::particle_is_instance(h)) {
      core::RigidBody bd(h);
      RMF::RigidParticle p= rigid_particle_factory.get(n, frame);
      algebra::Vector4D q= bd.get_reference_frame().
        get_transformation_to().get_rotation().get_quaternion();
      p.set_orientation(RMF::Floats(q.coordinates_begin(),
                                    q.coordinates_end()));
    }
    if (core::XYZR::particle_is_instance(h)) {
      core::XYZR d(h);
      particle_factory.get(n, frame).set_radius(d.get_radius());
    }
    if (atom::Mass::particle_is_instance(h)) {
      atom::Mass d(h);
      particle_factory.get(n, frame).set_mass(d.get_mass());
    }
    if (atom::Atom::particle_is_instance(h)) {
      atom::Atom d(h);
      atom_factory.get(n, frame).set_element(d.get_element());
    }
    if (atom::Residue::particle_is_instance(h)) {
      atom::Residue d(h);
      RMF::Residue r= residue_factory.get(n, frame);
      r.set_index(d.get_index());
      r.set_type(d.get_residue_type().get_string());
    }
    if (atom::Domain::particle_is_instance(h)) {
      atom::Domain d(h);
      domain_factory.get(n, frame).set_indexes( d.get_index_range().first,
                                                  d.get_index_range().second);
    }
    if (display::Colored::particle_is_instance(h)) {
      display::Colored d(h);
      RMF::Floats color(3);
      colored_factory.get(n, frame)
        .set_rgb_color(get_rmf_floats(d.get_color().get_red(),
                                      d.get_color().get_green(),
                                      d.get_color().get_blue()));
    }
    if (core::Typed::particle_is_instance(h)) {
      core::Typed d(h);
      typed_factory.get(n, frame).set_type_name(d.get_type().get_string());
    }
    if (atom::Chain::particle_is_instance(h)) {
      atom::Chain d(h);
      chain_factory.get(n, frame).set_chain_id(d.get_id()-'A');
    }
    if (atom::Diffusion::particle_is_instance(h)) {
      atom::Diffusion d(h);
      diffuser_factory.get(n, frame).set_diffusion_coefficient(d.get_d());
    }
    if (atom::Copy::particle_is_instance(h)) {
      atom::Copy d(h);
      copy_factory.get(n, frame).set_copy_index(d.get_copy_index());
    }
  }



#define GET_DECORATOR(type)                     \
  type d;                                       \
  if (type::particle_is_instance(cur)) {        \
    d=type(cur);                                \
  } else {                                      \
    d= type::setup_particle(cur);               \
  }

#define GET_DECORATOR_VALUE(type, value)        \
  type d;                                       \
  if (type::particle_is_instance(cur)) {        \
    d=type(cur);                                \
  } else {                                      \
    d= type::setup_particle(cur, value);        \
  }
  void copy_data(RMF::NodeConstHandle ncur, atom::Hierarchy cur, int frame,
                 IMP_HDF5_ACCEPT_MOLECULE_FACTORIES(Const)) {
    int real_frame=std::max(frame, 0);
    if (iparticle_factory.get_is(ncur, real_frame)) {
      GET_DECORATOR(core::XYZR);
      RMF::IntermediateParticleConst p=iparticle_factory.get(ncur, real_frame);
      d.set_x(p.get_coordinates()[0]);
      d.set_y(p.get_coordinates()[1]);
      d.set_z(p.get_coordinates()[2]);
      d.set_radius(p.get_radius());
    }
    if (particle_factory.get_is(ncur, real_frame)) {
      RMF::ParticleConst p=particle_factory.get(ncur, real_frame);
        GET_DECORATOR_VALUE(atom::Mass, p.get_mass());
        d.set_mass(p.get_mass());
    }
    if (rigid_particle_factory.get_is(ncur, real_frame)) {
      RMF::RigidParticleConst p=rigid_particle_factory.get(ncur, real_frame);
      RMF::Floats orient= p.get_orientation();
      algebra::Transformation3D
        tr(algebra::Rotation3D(algebra
                               ::Vector4D(orient.begin(),
                                          orient.end())),
           core::XYZ(cur).get_coordinates());
      algebra::ReferenceFrame3D rf(tr);
      GET_DECORATOR_VALUE(core::RigidBody, rf);
      d.set_reference_frame(rf);
    }
    // evil hack
    if (frame >= 0) return;
    if (atom_factory.get_is(ncur, real_frame)) {
      if (!atom::get_atom_type_exists(ncur.get_name())) {
        atom::add_atom_type(ncur.get_name(),
                            atom::Element(atom_factory.get(ncur,
                                                           real_frame)
                                          .get_element()));
      }
      atom::Atom::setup_particle(cur, atom::AtomType(ncur.get_name()));
    }

    if (residue_factory.get_is(ncur, real_frame)) {
      RMF::ResidueConst residue=residue_factory.get(ncur, real_frame);
      int b= residue.get_index();
      atom::Residue::setup_particle(cur,
                                    atom::ResidueType(residue.get_type()))
        .set_index(b);
    }
    if (domain_factory.get_is(ncur, real_frame)) {
      int b,e;
      boost::tie(b,e)= domain_factory.get(ncur, real_frame).get_indexes();
      if (e==b+1) {
      } else {
        atom::Domain::setup_particle(cur, b, e);
      }
    }
    if (colored_factory.get_is(ncur, real_frame)) {
      RMF::Floats c= colored_factory.get(ncur, real_frame).get_rgb_color();
      display::Colored::setup_particle(cur, display::Color(c[0], c[1], c[2]));
    }
    if (chain_factory.get_is(ncur, real_frame)) {
      int cci= chain_factory.get(ncur, real_frame).get_chain_id();
      atom::Chain::setup_particle(cur, cci+'A');
    }
    if (typed_factory.get_is(ncur, real_frame)) {
      std::string t= typed_factory.get(ncur, real_frame).get_type_name();
      core::ParticleType pt(t);
      core::Typed::setup_particle(cur, pt);
    }
    if (diffuser_factory.get_is(ncur, real_frame)) {
      double dv= diffuser_factory.get(ncur, real_frame)
        .get_diffusion_coefficient();
      atom::Diffusion::setup_particle(cur, dv);
    }
    if (copy_factory.get_is(ncur, real_frame)) {
      int dv= copy_factory.get(ncur, real_frame).get_copy_index();
      atom::Copy::setup_particle(cur, dv);
    }
  }
}




namespace {
  void save_conformation_internal(RMF::FileHandle parent,
                                  atom::Hierarchy hierarchy,
                                  unsigned int frame,
                                  boost::progress_display* pd,
                                  IMP_HDF5_ACCEPT_MOLECULE_FACTORIES()) {
    RMF::NodeHandle cur;
    try {
      cur= parent.get_node_from_association(hierarchy.get_particle());
    } catch (ValueException) {
      IMP_THROW("Unable to find association for " << hierarchy
                << " which is " << static_cast<void*>(hierarchy),
                ValueException);
    }
    copy_data(hierarchy, cur, frame, IMP_HDF5_PASS_MOLECULE_FACTORIES);
    //std::cout << "Processing " << hierarchy->get_name() << std::endl;
    unsigned int nc=hierarchy.get_number_of_children();
    if (nc==0 && pd) {
      ++(*pd);
    }
    for (unsigned int i=0; i< nc; ++i) {
      save_conformation_internal(parent, hierarchy.get_child(i), frame, pd,
                                 IMP_HDF5_PASS_MOLECULE_FACTORIES);
    }
  }
}

void save_frame(RMF::FileHandle fh,
                unsigned int frame, atom::Hierarchy hs) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_MOLECULE_FACTORIES(fh,);
  boost::scoped_ptr<boost::progress_display> pd;
  if (get_log_level()== PROGRESS) {
    pd.reset(new boost::progress_display(atom::get_leaves(hs).size(),
                                         std::cout));
  }
  save_conformation_internal(fh, hs, frame, pd.get(),
                             IMP_HDF5_PASS_MOLECULE_FACTORIES);
  fh.flush();
}


namespace {
  void add_hierarchy_internal(RMF::NodeHandle parent, atom::Hierarchy hierarchy,
                              boost::progress_display* pd,
                              IMP_HDF5_ACCEPT_MOLECULE_FACTORIES()) {
    RMF::NodeHandle cur= parent.add_child(get_name(hierarchy), REPRESENTATION);
    cur.set_association(hierarchy.get_particle());
    IMP_USAGE_CHECK(cur.get_file().get_node_from_association(hierarchy)==cur,
                    "In and out don't match: " << cur
                    << " vs "
                    << cur.get_file().get_node_from_association(hierarchy));
    copy_data(hierarchy, cur, 0, IMP_HDF5_PASS_MOLECULE_FACTORIES);
    //std::cout << "Processing " << hierarchy->get_name() << std::endl;
    unsigned int nc=hierarchy.get_number_of_children();
    if (nc==0 && pd) {
      ++(*pd);
    }
    for (unsigned int i=0; i< nc; ++i) {
      add_hierarchy_internal(cur, hierarchy.get_child(i), pd,
                             IMP_HDF5_PASS_MOLECULE_FACTORIES);
    }
  }


void create_bond(atom::Bond bd, RMF::FileHandle fh, RMF::PairIndexKey bt) {
    RMF::NodeHandle na
      = fh.get_node_from_association(bd.get_bonded(0).get_particle());
    RMF::NodeHandle nb
      = fh.get_node_from_association(bd.get_bonded(1).get_particle());
    RMF::NodeHandles nhs(2); nhs[0]=na; nhs[1]=nb;
    RMF::NodePairHandle obd= fh.add_node_set<2>(nhs, RMF::BOND);
    obd.set_value(bt, 0);
  }

}
void add_hierarchy(RMF::FileHandle fh, atom::Hierarchy hs) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_MOLECULE_FACTORIES(fh,);
  boost::scoped_ptr<boost::progress_display> pd;
  if (get_log_level()== PROGRESS) {
    pd.reset(new boost::progress_display(atom::get_leaves(hs).size(),
                                         std::cout));
  }
  add_hierarchy_internal(fh.get_root_node(), hs, pd.get(),
                         IMP_HDF5_PASS_MOLECULE_FACTORIES);
  atom::Bonds bds= atom::get_internal_bonds(hs);
  if (get_log_level()==PROGRESS) {
    pd.reset(new boost::progress_display(bds.size(),
                                         std::cout));
  }
  RMF::PairIndexKey bk;
  RMF::CategoryD<2> bond= internal::get_or_add_category<2>(fh,"bond");
  if (fh.get_has_key<RMF::IndexTraits, 2>(bond, "type")) {
    bk= fh.get_key<RMF::IndexTraits, 2>(bond, "type");
  } else {
    bk= fh.add_key<RMF::IndexTraits, 2>(bond, "type", false);
  }
  for (unsigned int i=0; i< bds.size(); ++i) {
    create_bond(bds[i], fh, bk);
    if (pd) ++(*pd);
  }
  fh.flush();
}



namespace {
  atom::Hierarchy read_internal(RMF::NodeConstHandle ncur, Model *model,
                                IMP_HDF5_ACCEPT_MOLECULE_FACTORIES(Const)) {
    if (ncur.get_type() != REPRESENTATION) return atom::Hierarchy();
    atom::Hierarchy cur= atom::Hierarchy::setup_particle(new Particle(model));
    cur->set_name(ncur.get_name());
    ncur.set_association(cur);
    copy_data(ncur, cur, -1, IMP_HDF5_PASS_MOLECULE_FACTORIES);
    NodeConstHandles children= ncur.get_children();
    for (unsigned int i=0; i < children.size(); ++i) {
      atom::Hierarchy h= read_internal(children[i], model,
                                       IMP_HDF5_PASS_MOLECULE_FACTORIES);
      if (h) cur.add_child(h);
    }
    return cur;
  }
}

atom::Hierarchies create_hierarchies(RMF::FileConstHandle fh, Model *model) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_MOLECULE_FACTORIES(fh,Const);
  RMF::NodeConstHandle root= fh.get_root_node();
  atom::Hierarchies ret;
  NodeConstHandles children= root.get_children();
  for (unsigned int i=0; i< children.size(); ++i) {
    if (children[i].get_type()== REPRESENTATION) {
      atom::Hierarchy c=read_internal(children[i], model,
                                      IMP_HDF5_PASS_MOLECULE_FACTORIES);
      if (c) ret.push_back(c);
    }
  }

  for (unsigned int i=0; i< fh.get_number_of_bonds(); ++i) {
    std::pair<RMF::NodeConstHandle, RMF::NodeConstHandle> p= fh.get_bond(i);
    void *aa= p.first.get_association();
    void *ab= p.second.get_association();
    if (aa && ab) {
      Particle *pa= static_cast<Particle*>(aa);
      Particle *pb= static_cast<Particle*>(ab);
      if (!atom::Bonded::particle_is_instance(pa)) {
        atom::Bonded::setup_particle(pa);
      }
      if (!atom::Bonded::particle_is_instance(pb)) {
        atom::Bonded::setup_particle(pb);
      }
      atom::create_bond(atom::Bonded(pa),
                        atom::Bonded(pb), 0);
    }
  }
  return ret;
}


namespace {
  typedef IMP::compatibility::map<Particle*, ParticlesTemp> RBM;
  void load_internal(RMF::FileConstHandle file, atom::Hierarchy h,
                     unsigned int frame, RBM &rigid_bodies,
                     IMP_HDF5_ACCEPT_MOLECULE_FACTORIES(Const)) {
    RMF::NodeConstHandle ncur
                  = file.get_node_from_association(h.get_particle());
    if (ncur != RMF::NodeConstHandle()) {
      copy_data(ncur, h, frame, IMP_HDF5_PASS_MOLECULE_FACTORIES);
    }
    if (core::RigidMember::particle_is_instance(h)) {
      Particle *rb= core::RigidMember(h).get_rigid_body();
      rigid_bodies[rb].push_back(h);
    }
    atom::Hierarchies children= h.get_children();
    for (unsigned int i=0; i < children.size(); ++i) {
      load_internal(file, children[i], frame, rigid_bodies,
                    IMP_HDF5_PASS_MOLECULE_FACTORIES);
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
}

void load_frame(RMF::FileConstHandle fh,
                unsigned int frame,
                atom::Hierarchy hs) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_MOLECULE_FACTORIES(fh,Const);
  RBM rigid_bodies;
  load_internal(fh, hs, frame, rigid_bodies,
                IMP_HDF5_PASS_MOLECULE_FACTORIES);
  for (RBM::const_iterator it= rigid_bodies.begin();
       it != rigid_bodies.end(); ++it) {
    if (it->second.size()<3) {
      IMP_WARN("Too few particles to update rigid body: "
               << it->first->get_name());
    } else {
      fix_rigid_body(core::RigidBody(it->first),
                     core::RigidMembers(it->second));
    }
  }
}


unsigned int get_number_of_frames(RMF::FileConstHandle fh,
                                  atom::Hierarchy) {
  RMF::Category physics= fh.get_category<1>("physics");
  RMF::FloatKey x
    = internal::get_or_add_key<FloatTraits>(fh, physics, "cartesian x",
                                            true);
  return fh.get_number_of_frames(x);
}


SaveHierarchyConfigurationOptimizerState::
SaveHierarchyConfigurationOptimizerState(atom::Hierarchies hs,
                                         RMF::FileHandle fh):
  OptimizerState("SaveHierarchyConfigurationOptimizerState %1%"),
  hs_(hs),
  fh_(fh){}


void SaveHierarchyConfigurationOptimizerState::do_update(unsigned int k) {
  for (unsigned int i=0;i< hs_.size(); ++i) {
    save_frame(fh_, k, hs_[i]);
  }
}

void SaveHierarchyConfigurationOptimizerState
::do_show(std::ostream &out) const {
  out << "  file: " << fh_ << std::endl;
  for (unsigned int i=0; i< hs_.size(); ++i) {
    out << "  hierarchy: " << hs_[i]->get_name() << std::endl;
  }
}

namespace {
  void associate_internal(RMF::NodeConstHandle nh, atom::Hierarchy h,
                          bool overwrite) {
    nh.set_association(h, overwrite);
    NodeConstHandles children= nh.get_children();
    if (h.get_number_of_children() != children.size()) {
      IMP_THROW("Mismatched sizes at " << h, ValueException);
    }
    for (unsigned int i=0; i< children.size(); ++i) {
      associate_internal(children[i], h.get_child(i), overwrite);
    }
  }
}

void set_hierarchies(RMF::FileConstHandle rh, atom::Hierarchies hs,
                     bool overwrite) {
  IMP_FUNCTION_LOG;
  RMF::NodeConstHandle root= rh.get_root_node();
  NodeConstHandles children= root.get_children();
  unsigned int hsi=0;
  for (unsigned int i=0; i< children.size(); ++i) {
    if (children[i].get_type()== REPRESENTATION) {
      if (hs.size() <= hsi) {
        IMP_THROW("Too few hierarchies for file", ValueException);
      }
      associate_internal(children[i], hs[hsi], overwrite);
      ++hsi;
    }
  }
  if (hsi != hs.size()) {
    IMP_THROW("Mismatched sizes", ValueException);
  }
  rh.flush();
}


IMPRMF_END_NAMESPACE
