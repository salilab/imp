/**
 *  \file IMP/algebra/BoundingBoxD.h   \brief A bounding box in D dimensions.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_BOUNDING_BOX_D_H
#define IMPALGEBRA_BOUNDING_BOX_D_H

#include <IMP/algebra/algebra_config.h>
#include "VectorD.h"
#include "internal/utility.h"
#include "algebra_macros.h"
#include <IMP/exception.h>

IMPALGEBRA_BEGIN_NAMESPACE

//! An axis-aligned bounding box.
/** The BoundingBoxD class provides a unified representation for bounding
    boxes in \imp. Geometric objects should have an associated namespace
    method like get_bounding_box() which returns the bounding boxes of objects.

    \note This class is a \ref geometricprimitives "geometric primitive".
*/
template <int D>
class BoundingBoxD {
  void make_empty() {
    for (int i = 0; i < D; ++i) {
      b_[0][i] = std::numeric_limits<double>::max();
      b_[1][i] = -std::numeric_limits<double>::max();
    }
  }

 public:
  //! Create an empty bounding box
  BoundingBoxD() {
    /* Let SWIG make uninitialized BoundingBoxKD objects (see issue #843).
       Otherwise, any function that returns a BoundingBoxKD will fail, since
       SWIG generates code that looks like:
         BoundingBoxKD result;
         ...
         result = call_imp_function()
       The usage check for BoundingBoxKD is moved from here to the SWIG wrapper
       itself, so a user still can't make a default-constructed BoundingBoxKD
       in Python.
     */
#if defined(IMP_SWIG_WRAPPER)
    if (D > 0) {
      make_empty();
    }
#else
    IMP_USAGE_CHECK(D > 0, "The constructor cannot be used "
                           << "with a variable dimension bounding box.");

    make_empty();
#endif
  }
  //! Create an empty bounding box
  explicit BoundingBoxD(unsigned int d) {
    IMP_USAGE_CHECK(D == -1, "The constructor can only be used "
                             << "with a variable dimension bounding box.");
    Floats lb(d), ub(d);
    for (unsigned int i = 0; i < d; ++i) {
      lb[i] = std::numeric_limits<double>::max();
      ub[i] = -std::numeric_limits<double>::max();
    }
    b_[0] = VectorD<D>(lb.begin(), lb.end());
    b_[1] = VectorD<D>(ub.begin(), ub.end());
  }
  //! Make from the lower and upper corners
  BoundingBoxD(const VectorD<D> &lb, const VectorD<D> &ub) {
    b_[0] = lb;
    b_[1] = ub;
    IMP_IF_CHECK(IMP::USAGE) {
      for (unsigned int i = 0; i < lb.get_dimension(); ++i) {
        IMP_USAGE_CHECK(lb[i] <= ub[i], "Invalid bounding box");
      }
    }
  }
  //! Creating a bounding box containing one point
  explicit BoundingBoxD(const VectorD<D> &v) {
    b_[0] = v;
    b_[1] = v;
  }

  //! Creating a bounding box from a set of points
  BoundingBoxD(const Vector<VectorD<D> > &points) {
    make_empty();
    for (unsigned int j = 0; j < points.size(); j++) {
      operator+=(points[j]);
    }
  }

  unsigned int get_dimension() const { return get_corner(0).get_dimension(); }

