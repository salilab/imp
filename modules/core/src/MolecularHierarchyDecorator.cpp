/**
 *  \file MolecularHierarchyDecorator.cpp   \brief Decorator for helping deal
 *                                                 with a hierarchy.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/MolecularHierarchyDecorator.h>
#include <IMP/core/NameDecorator.h>
#include <IMP/core/AtomDecorator.h>
#include <IMP/core/ResidueDecorator.h>

#include <sstream>
#include <set>

IMPCORE_BEGIN_NAMESPACE

IntKey MolecularHierarchyDecorator::type_key_;

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
  } else if (get_type() == RESIDUE) {
    ResidueDecorator ad= ResidueDecorator::cast(get_particle());
    if (ad != ResidueDecorator()) {
      ad.show(out, prefix);
    }
  } else {
    out << prefix << get_type_string() <<std::endl;
    NameDecorator nd= NameDecorator::cast(get_particle());
    if (nd != NameDecorator()) {
      out << prefix << "\"" <<  nd.get_name() << "\"" << std::endl;
    }
  }
}



IMP_DECORATOR_INITIALIZE(MolecularHierarchyDecorator,
                         HierarchyDecorator,
                         {
                           type_key_=IntKey("molecular hierarchy type");
                         })


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

Particles molecular_hierarchy_get_by_type(MolecularHierarchyDecorator mhd,
                                          MolecularHierarchyDecorator::Type t)
{
  Particles out;
  hierarchy_gather(mhd, MHDMatchingType(t),
                   std::back_inserter(out));
  return out;
}


namespace
{

struct MatchResidueIndex
{
  unsigned int index_;
  MatchResidueIndex(unsigned int i): index_(i) {}
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
molecular_hierarchy_get_residue(MolecularHierarchyDecorator mhd,
                                unsigned int index)
{
  IMP_check(mhd.get_type() == MolecularHierarchyDecorator::PROTEIN
            || mhd.get_type() == MolecularHierarchyDecorator::CHAIN
            || mhd.get_type() == MolecularHierarchyDecorator::NUCLEOTIDE,
            "Invalid type of MolecularHierarchyDecorator passed to get_residue",
            ValueException);
  MatchResidueIndex mi(index);
  HierarchyDecorator hd= hierarchy_find(mhd, mi);
  if (hd== HierarchyDecorator()) {
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

  Particle *fp= new Particle();
  parent.get_particle()->get_model()->add_particle(fp);
  MolecularHierarchyDecorator fd= MolecularHierarchyDecorator::create(fp);
  fd.set_type(MolecularHierarchyDecorator::FRAGMENT);

  for (unsigned int i=0; i< ps.size(); ++i) {
    parent.remove_child(ps[i]);
    fd.add_child(ps[i]);
  }

  parent.add_child_at(fd, index);
  return fd;
}

IMPCORE_END_NAMESPACE
