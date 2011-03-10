/**
 *  \file IMP/hdf5/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/hdf5/atom_io.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Domain.h>
#include <IMP/hdf5/operations.h>
#include <boost/progress.hpp>
IMPHDF5_BEGIN_NAMESPACE

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
  StringKey rt= get_or_add_key<StringTraits>(f, Sequence, "residue type");

#define IMP_HDF5_ACCEPT_MOLECULE_KEYS\
  FloatKey x, FloatKey y, FloatKey z, FloatKey r,                       \
    FloatKey m, IndexKey e, IndexKey ib, IndexKey ie,                   \
    StringKey rt

#define IMP_HDF5_PASS_MOLECULE_KEYS\
  x, y, z, r, m, e, ib, ie, rt

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
  }



#define GET_DECORATOR(type)                                             \
  type d;                                                               \
  if (type::particle_is_instance(cur)) {                                \
    d=type(cur);                                                        \
  } else {                                                              \
    d= type::setup_particle(cur);                                       \
  }
  void copy_data(NodeHandle ncur, atom::Hierarchy cur, unsigned int frame,
                 IMP_HDF5_ACCEPT_MOLECULE_KEYS) {
    if (ncur.get_has_value(x, frame)) {
      GET_DECORATOR(core::XYZ);
      d.set_x(ncur.get_value(x, frame));
      d.set_y(ncur.get_value(y, frame));
      d.set_z(ncur.get_value(z, frame));
    }
    if (ncur.get_has_value(r)) {
      GET_DECORATOR(core::XYZR);
      d.set_radius(ncur.get_value(r, frame));
    }
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
  }


  atom::Hierarchy get_hierarchy(int id, std::vector<Particle*> &map, Model *m) {
    if (map.size() > static_cast<unsigned int>(id) && map[id]) {
      return atom::Hierarchy(map[id]);
    } else {
      return atom::Hierarchy::setup_particle(new Particle(m));
    }
  }
}




namespace {
  void save_conformation_internal(RootHandle parent, atom::Hierarchy hierarchy,
                                  unsigned int frame,
                                  boost::progress_display* pd,
                                  IMP_HDF5_ACCEPT_MOLECULE_KEYS) {
    NodeHandle cur
      = parent.get_node_handle_from_association(hierarchy.get_particle());
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

void save_conformation(atom::Hierarchy hs, RootHandle fh,
                       unsigned int frame) {
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
void write_hierarchy(atom::Hierarchy hs, RootHandle fh) {
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
    copy_data(ncur, cur, 0, IMP_HDF5_PASS_MOLECULE_KEYS);
    NodeHandles children= ncur.get_children();
    for (unsigned int i=0; i < children.size(); ++i) {
      atom::Hierarchy h= read_internal(children[i], model,
                                       IMP_HDF5_PASS_MOLECULE_KEYS);
      if (h) cur.add_child(h);
    }
    return cur;
  }
}

atom::Hierarchies read_all_hierarchies(RootHandle fh, Model *model) {
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
  void load_internal(RootHandle file, atom::Hierarchy h, unsigned int frame,
                     IMP_HDF5_ACCEPT_MOLECULE_KEYS) {
    NodeHandle ncur= file.get_node_handle_from_association(h.get_particle());
    copy_data(ncur, h, frame, IMP_HDF5_PASS_MOLECULE_KEYS);
    atom::HierarchiesTemp children= h.get_children();
    for (unsigned int i=0; i < children.size(); ++i) {
      load_internal(file, children[i], frame,
                    IMP_HDF5_PASS_MOLECULE_KEYS);
    }
  }
}

void load_conformation(RootHandle fh,
                       atom::Hierarchy hs,
                       unsigned int frame) {
  IMP_HDF5_CREATE_MOLECULE_KEYS(fh);
  load_internal(fh, hs, frame, IMP_HDF5_PASS_MOLECULE_KEYS);
}


IMPHDF5_END_NAMESPACE
