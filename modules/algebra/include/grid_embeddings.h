/**
 *  \file IMP/algebra/grid_embeddings.h
 *  \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_GRID_EMBEDDINGS_H
#define IMPALGEBRA_GRID_EMBEDDINGS_H

#include <IMP/algebra/algebra_config.h>

#include <IMP/base/types.h>
#include "grid_indexes.h"
#include "Vector3D.h"
#include "BoundingBoxD.h"
#include <boost/iterator/transform_iterator.hpp>
#include <IMP/base/map.h>
#include <IMP/base/Vector.h>
#include <IMP/base/check_macros.h>
#include <IMP/base/exception.h>

#include <limits>

IMPALGEBRA_BEGIN_NAMESPACE

/** Embed a grid as an evenly spaced axis aligned grid.*/
template <int D>
class DefaultEmbeddingD {
  VectorD<D> origin_;
  VectorD<D> unit_cell_;
  // inverse
  VectorD<D> inverse_unit_cell_;
  template <class O>
  VectorD<D> get_elementwise_product(VectorD<D> v0,
                                     const O &v1) const {
    for (unsigned int i=0; i< get_dimension(); ++i) {
      v0[i]*= v1[i];
    }
    return v0;
  }
  template <class O>
  VectorD<D> get_uniform_offset(const O &v0,
                                double o) const {
    Floats ret(get_dimension());
    for (unsigned int i=0; i< get_dimension(); ++i) {
      ret[i]= v0[i]+o;
    }
    return VectorD<D>(ret.begin(), ret.end());
  }
  void initialize_from_box(Ints ns,
                           const BoundingBoxD<D> &bb) {
    Floats nuc(bb.get_dimension());
    for (unsigned int i=0; i< bb.get_dimension(); ++i) {
      double side= bb.get_corner(1)[i]- bb.get_corner(0)[i];
      IMP_USAGE_CHECK(side>0, "Can't have flat grid");
      nuc[i]= side/ns[i];
    }
    set_unit_cell(VectorD<D>(nuc.begin(), nuc.end()));
    set_origin(bb.get_corner(0));
  }
 public:
  DefaultEmbeddingD(const VectorD<D> &origin,
                    const VectorD<D> &cell) {
    set_origin(origin);
    set_unit_cell(cell);
  }
  DefaultEmbeddingD(){
  }
  void set_origin(const VectorD<D> &o) {
    origin_=o;
  }
  const VectorD<D> get_origin() const {
    return origin_;
  }
  unsigned int get_dimension() const {
    return get_origin().get_dimension();
  }
  void set_unit_cell(const VectorD<D> &o) {
    unit_cell_=o;
    Floats iuc(o.get_dimension());
    for (unsigned int i=0; i < get_dimension(); ++i) {
      iuc[i]=1.0/unit_cell_[i];
    }
    inverse_unit_cell_=VectorD<D>(iuc.begin(), iuc.end());
  }
#ifndef IMP_DOXYGEN
  //! Return the vector (1/u[0], 1/u[1], 1/u[2])
  const VectorD<D>& get_inverse_unit_cell() const {
    return inverse_unit_cell_;
  }
#endif
  //! Return the unit cell, relative to the origin.
  /** That is, the unit cell is
      \code
      BoundingBoxD<D>(get_zeros_vector_d<D>(),get_unit_cell());
      \endcode
  */
  const VectorD<D>& get_unit_cell() const {
    return unit_cell_;
  }
  //! Return the index that would contain the voxel if the grid extended there
  /** For example vectors below the "lower left" corner of the grid have
      indexes with all negative components. This operation will always
      succeed.
  */
  ExtendedGridIndexD<D> get_extended_index(const VectorD<D> &o) const {
    boost::scoped_array<int> index(new int[origin_.get_dimension()]);
    for (unsigned int i=0; i< get_dimension(); ++i ) {
      double d = o[i] - origin_[i];
      double fi= d*inverse_unit_cell_[i];
      index[i]= static_cast<int>(std::floor(fi));
    }
    return ExtendedGridIndexD<D>(index.get(), index.get()+get_dimension());
  }
  GridIndexD<D> get_index(const VectorD<D> &o) const {
    boost::scoped_array<int> index(new int[origin_.get_dimension()]);
    for (unsigned int i=0; i< get_dimension(); ++i ) {
      double d = o[i] - origin_[i];
      double fi= d*inverse_unit_cell_[i];
      index[i]= static_cast<int>(std::floor(fi));
    }
    return GridIndexD<D>(index.get(), index.get()+get_dimension());
  }
  /** \name Center
      Return the coordinates of the center of the voxel.
      @{
  */
  VectorD<D> get_center(const ExtendedGridIndexD<D> &ei) const {
    return origin_+ get_elementwise_product(get_unit_cell(),
                                            get_uniform_offset(ei, .5));
  }
  VectorD<D> get_center(const GridIndexD<D> &ei) const {
    return origin_+ get_elementwise_product(get_unit_cell(),
                                            get_uniform_offset(ei, .5));
  }
  /** @} */

