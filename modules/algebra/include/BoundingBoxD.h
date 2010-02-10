/**
 *  \file BoundingBoxD.h   \brief Simple D vector class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_BOUNDING_BOX_D_H
#define IMPALGEBRA_BOUNDING_BOX_D_H

#include "VectorD.h"


IMPALGEBRA_BEGIN_NAMESPACE

/** The BoundingBoxD class provides a unified representation for bounding
    boxes in \imp. Geometric objects should have an associated method like
    get_bounding_box() which creates the bounding boxes of objects.

    \ingroup valid_default
*/
template <unsigned int D>
class BoundingBoxD
{
  void make_empty() {
    for (unsigned int i=0; i< D; ++i) {
      lb_[i]= std::numeric_limits<double>::max();
      ub_[i]=-std::numeric_limits<double>::max();
    }
  }
public:
  // public for swig
  IMP_NO_DOXYGEN(typedef BoundingBoxD<D> This;)

  //! Create an empty bounding box
  BoundingBoxD() {
    make_empty();
  }
  //! Make from the lower and upper corners
  BoundingBoxD(const VectorD<D> &lb,
               const VectorD<D> &ub): lb_(lb), ub_(ub) {
  }
  //! Creating a bounding box containing one point
  BoundingBoxD(const VectorD<D> &v): lb_(v), ub_(v){}

  //! Creating a bounding box from a set of points
  BoundingBoxD(const std::vector<VectorD<D> > &points) {
    make_empty();
    for(unsigned int j=0;j<points.size();j++) {
      operator+=(points[j]);
    }
  }

  //! merge two bounding boxes
  const BoundingBoxD<D>& operator+=(const BoundingBoxD<D> &o) {
    for (unsigned int i=0; i< D; ++i) {
      lb_[i]= std::min(o.get_corner(0)[i], get_corner(0)[i]);
      ub_[i]= std::max(o.get_corner(1)[i], get_corner(1)[i]);
    }
    return *this;
  }

  //! merge two bounding boxes
  const BoundingBoxD<D>& operator+=(const VectorD<D> &o) {
    for (unsigned int i=0; i< D; ++i) {
      lb_[i]= std::min(o[i], get_corner(0)[i]);
      ub_[i]= std::max(o[i], get_corner(1)[i]);
    }
    return *this;
  }

  /** Grow the bounding box by o on all sizes. */
  const BoundingBoxD<D>& operator+=(double o) {
    for (unsigned int i=0; i< D; ++i) {
      lb_[i]= lb_[i]-o;
      ub_[i]= ub_[i]+o;
    }
    return *this;
  }

  //! For 0 return lower corner and 1 upper corner
  const VectorD<D>& get_corner(unsigned int i) const {
    IMP_USAGE_CHECK(i < 2, "Can only use 0 or 1", IndexException);
    if (i==0) return lb_;
    else return ub_;
  }

  bool get_contains(const VectorD<D> &o) const {
    for (unsigned int i=0; i< D; ++i) {
      if (o[i] < get_corner(0)[i]
          || o[i] > get_corner(1)[i]) return false;
    }
    return true;
  }
  //! True if the input bounding box is contained within this bounding box
  bool get_contains(const BoundingBoxD &bb) const {
    return get_contains(bb.get_corner(0)) && get_contains(bb.get_corner(1));
  }

  IMP_SHOWABLE_INLINE(out << lb_ << ": " << ub_);

private:
  VectorD<D> lb_, ub_;
};

#ifndef IMP_DOXYGEN
template <unsigned int D>
inline std::ostream &operator<<(std::ostream &out,
                                const BoundingBoxD<D> &s)
{
  s.show(out);
  return out;
}
#endif

IMPALGEBRA_EXPORT_TEMPLATE(BoundingBoxD<3>);

IMP_NO_SWIG(typedef BoundingBoxD<3> BoundingBox3D;)

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_BOUNDING_BOX_D_H */
