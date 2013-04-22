/**
 *  \file MolecularDynamicsMover.cpp
 *  \brief A modifier which perturbs XYZs or Nuisances with a constant energy
 *  MD simulation.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/MolecularDynamicsMover.h>
#include <IMP/isd/MolecularDynamics.h>
#include <IMP/core/XYZ.h>
#include <IMP/isd/Nuisance.h>
#include <IMP/random.h>

IMPISD_BEGIN_NAMESPACE

MolecularDynamicsMover::MolecularDynamicsMover(Model *m,
                                               unsigned nsteps,
                                               Float timestep) :
  core::MonteCarloMover(m, "MolecularDynamicsMover%1%"),
  nsteps_(nsteps)
{
    md_ = new MolecularDynamics(m);
    md_->set_maximum_time_step(timestep);
}

core::MonteCarloMoverResult MolecularDynamicsMover::do_propose()
{
    IMP_OBJECT_LOG;
    save_coordinates();
    md_->optimize(nsteps_);
    return core::MonteCarloMoverResult(md_->get_simulation_particle_indexes(),
                                        1.0);
}

void MolecularDynamicsMover::save_coordinates()
{
    IMP_OBJECT_LOG;
    ParticlesTemp ps = md_->get_simulation_particles();
    unsigned nparts = ps.size();
    coordinates_.clear();
    coordinates_.reserve(nparts);
    velocities_.clear();
    velocities_.reserve(nparts);
    for (unsigned i=0; i<nparts; i++)
    {
        bool isnuisance = Nuisance::particle_is_instance(ps[i]);
        bool isxyz = core::XYZ::particle_is_instance(ps[i]);
        IMP_USAGE_CHECK(isnuisance||isxyz,
                "Particle " << ps[i] << " is neither nuisance nor xyz!");
        if (isnuisance)
        {
            std::vector<double> x(1,
                    Nuisance(ps[i]).get_nuisance());
            coordinates_.push_back(x);
            std::vector<double> v(1,
                    ps[i]->get_value(FloatKey("vel")));
            velocities_.push_back(v);
        }
        if (isxyz)
        {
            std::vector<double> coords;
            core::XYZ d(ps[i]);
            coords.push_back(d.get_coordinate(0));
            coords.push_back(d.get_coordinate(1));
            coords.push_back(d.get_coordinate(2));
            coordinates_.push_back(coords);
            std::vector<double> v;
            v.push_back(ps[i]->get_value(FloatKey("vx")));
            v.push_back(ps[i]->get_value(FloatKey("vy")));
            v.push_back(ps[i]->get_value(FloatKey("vz")));
            velocities_.push_back(v);
        }
    }
}

void MolecularDynamicsMover::do_reject()
{
    IMP_OBJECT_LOG;
    ParticlesTemp ps = md_->get_simulation_particles();
    unsigned nparts = ps.size();
    IMP_USAGE_CHECK(coordinates_.size() == ps.size(),
            "The list of particles that move has been changed!");
    IMP_USAGE_CHECK(velocities_.size() == ps.size(),
            "The list of particles that move has been changed!");
    for (unsigned i=0; i<nparts; i++)
    {
        bool isnuisance = Nuisance::particle_is_instance(ps[i]);
        bool isxyz = core::XYZ::particle_is_instance(ps[i]);
        IMP_USAGE_CHECK(isnuisance||isxyz,
                "Particle " << ps[i] << " is neither nuisance nor xyz!");
        if (isnuisance)
        {
            IMP_USAGE_CHECK(coordinates_[i].size() == 1,
                    "wrong size for coordinates_["<<i<<"] !");
            IMP_USAGE_CHECK(velocities_[i].size() == 1,
                    "wrong size for velocities_["<<i<<"] !");
            Nuisance(ps[i]).set_nuisance(coordinates_[i][0]);
            ps[i]->set_value(FloatKey("vel"), velocities_[i][0]);
        }
        if (isxyz)
        {
            IMP_USAGE_CHECK(coordinates_[i].size() == 3,
                    "wrong size for coordinates_["<<i<<"] !");
            IMP_USAGE_CHECK(velocities_[i].size() == 3,
                    "wrong size for velocities_["<<i<<"] !");
            core::XYZ(ps[i]).set_coordinate(0, coordinates_[i][0]);
            core::XYZ(ps[i]).set_coordinate(1, coordinates_[i][1]);
            core::XYZ(ps[i]).set_coordinate(2, coordinates_[i][2]);
            ps[i]->set_value(FloatKey("vx"), velocities_[i][0]);
            ps[i]->set_value(FloatKey("vy"), velocities_[i][1]);
            ps[i]->set_value(FloatKey("vz"), velocities_[i][2]);
        }
    }
}

kernel::ModelObjectsTemp MolecularDynamicsMover::do_get_inputs() const {
    ParticleIndexes pis(md_->get_simulation_particle_indexes());
    kernel::ModelObjectsTemp ret(pis.size());
    for (unsigned int i=0; i< pis.size(); ++i) {
        ret[i] = get_model()->get_particle(pis[i]);
    }
    return ret;
}

IMPISD_END_NAMESPACE
