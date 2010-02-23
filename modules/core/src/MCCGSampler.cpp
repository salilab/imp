/**
 *  \file ConjugateGradients.cpp  \brief Simple conjugate gradients optimizer.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include <IMP/core/MCCGSampler.h>

#include <IMP/core/ConjugateGradients.h>
#include <IMP/core/MonteCarlo.h>
#include <IMP/core/BallMover.h>
#include <IMP/core/XYZ.h>
#include <IMP/core/internal/CoreListSingletonContainer.h>
#include <IMP/core/IncrementalBallMover.h>
#include <IMP/algebra/vector_generators.h>


IMPCORE_BEGIN_NAMESPACE

#define XK XYZ::get_xyz_keys()[0]
#define YK XYZ::get_xyz_keys()[1]
#define ZK XYZ::get_xyz_keys()[2]

MCCGSampler::Parameters::Parameters(){
  cg_steps_=10;
  attempts_=1000;
  mc_steps_=10000;
}

MCCGSampler::MCCGSampler(Model *m): Sampler(m, "MCCG Sampler %1"){
}

void MCCGSampler::set_bounding_box(const algebra::BoundingBox3D &bb) {
  FloatKeys xyzks= XYZ::get_xyz_keys();
  for (unsigned int i=0; i< xyzks.size(); ++i) {
    default_parameters_.bounds_[xyzks[i]]= std::make_pair(bb.get_corner(0)[i],
                                      bb.get_corner(1)[i]);
  }
}

void MCCGSampler::set_number_of_attempts(unsigned int at) {
  default_parameters_.attempts_=at;
}

void MCCGSampler::set_number_of_conjugate_gradient_steps(unsigned int cg) {
  default_parameters_.cg_steps_=cg;
}

void MCCGSampler::set_number_of_monte_carlo_steps(unsigned int cg) {
  default_parameters_.mc_steps_=cg;
}

void MCCGSampler::set_max_monte_carlo_step_size(double d) {
  default_parameters_.ball_sizes_[XK]=d;
  default_parameters_.ball_sizes_[YK]=d;
  default_parameters_.ball_sizes_[ZK]=d;
}

void MCCGSampler::set_max_monte_carlo_step_size(FloatKey k, double d) {
  default_parameters_.ball_sizes_[k]=d;
}

internal::CoreListSingletonContainer*
MCCGSampler::set_up_movers(const Parameters &pms,
                           MonteCarlo *mc) const {
  if (pms.opt_keys_[0] != XK
      && pms.opt_keys_[1] != YK && pms.opt_keys_[2] != ZK){
    IMP_THROW("Currently, the MCCGSampler can only handle "
              << "cartesian coordinates",
              ValueException);
  }
  ParticlesTemp ps;
  for (Model::ParticleIterator pit= mc->get_model()->particles_begin();
       pit != mc->get_model()->particles_end(); ++pit) {
    if (XYZ::particle_is_instance(*pit)
        && XYZ(*pit).get_coordinates_are_optimized()) {
      ps.push_back(*pit);
    }
  }
  IMP_NEW(internal::CoreListSingletonContainer, sc, ("mccg particles"));
  sc->set_particles(ps);
  IMP_NEW(IncrementalBallMover, bm,
          (sc, 2, pms.ball_sizes_.find(XK)->second));
  mc->add_mover(bm);
  return sc.release();
}

void MCCGSampler::randomize(const Parameters &pms,
                            internal::CoreListSingletonContainer *sc) const {
  algebra::BoundingBox3D
    bb(algebra::VectorD<3>(pms.bounds_.find(XK)->second.first,
                         pms.bounds_.find(YK)->second.first,
                         pms.bounds_.find(ZK)->second.first),
       algebra::VectorD<3>(pms.bounds_.find(XK)->second.second,
                         pms.bounds_.find(YK)->second.second,
                         pms.bounds_.find(ZK)->second.second));
  for (unsigned int i=0; i< sc->get_number_of_particles(); ++i) {
    XYZ d(sc->get_particle(i));
    d.set_coordinates(algebra::get_random_vector_in(bb));
  }
}


MCCGSampler::Parameters MCCGSampler::fill_in_parameters() const {
  Parameters pms= default_parameters_;
  pms.opt_keys_= XYZ::get_xyz_keys();
  if (pms.bounds_.empty()) {
    pms.bounds_[XK]= get_model()->get_range(XK);
    pms.bounds_[YK]= get_model()->get_range(YK);
    pms.bounds_[ZK]= get_model()->get_range(ZK);
  }
  // we have mc_steps_ steps to explore the whole space
  // each time we move a constant number of the particles
  // (r/l)^(na*np) is the number of voxels. this number
  // should be similar to number of steps
  // so l= r* pow(cg_steps_, 1/(na*np))
  // for now na=3, assume half the particles are optimized
  for (Parameters::Bounds::const_iterator it= pms.bounds_.begin();
       it != pms.bounds_.end(); ++it) {
    if (pms.ball_sizes_.find(it->first) == pms.ball_sizes_.end()) {
      double r= it->second.second-it->second.first;
      double l= r*std::pow(static_cast<double>(pms.cg_steps_), 1.0/(3*1000));
      pms.ball_sizes_[it->first]=l;
    }
  }
  return pms;
}

ConfigurationSet *MCCGSampler::sample() const {
  IMP_OBJECT_LOG;
  set_was_owned(true);
  get_model()->set_is_incremental(true);
  Pointer<ConfigurationSet> ret= new ConfigurationSet(get_model());
  Parameters pms= fill_in_parameters();
  IMP_NEW(MonteCarlo, mc, (get_model()));
  IMP_NEW(ConjugateGradients, cg, (get_model()));
  mc->set_local_optimizer(cg);
  mc->set_local_steps(pms.cg_steps_);
  mc->set_score_threshold(get_maximum_score()/2.0);
  mc->set_return_best(true);
  Pointer<internal::CoreListSingletonContainer> sc=set_up_movers(pms, mc);
  IMP_CHECK_OBJECT(sc);
  int failures=0;
  for (unsigned int i=0; i< pms.attempts_; ++i) {
    ret->set_configuration(-1);
    IMP_IF_LOG(TERSE) {
      IMP_LOG(TERSE, "Restored configuration to:\n");
      for (Model::ParticleIterator it= get_model()->particles_begin();
           it != get_model()->particles_end(); ++it) {
        IMP_LOG_WRITE(TERSE, (*it)->show(IMP_STREAM));
      }
    }
    randomize(pms,sc);
    IMP_IF_LOG(TERSE) {
      IMP_LOG(TERSE, "Randomized configuration to:\n");
      for (Model::ParticleIterator it= get_model()->particles_begin();
           it != get_model()->particles_end(); ++it) {
        IMP_LOG_WRITE(TERSE, (*it)->show(IMP_STREAM));
      }
    }
    try {
      mc->optimize(pms.mc_steps_);
    } catch (ModelException) {
      IMP_LOG(TERSE, "Optimization ended by exception" << std::endl);
      ++failures;
      continue;
    }
    if (get_is_good_configuration()) {
      IMP_IF_LOG(TERSE) {
        IMP_LOG(TERSE, "Found configuration:\n");
        for (Model::ParticleIterator it= get_model()->particles_begin();
             it != get_model()->particles_end(); ++it) {
          IMP_LOG_WRITE(TERSE, (*it)->show(IMP_STREAM));
        }
        IMP_LOG(TERSE, "Energy is " << get_model()->evaluate(false)
                << std::endl);
      }
      ret->save_configuration();
      IMP_IF_CHECK(USAGE_AND_INTERNAL) {
        double oe= get_model()->evaluate(false);
        ret->set_configuration(-1);
        ret->set_configuration(ret->get_number_of_configurations()-1);
        double ne= get_model()->evaluate(false);
        IMP_INTERNAL_CHECK(std::abs(ne-oe) < (ne+oe)*.1+.1,
                           "Energies to not match before and after save."
                           << "Expected " << oe << " got " << ne
                           << std::endl);
      }
    }
  }
  if (failures != 0) {
    IMP_LOG(WARNING, "The optimization failed " << failures
            << " times due to invalid attribute values or derivatives."
            << std::endl);
  }
  IMP_CHECK_OBJECT(mc);
  IMP_CHECK_OBJECT(cg);
  IMP_CHECK_OBJECT(sc);
  return ret.release();
}

void MCCGSampler::do_show(std::ostream &out) const {
  out << "attempts " << default_parameters_.attempts_ << std::endl;
}

IMPCORE_END_NAMESPACE
