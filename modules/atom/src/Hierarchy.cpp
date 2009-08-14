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

IntKey Hierarchy::get_type_key() {
  static IntKey k("molecular_hierarchy_type");
  return k;
}

void Hierarchy::show(std::ostream &out) const
{
  if (*this == Hierarchy()) {
    out << "NULL Molecular Hierarchy node";
    return;
  }
  if (get_type() == ATOM && Atom::particle_is_instance(get_particle())) {
    Atom ad(get_particle());
    ad.show(out);
  } else if ((get_type() == AMINOACID || get_type() == NUCLEICACID
              || get_type() == LIGAND)
             && Residue::particle_is_instance(get_particle())){
      Residue adt(get_particle());
      adt.show(out);
  } else if (get_type() == CHAIN
             && Chain::particle_is_instance(get_particle())){
      Chain adt(get_particle());
      adt.show(out);
  } else if (Domain::particle_is_instance(get_particle())) {
    Domain dd(get_particle());
    dd.show(out);
  } else {
    out << get_type_string() <<std::endl;
    out << "\"" <<  get_particle()->get_name() << "\"" << std::endl;
  }
}



namespace
{

struct MHDMatchingType
{
  MHDMatchingType(Hierarchy::Type t): t_(t){}

  bool operator()(Particle *p) const {
    Hierarchy mhd= Hierarchy::decorate_particle(p);
    if (mhd== Hierarchy()) {
      return false;
    } else {
      return mhd.get_type()==t_;
    }
  }

  Hierarchy::Type t_;
};

} // namespace

Hierarchies get_by_type(Hierarchy mhd,
                      Hierarchy::Type t)
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
    if (mhd.get_type() == Hierarchy::AMINOACID
        || mhd.get_type() == Hierarchy::NUCLEICACID) {
      Residue rd(p);
      return (rd.get_index() == index_);
    } else {
      if (mhd.get_number_of_children()==0) {
        IMP_LOG(VERBOSE, "Trying " << mhd << std::endl);
        Domain dd= Domain::decorate_particle(p);
        return dd && dd.get_begin_index() <= index_
          && dd.get_end_index()> index_;
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
  IMP_check(mhd.get_type() == Hierarchy::PROTEIN
            || mhd.get_type() == Hierarchy::CHAIN
            || mhd.get_type() == Hierarchy::NUCLEOTIDE,
            "Invalid type of Hierarchy passed to get_residue",
            ValueException);
  MatchResidueIndex mi(index);
  IMP::core::Hierarchy hd= breadth_first_find(mhd, mi);
  if (hd== IMP::core::Hierarchy()) {
    return Hierarchy();
  } else {
    return hd;
  }
}

#define TEST_FAIL(msg) \
  IMP_failure(msg, InvalidStateException)

namespace {
  struct Validator {
    typedef bool result_type;
    bool print_info;
    Validator(bool pi): print_info(pi){}
    bool operator()(Hierarchy h, bool) {
      if (h.get_number_of_children() ==0) {
        if (!core::XYZ::particle_is_instance(h)) {
          TEST_FAIL("Leaf " << h << " does not have coordinates");
        }
        if (!atom::Mass::particle_is_instance(h)) {
          TEST_FAIL("Leaf " << h << " does not have mass");
        }
      }
      if (h.get_parent() != Hierarchy()) {
        if (h.get_type()> h.get_parent().get_type()) {
          TEST_FAIL("Node " << h << " has a type "
                    << "which cannot be a child of its parent.");
        }
      }
      switch(h.get_type()) {
      case Hierarchy::ATOM:
        if (!Atom::particle_is_instance(h)) {
          TEST_FAIL("ATOM " << h << " is not an Atom");
        }
        break;
      case Hierarchy::AMINOACID:
      case Hierarchy::NUCLEICACID:
      case Hierarchy::LIGAND:
        if (!Residue::particle_is_instance(h)) {
          TEST_FAIL("Residue " << h << " is not a IMP.atom.Residue");
        }
        break;
      case Hierarchy::CHAIN:
        if (!Chain::particle_is_instance(h)) {
          TEST_FAIL("CHAIN " << h << " is not a IMP.atom.Chain");
        }
        break;
      case Hierarchy::FRAGMENT:
        if (!Fragment::particle_is_instance(h)) {
          TEST_FAIL("FRAGMENT " << h << " is not a IMP.atom.Fragment");
        }
        break;
      default:
        break;
      }
      return true;
    }
  };
}


bool Hierarchy::get_is_valid(bool print_info) const {
  try {
    IMP::core::depth_first_traversal_with_data(*this, Validator(print_info),
                                               false);
  } catch (const InvalidStateException &e) {
    return false;
  }
  return true;
}

Hierarchy
create_fragment(const Hierarchies &ps)
{
  IMP_check(!ps.empty(), "Need some particles",
            ValueException);
  Hierarchy parent= ps[0].get_parent();
  unsigned int index= ps[0].get_parent_index();
  IMP_IF_CHECK(CHEAP) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_check(ps[i].get_parent() == parent,
                "Parents don't match",
                ValueException);
    }
  }

  Particle *fp= new Particle(parent.get_particle()->get_model());
  Hierarchy fd= Hierarchy::setup_particle(fp,
                                       Hierarchy::FRAGMENT);

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



IMPATOM_END_NAMESPACE
