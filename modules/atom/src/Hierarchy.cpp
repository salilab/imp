/**
 *  \file Hierarchy.cpp   \brief Decorator for helping deal
 *                                                 with a hierarchy.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Fragment.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/core/XYZR.h>
#include <IMP/atom/estimates.h>
#include <IMP/atom/Molecule.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/algebra/geometric_alignment.h>
#include <IMP/core/rigid_bodies.h>

#include <IMP/base/set.h>

#include <boost/random/uniform_int.hpp>

#include <sstream>


IMPATOM_BEGIN_NAMESPACE

const IMP::core::HierarchyTraits&
Hierarchy::get_traits() {
  static IMP::core::HierarchyTraits ret("molecular_hierarchy");
  return ret;
}

void Hierarchy::show(std::ostream &out) const
{
  if (*this == Hierarchy()) {
    out << "nullptr Molecular Hierarchy node";
    return;
  }
  bool found=false;
  if (get_as_atom()) {
    found=true;
    out << get_as_atom() << " ";
  }
  if (get_as_residue()){
    found=true;
    out << get_as_residue() << " ";
  }
  if (get_as_chain()) {
    found=true;
    out << get_as_chain() << " ";
  } else if (get_as_molecule()) {
    found=true;
    out << get_as_molecule() << " ";
  }
  if (get_as_fragment()) {
    found=true;
    out << get_as_fragment() << " ";
  }
  if (get_as_domain()) {
    found=true;
    out << get_as_domain() << " ";
  }
  if (core::RigidBody::particle_is_instance(get_particle())) {
    found=true;
    out << core::RigidBody(get_particle());
  }
  if (core::RigidMember::particle_is_instance(get_particle())) {
      found=true;
      out << " rigid member: " << core::RigidMember(get_particle())
        .get_rigid_body()->get_name();
    }
  if (get_as_xyzr()) {
    found=true;
    out << " sphere: " << get_as_xyzr().get_sphere();
  } else if (get_as_xyz()) {
    found=true;
    out << " coordinates: " << get_as_xyz().get_coordinates();
  }
  if (!found) {
    out << "Hierarchy \"" <<  get_particle()->get_name()
        << "\"";
  }
}



namespace
{
#define IMP_IMPL_MATCH_TYPE(UCName, lcname, CAPSNAME)   \
  case CAPSNAME:                                        \
  return h.get_as_##lcname();

struct MHDMatchingType
{
  MHDMatchingType(GetByType t): t_(t){}

  bool operator()(Particle *p) const {
    Hierarchy h= Hierarchy::decorate_particle(p);
    switch(t_) {
      IMP_FOREACH_HIERARCHY_TYPE_STATEMENTS(IMP_IMPL_MATCH_TYPE);
    }
    IMP_FAILURE("Unhandled type in get_by_type.");
    return false;
  }

  GetByType t_;
};

} // namespace

Hierarchies get_by_type(Hierarchy mhd,
                        GetByType t)
{
  Hierarchies out;
  gather(mhd, MHDMatchingType(t),
         std::back_inserter(out));
  return out;
}


namespace
{

struct MatchResidueIndex
{
  int index_;
  MatchResidueIndex(int i): index_(i) {}
  bool operator()(Particle *p) const {
    Hierarchy mhd(p);
    if (mhd.get_as_residue()) {
      return (mhd.get_as_residue().get_index() == index_);
    } else {
      if (mhd.get_number_of_children()==0) {
        if (mhd.get_as_domain()) {
          Domain dd= mhd.get_as_domain();
          return  dd.get_begin_index() <= index_
            && dd.get_end_index()> index_;
        } else if (mhd.get_as_fragment()) {
          Fragment fd= mhd.get_as_fragment();
          return fd.get_contains_residue(index_);
        } else {
          return false;
        }
      } else {
        return false;
      }
    }
  }
};


} // namespace


Hierarchy
get_residue(Hierarchy mhd,
            unsigned int index)
{
  MatchResidueIndex mi(index);
  Hierarchy hd= core::find_breadth_first(mhd, mi);
  if (hd== IMP::core::Hierarchy()) {
    return Hierarchy();
  } else {
    return hd;
  }
}

#define TEST_FAIL(msg)                          \
  IMP_ERROR(msg);                               \
  IMP_THROW(msg, BadHierarchy)

namespace {
  struct BadHierarchy:public Exception
  {BadHierarchy(const char *str):Exception(str){}};
  struct Validator {
    typedef bool result_type;
    bool print_info;
    Validator(bool pi): print_info(pi){}
    bool operator()(Hierarchy h, bool) {
      if (h.get_number_of_children() ==0) {
        if (!h.get_as_xyzr()) {
          TEST_FAIL("Leaf " << h << " does not have coordinates and radius");
        }
        if (!h.get_as_mass()) {
          TEST_FAIL("Leaf " << h << " does not have mass");
        }
      }
      if (h.get_as_atom() && h.get_number_of_children() != 0) {
        TEST_FAIL("Atoms cannot have children");
      }
      if (h.get_as_atom()) {
        Atom a= h.get_as_atom();
        if (a.get_atom_type().get_string().find("HET:") == std::string::npos) {
          try {
            get_residue(a);
          } catch (...) {
            TEST_FAIL("Atom " << a << " is not part of residue");
          }
        }
        /*if (h.get_parent_index()==0) {
          Hierarchy p= h.get_parent();
          if (p) {
            AtomType last=AT_UNKNOWN;
            for (unsigned int i=0; i< p.get_number_of_children(); ++i) {
              Hierarchy c= p.get_child(i);
              if (!c.get_as_atom()) {
                TEST_FAIL("Child " << c << " is not an atom but should be.");
              }
              Atom ac= c.get_as_atom();
              AtomType cur= ac.get_atom_type();
              if (cur < last) {
                TEST_FAIL("Atoms out of order in residue " << p);
              }
              last=cur;
            }
          }
          }*/
      }
      if (h.get_as_residue()) {
        if (h.get_parent() && h.get_parent().get_child(0)==h
            && (Residue(h).get_is_protein()
                || Residue(h).get_is_dna()
                || Residue(h).get_is_rna())){
          Hierarchy p= h.get_parent();
          if (p) {
            int last_index= std::numeric_limits<int>::min();
            for (unsigned int i=0; i< p.get_number_of_children(); ++i) {
              Hierarchy c= p.get_child(i);
              /*if (!c.get_as_residue()) {
                TEST_FAIL("Sibling of residue is not residue at " << c);
                }*/
              if (c.get_as_residue()) {
                Residue rc(c);
                if (Residue(h).get_is_protein()
                    || Residue(h).get_is_dna()
                    || Residue(h).get_is_rna()) {
                  if (rc.get_index() < last_index) {
                    TEST_FAIL("Residue indexes out of order at " << rc);
                  }
                  last_index=rc.get_index();
                }
              } else if (c.get_as_fragment()){
                // should check order, but it is a pain at
              } else if (c.get_as_domain()) {
                // should check order
              }
            }
          }
        }
      }
      if (h.get_parent() != Hierarchy()) {
        Hierarchy p = h.get_parent();
        if ((h.get_as_atom() && !p.get_as_residue())
            || (p.get_as_residue() &&
                (h.get_as_chain() || h.get_as_domain()))
            ){
          TEST_FAIL("Node " << h
                    << " cannot be a child of its parent "
                    << h.get_parent());
        }
        /*if (p.get_as_fragment() || p.get_as_domain()) {
          if (!h.get_as_fragment() && !h.get_as_domain()
              && !h.get_as_residue()) {
            TEST_FAIL("Node " << h
                      << " has a parent with residue index information"
                      << " but does not itself has index information.");
          }
          }*/
      }
      if ((h.get_as_atom() && (h.get_as_residue()
                              || h.get_as_domain()
                              || h.get_as_chain()
                               || h.get_as_fragment()))
          || (h.get_as_residue() && (h.get_as_domain()
                                     || h.get_as_chain()
                                     || h.get_as_fragment()))
          || (h.get_as_fragment() && (h.get_as_domain()))) {
        TEST_FAIL("Node cannot have more than one type at once "
                  << h << " "
                  << static_cast<bool>(h.get_as_atom())
                  << static_cast<bool>(h.get_as_residue())
                  << static_cast<bool>(h.get_as_domain())
                  << static_cast<bool>(h.get_as_chain())
                  << static_cast<bool>(h.get_as_fragment()));
      }
      return true;
    }
  };
}


