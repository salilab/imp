/**
 *  \file vector_search.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/vector_search.h>
#include <IMP/base/utility.h>

IMPALGEBRA_BEGIN_NAMESPACE

DynamicNearestNeighbor3D::DynamicNearestNeighbor3D(const BoundingBox3D &bb,
                                                   const Vector3Ds &vs,
                                                   double query_estimate):
    Object("DynamicNearestNeighbor3D%1%"),
    grid_(query_estimate, bb) {
  coords_=vs;
  indexes_.resize(vs.size());
  for (unsigned int i=0; i< coords_.size(); ++i) {
    set_coordinates(i, coords_[i]);
  }
  audit();
}

Ints DynamicNearestNeighbor3D::get_in_ball(int id, double distance) const {
  IMP_IF_CHECK(USAGE_AND_INTERNAL) {
    audit();
  }
  //Index i= indexes_[id];
  BoundingBox3D bb(coords_[id]);
  bb+= distance;
  Ints ret;
  double distance2= square(distance);
  for (Grid::VoxelConstIterator it=grid_.voxels_begin(bb);
       it != grid_.voxels_end(bb); ++it) {
    const Ints cur= *it;
    for (unsigned int j=0; j< cur.size(); ++j) {
      if (get_squared_distance(coords_[cur[j]], coords_[id]) < distance2) {
        if (cur[j]==id) continue;
        ret.push_back(cur[j]);
      }
    }
  }
  return ret;
}

void DynamicNearestNeighbor3D::set_coordinates(int id, Vector3D nc) {
  if (indexes_[id] != Index()) {
    Ints &it= grid_[indexes_[id]];
    IMP_INTERNAL_CHECK(std::find(it.begin(), it.end(), id) != it.end(),
                       "Item not found in list");
    std::remove(it.begin(), it.end(), id);
    it.pop_back();
    IMP_INTERNAL_CHECK(std::find(it.begin(), it.end(), id) == it.end(),
                       "Item found in list");
  }
  EIndex ei= grid_.get_extended_index(nc);
  coords_[id]=nc;
  Index i= grid_.get_index(ei);
  grid_[i].push_back(id);
  indexes_[id]=i;
}

void DynamicNearestNeighbor3D::audit() const {
  Ints found;
  for (Grid::AllIndexIterator it= grid_.all_indexes_begin();
           it != grid_.all_indexes_end(); ++it) {
    found.insert(found.end(), grid_[*it].begin(),
                 grid_[*it].end());
  }
  using base::operator<<;
  using std::operator<<;
  for (unsigned int i=0; i< coords_.size(); ++i) {
    if (indexes_[i] != Index()) {
      IMP_INTERNAL_CHECK(std::find(grid_[indexes_[i]].begin(),
                                   grid_[indexes_[i]].end(),
                                   i) != grid_[indexes_[i]].end(),
                         "Item " << i << " not found in list: "
                         << grid_[indexes_[i]]);
    }
  }
  IMP_INTERNAL_CHECK(found.size()== coords_.size(),
                     "Wrong number found: " << found
                     << " is not of length " << coords_.size());
}

IMPALGEBRA_END_NAMESPACE
