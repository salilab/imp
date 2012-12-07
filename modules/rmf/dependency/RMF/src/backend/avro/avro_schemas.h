/**
 *  \file compatibility/hash.h
 *  \brief Make sure that we avoid errors in specialization of boost hash
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 */

#ifndef RMF_INTERNAL_AVRO_SCHEMAS_H
#define RMF_INTERNAL_AVRO_SCHEMAS_H

#include <RMF/config.h>
#include <avro/ValidSchema.hh>
#include <backend/avro/AllJSON.h>

namespace RMF {
  namespace internal {
    RMFEXPORT avro::ValidSchema get_All_schema();
    RMFEXPORT avro::ValidSchema get_File_schema();
    RMFEXPORT avro::ValidSchema get_Nodes_schema();
    RMFEXPORT avro::ValidSchema get_Data_schema();

    RMFEXPORT void show(const RMF_internal::Data &data,
                        std::ostream &out=std::cout);
  }
}


#endif  /* RMF_INTERNAL_UTILITY_H */
