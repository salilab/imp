/**
 *  \file SAXSProfile.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPSAXS_SAXS_PROFILE_H
#define IMPSAXS_SAXS_PROFILE_H

#include <IMP/Model.h>

#include <iostream>
#include <vector>

using std::ostream;
using std::istream;
using std::endl;
using std::vector;

#include "FormFactorTable.h"

/* Basic profile class, can be initialized from the input file
   (experimental or theoretical) or computed from the Model or
   RigidBodies of the Model (theoretical)
*/
class SAXSProfile {
public:
  //! init from file
  SAXSProfile(const string& file_name);

  //! init for theoretical profile
  SAXSProfile(float smin, float smax, float delta);

  class IntensityEntry {
  public:
    IntensityEntry() : s_(0.0), intensity_(0.0), error_(1.0) {}
    IntensityEntry(float s) : s_(s), intensity_(0.0), error_(1.0) {}

    friend ostream& operator<<(ostream& s, const IntensityEntry& e) {
      return s << e.s_ << " , " << e.intensity_ << " , " << e.error_ << endl;
    }

    friend istream& operator>>(istream& s, IntensityEntry& e) {
      return s >> e.s_ >> e.intensity_ >> e.error_;
    }

    float get_s() const { return s_; }
    float get_intensity() const { return intensity_; }
    float get_error() const { return error_; }

    void set_s(float s) { s_ = s; }
    void set_intensity(float intensity) { intensity_ = intensity; }
    void set_error(float error) { error_ = error; }

  protected:
    float s_;
    float intensity_;
    float error_;
  };

  //! reads SAXS profile from file
  void read_SAXS_file(const string& file_name);

  //! computes theoretical profile
  void calculate_profile(const vector<IMP::Particle*>& particles);

  //! computes theoretical profile contribution from iter-molecular
  // interactions between the particles
  void calculate_profile(const vector<IMP::Particle*>& particles1,
                         const vector<IMP::Particle*>& particles2);

private:
  void init();

protected:
  vector<IntensityEntry> profile_; // the profile
  float min_s_, max_s_; // minimal and maximal s values  in the profile
  float delta_s_; // profile sampling resolution
  FormFactorTable* ff_table_; // pointer to form factors table
};

#endif /* IMPSAXS_SAXS_PROFILE_H */
