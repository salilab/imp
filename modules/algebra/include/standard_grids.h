/**
 *  \file IMP/algebra/standard_grids.h
 *  \brief A class to represent a voxel grid.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
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

// They are created with %template in swig to get around inclusion order issues
#ifndef SWIG

/** A sparse, infinite grid of values. In python SparseUnboundedIntGrid3D
    is provided.*/
template <int D, class VT>
struct SparseUnboundedGridD
    : public GridD<D, SparseGridStorageD<D, VT, UnboundedGridRangeD<D> >, VT,
                   DefaultEmbeddingD<D> > {
  typedef GridD<D, SparseGridStorageD<D, VT, UnboundedGridRangeD<D> >, VT,
                DefaultEmbeddingD<D> > P;
  SparseUnboundedGridD(double side, const VectorD<D> &origin, VT def = VT())
      : P(side, origin, def) {}
  SparseUnboundedGridD() {}
  SparseUnboundedGridD(double side, unsigned int d, const VT &def = VT())
      : P(side, d, def) {}

};

/** A dense grid of values. In python DenseFloatGrid3D and DenseDoubleGrid3D are
    provided. See IMP_GRID3D_FOREACH_VOXEL() for a useful macro when operating
    on all voxels of the grid.
*/
template <class VT>
struct DenseGrid3D
    : public GridD<3, DenseGridStorageD<3, VT>, VT, DefaultEmbeddingD<3> > {
  typedef GridD<3, DenseGridStorageD<3, VT>, VT, DefaultEmbeddingD<3> > P;
  DenseGrid3D(double side, const BoundingBoxD<3> &bb, VT def = VT())
      : P(side, bb, def) {}
  DenseGrid3D(int xd, int yd, int zd, const BoundingBoxD<3> &bb, VT def = VT())
      : P(xd, yd, zd, bb, def) {}
  DenseGrid3D() {}

};

/** A sparse grid of values. In python SparseIntGrid3D is provided.*/
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
};

/** A sparse, infinite grid of values. In python SparseUnboundedIntGrid3D
    is provided.*/
template <class VT>
struct SparseUnboundedGrid3D
    : public GridD<3, SparseGridStorageD<3, VT, UnboundedGridRangeD<3> >, VT,
                   DefaultEmbeddingD<3> > {
  typedef GridD<3, SparseGridStorageD<3, VT, UnboundedGridRangeD<3> >, VT,
                DefaultEmbeddingD<3> > P;
  SparseUnboundedGrid3D(double side, const Vector3D &origin, VT def = VT())
      : P(side, origin, def) {}
  SparseUnboundedGrid3D() {}

};

#endif

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_STANDARD_GRIDS_H */
