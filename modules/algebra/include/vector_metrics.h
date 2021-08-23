/**
 *  \file IMP/algebra/vector_metrics.h   \brief Functions to generate vectors.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPALGEBRA_VECTOR_METRICS_H
#define IMPALGEBRA_VECTOR_METRICS_H

#include "VectorD.h"
#include "BoundingBoxD.h"
#include <IMP/types.h>
#include <IMP/Object.h>
#include <iostream>
#include <IMP/Pointer.h>
#include <IMP/WeakPointer.h>
#include <algorithm>
#include <numeric>

IMPALGEBRA_BEGIN_NAMESPACE

//! The base class for a metric on VectorKDs.
class IMPALGEBRAEXPORT VectorKDMetric : public Object {
 public:
  VectorKDMetric(std::string name);
  virtual double get_distance(const VectorKD &a, const VectorKD &b) const = 0;
  virtual VectorKD get_centroid(const VectorKDs &vs) const = 0;
};

IMP_OBJECTS(VectorKDMetric, VectorKDMetrics);

//! The l2 norm on the distance vector.
class IMPALGEBRAEXPORT EuclideanVectorKDMetric : public VectorKDMetric {
 public:
  EuclideanVectorKDMetric(std::string name = "EuclideanVectorKDMetric%1%");
  double get_distance(const VectorKD &a, const VectorKD &b) const IMP_OVERRIDE {
    return get_l2_norm(a - b);
  }
  VectorKD get_centroid(const VectorKDs &vs) const IMP_OVERRIDE {
    IMP_USAGE_CHECK(!vs.empty(), "Needs things to have a centroid");
    VectorKD sum = std::accumulate(vs.begin(), vs.end(),
                                   get_zero_vector_kd(vs[0].get_dimension()));
    return sum / vs.size();
  }
  IMP_OBJECT_METHODS(EuclideanVectorKDMetric);
};

/** The l-infinity norm on the difference between the two vectors. And the
    centroid is the center of the bounding box of the vectors.
 */
class IMPALGEBRAEXPORT MaxVectorKDMetric : public VectorKDMetric {
 public:
  MaxVectorKDMetric(std::string name = "MaxVectorKDMetric%1%");
  double get_distance(const VectorKD &a, const VectorKD &b) const IMP_OVERRIDE {
    return get_linf_norm(a - b);
  }
  VectorKD get_centroid(const VectorKDs &vs) const IMP_OVERRIDE {
    IMP_USAGE_CHECK(!vs.empty(), "Needs things to have a centroid");
    BoundingBoxKD bb = std::accumulate(vs.begin(), vs.end(),
                                       BoundingBoxKD(vs[0].get_dimension()));
    return .5 * (bb.get_corner(0) + bb.get_corner(1));
  }
  IMP_OBJECT_METHODS(MaxVectorKDMetric);
};

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_VECTOR_METRICS_H */
