/**
 *  \file MolecularHierarchyDecorator.cpp   \brief Decorator for helping deal
 *                                                 with a hierarchy.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/MolecularHierarchyDecorator.h>
#include <IMP/core/NameDecorator.h>

#include <sstream>
#include <set>


IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

const HierarchyTraits& MolecularHierarchyDecorator::get_traits() {
  static HierarchyTraits ret("molecular_hierarchy");
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
  } else {
    out << prefix << get_type_string() <<std::endl;
    NameDecorator nd= NameDecorator::cast(get_particle());
    if (nd != NameDecorator()) {
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

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE
