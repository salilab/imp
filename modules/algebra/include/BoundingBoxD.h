/**
 *  \file BoundingBoxD.h   \brief Simple D vector class.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_BOUNDING_BOX_D_H
#define IMPALGEBRA_BOUNDING_BOX_D_H

#include "VectorD.h"
#include "Transformation3D.h"

IMPALGEBRA_BEGIN_NAMESPACE

/** The BoundingBoxD class provides a unified representation for bounding
    boxes in \imp. Geometric objects should have an associated method like
    get_bounding_box() which creates the bounding boxes of objects.
    \noncomparable
    \addtogroup geometry
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

  template <class O>
  const BoundingBoxD<D> operator+(const O &o) {
    BoundingBoxD<D> ret(*this);
    ret+= o;
    return ret;
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

IMP_VOLUME_GEOMETRY_METHODS_D(BoundingBox, IMP_NOT_IMPLEMENTED,
                              return (g.get_point(1)[0]- g.get_point(0)[0])
                              *(g.get_point(1)[1]- g.get_point(0)[1])
                              *(g.get_point(1)[2]- g.get_point(0)[2]),
                              return g);


//! Return a bounding box containing the transformed box
inline BoundingBox3D get_transformed(const BoundingBox3D &bb,
                              const Transformation3D &tr) {
  BoundingBox3D nbb;
  for (unsigned int i=0; i< 2; ++i) {
    for (unsigned int j=0; j< 2; ++j) {
      for (unsigned int k=0; k< 2; ++k) {
        algebra::Vector3D v(bb.get_corner(i)[0],
                            bb.get_corner(j)[1],
                            bb.get_corner(k)[2]);
        nbb+= tr.transform(v);
      }
    }
  }
  return nbb;
}

IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_BOUNDING_BOX_D_H */