  /** \name Bounding box
      Return the bounding box of the voxel.
      @{
  */
  BoundingBoxD<D> get_bounding_box(const ExtendedGridIndexD<D> &ei) const {
    return BoundingBoxD<D>(origin_+ get_elementwise_product(unit_cell_,ei),
                           origin_
                           + get_elementwise_product(unit_cell_,
                                                     get_uniform_offset(ei,
                                                                        1)));
  }
  BoundingBoxD<D> get_bounding_box(const GridIndexD<D> &ei) const {
    return BoundingBoxD<D>(origin_+ get_elementwise_product(unit_cell_,ei),
                           origin_
                           + get_elementwise_product(unit_cell_,
                                                     get_uniform_offset(ei,
                                                                        1)));
  }
  /** @} */
  IMP_SHOWABLE_INLINE(DefaultEmbeddingD, out<< "origin: "<<  origin_
                      << "  unit cell: " << unit_cell_);
};

#if !defined(IMP_DOXYGEN)
typedef DefaultEmbeddingD<1> DefaultEmbedding1D;
typedef base::Vector<DefaultEmbedding1D> DefaultEmbedding1Ds;


typedef DefaultEmbeddingD<2> DefaultEmbedding2D;
typedef base::Vector<DefaultEmbedding2D> DefaultEmbedding2Ds;

typedef DefaultEmbeddingD<3> DefaultEmbedding3D;
typedef base::Vector<DefaultEmbedding3D> DefaultEmbedding3Ds;

typedef DefaultEmbeddingD<4> DefaultEmbedding4D;
typedef base::Vector<DefaultEmbedding4D> DefaultEmbedding4Ds;

typedef DefaultEmbeddingD<5> DefaultEmbedding5D;
typedef base::Vector<DefaultEmbedding5D> DefaultEmbedding5Ds;

typedef DefaultEmbeddingD<6> DefaultEmbedding6D;
typedef base::Vector<DefaultEmbedding6D> DefaultEmbedding6Ds;

typedef DefaultEmbeddingD<-1> DefaultEmbeddingKD;
typedef base::Vector<DefaultEmbeddingKD> DefaultEmbeddingKDs;
#endif


