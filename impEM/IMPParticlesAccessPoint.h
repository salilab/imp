#ifndef _IMP_PARTICLES_ACCESS_POINT_H
#define _IMP_PARTICLES_ACCESS_POINT_H

#include <vector>
#include <map>

#include "IMP/ModelData.h"
#include "IMP/Model.h"
#include "IMP/Particle.h"

#include "IMPEM_config.h"
#include "ParticlesAccessPoint.h"



namespace IMP
{

#define x_att_name "x"
#define y_att_name "y"
#define z_att_name "z"

//!
class IMPEMDLLEXPORT IMPParticlesAccessPoint : public ParticlesAccessPoint
{

public:
  IMPParticlesAccessPoint() {}
  //! Constructor
  /** \param[in] model 
      \param[in] particle_indexes a vector that contains the indexes of the
                                  particles.
      \param[in] radius_att_name the attribute name of the radius
      \param[in] weight_att_name the attribute name of the weight 
   */
  IMPParticlesAccessPoint(Model& model,
                          std::vector<int>& particle_indexes,
                          std::string radius_att_name,
                          std::string weight_att_name);

  //! copy constructor
  IMPParticlesAccessPoint( const IMPParticlesAccessPoint &other);

  void ReSelect(std::vector<int>& particle_indexes);

  int get_size() const {
    return particle_indexes.size();
  }
  float get_x(int ind) const {
    return model->get_particle(ind)->get_value(x_key);
  }
  float get_y(int ind) const {
    return model->get_particle(ind)->get_value(y_key);
  }
  float get_z(int ind) const {
    return model->get_particle(ind)->get_value(z_key);
  }
  float get_r(int ind) const {
    return model->get_particle(ind)->get_value(radius_key);
  }
  float get_w(int ind) const {
    return model->get_particle(ind)->get_value(weight_key);
  }
  void set_x(unsigned int ind, float xval) {
    model->get_particle(ind)->set_value(x_key,xval);
  }
  void set_y(unsigned int ind, float yval) {
    model->get_particle(ind)->set_value(y_key,yval);
  }
  void set_z(unsigned int ind, float zval) {
    model->get_particle(ind)->set_value(z_key,zval);
  }

  FloatKey get_x_key() const { return x_key;}
  FloatKey get_y_key() const { return y_key;}
  FloatKey get_z_key() const { return z_key;}
private:

  void create_keys(std::string radius_att_name_, std::string weight_att_name_);

  Model *model;
  std::vector<int> particle_indexes;
  FloatKey radius_key,weight_key,x_key,y_key,z_key;
};

} // namespace IMP

#endif /* _IMP_PARTICLES_ACCESS_POINT_H */
