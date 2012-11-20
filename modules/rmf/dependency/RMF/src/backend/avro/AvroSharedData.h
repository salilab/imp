/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_INTERNAL_AVRO_SHARED_DATA_H
#define RMF_INTERNAL_AVRO_SHARED_DATA_H

#include <RMF/config.h>
#include <RMF/internal/SharedData.h>
#include <RMF/infrastructure_macros.h>
#include <RMF/constants.h>
#include <RMF/internal/map.h>
#include <RMF/internal/set.h>
#include "AvroSharedData.types.h"

namespace RMF {
  namespace internal {

    template <class Base>
    class AvroSharedData: public Base {
      typedef Base P;
      RMF_internal::NodeData null_node_data_;

      const RMF_internal::NodeData &get_node_frame_data(int node,
                                                        Category cat,
                                                        int frame) const {
        const RMF_internal::Data &data= P::get_frame_data(cat, frame);
        RMF_INTERNAL_CHECK(data.frame==frame,
                           "Bad frame returned");
        std::map<std::string, RMF_internal::NodeData>::const_iterator
          nit= data.nodes.find(P::get_node_string(node));
        if (nit == data.nodes.end()) {
          return null_node_data_;
        } else {
          return nit->second;
        }
      }



      RMF_internal::NodeData &access_node_frame_data(int node,
                                                     Category cat,
                                                     int frame) {
        RMF_internal::Data &data= P::access_frame_data(cat, frame);
        RMF_INTERNAL_CHECK(data.frame==frame,
                           "Bad frame returned");
        return data.nodes[P::get_node_string(node)];
      }

      template <class TypeTraits, class AvroData>
      void extract_keys(Category cat,
                        const std::map<std::string, AvroData>
                        &data, set<Key<TypeTraits> > &ret ) {
        for (typename std::map<std::string, AvroData>::const_iterator iti
               = data.begin(); iti != data.end(); ++iti) {
          std::cout << "found " << iti->first << std::endl;
          ret.insert( P::template get_key_helper<TypeTraits>(cat,
                                                             iti->first));
        }
      }

    public:
      RMF_FOREACH_TYPE(RMF_AVRO_SHARED_TYPE);

      AvroSharedData(std::string g, bool create, bool read_only);
      AvroSharedData(std::string &buffer, bool create, bool read_only,
                     bool use_buffer);
      virtual ~AvroSharedData(){}
      std::string get_name(unsigned int node) const;
      unsigned int get_type(unsigned int node) const;
      int add_child(int node, std::string name, int t);
      void add_child(int node, int child_node);
      Ints get_children(int node) const;
      void save_frames_hint(int) {}
      unsigned int get_number_of_frames() const;
      std::string get_description() const;
      void set_description(std::string str);
      std::string get_producer() const;
      void set_producer(std::string str);
      std::string get_frame_name(int i) const;

      void set_current_frame(int frame){
        RMF_USAGE_CHECK(frame < static_cast<int>(get_number_of_frames()),
                        "Setting to invalid frame");
        P::set_current_frame(frame);
        RMF_INTERNAL_CHECK(P::get_current_frame()==frame,
                           "Didn't set frame");
      }

      int add_child_frame(int node, std::string name, int t);
      void add_child_frame(int node, int child_node);
      Ints get_children_frame(int node) const;
    };

  } // namespace internal
} /* namespace RMF */


#include "AvroSharedData.impl.h"

#endif /* RMF_INTERNAL_AVRO_SHARED_DATA_H */
