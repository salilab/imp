/**
 * \file cuda_helpers.h
 * \brief GPU implementations of some SAXS operations
 *
 * Copyright 2007-2023 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSAXS_INTERNAL_CUDA_HELPERS_H
#define IMPSAXS_INTERNAL_CUDA_HELPERS_H

namespace IMPcuda {
namespace saxs {
namespace internal {

void squared_distribution_2_profile_cuda(
           const double *r_dist, const float *q,
           const double *distances, float *intensity,
           double modulation_function_parameter, size_t r_size, size_t q_size);

} } }

#endif /* IMPSAXS_INTERNAL_CUDA_HELPERS_H */
