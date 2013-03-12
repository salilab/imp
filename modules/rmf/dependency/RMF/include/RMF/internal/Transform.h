/**
 *  \file RMF/internal/Transform.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_TRANSFORM_H
#define RMF_INTERNAL_TRANSFORM_H

#include <RMF/config.h>
#include "../types.h"
#include "../infrastructure_macros.h"

RMF_ENABLE_WARNINGS

namespace RMF {
namespace internal {

class Rotation {
  double v_[4];
  double matrix_[3][3];
  void fill_matrix();
  template <class A, class B>
  static double get_dotprod(const A& a, const B& b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
  }
public:
  Rotation(Rotation a, Rotation b);
  Rotation() {
    v_[0] = 1; v_[1] = 0; v_[2] = 0; v_[3] = 0;
    fill_matrix();
  }
  Rotation(const Floats &r) {
    std::copy(r.begin(), r.end(), v_);
  }

  //! Rotate a vector around the origin
  Floats get_rotated(const Floats &o) const {
    Floats ret(3);
    ret[0] = get_dotprod(o, matrix_[0]);
    ret[1] = get_dotprod(o, matrix_[1]);
    ret[2] = get_dotprod(o, matrix_[2]);
    return ret;
  }
  Floats get_quaternion() const {
    return Floats(v_, v_+4);
  }
  RMF_SHOWABLE(Rotation, Floats(v_, v_ + 4));
};

class Transform
{
  Rotation rot_;
  double trans_[3];
public:
  Transform() {
    trans_[0] = 0;
    trans_[1] = 0;
    trans_[2] = 0;
  }
  Transform(const Transform &a,
            const Transform &b): rot_(a.rot_, b.rot_) {
    Floats temp = a.get_transformed(Floats(b.trans_, b.trans_ + 3));
    std::copy(temp.begin(), temp.end(), trans_);
  }
  Transform(const Rotation& r,
            const Floats  &t):
    rot_(r) {
    std::copy(t.begin(), t.end(), trans_);
  }
  Floats get_transformed(const Floats &o) const {
    Floats ret = rot_.get_rotated(o);
    ret[0] += trans_[0];
    ret[1] += trans_[1];
    ret[2] += trans_[2];
    return ret;
  }
  Floats get_translation() const {
    return Floats(trans_, trans_+3);
  }
  Floats get_rotation() const {
    return rot_.get_quaternion();
  }
  RMF_SHOWABLE(Transform, "Rotation: " << rot_ << " and translation: "
                                       << Floats(trans_, trans_ + 3));
};

}   // namespace internal
} /* namespace RMF */
RMF_DISABLE_WARNINGS


#endif /* RMF_INTERNAL_TRANSFORM_H */
