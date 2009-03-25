/**
 *  \file endian.h
 *  \brief functions to deal with endian of EM images
 *  \author Javier Velazquez-Muriel
 *  Copyright 2007-9 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_ENDIAN_H
#define IMPALGEBRA_ENDIAN_H

#include "config.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>

IMPALGEBRA_BEGIN_NAMESPACE

//! Reads from file in normal or reverse order
/** \ingroup LittleBigEndian
 * If the reverse parameter is true, the data will be read in reverse order.
 */
void IMPALGEBRAEXPORT reversed_read(void *dest, size_t size,
                    size_t nitems, std::ifstream& f, bool reverse);

//! Writes to a file in normal or reversed order
/**
 * \ingroup LittleBigEndian
 *
 * This function is the same as fread from C, but at the end there is a flag
 * saying if data should be read in reverse order or not.
 *
 * If the reverse parameter is true, the data will be written in reverse order.
 */
void IMPALGEBRAEXPORT reversed_write(const void* src,size_t size,size_t nitems,
              std::ofstream& f,bool reverse = false);


//! Conversion between little and big endian. Goes both ways
/**
 * \ingroup LittleBigEndian
 */
#define little22bigendian(x) byte_swap((unsigned char*)& x,sizeof(x))
void IMPALGEBRAEXPORT byte_swap(unsigned char* b, int n);

//! Returns 1 if machine is big endian else 0
/**
 * \ingroup LittleBigEndian
 */
bool IMPALGEBRAEXPORT is_big_endian(void);

//! Returns 1 if machine is little endian else 0
/**
 * \ingroup LittleBigEndian
 */
bool IMPALGEBRAEXPORT is_little_endian(void);


IMPALGEBRA_END_NAMESPACE

#endif  /* IMPALGEBRA_ENDIAN_H */
