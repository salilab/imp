/**
 *  \file header_converters.h
 *  \brief Functions to convert between ImageHeader and DensityHeader
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_HEADER_CONVERTERS_H
#define IMPEM_HEADER_CONVERTERS_H

#include "config.h"
#include "internal/version_info.h"
#include "ImageHeader.h"
#include "DensityHeader.h"
#include <string.h>

IMPEM_BEGIN_NAMESPACE

//! Function to transfer the (compatible) information content from ImageHeader
//! to DensityHeader
void  ImageHeader_to_DensityHeader(const ImageHeader& h,DensityHeader& header);

//! Function to transfer the (compatible) information content from DensityHeader
//! to ImageHeader
void  DensityHeader_to_ImageHeader(const DensityHeader& header,ImageHeader& h);

IMPEM_END_NAMESPACE

#endif /* IMPEM_HEADER_CONVERTERS_H */
