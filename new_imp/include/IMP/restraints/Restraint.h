/**
 *  \file Restraint.h     \brief Abstract base class for all restraints.
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_RESTRAINT_H
#define __IMP_RESTRAINT_H

#include <vector>
#include <iostream>

#include "../IMP_config.h"
#include "../ModelData.h"
#include "../ScoreFunc.h"
#include "../boost/noncopyable.h"

namespace IMP
{

class Model;

//! Abstract class for representing restraints
class IMPDLLEXPORT Restraint : public boost::noncopyable
{
  friend class Model;

  //! Give accesss to model particle data.
  /** \param[in] model_data All particle data in the model.
   */
  void set_model_data(ModelData* model_data);

public:
  Restraint(std::string name=std::string());
  virtual ~Restraint();

  //! Return the score for this restraint for the current state of the model.
  /** \param[in] calc_deriv if true, first derivatives should also be
                            calculated.
      \return Current score.
   */
  virtual Float evaluate(bool calc_deriv) = 0;

  //! Set whether the restraint is active i.e. if it should be evaluated.
  /** \param[in] is_active If true, the restraint is active.
   */
  void set_is_active(const bool is_active);

  //! Get whether the restraint is active. i.e. if it should be evaluated.
  /** \return true if the restraint is active.
   */
  bool get_is_active(void) const;

  //! Check if all necessary particles are still active.
  /** If not, inactivate self. Called when at least one model particle
      has been inactivated.
   */
  virtual void check_particles_active(void);

  //! Show the current restraint.
  /** \param[in] out Stream to send restraint description to.
   */
  virtual void show(std::ostream& out = std::cout) const;

  virtual std::string version(void) const {
    return "unknown";
  }
  virtual std::string last_modified_by(void) const {
    return "unknown";
  }

  //! Get the name of the restraint
  const std::string& get_name() const {
    return name_;
  }

  //! Set the name of the restraint
  void set_name(const std::string &name) {
    name_=name;
  }

protected:
  //! all of the particle data
  ModelData* model_data_;

  /** restraint is active if active_ AND particles_active_
      true if restraint has not been deactivated
      if it is not active, evaluate should not be called
   */
  bool is_active_;

  //! true if all particles that restraint uses are active
  bool are_particles_active_;

  //! shouldn't be necessary, but keep around for debugging
  std::vector<Particle*> particles_;

  std::string name_;
};



inline std::ostream &operator<<(std::ostream &out, const Restraint &s)
{
  s.show(out);
  return out;
}


} // namespace IMP

#endif  /* __IMP_RESTRAINT_H */