bool Hierarchy::get_is_valid(bool print_info) const {
  try {
    IMP::core::visit_depth_first_with_data(*this, Validator(print_info),
                                               false);
  } catch (const BadHierarchy &) {
    return false;
  }
  return true;
}

Hierarchy
create_fragment(const Hierarchies &ps)
{
  IMP_USAGE_CHECK(!ps.empty(), "Need some particles");
  Hierarchy parent= ps[0].get_parent();
  unsigned int index= ps[0].get_child_index();
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_USAGE_CHECK(ps[i].get_parent() == parent,
                "Parents don't match");
    }
  }

  Particle *fp= new Particle(parent.get_particle()->get_model());
  Hierarchy fd= Fragment::setup_particle(fp);

  for (unsigned int i=0; i< ps.size(); ++i) {
    parent.remove_child(ps[i]);
    fd.add_child(ps[i]);
  }

  parent.add_child_at(fd, index);
  return fd;
}

Bonds get_internal_bonds(Hierarchy mhd)
{
  ParticlesTemp ps= core::get_all_descendants(mhd);
  IMP::base::set<Particle*> sps(ps.begin(), ps.end());
  Bonds ret;
  for (ParticlesTemp::iterator pit = ps.begin();
       pit != ps.end(); ++pit) {
    Particle *p = *pit;
    if (Bonded::particle_is_instance(p)) {
      Bonded b(p);
      for (unsigned int i=0; i< b.get_number_of_bonds(); ++i) {
        Particle *op = b.get_bonded(i).get_particle();
        if (op < p && sps.find(op) != sps.end()) {
          ret.push_back(b.get_bond(i));
        }
      }
    }
  }
  return ret;
}





