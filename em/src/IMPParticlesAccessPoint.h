#ifndef _IMP_PARTICLES_ACCESS_POINT_H
#define _IMP_PARTICLES_ACCESS_POINT_H

#include <vector>
#include <map>

#include "IMP/Model.h"
#include "IMP/Particle.h"

#include "IMPEM_config.h"
#include "ParticlesAccessPoint.h"



namespace IMP
{

namespace em
{

#define X_ATT_NAME "x"
#define Y_ATT_NAME "y"
#define Z_ATT_NAME "z"

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

  void reselect(std::vector<int>& particle_indexes);

  int get_size() const {
    return particle_indexes_.size();
  }
  float get_x(int ind) const {
    int ind_val = vec_pos2particle_ind(ind);
    return model_->get_particle(ind_val)->get_value(x_key_);
  }
  float get_y(int ind) const {
    int ind_val = vec_pos2particle_ind(ind);
    return model_->get_particle(ind_val)->get_value(y_key_);
  }
  float get_z(int ind) const {
    int ind_val = vec_pos2particle_ind(ind);
    return model_->get_particle(ind_val)->get_value(z_key_);
  }
  float get_r(int ind) const {
    int ind_val = vec_pos2particle_ind(ind);
    return model_->get_particle(ind_val)->get_value(radius_key_);
  }
  float get_w(int ind) const {
    int ind_val = vec_pos2particle_ind(ind);
    return model_->get_particle(ind_val)->get_value(weight_key_);
  }
  void set_x(unsigned int ind, float xval) {
    int ind_val = vec_pos2particle_ind(ind);
    model_->get_particle(ind_val)->set_value(x_key_, xval);
  }
  void set_y(unsigned int ind, float yval) {
    int ind_val = vec_pos2particle_ind(ind);
    model_->get_particle(ind_val)->set_value(y_key_, yval);
  }
  void set_z(unsigned int ind, float zval) {
    int ind_val = vec_pos2particle_ind(ind);
    model_->get_particle(ind_val)->set_value(z_key_, zval);
  }

  FloatKey get_x_key() const { return x_key_;}
  FloatKey get_y_key() const { return y_key_;}
  FloatKey get_z_key() const { return z_key_;}

  //! Given a position in the particle_indexes vector, returns the stored index.
  /** \param[in] position in the particle_indexes vector
      \return particle_indexes[vec_pos]
      \exception std::out_of_range vec_pos is out of range
      For example if particle_indexes vector is [4,5,6] then
      vec_pos2particle_ind(0) returns 4
   */  
  int vec_pos2particle_ind(int vec_pos) const {
    IMP_assert(((unsigned int)vec_pos) < particle_indexes_.size(),
               "the requested vec_pos " << vec_pos << " is out of range" );
    return particle_indexes_[vec_pos];
  }
private:

  void create_keys(std::string radius_att_name, std::string weight_att_name);

  Model *model_;
  std::vector<int> particle_indexes_;
  FloatKey radius_key_, weight_key_, x_key_, y_key_, z_key_;
};

} // namespace em

} // namespace IMP

#endif /* _IMP_PARTICLES_ACCESS_POINT_H */
