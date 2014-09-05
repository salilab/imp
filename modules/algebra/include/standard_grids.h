/**
 *  \file IMP/algebra/standard_grids.h
 *  \brief All grids that are in the Python API should be defined here.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_STANDARD_GRIDS_H
#define IMPALGEBRA_STANDARD_GRIDS_H

#include <IMP/algebra/algebra_config.h>
#include "GridD.h"
#include "grid_ranges.h"
#include "grid_storages.h"
#include "grid_embeddings.h"
IMPALGEBRA_BEGIN_NAMESPACE

#ifdef SWIG
// swig is getting confused
#define IMP_ALGEBRA_SWIG_GRID_METHODS(D, Q)                            \
  GridIndexD<D> add_voxel(const ExtendedGridIndexD<D> &i, const Q &q); \
  Q __getitem__(const GridIndexD<D> i) const;                          \
  void __setitem__(const GridIndexD<D> &i, const Q &q);                \
  Q __getitem__(const VectorD<D> i) const;                             \
  void __setitem__(const VectorD<D> &i, const Q &q);                   \
  BoundingBoxD<D> get_bounding_box(ExtendedGridIndexD<D> v) const;     \
  BoundingBoxD<D> get_bounding_box() const

#else
#define IMP_ALGEBRA_SWIG_GRID_METHODS(D, VT)
#endif

//! A sparse, infinite grid of values.
/** In Python SparseUnboundedIntGrid3D is provided. */
template <int D, class VT>
struct SparseUnboundedGridD
    : public GridD<D, SparseGridStorageD<D, VT, UnboundedGridRangeD<D> >, VT,
                   DefaultEmbeddingD<D> > {
  typedef GridD<D, SparseGridStorageD<D, VT, UnboundedGridRangeD<D> >, VT,
                DefaultEmbeddingD<D> > P;
  SparseUnboundedGridD(double side, const VectorD<D> &origin, VT def = VT())
      : P(side, origin, def) {}
  SparseUnboundedGridD() {}
  IMP_ALGEBRA_SWIG_GRID_METHODS(D, VT);
};

//! A dense grid of values.
/** In Python DenseFloatGrid3D and DenseDoubleGrid3D are provided.
    See IMP_GRID3D_FOREACH_VOXEL() for a useful macro when operating
    on all voxels of the grid.
*/
template <class VT>
struct DenseGrid3D
    : public GridD<3, DenseGridStorageD<3, VT>, VT, DefaultEmbeddingD<3> > {
  typedef GridD<3, DenseGridStorageD<3, VT>, VT, DefaultEmbeddingD<3> > P;
  DenseGrid3D(double side, const BoundingBoxD<3> &bb, VT def = VT())
      : P(side, bb, def) {}
  DenseGrid3D() {}
  IMP_ALGEBRA_SWIG_GRID_METHODS(3, VT);
};

template <int D, class VT>
struct DenseGridD
    : public GridD<D, DenseGridStorageD<D, VT>, VT, DefaultEmbeddingD<D> > {
  typedef GridD<D, DenseGridStorageD<D, VT>, VT, DefaultEmbeddingD<D> > P;
  DenseGridD(double side, const BoundingBoxD<D> &bb, VT def = VT())
      : P(side, bb, def) {}
  DenseGridD(const VectorD<D> &sides, const BoundingBoxD<D> &bb, VT def = VT())
      : P(sides, bb, def) {}
  DenseGridD() {}
  IMP_ALGEBRA_SWIG_GRID_METHODS(D, VT);
};

//! A sparse grid of values.
/** In Python SparseIntGrid3D is provided. */
template <class VT>
struct SparseGrid3D
    : public GridD<3, SparseGridStorageD<3, VT, BoundedGridRangeD<3> >, VT,
                   DefaultEmbeddingD<3> > {
  typedef GridD<3, SparseGridStorageD<3, VT, BoundedGridRangeD<3> >, VT,
                DefaultEmbeddingD<3> > P;
  SparseGrid3D(double side, const BoundingBoxD<3> &bb, VT def = VT())
      : P(side, bb, def) {}
  SparseGrid3D(int xd, int yd, int zd, const BoundingBoxD<3> &bb, VT def = VT())
      : P(xd, yd, zd, bb, def) {}
  SparseGrid3D() {}
  IMP_ALGEBRA_SWIG_GRID_METHODS(3, VT);
};

//! A sparse, infinite grid of values.
/** In Python SparseUnboundedIntGrid3D is provided. */
template <class VT>
struct SparseUnboundedGrid3D
    : public GridD<3, SparseGridStorageD<3, VT, UnboundedGridRangeD<3> >, VT,
                   DefaultEmbeddingD<3> > {
  typedef GridD<3, SparseGridStorageD<3, VT, UnboundedGridRangeD<3> >, VT,
                DefaultEmbeddingD<3> > P;
  SparseUnboundedGrid3D(double side, const Vector3D &origin, VT def = VT())
      : P(side, origin, def) {}
  SparseUnboundedGrid3D(const Vector3D &sides, const Vector3D &origin, VT def = VT())
      : P(sides, origin, def) {}
  SparseUnboundedGrid3D() {}
  IMP_ALGEBRA_SWIG_GRID_METHODS(3, VT);
};

/** A dense grid with logarithmic axes. */
struct DenseFloatLogGridKD : public GridD<-1, DenseGridStorageD<-1, float>,
                                          float, LogEmbeddingD<-1> > {
  typedef GridD<-1, DenseGridStorageD<-1, float>, float, LogEmbeddingD<-1> > P;
  DenseFloatLogGridKD(const Ints &sz, LogEmbeddingD<-1> le) : P(sz, le) {}
  IMP_ALGEBRA_SWIG_GRID_METHODS(-1, float);
};

/** A dense grid with logarithmic axes. */
struct DenseIntLogGrid3D
    : public GridD<3, DenseGridStorageD<3, int>, int, LogEmbeddingD<3> > {
  typedef GridD<3, DenseGridStorageD<3, int>, int, LogEmbeddingD<3> > P;
  DenseIntLogGrid3D(const Ints &sz, LogEmbeddingD<3> le) : P(sz, le) {}
  IMP_ALGEBRA_SWIG_GRID_METHODS(3, int);
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_STANDARD_GRIDS_H */