/*
  Volume of two spheres overlap is
  Vi= pi*(r0+r1-d)^2*(d^2+2*d*r1-3*r1^2+2*d*r0+6*r0*r1-3*r0^2)/(12*d)

  r1=r0=r
  d=(1-f)*2*r
  v=4/3pir^3*n-(n-1)Vi

  n=.5*(3*V+2*PI*r^3*f^3-6*PI*r^3*f^2)/((-3*f^2+f^3+2)*r^3*PI)
 */



core::RigidBody setup_as_rigid_body(Hierarchy h) {
  IMP_USAGE_CHECK(h.get_is_valid(true),
                     "Invalid hierarchy passed to setup_as_rigid_body");
  IMP_WARN("create_rigid_body should be used instead of setup_as_rigid_body"
           << " as the former allows one to get volumes correct at coarser"
           << " levels of detail.");
  core::XYZs leaves(get_leaves(h));
  core::RigidBody rbd
    = core::RigidBody::setup_particle(h, leaves);
  rbd.set_coordinates_are_optimized(true);
  ParticlesTemp internal= core::get_internal(h);
  for (unsigned int i=0; i< internal.size(); ++i) {
    if (internal[i] != h) {
      core::RigidMembers leaves(get_leaves(Hierarchy(internal[i])));
      if (!leaves.empty()) {
        algebra::ReferenceFrame3D rf
            = core::get_initial_reference_frame(get_as<ParticlesTemp>(leaves));
        core::RigidBody::setup_particle(internal[i], rf);
      }
    }
  }
  IMP_INTERNAL_CHECK(h.get_is_valid(true), "Invalid hierarchy produced");
  return rbd;
}


namespace {
  ParticlesTemp rb_process(Hierarchy h) {
    ParticlesTemp internal= core::get_internal(h);
    ParticlesTemp all=get_leaves(h);
    for (unsigned int i=0; i< internal.size(); ++i) {
      ParticlesTemp leaves(get_leaves(Hierarchy(internal[i])));
      if (!leaves.empty() && !core::XYZR::particle_is_instance(internal[i])) {
        setup_as_approximation(internal[i], leaves);
        all.push_back(internal[i]);
      }
    }
    return all;
  }
}

