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
#include <IMP/atom/Domain.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/core/XYZR.h>
#include <IMP/atom/estimates.h>
#include <IMP/core/Harmonic.h>
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/DistancePairScore.h>

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

void Hierarchy::show(std::ostream &out,
                                       std::string prefix) const
{
  if (*this == Hierarchy()) {
    out << "NULL Molecular Hierarchy node";
    return;
  }
  if (get_type() == ATOM && Atom::is_instance_of(get_particle())) {
    Atom ad(get_particle());
    ad.show(out, prefix);
  } else if ((get_type() == RESIDUE || get_type() == NUCLEICACID)
             && Residue::is_instance_of(get_particle())){
      Residue adt(get_particle());
      adt.show(out, prefix);
  } else if (get_type() == CHAIN
             && Chain::is_instance_of(get_particle())){
      Chain adt(get_particle());
      adt.show(out, prefix);
  } else if (Domain::is_instance_of(get_particle())) {
    Domain dd(get_particle());
    dd.show(out, prefix);
  } else {
    out << prefix << get_type_string() <<std::endl;
    out << prefix << "\"" <<  get_particle()->get_name() << "\"" << std::endl;
  }
}



namespace
{

struct MHDMatchingType
{
  MHDMatchingType(Hierarchy::Type t): t_(t){}

  bool operator()(Particle *p) const {
    Hierarchy mhd= Hierarchy::cast(p);
    if (mhd== Hierarchy()) {
      return false;
    } else {
      return mhd.get_type()==t_;
    }
  }

  Hierarchy::Type t_;
};

} // namespace

Particles get_by_type(Hierarchy mhd,
                      Hierarchy::Type t)
{
  Particles out;
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
    if (mhd.get_type() == Hierarchy::RESIDUE
        || mhd.get_type() == Hierarchy::NUCLEICACID) {
      Residue rd(p);
      return (rd.get_index() == index_);
    } else {
      if (mhd.get_number_of_children()==0) {
        Domain dd= Domain::cast(p);
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
    return Hierarchy(hd.get_particle());
  }
}



Hierarchy
create_fragment(const Hierarchys &ps)
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
  Hierarchy fd= Hierarchy::create(fp,
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
    if (Bonded::is_instance_of(p)) {
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

namespace {
IMPATOMEXPORT
Hierarchy clone_internal(Hierarchy d,
                                           std::map<Particle*,
                                           Particle*> &map) {
  Particle *p= new Particle(d.get_model());
  map[d.get_particle()]=p;
  Hierarchy nd;
  if (Atom::is_instance_of(d.get_particle())) {
    nd= Atom::create(p, Atom(d.get_particle()));
  } else if (Residue::is_instance_of(d.get_particle())) {
    nd= Residue::create(p, Residue(d.get_particle()));
  } else if (Domain::is_instance_of(d.get_particle())) {
    nd= Domain::create(p, Domain(d.get_particle()));
  } else if (Chain::is_instance_of(d.get_particle())) {
    nd= Chain::create(p, Chain(d.get_particle()));
  } else {
    nd=Hierarchy::create(p, d.get_type());
  }
  using core::XYZ;
  using core::XYZR;
  if (XYZR::is_instance_of(d.get_particle())){
    XYZR::create(p,
        algebra::Sphere3D(XYZ(d.get_particle()).get_coordinates(),
                          XYZR(d.get_particle()).get_radius()));
  } else if (XYZ::is_instance_of(d.get_particle())) {
    XYZ::create(p,
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
    if (Bonded::is_instance_of(np0)) {
      ne0=Bonded(np0);
    } else {
      ne0=Bonded::create(np0);
    }
    if (Bonded::is_instance_of(np1)) {
      ne1=Bonded(np1);
    } else {
      ne1=Bonded::create(np1);
    }
    copy_bond(ne0, ne1, bds[i]);
  }
  return nh;
}

/*
  Volume of two spheres overlap is
  Vi= pi*(r0+r1-d)^2*(d^2+2*d*r1-3*r1^2+2*d*r0+6*r0*r1-3*r0^2)/(12*d)

  r1=r0=r
  d=(1-f)*2*r
  v=4/3pir^3*n-(n-1)Vi

  n=.5*(3*V+2*PI*r^3*f^3-6*PI*r^3*f^2)/((-3*f^2+f^3+2)*r^3*PI)
 */


namespace {
  std::pair<int, double> compute_n(double V, double r, double f) {
    double n=.5*(3*V+2*PI*cube(r*f)-6*PI*cube(r)*square(f))
      /((-3*square(f)+cube(f)+2)*cube(r)*PI);
    int in= static_cast<int>(std::ceil(n));
    double rr= std::pow(V/(.666*(2*in-3*square(f)*n+cube(f)*n
                                +3*square(f)-cube(f))*PI), .333333);
    return std::make_pair(in, rr);
  }
}

IMPATOMEXPORT Restraint* create_protein(Particle *p,
                                        double resolution,
                                        int number_of_residues,
                                        int first_residue_index,
                                        double volume,
                                        double spring_strength) {
  if (volume < 0) {
    double mass= mass_in_kDa_from_number_of_residues(number_of_residues);
    volume= volume_from_mass_in_kDa(mass);
  }
  // assume a 20% overlap in the beads to make the protein not too bumpy
  double overlap_frac=.2;
  std::pair<int, double> nr= compute_n(volume, resolution, overlap_frac);
  Hierarchy pd
    =Hierarchy::create(p,
                              Hierarchy::PROTEIN);
  Particles ps;
  for (int i=0; i< nr.first; ++i) {
    Particle *pc= new Particle(p->get_model());
    Hierarchy pcd
      =Hierarchy::create(pc,
                              Hierarchy::FRAGMENT);
    pd.add_child(pcd);
    core::XYZR xyzd=core::XYZR::create(pc);
    xyzd.set_radius(nr.second);
    xyzd.set_coordinates_are_optimized(true);
    ps.push_back(pc);
  }
  IMP_NEW(core::Harmonic, h, ((1-overlap_frac)*2*nr.second, spring_strength));
  IMP_NEW(core::DistancePairScore, dps, (h));
  core::ConnectivityRestraint* cr= new core::ConnectivityRestraint(dps);
  cr->set_particles(ps);
  return cr;
}


IMPATOM_END_NAMESPACE
