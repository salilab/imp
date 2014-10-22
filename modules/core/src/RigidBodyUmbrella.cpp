/**
 *  \file RigidBodyUmbrella.cpp \brief Distance restraint between two particles.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/RigidBodyUmbrella.h>
#include <IMP/core/XYZ.h>

#include <IMP/kernel/Particle.h>
#include <IMP/kernel/Model.h>
#include <IMP/base/log.h>
#include <IMP/algebra/eigen3/Eigen/Dense>
#include <IMP/algebra/eigen3/Eigen/Geometry>

IMPCORE_BEGIN_NAMESPACE

RigidBodyUmbrella::RigidBodyUmbrella(kernel::Model *m, kernel::ParticleIndex pi,
                                     kernel::ParticleIndex ref, Floats x0,
                                     double alpha, double k, std::string name)
    : Restraint(m, name), pi_(pi), ref_(ref), x0_(x0), alpha_(alpha),
      k_(k) {
  IMP_USAGE_CHECK(x0.size() == 7, "Wrong size for x0, should be 7");
}

RigidBodyUmbrella::RigidBodyUmbrella(kernel::Model *m, kernel::ParticleIndex pi,
                                     kernel::ParticleIndex ref, double lambda,
                                     Floats x1, Floats x2, double alpha,
                                     double k, std::string name)
    : Restraint(m, name), pi_(pi), ref_(ref),
      x0_(interpolate(lambda, x1, x2)), alpha_(alpha), k_(k) {}

internal::Coord RigidBodyUmbrella::interpolate(double lambda, Floats x1,
                                               Floats x2) const {
    //checks
    IMP_USAGE_CHECK(x1.size() == 7, "Wrong size for x1, should be 7");
    IMP_USAGE_CHECK(x2.size() == 7, "Wrong size for x2, should be 7");
    IMP_USAGE_CHECK(lambda >= 0, "lambda should be >=0");
    IMP_USAGE_CHECK(lambda <= 1, "lambda should be <=1");
    //centroid
    IMP_Eigen::Vector3d cx1,cx2,cx0;
    cx1 << x1[0], x1[1], x1[2];
    cx2 << x2[0], x2[1], x2[2];
    cx0 = (1-lambda)*cx1 + lambda*cx2;
    //quaternion
    IMP_Eigen::Quaterniond qx1(x1[3],x1[4],x1[5],x1[6]);
    IMP_Eigen::Quaterniond qx2(x2[3],x2[4],x2[5],x2[6]);
    IMP_Eigen::Quaterniond qx0(qx1.slerp(lambda,qx2));
    qx0.normalize();
    //return it
    internal::Coord x0;
    x0.coms.push_back(cx0);
    x0.quats.push_back(qx0);
    return x0;
}

double RigidBodyUmbrella::unprotected_evaluate(DerivativeAccumulator
                                               * accum) const {
    if (accum) IMP_THROW("Derivatives not implemented", ModelException);
    kernel::ParticleIndexes pis(1, pi_);
    internal::Coord x(internal::get_coordinates_from_rbs(get_model(), pis,
                ref_));
    double d2 = internal::get_squared_distance(x, x0_, k_);
    double score = 0.5 * alpha_ * d2;
    return score;
}

kernel::ModelObjectsTemp RigidBodyUmbrella::do_get_inputs() const {
    kernel::Model *m = get_model();
    ModelObjectsTemp ret;
    //reference rb
    ret.push_back(m->get_particle(ref_));
    kernel::ParticleIndexes pref(
        RigidBody(m, ref_).get_member_indexes());
    for (unsigned i=0; i<pref.size(); i++)
        ret.push_back(m->get_particle(pref[i]));
    //target rb
    ret.push_back(m->get_particle(pi_));
    kernel::ParticleIndexes iref(
        RigidBody(m, pi_).get_member_indexes());
    for (unsigned i=0; i<iref.size(); i++)
        ret.push_back(m->get_particle(iref[i]));
    return ret;
}

IMPCORE_END_NAMESPACE
