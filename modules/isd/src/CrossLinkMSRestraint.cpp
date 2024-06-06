/**
 *  \file isd/CrossLinkMSRestraint.h
 *  \brief A sigmoid shaped restraint between
 *  residues with discrete classifier
 *  and ambiguous assignment. To be used with
 *  cross-linking mass-spectrometry data.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/CrossLinkMSRestraint.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/XYZR.h>
#include <IMP/isd/Scale.h>
#include <IMP/Restraint.h>
#include <cmath>
#include <limits.h>
#include <boost/math/special_functions/erf.hpp>

IMPISD_BEGIN_NAMESPACE

CrossLinkMSRestraint::CrossLinkMSRestraint(IMP::Model *m, double length,
                                           bool get_log_prob,
                                           std::string name)
    : Restraint(m, name), length_(length), get_log_prob_(get_log_prob) {
    constr_ = 0;
}

CrossLinkMSRestraint::CrossLinkMSRestraint(IMP::Model *m, 
                                           IMP::ParticleIndexAdaptor lengthi,
                                           bool get_log_prob,
                                           std::string name)
    : Restraint(m, name), lengthi_(lengthi), get_log_prob_(get_log_prob) {
    constr_ = 1;
}

CrossLinkMSRestraint::CrossLinkMSRestraint(IMP::Model *m, 
                                           double length, double slope,
                                           bool get_log_prob,
                                           std::string name)
    : Restraint(m, name), length_(length), slope_(slope), get_log_prob_(get_log_prob) {
    constr_ = 2;
}


double CrossLinkMSRestraint::get_probability() const {

    double onemprob = 1.0;

    double length = get_length();

    for (unsigned int k = 0; k < get_number_of_contributions(); ++k) {
        IMP::ParticleIndexPair ppi = ppis_[k];
        double dist;
        if (std::get<0>(ppi) != std::get<1>(ppi)) {
            core::XYZ d0(get_model(), std::get<0>(ppi));
            core::XYZ d1(get_model(), std::get<1>(ppi));
            dist =
            (d0.get_coordinates() - d1.get_coordinates()).get_magnitude();
        } else {
            // If the residues are assigned to the same particle-domain
            // get the distance as if the residue positions were randomly
            // taken from within the sphere representing the domain
            // Lund O, Protein Eng. 1997 Nov;10(11):1241-8.
            double R=core::XYZR(get_model(), std::get<0>(ppi)).get_radius();
            dist=36.0/35.0*R;
        }
        if (dist<0.0001){dist=0.0001;}
        IMP::ParticleIndexPair sigmas = sigmass_[k];
        IMP::ParticleIndex psii = psis_[k];
        double psi = isd::Scale(get_model(), psii).get_scale();
        double sigmai = isd::Scale(get_model(),
                                   std::get<0>(sigmas)).get_scale();
        double sigmaj = isd::Scale(get_model(),
                                   std::get<1>(sigmas)).get_scale();

        double voli = 4.0 / 3.0 * IMP::PI * sigmai * sigmai * sigmai;
        double volj = 4.0 / 3.0 * IMP::PI * sigmaj * sigmaj * sigmaj;

        double fi = 0;
        double fj = 0;

        if (dist < sigmai + sigmaj) {
            double xlvol = 4.0 / 3.0 * IMP::PI * (length / 2) * (length / 2) *
                           (length / 2);
            fi = std::min(voli, xlvol);
            fj = std::min(volj, xlvol);
        } else {
            double di = dist - sigmaj - length / 2;
            double dj = dist - sigmai - length / 2;
            fi = sphere_cap(sigmai, length / 2, std::abs(di));
            fj = sphere_cap(sigmaj, length / 2, std::abs(dj));
        }

        double pofr = fi * fj / voli / volj;
        
        if (constr_ == 2) {
            double prior=exp(-slope_*dist);
            onemprob = onemprob * (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi)) * prior);
        } else {
            onemprob = onemprob * (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi)) );
        }    

        // double tau=std::sqrt(sigmai*sigmai+sigmaj*sigmaj);
        // double lmd=(length_-dist)/std::sqrt(2)/tau;
        // double lpd=(length_+dist)/std::sqrt(2)/tau;
        // double erfm=terf(lmd);
        // double erfp=terf(lpd);
        // double expm=texp(-lmd*lmd);
        // double expp=texp(-lpd*lpd);
        // double
        // pofr=2*(2*(expp-expm)*tau+std::sqrt(2*IMP::PI)
        //      *dist*(erfm+erfp))/4/std::sqrt(2*IMP::PI)/dist;

        // std::cout << pofr << " " << expm << " " << expp << " " << erfp <<
        // std::endl ;
        
    }

    double prob = 1.0 - onemprob;
    

    
    // std::cout << prob << std::endl ;
    return prob;
}

double CrossLinkMSRestraint::sphere_cap(float r1, float r2, float d) const {
    double sc = 0;
    if (d <= std::max(r1, r2) - std::min(r1, r2)) {
        sc = std::min(4.0 / 3 * IMP::PI * r1 * r1 * r1,
                      4.0 / 3 * IMP::PI * r2 * r2 * r2);
    } else if (d >= r1 + r2) {
        sc = 0;
    } else {
        sc = (IMP::PI / 12 / d * (r1 + r2 - d) * (r1 + r2 - d)) *
             (d * d + 2 * d * r1 - 3 * r1 * r1 + 2 * d * r2 + 6 * r1 * r2 -
              3 * r2 * r2);
    }
    return sc;
}

double CrossLinkMSRestraint::unprotected_evaluate(DerivativeAccumulator *accum)
    const {
    double score = 0;
    if (get_log_prob_) {
       score = -log(get_probability());
    } else { 
       score = get_probability();
    }
    // std::cout << "here" << std::endl;

    // double prob=get_probability();
    // if (prob==0.0){score = std::numeric_limits<double>::max( );}
    // else{score=-log(prob);};
    if (accum) {
    };
    
    return score;
}

/* Return all particles whose attributes are read by the restraints. To
   do this, ask the pair score what particles it uses.*/
