/**
 *  \file compatibility/vector_property_map.h
 *  \brief Include the correct headers to get the boost vector_property_map
 *
 *  The location of the definition of boost::vector_property_map has changed
 *  in the boost versions supported by \imp. This header makes sure the correct
 *  file is included.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 */

#ifndef IMP_INTERNAL_VECTOR_PROPERTY_MAP_H
#define IMP_INTERNAL_VECTOR_PROPERTY_MAP_H

#include "compatibility_config.h"
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif


#endif  /* IMP_INTERNAL_VECTOR_PROPERTY_MAP_H */
