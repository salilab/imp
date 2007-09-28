/*
 *  IMP.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#if !defined(__imp_h)
#define __imp_h 1

#include <cmath>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <iomanip>

namespace imp
{

class Model;
class Particle;
class Rigid_Body;

extern Model& get_model(void);


// basic types used by IMP
typedef float Float;
typedef int Int;
typedef std::string String;

// if I don't do this as a template, I eliminate the swig errors
class Float_Index
{
public:
  Float_Index(int idx = 0) {
    index_ = idx;
  }

  ~Float_Index(void) {
  }

  int index(void) const {
    return index_;
  }

  void set_index(const int idx) {
    index_ = idx;
  }

protected:
  int index_;
};

class Int_Index
{
public:
  Int_Index(int idx = 0) {
    index_ = idx;
  }

  ~Int_Index(void) {
  }

  int index(void) const {
    return index_;
  }

  void set_index(const int idx) {
    index_ = idx;
  }

protected:
  int index_;
};

// indexes for retrieving particle information
template <class T>
class Data_Index
{
public:
  Data_Index(int idx = 0) {
    index_ = idx;
  }

  ~Data_Index(void) {
  }

  int index(void) const {
    return index_;
  }

  void set_index(const int idx) {
    index_ = idx;
  }

protected:
  int index_;
};

// typedefs for the particle variable and attribute indexes
// typedef Data_Index<Float> Float_Index;
typedef Data_Index<String> String_Index;

} // namespace imp

#include "log.h"
#include "Particle.h"
#include "Optimizer.h"
#include "Score_Func.h"
#include "Restraint.h"
#include "Complexes_Restraints.h"
#include "Restraint_Set.h"
#include "Rigid_Body.h"
#include "Model_Data.h"
#include "Model.h"
#include "Model_Loader.h"

#endif


