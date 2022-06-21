/**
 *  \file RMF/internal/Transform.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_TRANSFORM_H
#define RMF_INTERNAL_TRANSFORM_H

#include <ostream>

#include "RMF/Vector.h"
#include "RMF/config.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/types.h"

RMF_ENABLE_WARNINGS namespace RMF {
  namespace internal {

  class Rotation {
    Vector4 v_;
    double matrix_[3][3];
    void fill_matrix();
    template <class A, class B>
    static double get_dotprod(const A& a, const B& b) {
      return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    }

   public:
    Rotation(Rotation a, Rotation b);
    Rotation() {
      v_[0] = 1;
      v_[1] = 0;
      v_[2] = 0;
      v_[3] = 0;
      fill_matrix();
    }
    Rotation(const Vector4& r) : v_(r) {
      fill_matrix();
    }

    //! Rotate a vector around the origin
    Vector3 get_rotated(const Vector3& o) const {
      Vector3 ret;
      ret[0] = get_dotprod(o, matrix_[0]);
      ret[1] = get_dotprod(o, matrix_[1]);
      ret[2] = get_dotprod(o, matrix_[2]);
      return ret;
    }
    const Vector4& get_quaternion() const { return v_; }
    RMF_SHOWABLE(Rotation, v_);
  };

  class Transform {
    Rotation rot_;
    Vector3 trans_;

   public:
    Transform() {
      trans_[0] = 0;
      trans_[1] = 0;
      trans_[2] = 0;
    }
    Transform(const Transform& a, const Transform& b) : rot_(a.rot_, b.rot_) {
      trans_ = a.get_transformed(b.get_translation());
    }
    Transform(const Rotation& r, const Vector3& t) : rot_(r), trans_(t) {}
    Vector3 get_transformed(const Vector3& o) const {
      Vector3 ret = rot_.get_rotated(o);
      ret[0] += trans_[0];
      ret[1] += trans_[1];
      ret[2] += trans_[2];
      return ret;
    }
    const Vector3& get_translation() const { return trans_; }
    const Vector4& get_rotation() const { return rot_.get_quaternion(); }
    RMF_SHOWABLE(Transform, "Rotation: " << rot_
                                         << " and translation: " << trans_);
  };

  }  // namespace internal
} /* namespace RMF */
RMF_DISABLE_WARNINGS

#endif /* RMF_INTERNAL_TRANSFORM_H */
