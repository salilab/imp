/**
 *  \file IMP/algebra/endian.h
 *  \brief Functions to deal with byte order.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
*/

#ifndef IMPALGEBRA_ENDIAN_H
#define IMPALGEBRA_ENDIAN_H

#include <IMP/algebra/algebra_config.h>
#include <IMP/base_macros.h>
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
 * This function is the same as fwrite from C, but at the end there is a flag
 * saying if data should be written in reverse order or not.
 *
 * If the reverse parameter is true, the data will be written in reverse order.
 */
void IMPALGEBRAEXPORT reversed_write(const void* src, size_t size,
                                     size_t nitems, std::ofstream& f,
                                     bool reverse = false);

//! Returns 1 if machine is big endian else 0
bool IMPALGEBRAEXPORT get_is_big_endian();

//! Returns 1 if machine is little endian else 0
bool IMPALGEBRAEXPORT get_is_little_endian();
/** @} */

IMPALGEBRA_END_NAMESPACE

#endif /* IMPALGEBRA_ENDIAN_H */
