/**
 *  \file AllNonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of s.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/AllNonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"

#include <algorithm>

namespace IMP
{

static unsigned int min_grid_size=20;

AllNonbondedListScoreState
::AllNonbondedListScoreState(FloatKey radius, const Particles &ps):
  P(radius)
{
  set_particles(ps);
}


AllNonbondedListScoreState::~AllNonbondedListScoreState()
{
  cleanup(bins_);
}

float AllNonbondedListScoreState::side_from_r(float r) const
{
  if (r==0) return 1;
  else return r*1.6;
}

Particles AllNonbondedListScoreState::get_particles() const
{
  Particles ret;
  for (unsigned int i=0; i< bins_.size(); ++i) {
    ret.insert(ret.end(), bins_[i].grid->get_particles().begin(),
               bins_[i].grid->get_particles().end());
  }
  return ret;
}

void AllNonbondedListScoreState::set_particles(const Particles &ps)
{
  NonbondedListScoreState::clear_nbl();
  cleanup(bins_);
  repartition_points(ps, bins_);
}

void AllNonbondedListScoreState::add_particles(const Particles &ps)
{
  if (bins_.empty()) set_particles(ps);
  else {
#ifndef NDEBUG
    for (unsigned int i=0; i< ps.size(); ++i) {
      for (unsigned int j=0; j< bins_.size(); ++j) {
        for (unsigned int k=0; k< bins_[j].grid->get_particles().size(); ++k) {
          IMP_assert(ps[i] != bins_[j].grid->get_particles()[k],
                     "Can't add a particle that is already there");

          IMP_assert(ps[i]->get_index() 
                     != bins_[j].grid->get_particles()[k]->get_index(),
                     "Same particle index, different particles.");
        }
      }
    }
#endif

    for (unsigned int i=0; i< ps.size(); ++i) {
      float r= P::get_radius(ps[i]);
      bool found=false;
      for (unsigned int j=0; j< bins_.size(); ++j) {
        if (bins_[j].rmax >= r) {
          found=true;
          IMP_LOG(VERBOSE, "Adding particle "
                  << ps[i]->get_index() << " to bin " << j << std::endl);
          bins_[j].grid->add_particle(ps[i]);
          break;
        }
      }
      if (!found) {
        bins_.back().rmax=r;
        bins_.back().grid->add_particle(ps[i]);
      }
    }
  }
  IMP_LOG(TERSE, "Destroying nbl in  list due to additions"<< std::endl);
  NonbondedListScoreState::clear_nbl();
}



void AllNonbondedListScoreState::repartition_points(const Particles &ps,
                                                    std::vector<Bin> &out)
{
  cleanup(out);
  if (ps.empty()) return;
  float minr=std::numeric_limits<float>::max(), maxr=0;
  for (unsigned int i=0; i< ps.size(); ++i) {
    float r= P::get_radius(ps[i]);
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
  for (unsigned int i=0; i< ps.size(); ++i) {
    float r= P::get_radius(ps[i]);
    bool found=false;
    for (unsigned int j=0; ; ++j) {
      IMP_assert(j< cuts.size(), "Internal error in ASNBLSS");
      if (cuts[j] >= r) {
        ops[j].push_back(ps[i]);
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
    out.push_back(Bin());
    float rmax=0;
    for (unsigned int j=0; j< ops[i].size(); ++j) {
      rmax= std::max(rmax, P::get_radius(ops[i][j]));
    }
    out.back().rmax= rmax;
    internal::ParticleGrid *pg 
      = new internal::ParticleGrid(side_from_r(out.back().rmax));
    out.back().grid= pg;
    out.back().grid->add_particles(ops[i]);
  }
  IMP_LOG(VERBOSE, "Created " << out.size() << " grids" << std::endl);
  for (unsigned int i=0; i< out.size(); ++i) {
    IMP_LOG(VERBOSE, out[i].rmax
            << ": " << *out[i].grid << std::endl);
  }

#ifndef NDEBUG
  Particles all;
  for (unsigned int i=0; i< out.size(); ++i) {
    all.insert(all.end(), out[i].grid->get_particles().begin(),
               out[i].grid->get_particles().end());
  }
  std::sort(all.begin(), all.end());
  Particles::iterator it = std::unique(all.begin(), all.end());
  IMP_assert(it == all.end(), "Duplicate particles " << all.size());
  IMP_assert(all.size() == ps.size(), "Wrong number of particles at end "
             << all.size() << " " << ps.size());
#endif  
}

void AllNonbondedListScoreState::cleanup(std::vector<Bin> &bins)
{
  for (unsigned int i=0; i< bins.size(); ++i) {
    delete bins[i].grid;
  }
  bins.clear();
}

void AllNonbondedListScoreState::update()
{
  IMP_LOG(TERSE, "Updating nonbonded list"<< std::endl);
  NonbondedListScoreState::update();
  bool bad=false;
  for (unsigned int i=0; i< bins_.size(); ++i) {
    bad = bins_[i].grid->update() || bad;
    IMP_LOG(VERBOSE, bins_[i].rmax << std::endl << *bins_[i].grid << std::endl);
  }
  if (bad) {
    IMP_LOG(TERSE, "Destroying nbl in  list"<< std::endl);
    NonbondedListScoreState::clear_nbl();
  }
} 

void AllNonbondedListScoreState::generate_nbl(const Bin &particle_bin,
                                              const Bin &grid_bin,
                                              float cut)
{
  IMP_CHECK_OBJECT(particle_bin.grid);
  IMP_CHECK_OBJECT(grid_bin.grid);
  IMP_LOG(VERBOSE, "Generate nbl for " << particle_bin.rmax
          << " and " << grid_bin.rmax << std::endl);
  for (unsigned int k=0; k< particle_bin.grid->get_particles().size(); ++k) {
    Particle *p= particle_bin.grid->get_particles()[k];
    NonbondedListScoreState::AddToNBL f(this, p);
    XYZDecorator d= XYZDecorator::cast(p);
    internal::ParticleGrid::VirtualIndex index
      = grid_bin.grid->get_virtual_index(Vector3D(d.get_x(),
                                                  d.get_y(),
                                                  d.get_z()));
    IMP_LOG(VERBOSE, "Searching for " << p->get_index()
            << " from " << index
            << " of bin " << grid_bin.rmax << std::endl);
    grid_bin.grid->apply_to_nearby(f, index,
                                  cut+2*particle_bin.rmax + grid_bin.rmax,
                                  false);
  }

}

void AllNonbondedListScoreState::rebuild_nbl(Float cut)
{
  IMP_LOG(TERSE, "Rebuilding NBL with " << bins_.size() << " bins" 
          << " and cutoff " << cut << std::endl);
  for (unsigned int i=0; i< bins_.size(); ++i) {
    for (unsigned int j=i+1; j< bins_.size(); ++j) {
      generate_nbl(bins_[i], bins_[j], cut);
    }

    internal::ParticleGrid::Index last_index;
    for (internal::ParticleGrid::ParticleVoxelIterator it
           = bins_[i].grid->particle_voxels_begin();
         it != bins_[i].grid->particle_voxels_end(); ++it) {
      IMP_LOG(VERBOSE, "Searching with particle " << it->first->get_index()
              << std::endl);
      NonbondedListScoreState::AddToNBL f(this, it->first);
      bins_[i].grid->apply_to_nearby(f, it->second,
                                         cut+3*bins_[i].rmax,
                                         true);
      if (it->second != last_index) {
        IMP_LOG(VERBOSE, "Searching in " << it->second
                << std::endl);
        bins_[i].grid->apply_to_cell_pairs(f, it->second);
      }
      last_index=it->second;
    }
  }

  IMP_LOG(TERSE, "NBL has " << P::nbl_.size() << " pairs" << std::endl);

#ifndef NDEBUG
  {
    Particles ps;
    for (unsigned int i=0; i< bins_.size(); ++i) {
      if (bins_[i].grid) {
        ps.insert(ps.end(), bins_[i].grid->get_particles().begin(),
                  bins_[i].grid->get_particles().end());
      }
    }
    for (unsigned int i=0; i< ps.size(); ++i) {
      XYZDecorator di= XYZDecorator::cast(ps[i]);
      for (unsigned int j=0; j< i; ++j) {
        XYZDecorator dj= XYZDecorator::cast(ps[j]);
        IMP_assert(ps[i] != ps[j], "Duplicate particles in grid");
        if (distance(di, dj) - P::get_radius(ps[i]) - P::get_radius(ps[j])
            <= cut && !are_bonded(ps[i], ps[j])) {
          bool found=false;
          for (NonbondedIterator nit= nbl_.begin();
               nit != nbl_.end(); ++nit) {
            if (nit->first == ps[i] && nit->second == ps[j]
                || nit->first == ps[j] && nit->second == ps[i]) {
              IMP_assert(!found, "Entry is in list twice");
              found=true;
            }
          }
          IMP_assert(found, "Nonbonded list is missing " 
                     << ps[i]->get_index() << " " << di 
                     << " and " << ps[j]->get_index() << " " 
                     << dj << std::endl);
        }
      }
    }
  }
#endif
}


void AllNonbondedListScoreState::show(std::ostream &out) const
{
  out << "AllNonbondedListScoreState" << std::endl;
}

} // namespace IMP
