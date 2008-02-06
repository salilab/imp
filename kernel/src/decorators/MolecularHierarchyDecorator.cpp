/**
 *  \file MolecularHierarchyDecorator.cpp   \brief Decorator for helping deal
 *                                                 with a hierarchy.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#include <sstream>

#include "IMP/decorators/MolecularHierarchyDecorator.h"
#include "IMP/decorators/NameDecorator.h"
#include "IMP/decorators/AtomDecorator.h"
#include "IMP/decorators/ResidueDecorator.h"

namespace IMP
{

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


namespace internal
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

} // namespace internal

Particles get_particles(MolecularHierarchyDecorator mhd, 
                        MolecularHierarchyDecorator::Type t)
{
  Particles out;
  hierarchy_gather(mhd, internal::MHDMatchingType(t),
                   std::back_inserter(out));
  return out;
}

} // namespace IMP
