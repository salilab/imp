/**
 *  \file vector_metrics.cpp   \brief Functions to generate vectors.
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/vector_metrics.h>

IMPALGEBRA_BEGIN_NAMESPACE

VectorKDMetric::VectorKDMetric(std::string name) : Object(name) {}

EuclideanVectorKDMetric::EuclideanVectorKDMetric(std::string name)
    : VectorKDMetric(name) {}

MaxVectorKDMetric::MaxVectorKDMetric(std::string name) : VectorKDMetric(name) {}

IMP_OBJECT_SERIALIZE_IMPL(IMP::algebra::EuclideanVectorKDMetric);
IMP_OBJECT_SERIALIZE_IMPL(IMP::algebra::MaxVectorKDMetric);

IMPALGEBRA_END_NAMESPACE
