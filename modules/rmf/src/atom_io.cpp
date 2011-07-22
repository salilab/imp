/**
 *  \file IMP/rmf/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/rmf/atom_io.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Diffusion.h>
#include <IMP/atom/Copy.h>
#include <IMP/core/Typed.h>
#include <IMP/display/Colored.h>
#include <IMP/rmf/operations.h>
#include <IMP/compatibility/map.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/algebra/geometric_alignment.h>
#include <boost/progress.hpp>
IMPRMF_BEGIN_NAMESPACE

#define  IMP_HDF5_CREATE_MOLECULE_KEYS(node)\
  RootHandle f=node;                                                    \
  FloatKey x= get_or_add_key<FloatTraits>(f, Physics, "cartesian x",    \
                                          true);                        \
  FloatKey y= get_or_add_key<FloatTraits>(f, Physics, "cartesian y",    \
                                          true);                        \
  FloatKey z= get_or_add_key<FloatTraits>(f, Physics, "cartesian z",    \
                                          true);                        \
  FloatKey r= get_or_add_key<FloatTraits>(f, Physics, "radius");        \
  FloatKey m= get_or_add_key<FloatTraits>(f, Physics, "mass");          \
  IndexKey ib= get_or_add_key<IndexTraits>(f, Sequence,                 \
                                           "residue index begin");      \
  IndexKey ie= get_or_add_key<IndexTraits>(f, Sequence,                 \
                                           "residue index end");        \
  IndexKey e= get_or_add_key<IndexTraits>(f, Physics, "element");       \
  IndexKey ci= get_or_add_key<IndexTraits>(f, Sequence, "chain id");    \
  StringKey tk= get_or_add_key<StringTraits>(f, Sequence, "type");      \
  FloatKey cr= get_or_add_key<FloatTraits>(f, Shape, "rgb color red",   \
                                           false);                      \
  FloatKey cg= get_or_add_key<FloatTraits>(f, Shape, "rgb color green", \
                                           false);                      \
  FloatKey cb= get_or_add_key<FloatTraits>(f, Shape, "rgb color blue",  \
                                           false);                      \
  FloatKey dk= get_or_add_key<FloatTraits>(f, Physics, "D in cm2/s");   \
  StringKey rt= get_or_add_key<StringTraits>(f, Sequence, "residue type");\
  IntKey nk= get_or_add_key<IntTraits>(f, Sequence, "copy index");

#define IMP_HDF5_ACCEPT_MOLECULE_KEYS\
  FloatKey x, FloatKey y, FloatKey z, FloatKey r,                       \
    FloatKey m, IndexKey e, IndexKey ci, IndexKey ib, IndexKey ie,      \
    StringKey rt, FloatKey cr, FloatKey cg, FloatKey cb, StringKey tk,\
    FloatKey dk, IntKey nk

#define IMP_HDF5_PASS_MOLECULE_KEYS\
  x, y, z, r, m, e, ci, ib, ie, rt, cr, cg, cb, tk, dk, nk

namespace {
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
  void set_one(NodeHandle n, Key<TypeTag> k,
               T v, unsigned int frame) {
    n.set_value(k, v, frame);
  }

  void copy_data(atom::Hierarchy h, NodeHandle n,
                 unsigned int frame,
                 IMP_HDF5_ACCEPT_MOLECULE_KEYS) {
    if (core::XYZ::particle_is_instance(h)) {
      core::XYZ d(h);
      set_one(n, x, d.get_x(), frame);
      set_one(n, y, d.get_y(), frame);
      set_one(n, z, d.get_z(), frame);
    }
    if (core::XYZR::particle_is_instance(h)) {
      core::XYZR d(h);
      set_one(n, r, d.get_radius(), frame);
    }
    if (atom::Mass::particle_is_instance(h)) {
      atom::Mass d(h);
      set_one(n, m, d.get_mass(), frame);
    }
    if (atom::Atom::particle_is_instance(h)) {
      atom::Atom d(h);
      set_one(n, e, d.get_element(), frame);
    }
    if (atom::Residue::particle_is_instance(h)) {
      atom::Residue d(h);
      set_one(n, ib, d.get_index(), frame);
      set_one(n, ie, d.get_index()+1, frame);
      set_one(n, rt, d.get_residue_type().get_string(),
              frame);
    }
    if (atom::Domain::particle_is_instance(h)) {
      atom::Domain d(h);
      set_one(n, ib, d.get_index_range().first, frame);
      set_one(n, ie, d.get_index_range().second, frame);
    }
    if (display::Colored::particle_is_instance(h)) {
      display::Colored d(h);
      set_one(n, cr, d.get_color().get_red(), frame);
      set_one(n, cg, d.get_color().get_green(), frame);
      set_one(n, cb, d.get_color().get_blue(), frame);
    }
    if (core::Typed::particle_is_instance(h)) {
      core::Typed d(h);
      set_one(n, tk, d.get_type().get_string(), frame);
    }
    if (atom::Chain::particle_is_instance(h)) {
      atom::Chain d(h);
      set_one(n, ci, d.get_id()-'A', frame);
    }
    if (atom::Diffusion::particle_is_instance(h)) {
      atom::Diffusion d(h);
      set_one(n, dk, d.get_d_in_cm2_per_second(), frame);
    }
    if (atom::Copy::particle_is_instance(h)) {
      atom::Copy d(h);
      set_one(n, nk, d.get_copy_index(), frame);
    }
  }



#define GET_DECORATOR(type)                                             \
  type d;                                                               \
  if (type::particle_is_instance(cur)) {                                \
    d=type(cur);                                                        \
  } else {                                                              \
    d= type::setup_particle(cur);                                       \
  }
  void copy_data(NodeHandle ncur, atom::Hierarchy cur, int frame,
                 IMP_HDF5_ACCEPT_MOLECULE_KEYS) {
    int real_frame=std::max(frame, 0);
    if (ncur.get_has_value(x, real_frame)) {
      GET_DECORATOR(core::XYZ);
      d.set_x(ncur.get_value(x, real_frame));
      d.set_y(ncur.get_value(y, real_frame));
      d.set_z(ncur.get_value(z, real_frame));
    }
    if (ncur.get_has_value(r)) {
      GET_DECORATOR(core::XYZR);
      d.set_radius(ncur.get_value(r, real_frame));
    }
    // evil hack
    if (frame >= 0) return;
    if (ncur.get_has_value(e)) {
      if (!atom::get_atom_type_exists(ncur.get_name())) {
        atom::AtomType at=atom::add_atom_type(ncur.get_name(),
                                              atom::Element(ncur.get_value(e)));
      }
      atom::Atom::setup_particle(cur, atom::AtomType(ncur.get_name()));
    }
    if (ncur.get_has_value(m)) {
      if (atom::Mass::particle_is_instance(cur)) {
        atom::Mass(cur).set_mass(ncur.get_value(m));
      } else {
        atom::Mass::setup_particle(cur, ncur.get_value(m));
      }
    }

    if (ncur.get_has_value(rt)) {
      int b= ncur.get_value(ib);
      atom::Residue::setup_particle(cur,
                 atom::ResidueType(ncur.get_value(rt))).set_index(b);
    }
    if (ncur.get_has_value(ib)) {
      int b= ncur.get_value(ib);
      int e= ncur.get_value(ie);
      if (e==b+1) {
      } else {
        atom::Domain::setup_particle(cur, b, e);
      }
    }
    if (ncur.get_has_value(cr)) {
      float r= ncur.get_value(cr);
      float g= ncur.get_value(cg);
      float b= ncur.get_value(cb);
      display::Colored::setup_particle(cur, display::Color(r,g,b));
    }
    if (ncur.get_has_value(ci)) {
      int cci= ncur.get_value(ci);
      atom::Chain::setup_particle(cur, cci+'A');
    }
    if (ncur.get_has_value(tk)) {
      std::string t= ncur.get_value(tk);
      core::ParticleType pt(t);
      core::Typed::setup_particle(cur, pt);
    }
    if (ncur.get_has_value(dk)) {
      double dv= ncur.get_value(dk);
      atom::Diffusion::setup_particle(cur, dv);
    }
    if (ncur.get_has_value(nk)) {
      int dv= ncur.get_value(nk);
      atom::Copy::setup_particle(cur, dv);
    }
  }

#if 0
  atom::Hierarchy get_hierarchy(int id, std::vector<Particle*> &map, Model *m) {
    if (map.size() > static_cast<unsigned int>(id) && map[id]) {
      return atom::Hierarchy(map[id]);
    } else {
      return atom::Hierarchy::setup_particle(new Particle(m));
    }
  }
#endif
}




namespace {
  void save_conformation_internal(RootHandle parent, atom::Hierarchy hierarchy,
                                  unsigned int frame,
                                  boost::progress_display* pd,
                                  IMP_HDF5_ACCEPT_MOLECULE_KEYS) {
    NodeHandle cur;
    try {
      cur= parent.get_node_handle_from_association(hierarchy.get_particle());
    } catch (ValueException) {
      IMP_THROW("Unable to find association for " << hierarchy
                << " which is " << static_cast<void*>(hierarchy),
                ValueException);
    }
    copy_data(hierarchy, cur, frame, IMP_HDF5_PASS_MOLECULE_KEYS);
    //std::cout << "Processing " << hierarchy->get_name() << std::endl;
    unsigned int nc=hierarchy.get_number_of_children();
    if (nc==0 && pd) {
      ++(*pd);
    }
    for (unsigned int i=0; i< nc; ++i) {
      save_conformation_internal(parent, hierarchy.get_child(i), frame, pd,
                                 IMP_HDF5_PASS_MOLECULE_KEYS);
    }
  }
}

void save_frame(RootHandle fh,
               unsigned int frame, atom::Hierarchy hs) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_MOLECULE_KEYS(fh);
  boost::scoped_ptr<boost::progress_display> pd;
  if (get_log_level()< TERSE) {
    pd.reset(new boost::progress_display(atom::get_leaves(hs).size(),
                                         std::cout));
  }
  save_conformation_internal(fh, hs, frame, pd.get(),
                             IMP_HDF5_PASS_MOLECULE_KEYS);
}


namespace {
  void add_hierarchy_internal(NodeHandle parent, atom::Hierarchy hierarchy,
                             boost::progress_display* pd,
                             IMP_HDF5_ACCEPT_MOLECULE_KEYS) {
    NodeHandle cur= parent.add_child(get_name(hierarchy), REPRESENTATION);
    cur.set_association(hierarchy.get_particle());
    copy_data(hierarchy, cur, 0, IMP_HDF5_PASS_MOLECULE_KEYS);
    //std::cout << "Processing " << hierarchy->get_name() << std::endl;
    unsigned int nc=hierarchy.get_number_of_children();
    if (nc==0 && pd) {
      ++(*pd);
    }
    for (unsigned int i=0; i< nc; ++i) {
      add_hierarchy_internal(cur, hierarchy.get_child(i), pd,
                             IMP_HDF5_PASS_MOLECULE_KEYS);
    }
  }


  void create_bond(atom::Bond bd, RootHandle fh) {
    NodeHandle na
      = fh.get_node_handle_from_association(bd.get_bonded(0).get_particle());
    NodeHandle nb
      = fh.get_node_handle_from_association(bd.get_bonded(1).get_particle());
    fh.add_bond(na, nb, 0);
  }


}
void add_hierarchy(RootHandle fh, atom::Hierarchy hs) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_MOLECULE_KEYS(fh);
  boost::scoped_ptr<boost::progress_display> pd;
  if (get_log_level()< TERSE) {
    pd.reset(new boost::progress_display(atom::get_leaves(hs).size(),
                                         std::cout));
  }
  add_hierarchy_internal(fh, hs, pd.get(),
               IMP_HDF5_PASS_MOLECULE_KEYS);
  atom::Bonds bds= atom::get_internal_bonds(hs);
  if (get_log_level()<TERSE) {
    pd.reset(new boost::progress_display(bds.size(),
                                         std::cout));
  }
  for (unsigned int i=0; i< bds.size(); ++i) {
    create_bond(bds[i], fh);
    if (pd) ++(*pd);
  }
}



namespace {
  atom::Hierarchy read_internal(NodeHandle ncur, Model *model,
                                IMP_HDF5_ACCEPT_MOLECULE_KEYS) {
    if (ncur.get_type() != REPRESENTATION) return atom::Hierarchy();
    atom::Hierarchy cur= atom::Hierarchy::setup_particle(new Particle(model));
    cur->set_name(ncur.get_name());
    ncur.set_association(cur);
    copy_data(ncur, cur, -1, IMP_HDF5_PASS_MOLECULE_KEYS);
    NodeHandles children= ncur.get_children();
    for (unsigned int i=0; i < children.size(); ++i) {
      atom::Hierarchy h= read_internal(children[i], model,
                                       IMP_HDF5_PASS_MOLECULE_KEYS);
      if (h) cur.add_child(h);
    }
    return cur;
  }
}

atom::Hierarchies create_hierarchies(RootHandle fh, Model *model) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_MOLECULE_KEYS(fh);
  NodeHandle root= fh;
  atom::Hierarchies ret;
  NodeHandles children= root.get_children();
  for (unsigned int i=0; i< children.size(); ++i) {
    if (children[i].get_type()== REPRESENTATION) {
      atom::Hierarchy c=read_internal(children[i], model,
                                      IMP_HDF5_PASS_MOLECULE_KEYS);
      if (c) ret.push_back(c);
    }
  }

  for (unsigned int i=0; i< fh.get_number_of_bonds(); ++i) {
    std::pair<NodeHandle, NodeHandle> p= fh.get_bond(i);
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
  void load_internal(RootHandle file, atom::Hierarchy h, unsigned int frame,
                     RBM &rigid_bodies,
                     IMP_HDF5_ACCEPT_MOLECULE_KEYS) {
    NodeHandle ncur= file.get_node_handle_from_association(h.get_particle());
    if (ncur != NodeHandle()) {
      copy_data(ncur, h, frame, IMP_HDF5_PASS_MOLECULE_KEYS);
    }
    if (core::RigidMember::particle_is_instance(h)) {
      Particle *rb= core::RigidMember(h).get_rigid_body();
      rigid_bodies[rb].push_back(h);
    }
    atom::Hierarchies children= h.get_children();
    for (unsigned int i=0; i < children.size(); ++i) {
      load_internal(file, children[i], frame, rigid_bodies,
                    IMP_HDF5_PASS_MOLECULE_KEYS);
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

void load_frame(RootHandle fh,
                unsigned int frame,
                atom::Hierarchy hs) {
  IMP_FUNCTION_LOG;
  IMP_HDF5_CREATE_MOLECULE_KEYS(fh);
  RBM rigid_bodies;
  load_internal(fh, hs, frame, rigid_bodies, IMP_HDF5_PASS_MOLECULE_KEYS);
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


unsigned int get_number_of_frames(RootHandle fh,
                                  atom::Hierarchy) {
    FloatKey x= get_or_add_key<FloatTraits>(fh, Physics, "cartesian x",
                                            true);
    return fh.get_number_of_frames(x);
}


SaveHierarchyConfigurationOptimizerState::
SaveHierarchyConfigurationOptimizerState(atom::Hierarchies hs,
                                         RootHandle fh):
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
  void associate_internal(NodeHandle nh, atom::Hierarchy h,
                          bool overwrite) {
    nh.set_association(h, overwrite);
    NodeHandles children= nh.get_children();
    if (h.get_number_of_children() != children.size()) {
      IMP_THROW("Mismatched sizes at " << h, ValueException);
    }
    for (unsigned int i=0; i< children.size(); ++i) {
      associate_internal(children[i], h.get_child(i), overwrite);
    }
  }
}

void set_hierarchies(RootHandle rh, atom::Hierarchies hs,
                     bool overwrite) {
  IMP_FUNCTION_LOG;
  NodeHandle root= rh;
  NodeHandles children= root.get_children();
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
}


IMPRMF_END_NAMESPACE
