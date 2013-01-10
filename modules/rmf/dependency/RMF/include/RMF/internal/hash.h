/**
 *  \file compatibility/hash.h
 *  \brief Make sure that we avoid errors in specialization of boost hash
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_HASH_H
#define RMF_INTERNAL_HASH_H

#include <RMF/config.h>

#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wmismatched-tags"
#endif
#include <boost/functional/hash.hpp>
#include <boost/functional/hash/hash.hpp>
#ifdef __clang__
#  pragma clang diagnostic pop
#endif

// this specializes some hash methods
#include <boost/graph/adjacency_list.hpp>


#endif  /* RMF_INTERNAL_HASH_H */
