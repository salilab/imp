/**
 *  \file SAXSProfile.h   \brief A class for profile storing and computation
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#include "SAXSProfile.h"

#include <IMP/Vector3D.h>
#include <IMP/core/XYZDecorator.h>

#include <fstream>
#include <string>

using std::ifstream;
using std::cerr;
using std::string;

SAXSProfile::SAXSProfile(float smin, float smax, float delta) :
  min_s_(smin), max_s_(smax), delta_s_(delta)
{
  init();
}

void SAXSProfile::init() {
  profile_.clear();
  int number_of_entries = (max_s_ - min_s_)/delta_s_ +1;
  for(unsigned int i = 0; i<number_of_entries; i++) {
    IntensityEntry entry(min_s_ + i*delta_s_);
    profile_.push_back(entry);
  }
}

void SAXSProfile::read_SAXS_file(const string& file_name) {
  ifstream in_file(file_name.c_str());

  if(!in_file) {
    cerr << "Can't open file " << file_name << endl;
    exit(1);
  }

  // TODO: handle profiles with multiple comment lines
  // remove first comment line
  string line;
  getline(in_file, line);
  // TODO: handle profile reading without error column

  IntensityEntry entry;
  while(in_file >> entry) {
    if(entry.get_error() == 0.0) {
      cerr << "Zero intensity error! setting to 1" << endl;
      entry.set_error(1.0);
    }
    profile_.push_back(entry);
  }

  cerr << "Number of entries read " << profile_.size() << endl;
  in_file.close();

  // TODO: detemine smin, smax and delta
}

void SAXSProfile::calculate_profile(const vector<IMP::Particle*>& particles) {
  init();

  // copy coordinates in advance, to avoid n^2 copy operations
  vector<IMP::Vector3D> coordinates;
  for(unsigned int i = 0; i < particles.size(); i++) {
    coordinates.push_back(
             IMP::core::XYZDecorator::cast(particles[i]).get_coordinates());
  }

  // itarate over pairs of atoms
  // loop1
  for(unsigned int i = 0; i < coordinates.size(); i++) {
    const vector<float>& factors1 = ff_table_->getFormFactors(particles[i]);
    // loop2
    for(unsigned int j = i+1; j < coordinates.size(); j++) {
      const vector<float>& factors2 = ff_table_->getFormFactors(particles[j]);
      float dist = get_distance(coordinates[i], coordinates[j]);

      // iterate over intensity profile (assumes initialized profile: s, I(s)=0)
      for(unsigned int k = 0; k < profile_.size(); k++) {
        float x = dist * profile_[k].get_s();
        x = sin(x)/x;
        profile_[k].set_intensity(profile_[k].get_intensity() +
                                  2*x*factors1[i]*factors2[j]);
      } // end of profile iteration
    } // end of loop 2

    // add autocorrelation part
    for(unsigned int k = 0; k < profile_.size(); k++) {
      profile_[k].set_intensity(profile_[k].get_intensity() +
                                factors1[i]*factors1[i]);
    }
  } // end of loop1
}
