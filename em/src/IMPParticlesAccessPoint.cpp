#include "IMPParticlesAccessPoint.h"

namespace IMP
{

namespace em
{

IMPParticlesAccessPoint::IMPParticlesAccessPoint(Model& model,
    std::vector<int>& particle_indexes,
    std::string radius_att_name,
    std::string weight_att_name)
{

  model_ = &model;
  particle_indexes_ = particle_indexes;
  create_keys(radius_att_name, weight_att_name);
}


IMPParticlesAccessPoint::IMPParticlesAccessPoint(
    const IMPParticlesAccessPoint &other)
{
  model_ = other.model_;
  for (unsigned int i = 0; i < other.particle_indexes_.size(); ++i) {
    particle_indexes_.push_back(other.particle_indexes_[i]);
  }
  radius_key_= other.radius_key_;
  weight_key_= other.weight_key_;
  x_key_ = other.x_key_;
  y_key_ = other.y_key_;
  z_key_ = other.z_key_;
}


void IMPParticlesAccessPoint::reselect(std::vector<int>& particle_indexes)
{
  particle_indexes_ = particle_indexes;
}


void IMPParticlesAccessPoint::create_keys(std::string radius_att_name,
                                          std::string weight_att_name)
{
  radius_key_ = FloatKey(radius_att_name.c_str());
  weight_key_ = FloatKey(weight_att_name.c_str());
  x_key_      = FloatKey(X_ATT_NAME);
  y_key_      = FloatKey(Y_ATT_NAME);
  z_key_      = FloatKey(Z_ATT_NAME);
}

}   // namespace em

}   // namespace IMP
