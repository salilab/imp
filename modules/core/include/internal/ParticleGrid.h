/**
 *  \file ParticleGrid.h
 *  \brief A grid for keeping track of particles.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_PARTICLE_GRID_H
#define IMPCORE_PARTICLE_GRID_H

#include "../config.h"
#include "Grid3D.h"

#include <IMP/base_types.h>
#include <IMP/Pointer.h>
#include <IMP/algebra/Vector3D.h>

IMPCORE_BEGIN_NAMESPACE

namespace internal
{

/** \internal */
class ParticleGrid: public Object
{
  // don't need ref counting since mc_ has the same set of points
  typedef internal::Grid3D<Particles> Grid;
  Grid grid_;
  Float target_voxel_side_;

  void build_grid(const Particles &ps);
  void audit_particles(const Particles &ps) const;
  void add_particle_to_grid(Particle *p);
public:
  ParticleGrid();
  //! suggested grid edge size.
  ParticleGrid(Float sz, const Particles &ps);

  Float get_voxel_size() const {return target_voxel_side_;}

  void show(std::ostream &out) const;

  typedef Grid::VirtualIndex VirtualIndex;
  typedef Grid::Index Index;
  Grid::VirtualIndex get_virtual_index(algebra::Vector3D pt) const {
    return grid_.get_virtual_index(pt);
  }

  //! Apply the function F to all particles near to the center
  /** \param[in] f The functor. It should take a particle as an argument.
      \param[in] center The starting point to search from
      \param[in] cut The maximum distance to search
      \param[in] skip_lower If true, all voxels with lower index are skipped.
      Use this if searching for points which are already in the grid.
      \note that for various technical reasons, this does not include particles
      in the cell center.

   */
  template <class F>
  void apply_to_nearby(F f, const Grid::VirtualIndex &center,
                       float cut,
                       bool skip_lower) const {
    Grid::VirtualIndex lc, uc;

    if ( cut > target_voxel_side_*1000 ) {
      // This is needed to handle overflow
      lc=Grid::VirtualIndex(0,0,0);
      uc=Grid::VirtualIndex(grid_.get_number_of_voxels(0),
                            grid_.get_number_of_voxels(1),
                            grid_.get_number_of_voxels(2));
    } else {
      // it is important that this is not unsigned
      int ncells= static_cast<int>(std::ceil(cut/target_voxel_side_));
      // to allow laziness in rebuilding
      ++ncells;
      lc=Grid::VirtualIndex(center[0]-ncells,
                            center[1]-ncells,
                            center[2]-ncells);
      uc=Grid::VirtualIndex(center[0]+ncells,
                            center[1]+ncells,
                            center[2]+ncells);
    }
    //IMP_LOG(VERBOSE, "Iteration bounds are " << lc << " and "
    // << uc << std::endl);
    for (Grid::IndexIterator cur= grid_.indexes_begin(lc, uc);
         cur != grid_.indexes_end(lc, uc);
         ++cur){
      if ( skip_lower && center >= *cur) continue;
      if (grid_.get_voxel(*cur).empty()) continue;
      //IMP_LOG(VERBOSE, "Paired with " << cur << std::endl);

      for (unsigned int pi= 0;
           pi< grid_.get_voxel(*cur).size(); ++pi) {
        Particle *op = grid_.get_voxel(*cur)[pi];
        f(op);
      }
    }

  }

  template <class F>
  void apply_to_cell_pairs(F f, const Grid::Index &center) const {
    const Particles &ps= grid_.get_voxel(center);
    for (unsigned int i=0; i< ps.size(); ++i) {
      for (unsigned int j=0; j< i; ++j) {
        f(ps[i], ps[j]);
      }
    }
  }

  // This is not a real iterator yet
  class ParticleVoxelIterator
  {
    Grid::IndexIterator cvoxel_;
    Grid::IndexIterator evoxel_;
    Particles::const_iterator curp_;
    const Grid *grid_;
    typedef std::pair<Particle *, Grid::Index> VT;
    VT temp_;
    void find_voxel() {
      while (cvoxel_ != evoxel_ && grid_->get_voxel(*cvoxel_).empty()) {
        ++cvoxel_;
      }
      if (cvoxel_ != evoxel_) {
        curp_= grid_->get_voxel(*cvoxel_).begin();
        temp_= std::make_pair(*curp_, *cvoxel_);
      }
    }
  public:
    typedef VT value_type;
    typedef const value_type& reference_type;
    typedef const value_type* pointer_type;

    ParticleVoxelIterator(const Grid::IndexIterator& c,
                          const Grid::IndexIterator& e,
                          const Grid *grid): cvoxel_(c),
                                             evoxel_(e),
                                             grid_(grid) {
      IMP_assert(grid_, "NULL grid");
      find_voxel();
    }

    void operator++() {
      IMP_assert(cvoxel_ != evoxel_, "Too many increments");
      ++curp_;
      if (curp_== grid_->get_voxel(*cvoxel_).end()) {
        ++cvoxel_;
        find_voxel();
      } else {
        temp_= std::make_pair(*curp_, *cvoxel_);
      }
    }

    reference_type operator*() const {
      return temp_;
    }
    pointer_type operator->() const {
      return &temp_;
    }
    bool operator==(const ParticleVoxelIterator &o) const {
      return cvoxel_ == o.cvoxel_
             && ((cvoxel_ == evoxel_ && o.cvoxel_ == o.evoxel_)
                 || curp_ == o.curp_);
    }
    bool operator!=(const ParticleVoxelIterator &o) const {
      return !operator==(o);
    }
  };
  ParticleVoxelIterator particle_voxels_begin() const {
    return ParticleVoxelIterator(grid_.all_indexes_begin(),
                                 grid_.all_indexes_end(),
                                 &grid_);
  }
  ParticleVoxelIterator particle_voxels_end() const {
    return ParticleVoxelIterator(grid_.all_indexes_end(),
                                 grid_.all_indexes_end(),
                                 &grid_);
  }
};

IMP_OUTPUT_OPERATOR(ParticleGrid);

} // namespace internal

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_PARTICLE_GRID_H */