// write approximate function, remove rigid bodies for intermediates
core::RigidBody create_rigid_body(const Hierarchies& h,
                                  std::string name) {
  if (h.empty()) return core::RigidBody();
  for (unsigned int i=0; i< h.size(); ++i) {
    IMP_USAGE_CHECK(h[i].get_is_valid(true), "Invalid hierarchy passed.");
  }
  Particle *rbp= new Particle(h[0]->get_model());
  rbp->set_name(name);
  ParticlesTemp all;
  for (unsigned int i=0; i< h.size(); ++i) {
    ParticlesTemp cur= rb_process(h[i]);
    all.insert(all.end(), cur.begin(), cur.end());
  }
  core::RigidBody rbd
    = core::RigidBody::setup_particle(rbp, core::XYZs(all));
  rbd.set_coordinates_are_optimized(true);
  for (unsigned int i=0; i< h.size(); ++i) {
    IMP_INTERNAL_CHECK(h[i].get_is_valid(true), "Invalid hierarchy produced");
  }
  return rbd;
}

core::RigidBody create_rigid_body(Hierarchy h) {
  return create_rigid_body(Hierarchies(1,h), h->get_name()+" rigid body");
}



IMP::core::RigidBody create_compatible_rigid_body(Hierarchy h,
                                               Hierarchy reference) {
  ParticlesTemp hl= get_leaves(h);
  ParticlesTemp rl= get_leaves(reference);
  algebra::Transformation3D tr
    = algebra::get_transformation_aligning_first_to_second(rl, hl);
  algebra::Transformation3D rtr
    = core::RigidMember(reference).get_rigid_body().\
    get_reference_frame().get_transformation_to();
  algebra::Transformation3D rbtr= tr*rtr;

  Particle *rbp= new Particle(h->get_model());
  rbp->set_name(h->get_name()+" rigid body");
  ParticlesTemp all = rb_process(h);
  core::RigidBody rbd
    = core::RigidBody::setup_particle(rbp, algebra::ReferenceFrame3D(rbtr));
  for (unsigned int i=0; i< all.size(); ++i) {
    rbd.add_member(all[i]);
  }
  rbd.set_coordinates_are_optimized(true);
  IMP_INTERNAL_CHECK(h.get_is_valid(true), "Invalid hierarchy produced");
  return rbd;
}




namespace {

  Hierarchy clone_internal(Hierarchy d,
                           std::map<Particle*,
                           Particle*> &map, bool recurse) {
    Particle *p= new Particle(d.get_model());
    p->set_name(d->get_name());
    map[d.get_particle()]=p;
    Hierarchy nd;
    if (Atom::particle_is_instance(d.get_particle())) {
      nd= Atom::setup_particle(p, Atom(d.get_particle()));
    }
    if (Residue::particle_is_instance(d.get_particle())) {
      nd= Residue::setup_particle(p, Residue(d.get_particle()));
    }
    if (Domain::particle_is_instance(d.get_particle())) {
      nd= Domain::setup_particle(p, Domain(d.get_particle()));
    }
    if (Chain::particle_is_instance(d.get_particle())) {
      nd= Chain::setup_particle(p, Chain(d.get_particle()));
    }
    if (Molecule::particle_is_instance(d.get_particle())) {
      nd= Molecule::setup_particle(p, Molecule(d.get_particle()));
    }
    if (Fragment::particle_is_instance(d.get_particle())) {
      nd= Fragment::setup_particle(p, Fragment(d.get_particle()));
    }
    if (nd== Hierarchy()) nd=Hierarchy::setup_particle(p);
    using core::XYZ;
    using core::XYZR;
    if (XYZR::particle_is_instance(d.get_particle())){
      XYZR::setup_particle(p,
                           algebra::Sphere3D(XYZ(d.get_particle())
                                             .get_coordinates(),
                                             XYZR(d.get_particle())
                                             .get_radius()));
    } else if (XYZ::particle_is_instance(d.get_particle())) {
      XYZ::setup_particle(p,
                          XYZ(d.get_particle()).get_coordinates());
    }
    p->set_name(d.get_particle()->get_name());
    if (recurse) {
      for (unsigned int i=0 ;i< d.get_number_of_children(); ++i) {
        Hierarchy nc= clone_internal(d.get_child(i), map, true);
        nd.add_child(nc);
      }
    }
    return nd;
  }
}


