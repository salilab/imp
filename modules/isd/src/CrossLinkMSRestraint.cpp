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
#include <algorithm>
#include <limits.h>
#include <boost/math/special_functions/erf.hpp>

IMPISD_BEGIN_NAMESPACE

namespace {

// Volume of the intersection of two balls of radii a and b whose centers
// are r apart
double get_lens_volume(double a, double b, double r) {
  if (r >= a + b) {
    return 0.;
  } else if (r <= std::abs(a - b)) {
    double rmin = std::min(a, b);
    return 4. / 3. * IMP::PI * rmin * rmin * rmin;
  } else {
    return IMP::PI * (a + b - r) * (a + b - r)
           * (r * r + 2. * r * (a + b) - 3. * (a - b) * (a - b)) / (12. * r);
  }
}

// Area of the part of a sphere of radius r, centered at the origin, that
// lies within distance length of a point at distance dist from the origin
double get_sphere_area_within(double r, double dist, double length) {
  if (r + dist <= length) {
    return 4. * IMP::PI * r * r;
  } else if (r <= dist - length || r >= dist + length) {
    return 0.;
  } else {
    return IMP::PI * r * (length * length - (r - dist) * (r - dist)) / dist;
  }
}

// Probability that two points, drawn uniformly from balls of radii sigmai
// and sigmaj whose centers are dist apart, are closer than length
// (the forward model of Erzberger et al. 2014 and Robinson et al. 2015).
// The difference of the two points has density
// lens(sigmai, sigmaj, r) / (Vi * Vj) at any point a distance r from the
// origin (Vi = 4/3 pi sigmai^3), so this is the integral over r, from 0 to
// sigmai + sigmaj, of that density times the area of the sphere of radius
// r that lies within length of a point dist away from its center.
// The lens volume changes formula at r = |sigmai - sigmaj| and
// sigmai + sigmaj, and the area at r = |dist - length| and dist + length.
// Between these breakpoints their product is a polynomial in r of degree
// at most 6 (the 1/r of the partial lens cancels against the factor r in
// both 4 pi r^2 and the spherical cap), so 4-point Gauss-Legendre
// quadrature on each piece is exact.
double get_forward_model_probability(double dist, double sigmai,
                                     double sigmaj, double length) {
  // 4-point Gauss-Legendre rule on [-1, 1], exact for polynomials of degree
  // <= 7: the nodes are the roots of P4(t) = (35 t^4 - 30 t^2 + 3) / 8 and
  // the weights are 2 / ((1 - t^2) P4'(t)^2), that is
  //   t = +-sqrt(3/7 + (2/7) sqrt(6/5)), w = (18 - sqrt(30)) / 36
  //   t = +-sqrt(3/7 - (2/7) sqrt(6/5)), w = (18 + sqrt(30)) / 36
  // (Abramowitz & Stegun 1964, Table 25.4, p. 916).
  static const double gl_x[4] = {-0.8611363115940526, -0.3399810435848563,
                                 0.3399810435848563, 0.8611363115940526};
  static const double gl_w[4] = {0.3478548451374538, 0.6521451548625461,
                                 0.6521451548625461, 0.3478548451374538};
  double rmax = sigmai + sigmaj;
  double points[5] = {0., rmax, std::abs(sigmai - sigmaj),
                      std::abs(dist - length), dist + length};
  for (double &p : points) {
    p = std::max(0., std::min(rmax, p));
  }
  std::sort(points, points + 5);
  double integral = 0.;
  for (unsigned int i = 0; i < 4; ++i) {
    double half = 0.5 * (points[i + 1] - points[i]);
    if (half <= 0.) continue;
    double mid = 0.5 * (points[i + 1] + points[i]);
    for (unsigned int k = 0; k < 4; ++k) {
      double r = mid + half * gl_x[k];
      integral += half * gl_w[k] * get_lens_volume(sigmai, sigmaj, r)
                  * get_sphere_area_within(r, dist, length);
    }
  }
  double voli = 4. / 3. * IMP::PI * sigmai * sigmai * sigmai;
  double volj = 4. / 3. * IMP::PI * sigmaj * sigmaj * sigmaj;
  return std::max(0., std::min(1., integral / voli / volj));
}

}  // namespace

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

        double pofr = get_forward_model_probability(dist, sigmai, sigmaj,
                                                    length);

        if (constr_ == 2) {
            double prior=exp(-slope_*dist);
            onemprob = onemprob * (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi)) * prior);
        } else {
            onemprob = onemprob * (1.0 - (psi * (1.0 - pofr) + pofr * (1 - psi)) );
        }    
    }

    double prob = 1.0 - onemprob;
    

    
    // std::cout << prob << std::endl ;
    return prob;
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
