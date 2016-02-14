/**
 *  \file IMP/pmi/utilities.h
 *  \brief Useful utilities
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 */

#ifndef IMPPMI_UTILITIES_H
#define IMPPMI_UTILITIES_H

#include <IMP/pmi/pmi_config.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/DistanceRestraint.h>
#include <IMP/core/XYZR.h>
#include <IMP/atom/Molecule.h>
#include <IMP/atom/Copy.h>
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

//! Get the parent, or if non-tree Representation get the fake parent
inline atom::Hierarchy get_parent_representation(atom::Hierarchy h){
  ParticleIndex pidx = h.get_model()->get_attribute(
       atom::Hierarchy::get_traits().get_parent_key(),h.get_particle_index());
  return atom::Hierarchy(h.get_model(),pidx);
}


//! Walk up a PMI2 hierarchy/representations and get the "molname.copynum"
inline std::string get_molecule_name_and_copy(atom::Hierarchy h){
  do {
    if (atom::Molecule::get_is_setup(h) && atom::Copy::get_is_setup(h)) {
      return h->get_name() + "."
             + boost::lexical_cast<std::string>(atom::Copy(h).get_copy_index());
    }
  } while ((h = get_parent_representation(h)));
  IMP_THROW("Hierarchy " << h << " has no molecule name or copy num.", ValueException);
}

IMPPMI_END_NAMESPACE

#endif /* IMPPMI_UTILITIES_H */