Hierarchy create_clone(Hierarchy d) {
  std::map<Particle*,Particle*> map;
  Hierarchy nh= clone_internal(d, map, true);
  Bonds bds= get_internal_bonds(d);
  for (unsigned int i=0; i< bds.size(); ++i) {
    Bonded e0= bds[i].get_bonded(0);
    Bonded e1= bds[i].get_bonded(1);
    Particle *np0= map[e0.get_particle()];
    Particle *np1= map[e1.get_particle()];
    Bonded ne0, ne1;
    if (Bonded::particle_is_instance(np0)) {
      ne0=Bonded(np0);
    } else {
      ne0=Bonded::setup_particle(np0);
    }
    if (Bonded::particle_is_instance(np1)) {
      ne1=Bonded(np1);
    } else {
      ne1=Bonded::setup_particle(np1);
    }
    create_bond(ne0, ne1, bds[i]);
  }
  return nh;
}

Hierarchy create_clone_one(Hierarchy d) {
  std::map<Particle*,Particle*> map;
  return clone_internal(d, map, false);
}


struct True {
  template <class T>
  bool operator()(const T &) const{ return true;}
};

void destroy(Hierarchy d) {
  ParticlesTemp all;
  //core::Hierarchy h=d;

  core::gather(d, True(), std::back_inserter(all));
  for (unsigned int i=0; i< all.size(); ++i) {
    if (Bonded::particle_is_instance(all[i])) {
     Bonded b(all[i]);
      while (b.get_number_of_bonds() > 0) {
        destroy_bond(b.get_bond(b.get_number_of_bonds()-1));
      }
    }
    Hierarchy hc(all[i]);
    while (hc.get_number_of_children() > 0) {
      hc.remove_child(hc.get_child(hc.get_number_of_children()-1));
    }
  }

  // If this Hierarchy has a parent, remove the relationship
  Hierarchy parent = d.get_parent();
  if (parent) {
    parent.remove_child(d);
  }

  for (unsigned int i=0; i< all.size(); ++i) {
    all[i]->get_model()->remove_particle(all[i]);
  }
}


bool get_is_heterogen(Hierarchy h) {
  if (Atom::particle_is_instance(h)) {
    Atom a(h);
    bool ret= (a.get_atom_type() >= AT_UNKNOWN);
    IMP_INTERNAL_CHECK((ret && a.get_atom_type().get_string().find("HET:")==0)
                       || (!ret && a.get_atom_type().get_string().find("HET:")
                           == std::string::npos),
                       "Unexpected atom type found " << a.get_atom_type()
                       << (ret?" is ": " is not ") << "a heterogen.");
    return ret;
  } else {
    Residue r(h);
    return (r.get_residue_type()>= DTHY);
  }
}



algebra::BoundingBox3D get_bounding_box(const Hierarchy &h) {
  ParticlesTemp rep= get_leaves(h);
  algebra::BoundingBox3D bb;
  for (unsigned int i=0; i< rep.size(); ++i) {
    core::XYZR xyzr= core::XYZR::decorate_particle(rep[i]);
    if (xyzr) {
      bb+= algebra::get_bounding_box(xyzr.get_sphere());
    } else if (core::XYZ::particle_is_instance(rep[i])) {
      bb+= algebra::BoundingBox3D(core::XYZ(rep[i]).get_coordinates());
    }
  }
  IMP_LOG_VERBOSE( "Bounding box is " << bb << std::endl);
  return bb;
}


algebra::Sphere3D get_bounding_sphere(const Hierarchy &h) {
  ParticlesTemp rep= get_leaves(h);
  algebra::Sphere3Ds ss;
  for (unsigned int i=0; i< rep.size(); ++i) {
    core::XYZR xyzr= core::XYZR::decorate_particle(rep[i]);
    if (xyzr) {
      ss.push_back(xyzr.get_sphere());
    } else if (core::XYZ::particle_is_instance(rep[i])) {
      ss.push_back(algebra::Sphere3D(core::XYZ(rep[i]).get_coordinates(),
                                     0));
    }
  }
  return algebra::get_enclosing_sphere(ss);
}


IMP_FOREACH_HIERARCHY_TYPE_FUNCTIONS(IMP_GET_AS_DEF);

IMPATOM_END_NAMESPACE
