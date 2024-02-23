/**
 *  \file ToroidalPoreSurfaceDepthPairScore.cpp
 *  \brief A Harmonic Score on the z distance above a surface with a spherical indent
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include <IMP/npc/ToroidalPoreSurfaceDepthPairScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>
//#include <boost/lambda/lambda.hpp>

IMPNPC_BEGIN_NAMESPACE

ToroidalPoreSurfaceDepthPairScore::ToroidalPoreSurfaceDepthPairScore(double k) : k_(k) {}

// return score for a given particle pair
double ToroidalPoreSurfaceDepthPairScore::evaluate_index(Model *m,
                                                       const ParticleIndexPair &pip,
                                                       DerivativeAccumulator *da) const {

    // turn on logging for this method
    //IMP_OBJECT_LOG;

    // check that the first particle is the spherical indent
    IMP_USAGE_CHECK(SlabWithToroidalPore::get_is_setup(m, pip[0]) , "First particle passed is not of type SlabWithToroidalPore");

    //
    // assume they have coordinates
    SlabWithToroidalPore d1(m, pip[0]);
    core::XYZ d2(m, pip[1]);

    double x = d2.get_coordinate(0);
    double y = d2.get_coordinate(1);
    double z = d2.get_coordinate(2);

    // get the pore radius
    double rpre = d1.get_pore_radius();
    // get the slab thickness, is the distance between planes
    double zpre = d1.get_thickness();

    // check if optimized.
    //if (!d2.get_coordinates_are_optimized()) {
    //    return false;
    //}
    // could write a sub routine here but let's code it inline
    //double score=get_surface_distance()
    double score = 0.0;
    double dv;

    // compute if the particle is in the toroidal pore region in xy-plane
    double rxy = x*x + y*y;

    // if the particle is outside toroidal pore region
    // compute score just as function of z-height relative to the plane
    if (rxy > rpre*rpre) {
        // compute height relative to the pore slab
        bool is_above = z > 0.5*zpre;
        bool is_below = z < -0.5*zpre;
        // if outside slab, continue
        if (is_above || is_below) {
        }
        else {
            double z0;
            if (z >= 0.0) {
                z0 = 0.5*zpre;
            }
            else if (z < 0.0) {
                z0 = -0.5*zpre;
            }

            score += 0.5 * k_ * square(z - z0);

            if (da) {
                dv = -k_ * (z - z0);
                algebra::Vector3D udelta = algebra::Vector3D(0.0, 0.0, 1.0);
                d2.add_to_derivatives(udelta * dv, *da);
            }
        }

    }

    else {
        // compute vector towards radial origin of the toroidal region
        double r = std::sqrt(rxy);
        // cutoff for numerical overflows
        if (r < 1e-9) {
            r = 1e-9;
        }
        double rscl = rpre / r;
        // compute distance between particle and major radius of torus
        double dist;
        dist = std::sqrt((x-rscl*x)*(x-rscl*x) + (y-rscl*y)*(y-rscl*y) + z*z);

        double rmin = d1.get_vertical_minor_radius();

        // if particle is not under membrane, no score contribution
        if (dist < rmin) {
           score += 0.5 * k_ * square(dist - rmin);

           if (da) {
               dv = k_ * (dist - rmin);
               algebra::Vector3D udelta = algebra::Vector3D(x-rscl*x, y-rscl*y, z);
               d2.add_to_derivatives(udelta * dv, *da);
           }
        }
    }

    //IMP_LOG_PROGRESS("OUTPUT SCORE: " << score << "\n");
    return score;
}

ModelObjectsTemp ToroidalPoreSurfaceDepthPairScore::do_get_inputs(
        Model *m, const ParticleIndexes &pis) const {
    return IMP::get_particles(m, pis);
}

IMPNPC_END_NAMESPACE

