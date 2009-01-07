/**
 *  \file BipartiteNonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/core/BipartiteNonbondedListScoreState.h>
#include <IMP/core/internal/bbox_nbl_helpers.h>
#include <IMP/core/XYZDecorator.h>
#include <IMP/deprecation.h>
#include <IMP/internal/utility.h>

#include <vector>

IMPCORE_BEGIN_NAMESPACE

//! Turn the default into an actual algorithm and work around missing algorithms
/** This cannot be shared with AllNBL because that one has grid and this does
    not.
 */
static BipartiteNonbondedListScoreState::Algorithm
translate_algorithm(BipartiteNonbondedListScoreState::Algorithm a)
{
#ifdef IMP_USE_CGAL
  switch (a) {
  case BipartiteNonbondedListScoreState::DEFAULT:
    return BipartiteNonbondedListScoreState::BBOX;
  default:
    return a;
  }
#else
  switch (a) {
  case BipartiteNonbondedListScoreState::BBOX:
    IMP_WARN("BipartiteNonbondedListScoreState::BBOX requires CGAL support. "
             << "GRID used instead." << std::endl);
  case BipartiteNonbondedListScoreState::DEFAULT:
    return BipartiteNonbondedListScoreState::QUADRATIC;
  default:
    return a;
  }
#endif
}



BipartiteNonbondedListScoreState
::BipartiteNonbondedListScoreState(Float cut,
                                   FloatKey rk,
                                   Algorithm a):
  P(cut, rk), a_(translate_algorithm(a))
{
  FloatKeys ks;
  ks.push_back(rk);
  mc0_= new MaxChangeScoreState(XYZDecorator::get_xyz_keys());
  mc1_= new MaxChangeScoreState(XYZDecorator::get_xyz_keys());
  mcr_= new MaxChangeScoreState(ks);

  IMP_DEPRECATED(BipartiteNonbondedListScoreState,
                 CloseBipartitePairsScoreState);
}

BipartiteNonbondedListScoreState
::BipartiteNonbondedListScoreState(Float cut, FloatKey rk,
                                   const Particles &ps0,
                                   const Particles &ps1,
                                   Algorithm a):
  P(cut, rk), a_(translate_algorithm(a))
{
  FloatKeys ks;
  ks.push_back(rk);
  mc0_= new MaxChangeScoreState(XYZDecorator::get_xyz_keys());
  mc1_= new MaxChangeScoreState(XYZDecorator::get_xyz_keys());
  mcr_= new MaxChangeScoreState(ks);
  set_particles(ps0, ps1);
}


BipartiteNonbondedListScoreState::~BipartiteNonbondedListScoreState() {}

void BipartiteNonbondedListScoreState::do_before_evaluate()
{
  mc0_->before_evaluate(ScoreState::get_before_evaluate_iteration());
  mc0_->before_evaluate(ScoreState::get_before_evaluate_iteration());
  mcr_->before_evaluate(ScoreState::get_before_evaluate_iteration());
  Float mc= std::max(mc0_->get_max(), mc1_->get_max());

  Float cost;
  switch (a_){
  case QUADRATIC:
    cost= 10*mc0_->get_number_of_particles()* mc1_->get_number_of_particles();
    break;
  case BBOX:
    cost= 1000 * mcr_->get_number_of_particles();
    break;
  default:
    IMP_assert(0, "Bad algorithm");
    cost= 1000 * mcr_->get_number_of_particles();
  }

  if (P::update(mc+ mcr_->get_max(), cost)) {
    mc0_->reset();
    mc1_->reset();
    mcr_->reset();
  }
  IMP_IF_CHECK(EXPENSIVE) {
    check_nbl();
  }
}

