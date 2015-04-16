/**
 *  \file IMP/pmi/utilities.h
 *  \brief Useful utilities
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 */

#ifndef IMPPMI_UTILITIES_H
#define IMPPMI_UTILITIES_H

#include <IMP/pmi/pmi_config.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/DistanceRestraint.h>
#include <IMP/core/XYZR.h>

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


IMPPMI_END_NAMESPACE

#endif /* IMPPMI_UTILITIES_H */