  //! Extend the current bounding box to include the other
  const BoundingBoxD<D> &operator+=(const BoundingBoxD<D> &o) {
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      b_[0][i] = std::min(o.get_corner(0)[i], get_corner(0)[i]);
      b_[1][i] = std::max(o.get_corner(1)[i], get_corner(1)[i]);
    }
    return *this;
  }

  //! Extend the current bounding box to include the point
  const BoundingBoxD<D> &operator+=(const VectorD<D> &o) {
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      b_[0][i] = std::min(o[i], b_[0][i]);
      b_[1][i] = std::max(o[i], b_[1][i]);
    }
    return *this;
  }

  //! Grow the bounding box by o on all sizes.
  const BoundingBoxD<D> &operator+=(double o) {
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      b_[0][i] = b_[0][i] - o;
      b_[1][i] = b_[1][i] + o;
    }
    return *this;
  }
  //! Returning a bounding box containing both
  template <class O>
  const BoundingBoxD<D> operator+(const BoundingBoxD<D> &o) const {
    BoundingBoxD<D> ret(*this);
    ret += o;
    return ret;
  }
  //! Return a bounding box grown by o on all sides
  template <class O>
  const BoundingBoxD<D> operator+(const O &o) const {
    BoundingBoxD<D> ret(*this);
    ret += o;
    return ret;
  }

  //! For 0 return lower corner and for 1, the upper corner
  const VectorD<D> &get_corner(unsigned int i) const {
    IMP_USAGE_CHECK(i < 2, "Can only use 0 or 1");
    return b_[i];
  }

  //! True if the point o is contained within this bounding box
  bool get_contains(const VectorD<D> &o) const {
    for (unsigned int i = 0; i < get_dimension(); ++i) {
      if (o[i] < get_corner(0)[i] || o[i] > get_corner(1)[i]) return false;
    }
    return true;
  }
  //! True if the input bounding box is completely contained within this one
  bool get_contains(const BoundingBoxD &bb) const {
    return get_contains(bb.get_corner(0)) && get_contains(bb.get_corner(1));
  }

  IMP_SHOWABLE_INLINE(BoundingBoxD, out << b_[0] << ": " << b_[1]);

 private:
  VectorD<D> b_[2];
};
//! See BoundingBoxD
template <int D>
inline double get_volume(const BoundingBoxD<D> &bb) {
  double v = 1;
  for (unsigned int i = 0; i < bb.get_dimension(); ++i) {
    v *= bb.get_corner(1)[i] - bb.get_corner(0)[i];
  }
  return v;
}

IMP_VOLUME_GEOMETRY_METHODS_D(BoundingBox, bounding_box, IMP_UNUSED(g);
                              IMP_NOT_IMPLEMENTED,
                              return (g.get_corner(1)[0] - g.get_corner(0)[0]) *
                                     (g.get_corner(1)[1] - g.get_corner(0)[1]) *
                                     (g.get_corner(1)[2] - g.get_corner(0)[2]),
                              return g);

//! Box with radius one
/** \see BoundingBoxD */
template <unsigned int D>
inline BoundingBoxD<D> get_unit_bounding_box_d() {
  return BoundingBoxD<D>(-get_ones_vector_d<D>(), get_ones_vector_d<D>());
}

//! Box with radius one
/** \see BoundingBoxD */
inline BoundingBoxD<-1> get_unit_bounding_box_kd(unsigned int d) {
  return BoundingBoxD<-1>(-get_ones_vector_kd(d), get_ones_vector_kd(d));
}

//! Cube with radius of length \c radius
/** \see BoundingBoxD */
template <unsigned int D>
inline BoundingBoxD<D> get_cube_d(double radius) {
  return BoundingBoxD<D>(-radius * get_ones_vector_d<D>(),
                         radius * get_ones_vector_d<D>());
}

//! Cube with radius of length \c side
/** \see BoundingBoxD */
inline BoundingBoxD<-1> get_cube_kd(unsigned int d, double radius) {
  return BoundingBoxD<-1>(-radius * get_ones_vector_kd(d),
                          radius * get_ones_vector_kd(d));
}

//! Return true if they intersect
/** \see BoundingBoxD */
template <int D>
inline bool get_interiors_intersect(const BoundingBoxD<D> &a,
                                    const BoundingBoxD<D> &b) {
  IMP_USAGE_CHECK(a.get_dimension() == b.get_dimension(),
                  "Dimensions of bounding boxes don't match.");
  for (unsigned int i = 0; i < a.get_dimension(); ++i) {
    if (a.get_corner(0)[i] > b.get_corner(1)[i]) return false;
    if (b.get_corner(0)[i] > a.get_corner(1)[i]) return false;
  }
  return true;
}

