/**
 *  \file IMP/algebra/endian.h
 *  \brief functions to deal with endian of EM images
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_ENDIAN_H
#define IMPALGEBRA_ENDIAN_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/base/base_macros.h>
#include <iostream>
#include <fstream>

IMPALGEBRA_BEGIN_NAMESPACE

/** \name Endian
    \imp provides a variety of functionality to manage byte order in
    input and output data.
    @{
*/

//! Reads from file in normal or reverse order
/** If the reverse parameter is true, the data will be read in reverse order.
 */
void IMPALGEBRAEXPORT reversed_read(void* dest, size_t size, size_t nitems,
                                    std::ifstream& f, bool reverse);

//! Writes to a file in normal or reversed order
/**
 * This function is the same as fread from C, but at the end there is a flag
 * saying if data should be read in reverse order or not.
 *
 * If the reverse parameter is true, the data will be written in reverse order.
 */
void IMPALGEBRAEXPORT reversed_write(const void* src, size_t size,
                                     size_t nitems, std::ofstream& f,
                                     bool reverse = false);

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
void IMPALGEBRAEXPORT byte_swap(unsigned char* b, int n);
#endif

//! Conversion between little and big endian. Goes both ways
template <class T>
inline void get_swapped_endian(T& x) {
  byte_swap((unsigned char*)&x, sizeof(T));
}

//! Returns 1 if machine is big endian else 0
bool IMPALGEBRAEXPORT get_is_big_endian();

//! Returns 1 if machine is little endian else 0
bool IMPALGEBRAEXPORT get_is_little_endian();
/** @} */

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_ENDIAN_H */
