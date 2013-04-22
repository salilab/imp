/**
 *  \file Nuisance.cpp
 *  \brief defines a one-dimensional nuisance parameter (sigma, gamma, ...)
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/isd/Nuisance.h>

IMPISD_BEGIN_NAMESPACE

Nuisance Nuisance::setup_particle(Particle *p, double nuisance) {
    if (!Nuisance::particle_is_instance(p))
    {
        p->add_attribute(get_nuisance_key(), nuisance);
    } else {
        Nuisance(p).set_nuisance(nuisance);
    }
    return Nuisance(p);
}

FloatKey Nuisance::get_nuisance_key() {
  static FloatKey k("nuisance");
  return k;
}
void Nuisance::set_nuisance(Float d) {
    Float d_=d;
    Particle *p=get_particle();
    if (has_lower())
    {
        Float lo = get_lower();
        if (d < lo)  d_ = lo;
    }
    if (has_upper())
    {
        Float up = get_upper();
        if (d > up)  d_ = up;
    }
    p->set_value(get_nuisance_key(), d_);
}

bool Nuisance::has_lower() const {
  Particle * p = get_particle();
  return p->has_attribute(get_lower_key())
           || p->has_attribute(get_lower_particle_key());
}
Float Nuisance::get_lower() const {
    Particle *p = get_particle();
    bool hasfloat = p->has_attribute(get_lower_key());
    bool hasparticle = p->has_attribute(get_lower_particle_key());
    if (hasfloat)
    {
        double floatval = p->get_value(get_lower_key());
        if (hasparticle)
        {
            Particle *d = p->get_value(get_lower_particle_key());
            double particleval = d->get_value(get_nuisance_key());
            if (floatval > particleval)
            {
                return floatval;
            } else {
                return particleval;
            }
        } else {
            return floatval;
        }
    } else if (hasparticle) {
        Particle *d = p->get_value(get_lower_particle_key());
        return d->get_value(get_nuisance_key());
    } else {
        return - std::numeric_limits<double>::infinity();
    }
  }
FloatKey Nuisance::get_lower_key() {
  static FloatKey k("lower");
  return k;
}
ParticleIndexKey Nuisance::get_lower_particle_key() {
  static ParticleIndexKey k("lower");
  return k;
}
void Nuisance::set_lower(Float d) {
    Particle * p = get_particle();
    if (! p->has_attribute(get_lower_key()))
            p->add_attribute(get_lower_key(), d);
    p->set_value(get_lower_key(), d);
    enforce_bounds();
}
void Nuisance::set_lower(Particle * d) {
    IMP_IF_CHECK(USAGE_AND_INTERNAL) { Nuisance::decorate_particle(d); }
    Particle * p = get_particle();
    if (! p->has_attribute(get_lower_particle_key()))
            p->add_attribute(get_lower_particle_key(), d);
    p->set_value(get_lower_particle_key(), d);
    enforce_bounds();
}
void Nuisance::remove_lower(){
    Particle * p = get_particle();
    if (p->has_attribute(get_lower_key()))
            p->remove_attribute(get_lower_key());
    if (p->has_attribute(get_lower_particle_key()))
            p->remove_attribute(get_lower_particle_key());
    remove_bounds();
}

bool Nuisance::has_upper() const {
  Particle * p = get_particle();
  return p->has_attribute(get_upper_key())
           || p->has_attribute(get_upper_particle_key());
}
Float Nuisance::get_upper() const {
    Particle *p = get_particle();
    bool hasfloat = p->has_attribute(get_upper_key());
    bool hasparticle = p->has_attribute(get_upper_particle_key());
    if (hasfloat)
    {
        double floatval = p->get_value(get_upper_key());
        if (hasparticle)
        {
            Particle *d = p->get_value(get_upper_particle_key());
            double particleval = d->get_value(get_nuisance_key());
            if (floatval < particleval)
            {
                return floatval;
            } else {
                return particleval;
            }
        } else {
            return floatval;
        }
    } else if (hasparticle) {
        Particle *d = p->get_value(get_upper_particle_key());
        return d->get_value(get_nuisance_key());
    } else {
        return std::numeric_limits<double>::infinity();
    }
  }
FloatKey Nuisance::get_upper_key() {
  static FloatKey k("upper");
  return k;
}
ParticleIndexKey Nuisance::get_upper_particle_key() {
  static ParticleIndexKey k("upper");
  return k;
}
void Nuisance::set_upper(Float d) {
    Particle * p = get_particle();
    if (! p->has_attribute(get_upper_key()))
            p->add_attribute(get_upper_key(), d);
    p->set_value(get_upper_key(), d);
    enforce_bounds();
}
void Nuisance::set_upper(Particle * d) {
    IMP_IF_CHECK(USAGE_AND_INTERNAL) { Nuisance::decorate_particle(d); }
    Particle * p = get_particle();
    if (! p->has_attribute(get_upper_particle_key()))
            p->add_attribute(get_upper_particle_key(), d);
    p->set_value(get_upper_particle_key(), d);
    enforce_bounds();
}
void Nuisance::remove_upper(){
    Particle * p = get_particle();
    if (p->has_attribute(get_upper_key()))
            p->remove_attribute(get_upper_key());
    if (p->has_attribute(get_upper_particle_key()))
            p->remove_attribute(get_upper_particle_key());
    remove_bounds();
}

void Nuisance::show(std::ostream &out) const {
  if (has_lower()) out << get_lower() << " < ";
  out << " Nuisance = " << get_nuisance();
  if (has_upper()) out << " < " << get_upper();
}

ObjectKey Nuisance::get_ss_key(){
    static ObjectKey k("nuisance_ss");
    return k;
}
void Nuisance::enforce_bounds(){
    Particle* p=get_particle();
    ObjectKey k(get_ss_key());
    if (p->has_attribute(k)) return;
    NuisanceScoreState* ss = new NuisanceScoreState(p);
    p->get_model()->add_score_state(ss);
    p->add_attribute(k,ss);
}
void Nuisance::remove_bounds(){
    Particle* p=get_particle();
    ObjectKey k(get_ss_key());
    if (p->has_attribute(k)) return;
    NuisanceScoreState* ss(static_cast<NuisanceScoreState*>(p->get_value(k)));
    p->get_model()->remove_score_state(ss);
    p->remove_attribute(k);
    delete ss;
}

void NuisanceScoreState::do_before_evaluate()
{
    IMP_LOG_TERSE( "NSS: do_before_evaluate()" << std::endl);
    Nuisance nuis(Nuisance::decorate_particle(p_));
    nuis.set_nuisance(nuis.get_nuisance());
}
void NuisanceScoreState::do_after_evaluate(DerivativeAccumulator *) { }
ContainersTemp NuisanceScoreState::get_input_containers() const {
  return ContainersTemp();
}
ContainersTemp NuisanceScoreState::get_output_containers() const {
  return ContainersTemp();
}
ParticlesTemp NuisanceScoreState::get_input_particles() const {
  //gpir needs to update internal values computed from particles
  return ParticlesTemp(1,p_);
}
ParticlesTemp NuisanceScoreState::get_output_particles() const {
  //gpir does not change particles' attributes.
  return ParticlesTemp();
}
void NuisanceScoreState::do_show(std::ostream &out) const
{
    out << "Nuisance Score State" << std::endl;
}

IMPISD_END_NAMESPACE