/** Embedding of a grid as log-evenly spaced axis aligned grid.*/
template <int D>
class LogEmbeddingD {
  VectorD<D> origin_;
  VectorD<D> unit_cell_;
  VectorD<D> base_;
  template <class O>
  VectorD<D> get_coordinates(const O &index) const {
    VectorD<D> ret= origin_;
    for (unsigned int i=0; i< unit_cell_.get_dimension(); ++i) {
      IMP_USAGE_CHECK(index[i] >=0, "Out of range index in log graph.'");
      if (base_[i] != 1) {
        IMP_USAGE_CHECK(index[i] >= 0,
                        "Log grid axis must have positive index.");
        ret[i]+=unit_cell_[i]*(1.0-std::pow(base_[i], index[i]))
            /(1.0-base_[i]);
      } else {
        ret[i]+=unit_cell_[i]*index[i];
      }
    }
    return ret;
  }
  template <class O>
  VectorD<D> get_uniform_offset(const O &v0,
                                double o) const {
    Floats ret(get_dimension());
    for (unsigned int i=0; i< get_dimension(); ++i) {
      ret[i]= v0[i]+o;
    }
    return VectorD<D>(ret.begin(), ret.end());
  }
  void initialize_from_box(Ints ns,
                           const BoundingBoxD<D> &bb) {
    Floats nuc(bb.get_dimension());
    for (unsigned int i=0; i< bb.get_dimension(); ++i) {
      double side= bb.get_corner(1)[i]- bb.get_corner(0)[i];
      IMP_USAGE_CHECK(side>0, "Can't have flat grid");
      nuc[i]= side/ns[i];
    }
    set_unit_cell(VectorD<D>(nuc.begin(), nuc.end()));
    set_origin(bb.get_corner(0));
  }
 public:
  LogEmbeddingD(const VectorD<D> &origin,
                const VectorD<D> &cell,
                const VectorD<D> &base) {
    set_origin(origin);
    set_unit_cell(cell, base);
  }
  /** Embedding of a grid as a log-evenly distributed axis-aligned grid
      over the bounding box bb.

      @param bb the bounding box in which the grid is embedded
      @param bases bases[i] is a positive log base used for the grid
                   spacing in dimension i. Set base[i] to 1 in order
                   to create aa standard evenly spaced grid along
                   dimension i.
      @param counts counts[i] is the number of discrete points in dimension i
      @param bound_centers if true, then the bounding box tightly bounds
             the centers of the voxels, not their extents.
  */
  LogEmbeddingD(const BoundingBoxD<D> &bb,
                const VectorD<D> &bases,
                const Ints &counts,
                bool bound_centers=false) {
    set_origin(bb.get_corner(0));
    VectorD<D> cell=bb.get_corner(0);
    for (unsigned int i=0; i< bases.get_dimension(); ++i) {
      IMP_ALWAYS_CHECK( bases[i] > 0,
                        "LogEmbedding base #" << i << " cannot be negative",
                        IMP::base::ValueException );
      // cell[i](1-base[i]^counts[i])/(1-base[i])=width[i]
      if (bases[i]!= 1) {
        cell[i]= (bb.get_corner(1)[i]-bb.get_corner(0)[i])*(bases[i]-1)
            /(std::pow(bases[i], counts[i])-1.0);
      } else {
        cell[i]= (bb.get_corner(1)[i]-bb.get_corner(0)[i])/counts[i];
      }
      IMP_INTERNAL_CHECK(.9*cell[i] < bb.get_corner(1)[i]-bb.get_corner(0)[i],
                         "Too large a cell side");
      IMP_INTERNAL_CHECK(cell[i] >0, "Non-positive cell side");
    }
    set_unit_cell(cell, bases);
    if (bound_centers) {
      VectorD<D> lower_corner
          = get_center(GridIndexD<D>(Ints(bases.get_dimension(), 0)));
      VectorD<D> upper_corner
          = get_coordinates(get_uniform_offset(GridIndexD<D>(counts), -.5));
      VectorD<D> extents= upper_corner-lower_corner;
      VectorD<D> uc= cell;
      VectorD<D> orig=uc;
      for (unsigned int i=0; i< uc.get_dimension(); ++i) {
        uc[i]= (bb.get_corner(1)[i]-bb.get_corner(0)[i])/extents[i] * uc[i];
        if (base_[i]==1) {
          orig[i]= bb.get_corner(0)[i]-.5*uc[i];
        } else {
          /*orig[i]+ uc[i]*(1.0-std::pow(base_[i], index[i]))
            /(1.0-base_[i])==bb[i]*/
          orig[i]= bb.get_corner(0)[i]-uc[i]*(1.0-std::pow(bases[i], .5))
              /(1.0-bases[i]);
        }
      }
      set_origin(orig);
      set_unit_cell(uc, bases);
    }
  }
  LogEmbeddingD(const VectorD<D> &,
                const VectorD<D> &) {
    IMP_FAILURE("not supported");
  }
  LogEmbeddingD(){}
  void set_origin(const VectorD<D> &o) {
    origin_=o;
  }
  const VectorD<D> get_origin() const {
    return origin_;
  }
  unsigned int get_dimension() const {
    return get_origin().get_dimension();
  }
  void set_unit_cell(const VectorD<D> &o,
                     const VectorD<D> &base) {
    unit_cell_=o;
    base_=base;
  }
  void set_unit_cell(const VectorD<D> &o) {
    unit_cell_=o;
  }
  //! Return the unit cell, relative to the origin.
  /** That is, the unit cell is
      \code
      BoundingBoxD<D>(get_zeros_vector_d<D>(),get_unit_cell());
      \endcode
  */
  const VectorD<D>& get_unit_cell() const {
    return unit_cell_;
  }
  //! Return the index that would contain the voxel if the grid extended there
  /** For example vectors below the "lower left" corner of the grid have
      indexes with all negative components. This operation will always
      succeed.
  */
  ExtendedGridIndexD<D> get_extended_index(const VectorD<D> &o) const {
    boost::scoped_array<int> index(new int[origin_.get_dimension()]);
    for (unsigned int i=0; i< get_dimension(); ++i ) {
      double d = o[i] - origin_[i];
      // cache everything
      double fi= d/unit_cell_[i];
      double li= std::log(fi)/std::log(base_[i]);
      index[i]= static_cast<int>(std::floor(li));
    }
    return ExtendedGridIndexD<D>(index.get(), index.get()+get_dimension());
  }
  GridIndexD<D> get_index(const VectorD<D> &o) const {
    ExtendedGridIndexD<D> ei=get_extended_index(o);
    return GridIndexD<D>(ei.begin(), ei.end());
  }
  /** \name Center
      Return the coordinates of the center of the voxel.
      @{
  */
  VectorD<D> get_center(const ExtendedGridIndexD<D> &ei) const {
    return get_coordinates(get_uniform_offset(ei, .5));
  }
  VectorD<D> get_center(const GridIndexD<D> &ei) const {
    return get_coordinates(get_uniform_offset(ei, .5));
  }
  /** @} */

