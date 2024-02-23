/**
 *  \file SphericalIndentSurfaceDepthPairScore.cpp
 *  \brief
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include <IMP/npc/SphericalIndentSurfaceDepthPairScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>
//#include <boost/lambda/lambda.hpp>

IMPNPC_BEGIN_NAMESPACE

SphericalIndentSurfaceDepthPairScore::SphericalIndentSurfaceDepthPairScore(double k) : k_(k) {}

// return score for a given particle pair
double SphericalIndentSurfaceDepthPairScore::evaluate_index(Model *m,
                                       const ParticleIndexPair &pip,
                                       DerivativeAccumulator *da) const {

    // turn on logging for this method
    //IMP_OBJECT_LOG;

    // check that the first particle is the spherical indent
    IMP_USAGE_CHECK(SlabWithSphericalIndent::get_is_setup(m, pip[0]) , "First particle passed is not of type SlabWithSphericalIndent");

    //
    // assume they have coordinates
    SlabWithSphericalIndent d1(m, pip[0]);
    core::XYZ d2(m, pip[1]);

    double x = d2.get_coordinate(0);
    double y = d2.get_coordinate(1);
    double z = d2.get_coordinate(2);

    // check if optimized.
    //if (!d2.get_coordinates_are_optimized()) {
    //    return false;
    //}
    // could write a sub routine here but let's code it directly
    //double score=get_surface_distance()
    double score;
    double dv;
    double r_norm;

    // get the surface distance.
    double R = d1.get_sphere_radius();
    double h = d1.get_sphere_depth();

    // compute the base circe radius
    double a_sqrd = h*(2.0*R-h);

    // compute score for being above the indent
    if ((x*x+y*y) < a_sqrd) {
        // if z is above plane, ignore
        if (z < 0.0) {
            score = 0.0;
        }
        else {
            r_norm = sqrt(x*x + y*y + square(z - R + h));
            if (r_norm > R) {
                score = 0.5 * k_ * square(r_norm - R);
                // do derivatives
                if (da) {
                    dv = -k_ * (r_norm - R);
                    algebra::Vector3D udelta = algebra::Vector3D(x, y, z-R+h); // vec to sphere center
                    d2.add_to_derivatives(udelta * dv / r_norm, *da);
                }
            }
            else {
                score = 0.0;
            }

        }
    }
    // compute score for being outside the indent
    else {

        // if particle is above xy plane then no contribution
        if (z < 0.0) {
            score = 0.0;
        }
        else {
            score = 0.5 * k_ * z*z;

            // do derivatives
            if (da) {
                dv = -k_ * z;
                algebra::Vector3D udelta = algebra::Vector3D(0.0, 0.0, 1.0);
                d2.add_to_derivatives(udelta * dv, *da);
            }
        }
    }


    return score;
}


ModelObjectsTemp SphericalIndentSurfaceDepthPairScore::do_get_inputs(
        Model *m, const ParticleIndexes &pis) const {
    return IMP::get_particles(m, pis);
}

IMPNPC_END_NAMESPACE

