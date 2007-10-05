/*
 *  Base_Types.h
 *  IMP
 *
 *  Copyright 2007 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_BASE_TYPES_H
#define __IMP_BASE_TYPES_H

#include <string>

#include "IMP_config.h"

namespace imp
{

// basic types used by IMP
typedef float Float;
typedef int Int;
typedef std::string String;

// if I don't do this as a template, I eliminate the swig errors
class IMPDLLEXPORT Float_Index
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

class IMPDLLEXPORT Int_Index
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
class IMPDLLEXPORT Data_Index
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

#endif  /* __IMP_BASE_TYPES_H */
