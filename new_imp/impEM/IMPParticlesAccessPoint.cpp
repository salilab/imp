#include "IMPParticlesAccessPoint.h"

namespace IMP
{

IMPParticlesAccessPoint::IMPParticlesAccessPoint(Model& model_,
    std::vector<int>& particle_indexes_,
    std::string radius_att_name_,
    std::string weight_att_name_)
{

  model = &model_;
  particle_indexes =  particle_indexes_;
  create_keys(radius_att_name_,weight_att_name_);
}


IMPParticlesAccessPoint::IMPParticlesAccessPoint(
    const IMPParticlesAccessPoint &other) {
  model = other.model;
  for (unsigned int i=0;i<other.particle_indexes.size();i++) {
    particle_indexes.push_back(other.particle_indexes[i]);
  }
  radius_key=other.radius_key;
  weight_key=other.weight_key;
  x_key=other.x_key;
  y_key=other.y_key;
  z_key=other.z_key;
}


void IMPParticlesAccessPoint::ReSelect(std::vector<int>& particle_indexes_) {
  particle_indexes = particle_indexes_;
}


void IMPParticlesAccessPoint::create_keys(std::string radius_att_name_,
                                          std::string weight_att_name_) {
  radius_key = FloatKey(radius_att_name_.c_str());
  weight_key = FloatKey(weight_att_name_.c_str());
  x_key      = FloatKey(x_att_name);
  y_key      = FloatKey(y_att_name);
  z_key      = FloatKey(z_att_name);
}


}   // namespace IMP
