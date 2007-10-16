#include "IMPParticlesAccessPoint.h"

namespace IMP
{

IMPParticlesAccessPoint::IMPParticlesAccessPoint(Model& model,
    std::vector<int>& particle_indexes,
    std::string radius_att_name,
    std::string weight_att_name)
{
  model_=model;
  particle_indexes_ =  particle_indexes_;
  radius_att_name_  =  radius_att_name;
  weight_att_name_  =  weight_att_name;
}


void IMPParticlesAccessPoint::InitSingleAttIndexes(std::vector<int> &data,
    const std::string &attribute_name)
{
  data.clear();
  for (std::vector<int>::const_iterator it = particle_indexes_.begin(); it != particle_indexes_.end(); it++) {
    data.push_back(model_.get_particle(*it)->get_float_index(attribute_name).index());
  }
}


void IMPParticlesAccessPoint::InitAllAttIndexes()
{
  InitSingleAttIndexes(x_indexes_,"X");
  InitSingleAttIndexes(x_indexes_,"Y");
  InitSingleAttIndexes(x_indexes_,"Z");
  InitSingleAttIndexes(x_indexes_,radius_att_name_);
  InitSingleAttIndexes(x_indexes_,weight_att_name_);
}


void IMPParticlesAccessPoint::ReSelect(std::vector<int>& particle_indexes)
{
  particle_indexes_ = particle_indexes_;
  InitAllAttIndexes();
}


}   // namespace IMP
