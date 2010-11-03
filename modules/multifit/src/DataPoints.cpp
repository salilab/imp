/**
 *  \file DataPoints.cpp
 *  \brief Handling of data for anchor points segmentation
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/multifit/DataPoints.h>
#include <IMP/em/converters.h>
#include <IMP/em/Voxel.h>
IMPMULTIFIT_BEGIN_NAMESPACE
void XYZDataPoints::populate_data_points(const algebra::Vector3Ds &vecs) {
  vecs_ = vecs;
  int dim = atts_.size();
  std::cout<<"dim size:"<<dim<<" and number of vecs: "<<vecs_.size()<<std::endl;
  for(unsigned int i=0;i<vecs_.size();i++) {
    data_.push_back(Array1DD(dim));
    for(int d=0;d<dim;d++) {
      data_[i][d] = double(vecs_[i][d]);
    }
  }
}
void ParticlesDataPoints::populate_data_points(const Particles &ps) {
  ps_ = ps;
  int dim = atts_.size();
  for(unsigned int i=0;i<ps.size();i++) {
    data_.push_back(Array1DD(dim));
    for(int d=0;d<dim;d++) {
      data_[i][d] = double(ps[i]->get_value(atts_[d]));
    }
    vecs_.push_back(core::XYZ(ps[i]).get_coordinates());
  }
}

DensityDataPoints::DensityDataPoints(em::DensityMap *dens,
                                     float density_threshold)
  : XYZDataPoints() {
  //  atts_ = core::XYZ::get_xyz_keys();
  dens_=dens;
  threshold_ = density_threshold;
  max_value_ = dens_->get_max_value();
  min_value_ = dens_->get_min_value();
  std::cout<<"before density2particles"<<std::endl;
  algebra::Vector3Ds vecs = em::density2vectors(*dens_,threshold_);
  std::cout<<"number of vectors:"<<vecs.size()<<std::endl;
  IMP_INTERNAL_CHECK(vecs.size()>0,
           "No data points were found above the input threshold ("<<
            threshold_<<"). The maximum value is"<<max_value_<<
            " and the minimum value is : " << min_value_<<std::endl);
  IMP_LOG(VERBOSE,"Number of data points:"<<vecs.size()<<std::endl);
  std::cout<<"before populate"<<std::endl;
  XYZDataPoints::populate_data_points(vecs);
  std::cout<<"after populate"<<std::endl;
}

Array1DD DensityDataPoints::sample() const {
    bool found = false;
    int num_trails=0;
    int p_ind=0;
    do {
      IMP_LOG(VERBOSE,"trail number:"<<num_trails<<std::endl);
      ++num_trails;
      p_ind = statistics::internal::random_int(data_.size());
      if (dens_->get_value(data_[p_ind][0],
                           data_[p_ind][1],
                           data_[p_ind][2])
          > (max_value_-min_value_)*
          statistics::internal::random_uniform()+min_value_) {
        found = true;
      }
    }while ((not found) and (num_trails<150)); //TODO - add to multifit param
    if (!found) {
      std::cerr<< "Could not sample DensityDataPoints."
               <<"Probably a problem with reading the map" <<std::endl;
      std::cerr<<"Max value:"<<max_value_<<std::endl;
      std::cerr<<"Min value:"<<min_value_<<std::endl;
    }
    return data_[statistics::internal::random_int(data_.size())];
  }
IMPMULTIFIT_END_NAMESPACE
