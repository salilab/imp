/**
 *  \file endian.h
 *  \brief functions to deal with endian of EM images
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-8 Sali Lab. All rights reserved.
*/

#ifndef IMPEM_ENDIAN_H
#define IMPEM_ENDIAN_H

#include "config.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>

IMPEM_BEGIN_NAMESPACE

//! Reads from file in normal or reverse order
/** \ingroup LittleBigEndian
 * If the reverse parameter is true, the data will be read in reverse order.
 */
void reversed_read(void *dest, size_t size, size_t nitems, std::ifstream& f,
                bool reverse);

//! Writes to a file in normal or reversed order
/**
 * \ingroup LittleBigEndian
 *
 * This function is the same as fread from C, but at the end there is a flag
 * saying if data should be read in reverse order or not.
 *
 * If the reverse parameter is true, the data will be read in reverse order.
 */
void reversed_write(const void* src,size_t size,size_t nitems,
              std::ofstream& f,bool reverse = false);


//! Conversion between little and big endian. Goes both ways
/**
 * \ingroup LittleBigEndian
 */
#define little22bigendian(x) byte_swap((unsigned char*)& x,sizeof(x))
void byte_swap(unsigned char* b, int n);

//! Returns 1 if machine is big endian else 0
/**
 * \ingroup LittleBigEndian
 */
bool is_big_endian(void);

//! Returns 1 if machine is little endian else 0
/**
 * \ingroup LittleBigEndian
 */
bool is_little_endian(void);


IMPEM_END_NAMESPACE

#endif  /* IMPEM_ENDIAN_H */
