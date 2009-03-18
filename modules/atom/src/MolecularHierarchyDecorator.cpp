/**
 *  \file MolecularHierarchyDecorator.cpp   \brief Decorator for helping deal
 *                                                 with a hierarchy.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/MolecularHierarchyDecorator.h>
#include <IMP/core/NameDecorator.h>
#include <IMP/atom/AtomDecorator.h>
#include <IMP/atom/ResidueDecorator.h>
#include <IMP/atom/DomainDecorator.h>
#include <IMP/core/LeavesParticleRefiner.h>

#include <sstream>
#include <set>


IMPATOM_BEGIN_NAMESPACE

const IMP::core::HierarchyTraits&
MolecularHierarchyDecorator::get_traits() {
  static IMP::core::HierarchyTraits ret("molecular_hierarchy");
  return ret;
}

IntKey MolecularHierarchyDecorator::get_type_key() {
  static IntKey k("molecular_hierarchy_type");
  return k;
}

void MolecularHierarchyDecorator::show(std::ostream &out,
                                       std::string prefix) const
{
  if (is_default()) {
    out << "NULL Molecular Hierarchy node";
    return;
  }
  if (get_type() == ATOM) {
    AtomDecorator ad= AtomDecorator::cast(get_particle());
    if (ad != AtomDecorator()) {
      ad.show(out, prefix);
    }
  } else if (get_type() == RESIDUE || get_type() == NUCLEICACID) {
    ResidueDecorator ad= ResidueDecorator::cast(get_particle());
    if (ad != ResidueDecorator()) {
      ad.show(out, prefix);
    }
  } else {
    out << prefix << get_type_string() <<std::endl;
    IMP::core::NameDecorator nd= IMP::core::NameDecorator::cast(get_particle());
    if (nd != IMP::core::NameDecorator()) {
      out << prefix << "\"" <<  nd.get_name() << "\"" << std::endl;
    }
  }
}



namespace
{

struct MHDMatchingType
{
  MHDMatchingType(MolecularHierarchyDecorator::Type t): t_(t){}

  bool operator()(Particle *p) const {
    MolecularHierarchyDecorator mhd= MolecularHierarchyDecorator::cast(p);
    if (mhd== MolecularHierarchyDecorator()) {
      return false;
    } else {
      return mhd.get_type()==t_;
    }
  }

  MolecularHierarchyDecorator::Type t_;
};

} // namespace

Particles get_by_type(MolecularHierarchyDecorator mhd,
                      MolecularHierarchyDecorator::Type t)
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
    MolecularHierarchyDecorator mhd(p);
    if (mhd.get_type() == MolecularHierarchyDecorator::RESIDUE
        || mhd.get_type() == MolecularHierarchyDecorator::NUCLEICACID) {
      ResidueDecorator rd(p);
      return (rd.get_index() == index_);
    } else {
      return false;
    }
  }
};

} // namespace


ResidueDecorator
get_residue(MolecularHierarchyDecorator mhd,
            unsigned int index)
{
  IMP_check(mhd.get_type() == MolecularHierarchyDecorator::PROTEIN
            || mhd.get_type() == MolecularHierarchyDecorator::CHAIN
            || mhd.get_type() == MolecularHierarchyDecorator::NUCLEOTIDE,
            "Invalid type of MolecularHierarchyDecorator passed to get_residue",
            ValueException);
  MatchResidueIndex mi(index);
  IMP::core::HierarchyDecorator hd= breadth_first_find(mhd, mi);
  if (hd== IMP::core::HierarchyDecorator()) {
    return ResidueDecorator();
  } else {
    return ResidueDecorator(hd.get_particle());
  }
}



MolecularHierarchyDecorator
create_fragment(const MolecularHierarchyDecorators &ps)
{
  IMP_check(!ps.empty(), "Need some particles",
            ValueException);
  MolecularHierarchyDecorator parent= ps[0].get_parent();
  unsigned int index= ps[0].get_parent_index();
  IMP_IF_CHECK(CHEAP) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_check(ps[i].get_parent() == parent,
                "Parents don't match",
                ValueException);
    }
  }

  Particle *fp= new Particle(parent.get_particle()->get_model());
  MolecularHierarchyDecorator fd= MolecularHierarchyDecorator::create(fp,
                                       MolecularHierarchyDecorator::FRAGMENT);

  for (unsigned int i=0; i< ps.size(); ++i) {
    parent.remove_child(ps[i]);
    fd.add_child(ps[i]);
  }

  parent.add_child_at(fd, index);
  return fd;
}

BondDecorators get_internal_bonds(MolecularHierarchyDecorator mhd)
{
  Particles ps= get_all_descendants(mhd);
  std::set<Particle*> sps(ps.begin(), ps.end());
  BondDecorators ret;
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (BondedDecorator::is_instance_of(ps[i])){
      BondedDecorator b(ps[i]);
      for (unsigned int i=0; i< b.get_number_of_bonds(); ++i) {
        Particle *op= b.get_bonded(i).get_particle();
        if (op < ps[i]
            && sps.find(op) != sps.end()) {
          ret.push_back(b.get_bond(i));
        }
      }
    }
  }
  return ret;
}

namespace {
IMPATOMEXPORT
MolecularHierarchyDecorator clone_internal(MolecularHierarchyDecorator d,
                                           std::map<Particle*,
                                           Particle*> &map) {
  Particle *p= new Particle(d.get_model());
  map[d.get_particle()]=p;
  MolecularHierarchyDecorator nd
    =MolecularHierarchyDecorator::create(p, d.get_type());
  for (unsigned int i=0 ;i< d.get_number_of_children(); ++i) {
    MolecularHierarchyDecorator nc= clone_internal(d.get_child(i), map);
    nd.add_child(nc);
  }
  if (AtomDecorator::is_instance_of(d.get_particle())) {
    AtomDecorator::create(p, AtomDecorator(d.get_particle()));
  }
  if (ResidueDecorator::is_instance_of(d.get_particle())) {
    ResidueDecorator::create(p, ResidueDecorator(d.get_particle()));
  }
  if (DomainDecorator::is_instance_of(d.get_particle())) {
    DomainDecorator::create(p, DomainDecorator(d.get_particle()));
  }
  if (core::NameDecorator::is_instance_of(d.get_particle())) {
    core::NameDecorator::create(p,
                    core::NameDecorator(d.get_particle()).get_name());
  }
  return nd;
}
}

IMPATOMEXPORT
MolecularHierarchyDecorator clone(MolecularHierarchyDecorator d) {
  std::map<Particle*,Particle*> map;
  MolecularHierarchyDecorator nh= clone_internal(d, map);
  BondDecorators bds= get_internal_bonds(d);
  for (unsigned int i=0; i< bds.size(); ++i) {
    BondedDecorator e0= bds[i].get_bonded(0);
    BondedDecorator e1= bds[i].get_bonded(1);
    Particle *np0= map[e0.get_particle()];
    Particle *np1= map[e1.get_particle()];
    BondedDecorator ne0, ne1;
    if (BondedDecorator::is_instance_of(np0)) {
      ne0=BondedDecorator(np0);
    } else {
      ne0=BondedDecorator::create(np0);
    }
    if (BondedDecorator::is_instance_of(np1)) {
      ne1=BondedDecorator(np1);
    } else {
      ne1=BondedDecorator::create(np1);
    }
    copy_bond(ne0, ne1, bds[i]);
  }
  return nh;
}


core::RigidBodyTraits get_molecular_rigid_body_traits() {
  static core::RigidBodyTraits tr(
    new core::LeavesParticleRefiner(MolecularHierarchyDecorator::get_traits()),
                                  "rigid_molecule");
  return tr;
}


IMPATOM_END_NAMESPACE
