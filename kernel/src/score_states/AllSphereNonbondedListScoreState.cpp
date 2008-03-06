/**
 *  \file AllSphereNonbondedListScoreState.cpp
 *  \brief Allow iteration through pairs of a set of spheres.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#include "IMP/score_states/AllSphereNonbondedListScoreState.h"
#include "IMP/decorators/XYZDecorator.h"

#include <algorithm>

namespace IMP
{

static unsigned int min_grid_size=20;

AllSphereNonbondedListScoreState
::AllSphereNonbondedListScoreState(const Particles &ps, FloatKey radius):
  rk_(radius)
{
  set_particles(ps);
}

AllSphereNonbondedListScoreState::~AllSphereNonbondedListScoreState()
{
  cleanup(bins_);
  cleanup(fixed_bins_);
}

void AllSphereNonbondedListScoreState::set_particles(const Particles &ps)
{
  NonbondedListScoreState::clear_nbl();
  // split into moving and fixed and call repartition twice
  Particles moving, fixed;
  for (unsigned int i=0; i< ps.size(); ++i) {
    XYZDecorator d= XYZDecorator::cast(ps[i]);
    if (d.get_coordinates_are_optimized()) {
      moving.push_back(ps[i]);
    } else {
      fixed.push_back(ps[i]);
    }
  }
  cleanup(bins_);
  cleanup(fixed_bins_);
  repartition_points(moving, bins_);
  repartition_points(fixed, fixed_bins_);
  for (unsigned int i=0; i< fixed_bins_.size(); ++i) {
    fixed_bins_[i].grid->update();
  }
}

void AllSphereNonbondedListScoreState::repartition_points(const Particles &ps,
                                                          std::vector<Bin> &out)
{
  cleanup(out);
  if (ps.empty()) return;
  float minr=std::numeric_limits<float>::max(), maxr=0;
  for (unsigned int i=0; i< ps.size(); ++i) {
    float r= ps[i]->get_value(rk_);
    if ( r > maxr) maxr=r;
    if ( r > 0 && r < minr) minr=r;
  }
  float curr=minr*2;
  Floats cuts;
  do {
    cuts.push_back(curr);
    curr *= 2;
  } while (curr < maxr);
  cuts.push_back(2*maxr);

  std::vector<Particles> ops(cuts.size());
  for (unsigned int i=0; i< ps.size(); ++i) {
    float r= ps[i]->get_value(rk_);
    for (unsigned int j=0; ; ++j) {
      IMP_assert(j< cuts.size(), "Internal error in ASNBLSS");
      if (cuts[j] > r) {
        ops[j].push_back(ps[i]);
        break;
      }
    }
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
      rmax= std::max(rmax, ops[i][j]->get_value(rk_));
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

void AllSphereNonbondedListScoreState::cleanup(std::vector<Bin> &bins)
{
  for (unsigned int i=0; i< bins.size(); ++i) {
    delete bins[i].grid;
  }
  bins.clear();
}

void AllSphereNonbondedListScoreState::update()
{
  NonbondedListScoreState::update();
  bool bad=false;
  for (unsigned int i=0; i< bins_.size(); ++i) {
    if (bins_[i].grid->update()) bad=true;
    IMP_LOG(VERBOSE, bins_[i].rmax << std::endl << *bins_[i].grid << std::endl);
  }
  if (bad) {
    IMP_LOG(VERBOSE, "Destroying nbl in Sphere list"<< std::endl);
    NonbondedListScoreState::clear_nbl();
  }
} 

void AllSphereNonbondedListScoreState::generate_nbl(const Bin &particle_bin,
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

void AllSphereNonbondedListScoreState::rebuild_nbl(Float cut)
{
  IMP_LOG(VERBOSE, "Rebuilding NBL with " << bins_.size() << " dynamic and "
          << fixed_bins_.size() << " fixed" << std::endl);
  for (unsigned int i=0; i< bins_.size(); ++i) {
    for (unsigned int j=i+1; j< bins_.size(); ++j) {
      generate_nbl(bins_[i], bins_[j], cut);
    }

    for (unsigned int j=0; j< fixed_bins_.size(); ++j) {
      generate_nbl(bins_[i], fixed_bins_[j], cut);
    }

    // same code as AllNonbonded. Would be nice to consolidate
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

#ifndef NDEBUG
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
      if (distance(di, dj) - ps[i]->get_value(rk_) - ps[j]->get_value(rk_)
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
                   << " and " << ps[j]->get_index() << " " << dj << std::endl);
      }
    }
  }
#endif
}


void AllSphereNonbondedListScoreState::show(std::ostream &out) const
{
  out << "AllSphereNonbondedListScoreState" << std::endl;
}

} // namespace IMP
