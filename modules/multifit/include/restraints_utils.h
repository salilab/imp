/**
 *  \file restraints_utils.h
 *  \brief restraints setup utilities
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_RESTRAINTS_UTILS_H
#define IMPMULTIFIT_RESTRAINTS_UTILS_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/Object.h>
#include <IMP/file.h>
#include "multifit_config.h"
#include "FittingSolutionRecord.h"
IMPMULTIFIT_BEGIN_NAMESPACE
//! Generates filenames for precalculated restraint scores
class IMPMULTIFITEXPORT PairsValues {
public:
  PairsValues(Int first_len,Int second_len) {
    first_len_=first_len;
    second_len_=second_len;
    values_.insert(values_.end(),first_len_*second_len_,0.);
  }
  inline Float get_value(Int first_ind, Int second_ind) const {
    Int ind = first_ind*second_len_+second_ind;
    IMP_INTERNAL_CHECK(static_cast<unsigned int>(ind)<values_.size(),
                       "index out of range \n");
    IMP_INTERNAL_CHECK(first_ind<first_len_,"first index out of range \n");
    return values_[ind];
  }
  void set_value(Int first_ind, Int second_ind,Float val)  {
    Int ind = first_ind*second_len_+second_ind;
    IMP_INTERNAL_CHECK(static_cast<unsigned int>(ind)<values_.size(),
                       "index out of range \n");
    IMP_INTERNAL_CHECK(first_ind<first_len_,"first index out of range \n");
    values_[ind] = val;
  }
protected:
  Int first_len_,second_len_;
  Floats values_;
};
//! Generates filenames for precalculated restraint scores
class IMPMULTIFITEXPORT RestraintFilenameGenerator {
public:
  RestraintFilenameGenerator() {
    path_="./";
  }
  //! Constructor
  /**
   \param[in] path  the path, should end with "/"
   */
  RestraintFilenameGenerator(std::string path) {
    path_=path;
  }
  std::string get_single_filename(Particle *p);
  std::string get_pairwise_filename(Particle *p1,Particle *p2);

  inline const char * get_path() const {return path_.c_str();}
protected:
  std::string path_;
};
IMPMULTIFITEXPORT Floats read_singleton_restraint_values(
   const std::string &filename);
IMPMULTIFITEXPORT PairsValues read_pairwise_restraint_values(
   const std::string &filename,Int first_len,Int second_len);
IMPMULTIFIT_END_NAMESPACE

#endif  /* IMPMULTIFIT_RESTRAINTS_UTILS_H */
