/**
 *  \file vector_search.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/vector_search.h>
#include <IMP/base/utility.h>
#include <IMP/algebra/utility.h>
#include <IMP/base/log.h>
#include <IMP/base/exception.h>

IMPALGEBRA_BEGIN_NAMESPACE

DynamicNearestNeighbor3D::DynamicNearestNeighbor3D(const Vector3Ds &vs,
                                                   double query_estimate):
    Object("DynamicNearestNeighbor3D%1%"),
  grid_(query_estimate, get_zero_vector_d<3>()),coords_(vs),
  indexes_(vs.size()) {
  for (unsigned int i=0; i< coords_.size(); ++i) {
    set_coordinates_internal(i, coords_[i]);
  }
  audit();
}

Ints DynamicNearestNeighbor3D::get_in_ball(int id, double distance) const {
  IMP_OBJECT_LOG;
  set_was_used(true);
  IMP_IF_CHECK(base::USAGE_AND_INTERNAL) {
    audit();
  }
  //Index i= indexes_[id];
  BoundingBox3D bb(coords_[id]);
  bb+= distance;
  Ints ret;
  double distance2= get_squared(distance);
  IMP_LOG_VERBOSE( "Searching from " << bb << " which is "
          << grid_.get_extended_index(bb.get_corner(0))
          << " to " << grid_.get_extended_index(bb.get_corner(1)) << std::endl);
  for (Grid::IndexIterator it=grid_.indexes_begin(bb);
       it != grid_.indexes_end(bb); ++it) {
    Grid::Index ind= *it;
    const Ints cur= grid_[ind];
    using base::operator<<;
    IMP_LOG_VERBOSE( "Investigating " << ind << ": " << cur << std::endl);
    for (unsigned int j=0; j< cur.size(); ++j) {
      if (get_squared_distance(coords_[cur[j]], coords_[id]) < distance2) {
        if (cur[j]==id) continue;
        ret.push_back(cur[j]);
      }
    }
  }
  return ret;
}

void DynamicNearestNeighbor3D::set_coordinates_internal(int id, Vector3D nc) {
  EIndex ei= grid_.get_extended_index(nc);
  coords_[id]=nc;
  Index i;
  if (grid_.get_has_index(ei)) {
    i= grid_.get_index(ei);
    grid_[i].push_back(id);
  } else {
    i=grid_.add_voxel(ei, Ints(1, id));
  }
  indexes_[id]=i;
  IMP_LOG_VERBOSE( "New voxel for " << id << " at " << nc
          << " is " << indexes_[id] << std::endl);
}

void DynamicNearestNeighbor3D::set_coordinates(int id, Vector3D nc) {
  IMP_OBJECT_LOG;
  Grid::Index ind= indexes_[id];
  Ints &it= grid_[ind];
  IMP_INTERNAL_CHECK(std::find(it.begin(), it.end(), id) != it.end(),
                     "Item not found in list");
  std::remove(it.begin(), it.end(), id);
  it.pop_back();
  IMP_INTERNAL_CHECK(std::find(it.begin(), it.end(), id) == it.end(),
                     "Item found in list");
  set_coordinates_internal(id, nc);
  if (it.empty()) {
    grid_.remove_voxel(ind);
  }
#if IMP_HAS_CHECKS >= IMP_INTERNAL
  this->audit();
#endif
}

void DynamicNearestNeighbor3D::audit() const {
  Ints found;
  for (Grid::AllConstIterator it= grid_.all_begin();
           it != grid_.all_end(); ++it) {
    found.insert(found.end(), it->second.begin(),
                 it->second.end());
  }
  using base::operator<<;
  for (unsigned int i=0; i< coords_.size(); ++i) {
    IMP_INTERNAL_CHECK(std::find(grid_[indexes_[i]].begin(),
                                 grid_[indexes_[i]].end(),
                                 i) != grid_[indexes_[i]].end(),
                       "Item " << i << " not found in list: "
                       << grid_[indexes_[i]]);
    Grid::ExtendedIndex ei= grid_.get_extended_index(coords_[i]);
    IMP_INTERNAL_CHECK(grid_.get_has_index(ei),
                       "Voxel for " << i << " is empty.");
    Grid::Index ind= grid_.get_index(ei);
    IMP_INTERNAL_CHECK(ind==indexes_[i],
                       "Indexes don't match: "
                       << ind << " vs " << indexes_[i]);
  }
  IMP_INTERNAL_CHECK(found.size()== coords_.size(),
                     "Wrong number found: " << found
                     << " is not of length " << coords_.size());
}

IMPALGEBRA_END_NAMESPACE
