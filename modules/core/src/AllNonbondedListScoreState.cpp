/**
 *  \file AllNonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of s.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include <IMP/core/AllNonbondedListScoreState.h>
#include <IMP/core/MaxChangeScoreState.h>
#include <IMP/core/internal/ParticleGrid.h>
#include <IMP/core/internal/bbox_nbl_helpers.h>
#include <IMP/core/XYZDecorator.h>

#include <IMP/internal/utility.h>

#include <vector>

IMPCORE_BEGIN_NAMESPACE

//! Turn the default into an actual algorithm and work around missing algorithms
static AllNonbondedListScoreState::Algorithm
translate_algorithm(AllNonbondedListScoreState::Algorithm a)
{
#ifdef IMP_USE_CGAL
  switch (a) {
  case AllNonbondedListScoreState::DEFAULT:
    return AllNonbondedListScoreState::BBOX;
  default:
    return a;
  }
#else
  switch (a) {
  case AllNonbondedListScoreState::BBOX:
    IMP_WARN("AllNonbondedListScoreState::BBOX requires CGAL support. "
             << "GRID used instead." << std::endl);
  case AllNonbondedListScoreState::DEFAULT:
    return AllNonbondedListScoreState::GRID;
  default:
    return a;
  }
#endif
}


AllNonbondedListScoreState::AllNonbondedListScoreState(Float cut,
                                                       FloatKey rk,
                                                       const Particles &ps,
                                                       Algorithm a):
  P(cut, rk), a_(translate_algorithm(a))
{
  FloatKeys ks;
  ks.push_back(rk);
  mc_= new MaxChangeScoreState(XYZDecorator::get_xyz_keys());
  mcr_= new MaxChangeScoreState(ks);
  add_particles(ps);
}

AllNonbondedListScoreState::AllNonbondedListScoreState(Float cut,
                                                       FloatKey rk,
                                                       Algorithm a):
  P(cut, rk), a_(translate_algorithm(a))
{
  FloatKeys ks;
  ks.push_back(rk);
  mc_= new MaxChangeScoreState(XYZDecorator::get_xyz_keys());
  mcr_= new MaxChangeScoreState(ks);
}

AllNonbondedListScoreState::~AllNonbondedListScoreState()
{
}

void AllNonbondedListScoreState::do_before_evaluate()
{
  mc_->before_evaluate(ScoreState::get_before_evaluate_iteration());
  mcr_->before_evaluate(ScoreState::get_before_evaluate_iteration());
  Float mc=mc_->get_max()+ mcr_->get_max();
  Float cost;
  switch (a_){
  case QUADRATIC:
    cost = 10 * square(mc_->get_number_of_particles());
    break;
  case BBOX:
    cost = 1000 * mc_->get_number_of_particles();
    break;
  case GRID:
    // completely made up
    cost = 2000 * mc_->get_number_of_particles();
    break;
  default:
    IMP_failure("Bad algorithm", ErrorException);
    cost = 10 * mc_->get_number_of_particles();
  }
  if (P::update(mc, cost)) {
    mc_->reset();
    mcr_->reset();
  }
  IMP_IF_CHECK(EXPENSIVE) {
    check_nbl();
  }
}


void AllNonbondedListScoreState::rebuild_nbl()
{
  IMP_LOG(TERSE, "Rebuilding AllNBL with cutoff "
          << P::get_cutoff() << " and slack " << P::get_slack() << std::endl);
  if (a_== QUADRATIC) {
    const Particles &moving= mc_->get_particles();
    for (unsigned int j=0; j< moving.size(); ++j) {
      for (unsigned int i=0; i< j; ++i) {
        P::add_if_box_overlap(moving[i], moving[j]);
      }
    }
  } else if (a_ == GRID) {
    grid_rebuild_nbl();
  } else if (a_== BBOX) {
    internal::bbox_scan(mc_->get_particles(), P::get_radius_key(),
                        P::get_slack(), P::get_cutoff(),
                        internal::NBLAddPairIfNonbonded(this));

  } else {
    IMP_failure("Bad algorithm in AllNBL::rebuild", ErrorException);
  }
  set_nbl_is_valid(true);
  IMP_LOG(TERSE, "NBL has " << P::get_number_of_nonbonded()
          << " pairs" << std::endl);
}

void AllNonbondedListScoreState::set_particles(const Particles &ps)
{
  mc_->clear_particles();
  mc_->add_particles(ps);
  mcr_->clear_particles();
  mcr_->add_particles(particles_with_radius(ps));
  P::set_nbl_is_valid(false);
}


void AllNonbondedListScoreState::add_particles(const Particles &ps)
{
  if (P::get_nbl_is_valid()) {
    if (a_== QUADRATIC || a_ == GRID) {
      const Particles &moving= mc_->get_particles();
      for (unsigned int j=0; j< moving.size(); ++j) {
        for (unsigned int i=0; i< ps.size(); ++i) {
          P::add_if_box_overlap(ps[i], moving[j]);
        }
      }
    } else if (a_== BBOX) {
      internal::bipartite_bbox_scan(mc_->get_particles(), ps,
                                    P::get_radius_key(),
                                    P::get_slack(), P::get_cutoff(),
                                    internal::NBLAddPairIfNonbonded(this));
    }
  }
  mcr_->add_particles(particles_with_radius(ps));
  mc_->add_particles(ps);
}

void AllNonbondedListScoreState::clear_particles()
{
  mc_->clear_particles();
  mcr_->clear_particles();
  P::set_nbl_is_valid(false);
  P::set_nbl_is_valid(true);
}


void AllNonbondedListScoreState::show(std::ostream &out) const
{
  out << "AllNonbondedListScoreState" << std::endl;
}


void AllNonbondedListScoreState::set_algorithm(Algorithm a)
{
  a_=translate_algorithm(a);
}


// methods for grid

static unsigned int min_grid_size=20;

float AllNonbondedListScoreState::grid_side_from_r(float r) const
{
  if (r==0) return 1;
  else return r*1.6;
}

void AllNonbondedListScoreState
::grid_partition_points(IMP::internal::Vector<internal::ParticleGrid*> &bins)
{
  if (mc_->get_particles().empty()) return;
  GetRadius gr= P::get_radius_object();
  float minr=std::numeric_limits<float>::max(), maxr=0;
  for (unsigned int i=0; i< mc_->get_particles().size(); ++i) {
    float r= gr(mc_->get_particles()[i]);
    if ( r > maxr) maxr=r;
    if ( r > 0 && r < minr) minr=r;
  }
  minr= std::min(maxr, minr);
  float curr=minr*2;
  Floats cuts;
  cuts.push_back(0);
  do {
    cuts.push_back(curr);
    curr *= 2;
  } while (curr < maxr);
  cuts.push_back(2*maxr);

  std::vector<Particles> ops(cuts.size());
  for (unsigned int i=0; i< mc_->get_particles().size(); ++i) {
    float r= gr(mc_->get_particles()[i]);
    bool found=false;
    for (unsigned int j=0; ; ++j) {
      IMP_assert(j< cuts.size(), "Internal error in ASNBLSS");
      if (cuts[j] >= r) {
        ops[j].push_back(mc_->get_particles()[i]);
        found=true;
        break;
      }
    }
    IMP_assert(found, "Didn't put particle anywhere");
  }
  // consolidate
  for (unsigned int i=1; i< ops.size(); ++i) {
    if (ops[i-1].size() + ops[i].size() < min_grid_size) {
      ops[i].insert(ops[i].end(), ops[i-1].begin(), ops[i-1].end());
      ops[i-1].clear();
    }
  }
  for (unsigned int i=0; i< cuts.size(); ++i) {
    if (ops[i].empty()) continue;
    float rmax=0;
    for (unsigned int j=0; j< ops[i].size(); ++j) {
      rmax= std::max(rmax, gr(ops[i][j]));
    }
    bins.push_back(new internal::ParticleGrid(grid_side_from_r(rmax), ops[i]));
  }
  IMP_LOG(VERBOSE, "Created " << bins.size() << " grids" << std::endl);
  for (unsigned int i=0; i< bins.size(); ++i) {
    IMP_LOG(VERBOSE, *bins[i] << std::endl);
  }
}


void AllNonbondedListScoreState
::grid_generate_nbl(const internal::ParticleGrid *particle_bin,
                    const internal::ParticleGrid *grid_bin)
{
  IMP_CHECK_OBJECT(particle_bin);
  IMP_CHECK_OBJECT(grid_bin);
  IMP_LOG(VERBOSE, "Generate nbl for pair " << std::endl);
  for (internal::ParticleGrid::ParticleVoxelIterator
         it= particle_bin->particle_voxels_begin();
       it != particle_bin->particle_voxels_end(); ++it) {
    Particle *p= it->first;
    internal::NBLAddIfNonbonded f(this, p);
    XYZDecorator d= XYZDecorator::cast(p);
    internal::ParticleGrid::VirtualIndex index
      = grid_bin->get_virtual_index(Vector3D(d.get_x(),
                                             d.get_y(),
                                             d.get_z()));
    IMP_LOG(VERBOSE, "Searching for " << p->get_index()
            << " from " << index << std::endl);
    grid_bin->apply_to_nearby(f, index,
                                   P::get_cutoff() + 2*P::get_slack(),
                                   false);
  }

}

void AllNonbondedListScoreState::grid_rebuild_nbl()
{
  IMP_LOG(TERSE, "Rebuilding NBL with Grid and cutoff "
          << P::get_cutoff() << std::endl );
  IMP::internal::Vector<internal::ParticleGrid*> bins;
  grid_partition_points(bins);

  for (unsigned int i=0; i< bins.size(); ++i) {
    for (unsigned int j=i+1; j< bins.size(); ++j) {
      grid_generate_nbl(bins[i], bins[j]);
    }

    internal::ParticleGrid::Index last_index;
    for (internal::ParticleGrid::ParticleVoxelIterator it
           = bins[i]->particle_voxels_begin();
         it != bins[i]->particle_voxels_end(); ++it) {
      IMP_LOG(VERBOSE, "Searching with particle " << it->first->get_index()
              << std::endl);
      internal::NBLAddIfNonbonded f(this, it->first);
      bins[i]->apply_to_nearby(f, it->second,
                                     P::get_cutoff()+2*P::get_slack(),
                                         true);

      if (it->second != last_index) {
        internal::NBLAddPairIfNonbonded fp(this);
        IMP_LOG(VERBOSE, "Searching in " << it->second
                << std::endl);
        bins[i]->apply_to_cell_pairs(fp, it->second);
      }
      last_index=it->second;
    }
  }
}


// debugging
void AllNonbondedListScoreState::check_nbl() const
{
  if (!get_nbl_is_valid()) return;
  const Particles &ps= mc_->get_particles();
  P::GetRadius gr= P::get_radius_object();
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZDecorator di= XYZDecorator::cast(ps[i]);
    for (unsigned int j=0; j< i; ++j) {
      XYZDecorator dj= XYZDecorator::cast(ps[j]);
      IMP_assert(ps[i] != ps[j], "Duplicate particles in list");
      if (distance(di, dj) - gr(ps[i]) - gr(ps[j])
          <= P::get_cutoff() && !are_bonded(ps[i], ps[j])) {
        bool found=false;
        for (NonbondedIterator nit= P::nonbonded_begin();
             nit != P::nonbonded_end(); ++nit) {
          if ((nit->first == ps[i] && nit->second == ps[j])
              || (nit->first == ps[j] && nit->second == ps[i])) {
            IMP_assert(!found, "Entry is in list twice");
            found=true;
          }
        }
        IMP_assert(found, "Nonbonded list is missing "
                   << ps[i]->get_index() << " " << di
                   << " " << gr(ps[i])
                   << " and " << ps[j]->get_index() << " "
                   << dj << gr(ps[j])
                   << " size is " << get_number_of_nonbonded()
                   << " distance is " << distance(di, dj)
                   << " max is " << mc_->get_max() << std::endl);
      }
    }
  }
}

IMPCORE_END_NAMESPACE
