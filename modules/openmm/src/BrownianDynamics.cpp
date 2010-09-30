/**
 *  \file BrownianDynamics.cpp  \brief Simple Brownian dynamics optimizer.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/openmm/BrownianDynamics.h>
#include <IMP/core/XYZ.h>
#include <IMP/algebra/Vector3D.h>
#include <OpenMM.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/atom/Diffusion.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/internal/SpecialCaseRestraints.h>

IMPOPENMM_BEGIN_NAMESPACE


BrownianDynamics::BrownianDynamics(atom::SimulationParameters si,
                                   SingletonContainer *sc) :
  Optimizer(si->get_model()),
  si_(si)
{
  if (sc) sc_=sc;
}


void BrownianDynamics::do_show(std::ostream &) const {
}

SingletonContainer *BrownianDynamics::setup_particles() const
{
  if (sc_) {
    // check them
    atom::Diffusions(sc_->particles_begin(),
                     sc_->particles_end());
    return sc_;
  } else {
    container::ListSingletonContainer *lsc
      = new container::ListSingletonContainer(get_model(),
                                              "Brownian dynamics particles");
    for (Model::ParticleIterator it = get_model()->particles_begin();
         it != get_model()->particles_end(); ++it) {
      Particle *p = *it;
      atom::Diffusion d= atom::Diffusion::decorate_particle(p);
      if (d && d.get_coordinates_are_optimized()) {
        lsc->add_particle(p);
      }
    }
    return lsc;
  }
}


SingletonContainer *BrownianDynamics::get_diffusing_particles() const {
  return setup_particles();
}

bool handle_harmonic(const IMP::internal::Map<Particle*, int> &index,
                     std::vector< std::pair<int,int> > &bond_pairs,
                     OpenMM::HarmonicBondForce *bondStretch,
                     const ParticlePair &pp,
                     double x0, double k) {
  if (index.find(pp[0]) != index.end()
      && index.find(pp[1]) != index.end()) {
    bond_pairs.push_back(std::make_pair(index.find(pp[0])->second,
                                        index.find(pp[1])->second));
    bondStretch->addBond(bond_pairs.back().first,
                         bond_pairs.back().second,
                         x0*OpenMM::NmPerAngstrom,
                         k*2 * OpenMM::KJPerKcal
                         * OpenMM::AngstromsPerNm * OpenMM::AngstromsPerNm);
    return true;
  } else return false;
}

bool handle_ev() {
  return true;
}

double BrownianDynamics::optimize(unsigned int ns) {
  try {
    std::auto_ptr<OpenMM::System> system(new OpenMM::System());

    OpenMM::Platform::loadPluginsFromDirectory
      (OpenMM::Platform::getDefaultPluginsDirectory());

    OpenMM::NonbondedForce* nonbond     = new OpenMM::NonbondedForce();
    OpenMM::HarmonicBondForce* bondStretch = new OpenMM::HarmonicBondForce();
    system->addForce(nonbond);
    system->addForce(bondStretch);
    using OpenMM::Vec3;
    IMP::internal::OwnerPointer<SingletonContainer> sc
      = get_diffusing_particles();
    IMP_LOG(TERSE, "Performing BD on " << sc->get_number_of_particles()
            << " particles." << std::endl);
    std::vector<Vec3> initialPosInNm;
    IMP_FOREACH_SINGLETON(sc, {
        system->addParticle(atom::Mass(_1).get_mass());
        nonbond->addParticle(0,
                             core::XYZR(_1).get_radius()*OpenMM::NmPerAngstrom
                             * OpenMM::SigmaPerVdwRadius,
                             0      * OpenMM::KJPerKcal);// vdw energy
        // Convert the initial position to nm and append to the array.
        algebra::Vector3D v= core::XYZ(_1).get_coordinates();
        const Vec3 posInNm(v[0] * OpenMM::NmPerAngstrom,
                           v[1] * OpenMM::NmPerAngstrom,
                           v[2] * OpenMM::NmPerAngstrom);
        initialPosInNm.push_back(posInNm);
      });
    IMP::internal::Map<Particle*, int> index;
    IMP_FOREACH_SINGLETON(sc, {index[_1]= index.size();});
    get_model()->update();
    atom::internal::SpecialCaseRestraints scr(get_model(),sc->get_particles());
    std::vector< std::pair<int,int> > bondPairs;
    scr.add_restraint_set(get_model()->get_root_restraint_set(),
                          boost::bind(handle_harmonic, index, bondPairs,
                                      bondStretch,
                                      _1, _2, _3),
                          handle_ev);
    nonbond->createExceptionsFromBonds(bondPairs, 1, 1);
    //(bondPairs, Coulomb14Scale, LennardJones14Scale)
    std::auto_ptr<OpenMM::BrownianIntegrator>
      integrator(new OpenMM::BrownianIntegrator(si_.get_temperature(), 1,
                                                si_.get_maximum_time_step()
                                                * OpenMM::PsPerFs));
    std::auto_ptr<OpenMM::Context>
      context(new OpenMM::Context(*system, *integrator));
    context->setPositions(initialPosInNm);
    integrator->step(ns);

    int infoMask = 0;
    infoMask = OpenMM::State::Positions;
    // Forces are also available (and cheap).

    const OpenMM::State state = context->getState(infoMask);
    double timeInPs = state.getTime(); // OpenMM time is in ps already
    si_.set_current_time(si_.get_current_time()+ timeInPs/OpenMM::PsPerFs);
    // Copy OpenMM positions into atoms array and change units from nm
    // to Angstroms.
    const std::vector<Vec3>& positionsInNm = state.getPositions();
    unsigned int i=0;
    IMP_FOREACH_SINGLETON(sc, {
        core::XYZ d(_1);
        for (unsigned int j=0; j< 3; ++j) {
          d.set_coordinate(j, positionsInNm[i][j] * OpenMM::AngstromsPerNm);
        }
        ++i;
      });
  } catch (const std::exception& e) {
    IMP_THROW("OpenMM exception " << e.what(),
              ValueException);
  }
  //platformName = omm->context->getPlatform().getName();
  return get_model()->evaluate(false);
}


IMPOPENMM_END_NAMESPACE
