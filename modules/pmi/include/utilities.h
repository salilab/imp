/**
 *  \file IMP/pmi/utilities.h
 *  \brief Useful utilities
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPPMI_UTILITIES_H
#define IMPPMI_UTILITIES_H

#include <IMP/pmi/pmi_config.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/DistanceRestraint.h>
#include <IMP/core/XYZR.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Molecule.h>
#include <IMP/atom/Copy.h>
#include <IMP/atom/Selection.h>
#include <IMP/core/internal/dihedral_helpers.h>
#include <IMP/Vector.h>
#include <boost/lexical_cast.hpp>

IMPPMI_BEGIN_NAMESPACE

//! Create an elastic network restraint set
RestraintSet * create_elastic_network(const Particles &ps,
                                      Float dist_cutoff,
                                      Float strength){
  IMP_NEW(RestraintSet,rs,(ps[0]->get_model(),"ElasticNetwork"));
  int nps=ps.size();
  for (int n1=0;n1<nps;n1++){
    for (int n2=n1+1;n2<nps;n2++){
      Float dist = core::get_distance(core::XYZ(ps[n1]),
                                      core::XYZ(ps[n2]));
      if (dist<dist_cutoff){
        IMP_NEW(core::HarmonicDistancePairScore,hdist,
                (dist,strength));
        Pointer<Restraint> rx = IMP::create_restraint(hdist.get(),
                                           ParticlePair(ps[n1],ps[n2]));
        rs->add_restraint(rx);
      }
    }
  }
  return rs.release();
}

Float get_dihedral_angle(const atom::Atom &p1,
                         const atom::Atom &p2,
                         const atom::Atom &p3,
                         const atom::Atom &p4){
  return core::internal::dihedral(p1,p2,p3,p4,nullptr,nullptr,nullptr,nullptr);
}

inline Float get_bipartite_minimum_sphere_distance(const IMP::core::XYZRs& m1,
                                                   const IMP::core::XYZRs& m2) {

  double mindist = -1.0 ;

  for (unsigned int k1 = 0; k1 < m1.size(); ++k1) {
    for (unsigned int k2 = 0; k2 < m2.size(); ++k2) {
       double dist = IMP::core::get_distance(m1[k1],m2[k2]);
       if (mindist < 0.0){mindist=dist;}
       if (mindist > dist){mindist=dist;}
      }
   }
  if (mindist < 0 ){mindist = 0;}
  return mindist;
}


inline Floats get_list_of_bipartite_minimum_sphere_distance(const ParticlesTemps & pss) {
  Floats mindistances;
  for (unsigned int k1 = 0; k1 < pss.size()-1; ++k1) {
    for (unsigned int k2 = k1+1; k2 < pss.size(); ++k2) {
       IMP::core::XYZRs xyzrs1=IMP::core::XYZRs(pss[k1]);
       IMP::core::XYZRs xyzrs2=IMP::core::XYZRs(pss[k2]);
       double dist = IMP::pmi::get_bipartite_minimum_sphere_distance(xyzrs1,xyzrs2);
       mindistances.push_back(dist);
      }
   }
  return mindistances;
}

//! Walk up a PMI2 hierarchy/representations and get the "molname.copynum"
inline std::string get_molecule_name_and_copy(atom::Hierarchy h){
  return atom::get_molecule_name(h) + "." +
    boost::lexical_cast<std::string>(atom::get_copy_index(h));
}

//! Walk up a PMI2 hierarchy/representations and check if the root is named System
inline bool get_is_canonical(atom::Hierarchy h){
  while (h) {
    if (h->get_name()=="System") {
      return true;
    }
    h = get_parent_representation(h);
  }
  return false;
}

IMPPMI_END_NAMESPACE

#endif /* IMPPMI_UTILITIES_H */