void BipartiteNonbondedListScoreState::process_sets(const Particles &p0,
                                                    const Particles &p1)
{
  switch (a_) {
  case QUADRATIC:
    for (unsigned int j=0; j< p0.size(); ++j) {
      for (unsigned int i=0; i< p1.size(); ++i) {
        P::add_if_box_overlap(p1[i], p0[j]);
      }
    }
    break;
  case BBOX:
    internal::bipartite_bbox_scan(p0, p1, P::get_radius_key(),
                                  P::get_slack(), P::get_cutoff(),
                                  internal::NBLAddPairIfNonbonded(this));
    break;
  default:
    IMP_failure("Can't find algorithm in BipartiteNonbondedListScoreState",
                ErrorException);
  }
}

void BipartiteNonbondedListScoreState::rebuild_nbl()
{
  IMP_LOG(TERSE, "Rebuilding BNBL with cutoff "
          << P::get_cutoff() << " and slack " << P::get_slack() << std::endl);
  process_sets(mc0_->get_particles(),  mc1_->get_particles());
  P::set_nbl_is_valid(true);
  IMP_LOG(TERSE, "NBL has " << P::get_number_of_nonbonded()
          << " pairs" << std::endl);
}

void BipartiteNonbondedListScoreState::set_algorithm(Algorithm a)
{
  a_=translate_algorithm(a);
}


void BipartiteNonbondedListScoreState::set_particles(const Particles &ps0,
                                                     const Particles &ps1)
{
  mc0_->clear_particles();
  mc0_->add_particles(ps0);
  mc1_->clear_particles();
  mc1_->add_particles(ps1);
  mcr_->clear_particles();
  mcr_->add_particles(P::particles_with_radius(ps0));
  mcr_->add_particles(P::particles_with_radius(ps1));
  P::set_nbl_is_valid(false);
}


void BipartiteNonbondedListScoreState::add_particles_0(const Particles &ps)
{
  if (P::get_nbl_is_valid()) process_sets(ps, mc1_->get_particles());
  mc0_->add_particles(ps);
  mcr_->add_particles(P::particles_with_radius(ps));
  P::set_nbl_is_valid(false);
}

void BipartiteNonbondedListScoreState::add_particles_1(const Particles &ps)
{
  if (P::get_nbl_is_valid()) process_sets(ps, mc0_->get_particles());
  mc1_->add_particles(ps);
  mcr_->add_particles(P::particles_with_radius(ps));
  P::set_nbl_is_valid(false);
}

void BipartiteNonbondedListScoreState::clear_particles()
{
  mc0_->clear_particles();
  mc1_->clear_particles();
  mcr_->clear_particles();
  P::set_nbl_is_valid(false);
  P::set_nbl_is_valid(true);
}


void BipartiteNonbondedListScoreState::show(std::ostream &out) const
{
  out << "BipartiteNonbondedListScoreState" << std::endl;
}

void BipartiteNonbondedListScoreState::check_nbl() const
{
  const Particles &ps0= mc0_->get_particles();
  const Particles &ps1= mc1_->get_particles();
  GetRadius gr= P::get_radius_object();
  for (unsigned int i=0; i< ps0.size(); ++i) {
    XYZDecorator di= XYZDecorator::cast(ps0[i]);
    for (unsigned int j=0; j< ps1.size(); ++j) {
      XYZDecorator dj= XYZDecorator::cast(ps1[j]);
      IMP_assert(ps0[i] != ps1[j], "Duplicate particles in list");
      if (distance(di, dj) - gr(ps0[i]) - gr(ps1[j])
          <= P::get_cutoff() && !are_bonded(ps0[i], ps1[j])) {
        bool found=false;
        for (NonbondedIterator nit= P::nonbonded_begin();
             nit != P::nonbonded_end(); ++nit) {
          if ((nit->first == ps0[i] && nit->second == ps1[j])
              || (nit->first == ps1[j] && nit->second == ps0[i])) {
            IMP_assert(!found, "Entry is in list twice");
            found=true;
          }
        }
        IMP_assert(found, "Nonbonded list is missing "
                   << ps0[i]->get_index() << " " << di
                   << " " << gr(ps0[i])
                   << " and " << ps1[j]->get_index() << " "
                   << dj << gr(ps1[j])
                   << " size is " << get_number_of_nonbonded() << std::endl);
      }
    }
  }
}

IMPCORE_END_NAMESPACE