ModelObjectsTemp CrossLinkMSRestraint::do_get_inputs() const {
    ParticlesTemp ret;
    for (unsigned int k = 0; k < get_number_of_contributions(); ++k) {
        if (std::get<0>(ppis_[k]) == std::get<1>(ppis_[k])) {
            ret.push_back(get_model()->get_particle(std::get<0>(ppis_[k])));
        } else {
            ret.push_back(get_model()->get_particle(std::get<0>(ppis_[k])));
            ret.push_back(get_model()->get_particle(std::get<1>(ppis_[k])));
        }
        if (std::get<0>(sigmass_[k]) == std::get<1>(sigmass_[k])) {
            ret.push_back(get_model()->get_particle(std::get<0>(sigmass_[k])));
        } else {
            ret.push_back(get_model()->get_particle(std::get<0>(sigmass_[k])));
            ret.push_back(get_model()->get_particle(std::get<1>(sigmass_[k])));
        }
        ret.push_back(get_model()->get_particle(psis_[k]));
    }
    return ret;
}

RestraintInfo *CrossLinkMSRestraint::get_static_info() const {
  if (protein1_.empty() || protein2_.empty()) {
    return nullptr;
  } else {
    IMP_NEW(RestraintInfo, ri, ());
    ri->add_string("protein1", protein1_);
    ri->add_string("protein2", protein2_);
    ri->add_int("residue1", residue1_);
    ri->add_int("residue2", residue2_);

    ParticleIndexes ps;
    for (const auto &sigma : sigmass_) {
      ps.push_back(std::get<0>(sigma));
      ps.push_back(std::get<1>(sigma));
    }
    ri->add_particle_indexes("sigmas", ps);

    ri->add_particle_indexes("psis", psis_);

    ps.clear();
    for (const auto &ppi : ppis_) {
      ps.push_back(std::get<0>(ppi));
      ps.push_back(std::get<1>(ppi));
    }
    ri->add_particle_indexes("endpoints", ps);

    return ri.release();
  }
}

IMP_OBJECT_SERIALIZE_IMPL(IMP::isd::CrossLinkMSRestraint);

IMPISD_END_NAMESPACE
