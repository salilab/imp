/**
 *  \file Hierarchy.cpp   \brief Decorator for helping deal
 *                                                 with a hierarchy.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
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
#include <IMP/core/Hierarchy.h>
#include <IMP/atom/estimates.h>
#include <IMP/core/Harmonic.h>
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/core/internal/Grid3D.h>

#include <boost/random/uniform_int.hpp>

#include <sstream>
#include <set>


IMPATOM_BEGIN_NAMESPACE


const IMP::core::HierarchyTraits&
Hierarchy::get_traits() {
  static IMP::core::HierarchyTraits ret("molecular_hierarchy");
  return ret;
}

void Hierarchy::show(std::ostream &out) const
{
  if (*this == Hierarchy()) {
    out << "NULL Molecular Hierarchy node";
    return;
  }
  if (get_as_atom()) {
    out << get_as_atom();
  } else if (get_as_residue()){
    out << get_as_residue();
  } else if (get_as_chain()) {
    out << get_as_chain();
  } else if (get_as_domain()) {
    out << get_as_domain();
  } else if (get_as_xyzr()) {
    out << get_as_xyzr();
  } else if (get_as_xyz()) {
    out << get_as_xyz();
  } else {
    out << "Hierarchy \"" <<  get_particle()->get_name()
        << "\"" << std::endl;
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
      IMP_FOREACH_HIERARCHY_TYPE(IMP_IMPL_MATCH_TYPE)
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
  IMP::core::Hierarchy hd= breadth_first_find(mhd, mi);
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
        if (!h.get_as_xyz()) {
          TEST_FAIL("Leaf " << h << " does not have coordinates");
        }
        if (!h.get_as_mass()) {
          TEST_FAIL("Leaf " << h << " does not have mass");
        }
      }
      if (h.get_as_atom() && h.get_number_of_children() != 0) {
        TEST_FAIL("Atoms cannot have children");
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
      }
      if ((h.get_as_atom() && (h.get_as_residue()
                              || h.get_as_domain()
                              || h.get_as_chain()
                               || h.get_as_fragment()))
          || (h.get_as_fragment() && (h.get_as_domain()
                                     || h.get_as_chain()
                                      || h.get_as_fragment()))
          || (h.get_as_domain() && ( h.get_as_chain()
                                     || h.get_as_fragment()))) {
        TEST_FAIL("Node cannot have more than onetype at once");
      }
      return true;
    }
  };
}


bool Hierarchy::get_is_valid(bool print_info) const {
  try {
    IMP::core::depth_first_traversal_with_data(*this, Validator(print_info),
                                               false);
  } catch (const BadHierarchy &) {
    return false;
  }
  return true;
}

Hierarchy
create_fragment(const Hierarchies &ps)
{
  IMP_USAGE_CHECK(!ps.empty(), "Need some particles",
            ValueException);
  Hierarchy parent= ps[0].get_parent();
  unsigned int index= ps[0].get_parent_index();
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_USAGE_CHECK(ps[i].get_parent() == parent,
                "Parents don't match",
                ValueException);
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
  Particles ps= get_all_descendants(mhd);
  std::set<Particle*> sps(ps.begin(), ps.end());
  Bonds ret;
  for (Particles::iterator pit = ps.begin(); pit != ps.end(); ++pit) {
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



core::RigidBody rigid_body_setup_hierarchy(Hierarchy h) {
  core::XYZs leaves(get_leaves(h));
  core::RigidBody rbd
    = core::RigidBody::setup_particle(h, leaves);
  rbd.set_coordinates_are_optimized(true);
  Particles internal= core::get_internal(h);
  for (unsigned int i=0; i< internal.size(); ++i) {
    if (internal[i] != h) {
      core::RigidMembers leaves(get_leaves(Hierarchy(internal[i])));
      core::RigidBody::setup_particle(internal[i], leaves);
    }
  }
  return rbd;
}






namespace {

  Hierarchy clone_internal(Hierarchy d,
                                 std::map<Particle*,
                                 Particle*> &map) {
    Particle *p= new Particle(d.get_model());
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
    for (unsigned int i=0 ;i< d.get_number_of_children(); ++i) {
      Hierarchy nc= clone_internal(d.get_child(i), map);
      nd.add_child(nc);
    }
    return nd;
  }
}


Hierarchy clone(Hierarchy d) {
  std::map<Particle*,Particle*> map;
  Hierarchy nh= clone_internal(d, map);
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
    copy_bond(ne0, ne1, bds[i]);
  }
  return nh;
}


struct True {
  template <class T>
  bool operator()(const T &t) const{ return true;}
};

void destroy(Hierarchy d) {
  Hierarchies all;
  core::gather(d, True(), std::back_inserter(all));
  for (unsigned int i=0; i< all.size(); ++i) {
    if (Bonded::particle_is_instance(all[i])) {
     Bonded b(all[i]);
      while (b.get_number_of_bonds() > 0) {
        unbond(b.get_bond(b.get_number_of_bonds()-1));
      }
    }
    while (all[i].get_number_of_children() > 0) {
      all[i].remove_child(all[i].get_child(all[i].get_number_of_children()-1));
    }
  }
  for (unsigned int i=0; i< all.size(); ++i) {
    all[i].get_particle()->get_model()->remove_particle(all[i]);
  }
}




algebra::BoundingBox3D get_bounding_box(const Hierarchy &h) {
  Particles rep= get_leaves(h);
  algebra::BoundingBox3D bb;
  for (unsigned int i=0; i< rep.size(); ++i) {
    core::XYZR xyzr= core::XYZR::decorate_particle(rep[i]);
    if (xyzr) {
      bb+= algebra::get_bounding_box(xyzr.get_sphere());
    } else if (core::XYZ::particle_is_instance(rep[i])) {
      bb+= algebra::BoundingBox3D(core::XYZ(rep[i]).get_coordinates());
    }
  }
  IMP_LOG(VERBOSE, "Bounding box is " << bb << std::endl);
  return bb;
}

algebra::Sphere3D get_bounding_sphere(const Hierarchy &h) {
  Particles rep= get_leaves(h);
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
  return algebra::enclosing_sphere(ss);
}


IMP_FOREACH_HIERARCHY_TYPE(IMP_GET_AS_DEF)

IMPATOM_END_NAMESPACE
