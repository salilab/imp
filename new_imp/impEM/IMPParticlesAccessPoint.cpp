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
  radius_att_name =  radius_att_name_;
  weight_att_name  =  weight_att_name_;
}


void IMPParticlesAccessPoint::InitSingleAttIndexes(std::vector<int> &data,
    const std::string &attribute_name)
{
  data.clear();
  for (std::vector<int>::const_iterator it = particle_indexes.begin(); it != particle_indexes.end(); it++) {
    data.push_back(model->get_particle(*it)->get_float_index(attribute_name).get_index());
  }
}


void IMPParticlesAccessPoint::InitAllAttIndexes()
{
  InitSingleAttIndexes(x_indexes,"X");
  InitSingleAttIndexes(y_indexes,"Y");
  InitSingleAttIndexes(z_indexes,"Z");
  InitSingleAttIndexes(r_att_indexes,radius_att_name);
  InitSingleAttIndexes(w_att_indexes,weight_att_name);
}


void IMPParticlesAccessPoint::ReSelect(std::vector<int>& particle_indexes_)
{
  particle_indexes = particle_indexes_;
  InitAllAttIndexes();
}


}   // namespace IMP
