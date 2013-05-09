/**
 *  \file vector_generators.cpp
 *  \brief Support for rigid bodies.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/algebra/vector_metrics.h>

IMPALGEBRA_BEGIN_NAMESPACE

VectorKDMetric::VectorKDMetric(std::string name) : Object(name) {}

EuclideanVectorKDMetric::EuclideanVectorKDMetric(std::string name)
    : VectorKDMetric(name) {}
MaxVectorKDMetric::MaxVectorKDMetric(std::string name) : VectorKDMetric(name) {}
IMPALGEBRA_END_NAMESPACE
