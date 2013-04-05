/**
 *  \file DataPoints.cpp
 *  \brief Handling of data for anchor points segmentation
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/DensityDataPoints.h>
#include <IMP/em/converters.h>
#include <IMP/em/Voxel.h>
IMPMULTIFIT_BEGIN_NAMESPACE

void DensityDataPoints::set_density(em::DensityMap *dmap) {
  algebra::BoundingBox3D bb = em::get_bounding_box(dmap);
  dens_.reset(new DensGrid(dmap->get_spacing(),bb));
  em::emreal* d_data = dmap->get_data();
  algebra::Vector3D loc;
  for(long l=0;l<dmap->get_number_of_voxels();l++) {
    loc = dmap->get_location_by_voxel(l);
    (*dens_)[dens_->get_nearest_index(loc)]=d_data[l];
  }
}

void DensityDataPoints::set_density(const DensGrid &dens) {
  algebra::BoundingBox3D bb = dens.get_bounding_box();
  float spacing = dens.get_unit_cell()[0];
  dens_.reset(new DensGrid(spacing,bb));
  //copy data
  DensGrid::ExtendedIndex lb = dens.get_extended_index(bb.get_corner(0)),
      ub = dens.get_extended_index(bb.get_corner(1));
  for (DensGrid::IndexIterator it= dens.indexes_begin(lb,ub);
       it != dens.indexes_end(lb, ub); ++it) {
    (*dens_)[*it]=dens[*it];
  }
}

void DensityDataPoints::set_max_min_density_values() {
  max_value_=-INT_MAX;
  min_value_=INT_MAX;
  algebra::BoundingBox3D bb = dens_->get_bounding_box();
  DensGrid::ExtendedIndex lb = dens_->get_extended_index(bb.get_corner(0)),
      ub = dens_->get_extended_index(bb.get_corner(1));
  for (DensGrid::IndexIterator it= dens_->indexes_begin(lb,ub);
       it != dens_->indexes_end(lb, ub); ++it) {
    if ((*dens_)[*it]<min_value_) min_value_=(*dens_)[*it];
    if ((*dens_)[*it]>max_value_) max_value_=(*dens_)[*it];
  }
}

DensityDataPoints::DensityDataPoints(em::DensityMap *dens,
                                     float density_threshold)
  : XYZDataPoints() {
  set_density(dens);
  //  dens_=dens;
  threshold_ = density_threshold;
  set_max_min_density_values();
  //  max_value_ = dens->get_max_value();
  //  min_value_ = dens->get_min_value();
  //  algebra::Vector3Ds vecs = em::density2vectors(dens,threshold_);
  populate_data();
}
void DensityDataPoints::populate_data() {
  algebra::Vector3Ds vecs;
  algebra::BoundingBox3D bb = dens_->get_bounding_box();
  DensGrid::ExtendedIndex lb = dens_->get_extended_index(bb.get_corner(0)),
      ub = dens_->get_extended_index(bb.get_corner(1));
  for (DensGrid::IndexIterator it= dens_->indexes_begin(lb,ub);
       it != dens_->indexes_end(lb, ub); ++it) {
    if ((*dens_)[*it]>threshold_) {vecs.push_back(dens_->get_center(*it));}
  }
  /*std::cout<<
           "Number of data points were found above the input threshold ("<<
    threshold_<<") is "<<vecs.size()<<" . The maximum value is"<<max_value_<<
            " and the minimum value is : " << min_value_<<std::endl;
  */

  IMP_INTERNAL_CHECK(vecs.size()>0,
           "No data points were found above the input threshold ("<<
            threshold_<<"). The maximum value is"<<max_value_<<
            " and the minimum value is : " << min_value_<<std::endl);
  IMP_LOG_VERBOSE("Number of data points:"<<vecs.size()<<std::endl);
  XYZDataPoints::populate_data_points(vecs);
}
DensityDataPoints::DensityDataPoints(const DensGrid &dens,
                                     float density_threshold)
  : XYZDataPoints() {
  set_density(dens);
  threshold_ = density_threshold;
  set_max_min_density_values();
  populate_data();
}


IMP::statistics::internal::Array1DD DensityDataPoints::sample() const {
    bool found = false;
    int num_trails=0;
    int p_ind=0;
    do {
      IMP_LOG_VERBOSE("trail number:"<<num_trails<<std::endl);
      ++num_trails;
      p_ind = statistics::internal::random_int(data_.size());
      if ((*dens_)[dens_->get_nearest_index(
             algebra::Vector3D(data_[p_ind][0],
                               data_[p_ind][1],data_[p_ind][2]))]
          > (max_value_-min_value_)*
          statistics::internal::random_uniform()+min_value_) {
        found = true;
      }
    }while (!found && num_trails<150); //TODO - add to multifit param
    if (!found) {
      std::cerr<< "Could not sample DensityDataPoints."
               <<"Probably a problem with reading the map" <<std::endl;
      std::cerr<<"Max value:"<<max_value_<<std::endl;
      std::cerr<<"Min value:"<<min_value_<<std::endl;
      return data_[statistics::internal::random_int(data_.size())];
    }
    return data_[p_ind];
  }

em::DensityMap *grid2map(const DensGrid &dg,float spacing) {
  algebra::BoundingBox3D bb = algebra::get_bounding_box(dg);
  em::DensityMap *r_map = em::create_density_map(bb,spacing);
  r_map->set_origin(dg.get_origin());
  DensGrid::ExtendedIndex lb = dg.get_extended_index(bb.get_corner(0)),
    ub = dg.get_extended_index(bb.get_corner(1));
  for (DensGrid::IndexIterator it= dg.indexes_begin(lb,ub);
       it != dg.indexes_end(lb, ub); ++it) {
    algebra::Vector3D cen=dg.get_center(*it);
    r_map->set_value(cen[0],cen[1],cen[2],dg[*it]);
  }
  return r_map;
}


IMPMULTIFIT_END_NAMESPACE
