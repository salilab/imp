/**
 *  \file SlabWithToroidalPoreMBMScore.cpp
 *  \brief A Harmonic Score on the z distance above a surface with a spherical indent
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 */

#include <IMP/npcassembly/SlabWithToroidalPoreMBMScore.h>
#include <IMP/core/XYZ.h>
#include <IMP/UnaryFunction.h>
//#include <boost/lambda/lambda.hpp>

IMPNPCASSEMBLY_BEGIN_NAMESPACE

SlabWithToroidalPoreMBMScore::SlabWithToroidalPoreMBMScore(double x0_upper, double x0_lower, double k) : x0_upper_(x0_upper), x0_lower_(x0_lower), k_(k) {}

// return score for a given particle pair
double SlabWithToroidalPoreMBMScore::evaluate_index(Model *m,
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
    // specifically, applies harmonic "walls" to enforce that particle
    // is between x0_upper and x0_lower
    if (rxy > rpre*rpre) {
        double z0_upper, z0_lower;
        // compute height of upper and lower references
        if (z > 0.0) {
            z0_upper = 0.5*zpre + x0_upper_;
	    z0_lower = 0.5*zpre + x0_lower_;
	    	
	    // compute score for top slab of membrane
	    if (z > z0_upper) {
	        score += 0.5 * k_ * square(z - z0_upper);

		// compute derivative
		dv = -k_ * (z - z0_upper); 
	    }
	    else if (z < z0_lower) {
	        score += 0.5 * k_ * square(z - z0_lower);

		// compute derivative
		dv = -k_ * (z - z0_lower);
	    } 
        }

        else {
            z0_upper = -0.5*zpre - x0_upper_;
            z0_lower = -0.5*zpre - x0_lower_;

	    // compute score for bottom side of membrane
	    if (z < z0_upper) {
	        score += 0.5 * k_ * square(z - z0_upper);
		
                // compute derivative
		dv = -k_ * (z - z0_upper);
	    }
	    else if (z > z0_lower) {
	        score += 0.5 * k_ * square(z - z0_lower);

		// compute derivative
		dv = -k_ * (z - z0_lower);
	    }
        }

        // compute derivative using computed dv 
        if (da) {
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
        double d0_upper = rmin + x0_upper_;
        double d0_lower = rmin + x0_lower_;

        // compute score and derivative 
        if (dist > d0_upper) { 
            score += 0.5 * k_ * square(dist - d0_upper);
            dv = -k_ * (dist - d0_upper);
	}
	else if (dist < d0_lower) {
            score += 0.5 * k_ * square(dist - d0_lower);
            dv = -k_ * (dist - d0_lower);
	}

        if (da) {
            algebra::Vector3D udelta = algebra::Vector3D(x-rscl*x, y-rscl*y, z);
            d2.add_to_derivatives(udelta * dv, *da);
        }
    }

    //IMP_LOG_PROGRESS("OUTPUT SCORE: " << score << "\n");
    return score;
}


/*
Double SlabWithToroidalPoreMBMScore::get_surface_distance(algebra::Vector3D* coord) const {

    double ret;
    // for now just do distance in to restrain the z-distance from the surface
    double z = coord[2];

    ret = ()

}
*/

ModelObjectsTemp SlabWithToroidalPoreMBMScore::do_get_inputs(
        Model *m, const ParticleIndexes &pis) const {
    return IMP::get_particles(m, pis);
}

IMPNPCASSEMBLY_END_NAMESPACE

