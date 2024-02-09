/**
 *  \file SlabWithToroidalPoreGoPairScore.cpp
 *  \brief A Harmonic Score on the z distance above a surface with a spherical indent
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include <IMP/npc/SlabWithToroidalPoreGoPairScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>
//#include <boost/lambda/lambda.hpp>

IMPNPC_BEGIN_NAMESPACE

SlabWithToroidalPoreGoPairScore::SlabWithToroidalPoreGoPairScore(double x0, double k) : x0_(x0), k_(k) {}

// return score for a given particle pair
double SlabWithToroidalPoreGoPairScore::evaluate_index(Model *m,
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

    IMP_USAGE_CHECK(d1.get_minor_radius_h2v_aspect_ratio() == 1.0, "Toroidal Go-like Score must be used with aspect ratio of 1.0.");

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
    // compute score as function of z-height relative to the plane
    if (rxy > rpre*rpre) {
        double z0;
        // compute height relative to the pore slab
        if (z > 0.0) {
           z0 = 0.5*zpre + x0_;
        }

        else {
           z0 = -0.5*zpre - x0_;
        }

        // compute score
        score += 0.5 * k_ * square(z - z0);
        // compute derivative
        if (da) {
            dv = -k_ * (z - z0);
            algebra::Vector3D udelta = algebra::Vector3D(0.0, 0.0, 1.0);
            d2.add_to_derivatives(udelta * dv, *da);
        }

    }

    else {
        // compute vector towards radial origin of the toroidal region
        // ASSUMES NON-ELLIPSOIDAL TOROID, i.e. aspect ratio = 1

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
        double d0 = rmin + x0_;

        // compute score

        score += 0.5 * k_ * square(dist - d0);

        if (da) {
            dv = -k_ * (dist - d0);
            algebra::Vector3D udelta = algebra::Vector3D(x-rscl*x, y-rscl*y, z);
            d2.add_to_derivatives(udelta * dv, *da);

        }
    }

    //IMP_LOG_PROGRESS("OUTPUT SCORE: " << score << "\n");
    return score;
}


/*
Double SlabWithToroidalPoreGoPairScore::get_surface_distance(algebra::Vector3D* coord) const {

    double ret;
    // for now just do distance in to restrain the z-distance from the surface
    double z = coord[2];

    ret = ()

}
*/

ModelObjectsTemp SlabWithToroidalPoreGoPairScore::do_get_inputs(
        Model *m, const ParticleIndexes &pis) const {
    return IMP::get_particles(m, pis);
}

IMPNPC_END_NAMESPACE