//! Return the intersecting bounding box
/** \see BoundingBoxD */
template <int D>
inline BoundingBoxD<D> get_intersection(const BoundingBoxD<D> &a,
                                        const BoundingBoxD<D> &b) {
  /* Make sure that for D=-1 the vectors ic[01] get the correct dimension */
  VectorD<D> ic0 = a.get_corner(0);
  VectorD<D> ic1 = a.get_corner(1);
  // set low
  int j = 0;
  for (unsigned int i = 0; i < a.get_dimension(); ++i) {
    if (a.get_corner(j)[i] > b.get_corner(j)[i]) {
      ic0[i] = a.get_corner(j)[i];
    } else {
      ic0[i] = b.get_corner(j)[i];
    }
  }
  // set top
  j = 1;
  for (unsigned int i = 0; i < a.get_dimension(); ++i) {
    if (a.get_corner(j)[i] < b.get_corner(j)[i]) {
      ic1[i] = a.get_corner(j)[i];
    } else {
      ic1[i] = b.get_corner(j)[i];
    }
  }
  return BoundingBoxD<D>(ic0, ic1);
}

//! Return the union bounding box
/** This is the same as doing a+b.
    \see BoundingBoxD
*/
template <int D>
inline BoundingBoxD<D> get_union(BoundingBoxD<D> a, const BoundingBoxD<D> &b) {
  a += b;
  return a;
}

//! Return the maximum axis aligned extent
/** \see BoundingBoxD */
template <int D>
inline double get_maximum_length(const BoundingBoxD<D> &a) {
  double e = a.get_corner(1)[0] - a.get_corner(0)[0];
  for (unsigned int i = 1; i < a.get_dimension(); ++i) {
    double ce = a.get_corner(1)[0] - a.get_corner(0)[0];
    e = std::max(ce, e);
  }
  return e;
}

//! Return a list of the 2^D bounding points for the bounding box
/** \see BoundingBoxD */
template <int D>
inline Vector<VectorD<D> > get_vertices(const BoundingBoxD<D> &bb) {
  if (D == 1) {
    Vector<VectorD<D> > ret(2);
    ret[0] = bb.get_corner(0);
    ret[1] = bb.get_corner(1);
    return ret;
  }
  if (D == -1) {
    IMP_NOT_IMPLEMENTED;
  }
  VectorD<internal::DMinus1<D>::D> c0, c1;
  for (int i = 0; i < D - 1; ++i) {
    c0[i] = bb.get_corner(0)[i];
    c1[i] = bb.get_corner(1)[i];
  }
  BoundingBoxD<internal::DMinus1<D>::D> bbm1(c0, c1);
  Vector<VectorD<internal::DMinus1<D>::D> > recurse = get_vertices(bbm1);
  Vector<VectorD<D> > ret;
  for (unsigned int i = 0; i < recurse.size(); ++i) {
    VectorD<D> cur;
    for (int j = 0; j < D - 1; ++j) {
      cur[j] = recurse[i][j];
    }
    cur[D - 1] = bb.get_corner(0)[D - 1];
    ret.push_back(cur);
    cur[D - 1] = bb.get_corner(1)[D - 1];
    ret.push_back(cur);
  }
  return ret;
}

//! Return the edges of the box as indices into the vertices list
/** \see BoundingBoxD */
inline IntPairs get_edges(const BoundingBoxD<3> &) {
  static const IntPair edges[12] = {
      IntPair(0, 1), IntPair(0, 2), IntPair(0, 4), IntPair(1, 3),
      IntPair(1, 5), IntPair(2, 3), IntPair(2, 6), IntPair(3, 7),
      IntPair(4, 5), IntPair(4, 6), IntPair(5, 7), IntPair(6, 7)};
  static IntPairs ret(edges, edges + 12);
  return ret;
}

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_BOUNDING_BOX_D_H */