  /** \name Bounding box
      Return the bounding box of the voxel.
      @{
  */
  BoundingBoxD<D> get_bounding_box(const ExtendedGridIndexD<D> &ei) const {
    return BoundingBoxD<D>(get_coordinates(ei),
                           get_coordinates(get_uniform_offset(ei,
                                                              1)));
  }
  BoundingBoxD<D> get_bounding_box(const GridIndexD<D> &ei) const {
    return get_bounding_box(ExtendedGridIndexD<D>(ei.begin(), ei.end()));
  }
  /** @} */
  IMP_SHOWABLE_INLINE(LogEmbeddingD, out<< "origin: " << origin_
                      << " base: " << base_);
};




#if !defined(IMP_DOXYGEN)
typedef LogEmbeddingD<1> LogEmbedding1D;
typedef base::Vector<LogEmbedding1D> LogEmbedding1Ds;


typedef LogEmbeddingD<2> LogEmbedding2D;
typedef base::Vector<LogEmbedding2D> LogEmbedding2Ds;

typedef LogEmbeddingD<3> LogEmbedding3D;
typedef base::Vector<LogEmbedding3D> LogEmbedding3Ds;

typedef LogEmbeddingD<4> LogEmbedding4D;
typedef base::Vector<LogEmbedding4D> LogEmbedding4Ds;

typedef LogEmbeddingD<5> LogEmbedding5D;
typedef base::Vector<LogEmbedding5D> LogEmbedding5Ds;

typedef LogEmbeddingD<6> LogEmbedding6D;
typedef base::Vector<LogEmbedding6D> LogEmbedding6Ds;

typedef LogEmbeddingD<-1> LogEmbeddingKD;
typedef base::Vector<LogEmbeddingKD> LogEmbeddingKDs;
#endif


IMPALGEBRA_END_NAMESPACE


#endif  /* IMPALGEBRA_GRID_EMBEDDINGS_H */
