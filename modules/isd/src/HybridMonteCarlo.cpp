/**
 *  \file isd/HybridMonteCarlo.cpp
 *  \brief The hybrid monte carlo algorithm
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/HybridMonteCarlo.h>

IMPISD_BEGIN_NAMESPACE

HybridMonteCarlo::HybridMonteCarlo(Model *m, Float kT, unsigned steps, Float
        timestep, unsigned persistence) : MonteCarlo(m)
{

    mv_ = new MolecularDynamicsMover(m, steps, timestep);
    add_mover(mv_);
    md_ = mv_->get_md();
    set_kt(kT);
    set_number_of_md_steps(steps);
    set_timestep(timestep);
    set_persistence(persistence);
    set_return_best(false);
    set_move_probability(1);
    persistence_counter_=0;
}

void HybridMonteCarlo::do_step()
{
    persistence_counter_ += 1;
    if (persistence_counter_ == persistence_)
    {
        persistence_counter_ = 0;
        //boltzmann constant in kcal/mol
        static const double kB = 8.31441 / 4186.6;
        md_->assign_velocities(get_kt() / kB);
    }
    double last = do_evaluate(get_model()->get_particles());
    ParticlesTemp moved = do_move(get_move_probability());
    double energy = do_evaluate(moved);
    do_accept_or_reject_move(energy, last);
}

  Float HybridMonteCarlo::get_kinetic_energy() const
{
    return md_->get_kinetic_energy();
}

  Float HybridMonteCarlo::get_potential_energy() const
{
    return get_scoring_function()->evaluate(false);
}

  Float HybridMonteCarlo::get_total_energy() const
{
    return get_kinetic_energy()+get_potential_energy();
}

  //set md timestep
  void HybridMonteCarlo::set_timestep(Float ts)
{
    md_->set_maximum_time_step(ts);
}


  double HybridMonteCarlo::get_timestep() const
{
    return md_->get_maximum_time_step();
}

  //set number of md steps per mc step
  void HybridMonteCarlo::set_number_of_md_steps(unsigned nsteps)
{
    mv_->set_number_of_md_steps(nsteps);
}

  unsigned HybridMonteCarlo::get_number_of_md_steps() const
{
    return mv_->get_number_of_md_steps();
}

  //set how many mc steps happen until you redraw the momenta
  void HybridMonteCarlo::set_persistence(unsigned persistence)
{
    persistence_ = persistence;
}

  unsigned HybridMonteCarlo::get_persistence() const
{
    return persistence_;
}

  //return pointer to isd::MolecularDynamics instance
  //useful if you want to set other stuff that is not exposed here
  MolecularDynamics* HybridMonteCarlo::get_md() const
{
    return md_;
}

void HybridMonteCarlo::do_show(std::ostream &out) const
{
    out << "HybridMonteCarlo" << std::endl;
}

IMPISD_END_NAMESPACE
