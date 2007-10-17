#ifndef _IMP_PARTICLES_ACCESS_POINT_H
#define _IMP_PARTICLES_ACCESS_POINT_H

/*
  CLASS

  KEYWORDS

  AUTHORS
  Keren Lasker (mailto: kerenl@salilab.org)


  OVERVIEW TEXT

*/
#include <vector>
#include <map>


#include "ParticlesAccessPoint.h"
#include "Particle.h"
#include "IMP_config.h"
#include "ModelData.h"
#include "Model.h"

namespace IMP
{

class IMPDLLEXPORT IMPParticlesAccessPoint : public ParticlesAccessPoint
{

public:
  IMPParticlesAccessPoint() {}

  IMPParticlesAccessPoint(Model& model,
                          std::vector<int>& particle_indexes,
                          std::string radius_att_name,
                          std::string weight_att_name);


  void InitSingleAttIndexes(std::vector<int> &data,
                            const std::string &attribute_name);
  void InitAllAttIndexes();
  void ReSelect(std::vector<int>& particle_indexes);

  int size() {
    return particle_indexes.size();
  }
  int x_float_ind(int ind) {
    return x_indexes[ind];
  }
  int y_float_ind(int ind) {
    return y_indexes[ind];
  }

  int z_float_ind(int ind) {
    return z_indexes[ind];
  }

  float x(int ind) {
    return model->get_model_data()->get_float(x_indexes[ind]);
  }
  float y(int ind) {
    return model->get_model_data()->get_float(y_indexes[ind]);
  }
  float z(int ind) {
    return model->get_model_data()->get_float(z_indexes[ind]);
  }
  float r(int ind) {
    return model->get_model_data()->get_float(r_att_indexes[ind]);
  }
  float w(int ind) {
    return model->get_model_data()->get_float(w_att_indexes[ind]);
  }
private:
  Model *model;
  std::vector<int> particle_indexes;
  std::vector<int> x_indexes,y_indexes,z_indexes;
  std::vector<int> r_att_indexes,w_att_indexes;
  std::string radius_att_name,weight_att_name;

};

}
#endif /* _IMP_PARTICLES_ACCESS_POINT_H */
