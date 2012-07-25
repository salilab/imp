/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_INTERNAL_PROTO_BUF_SHARED_DATA_H
#define IMPLIBRMF_INTERNAL_PROTO_BUF_SHARED_DATA_H

#include "../RMF_config.h"
#ifdef RMF_USE_PROTOBUF
#include "SharedData.h"
#include "../infrastructure_macros.h"
#include "RMF.pb.h"
#include <boost/shared_ptr.hpp>


#define IMP_RMF_PROTO_INDEX(base, field, index) \
  (base).field(index)

#define IMP_RMF_PROTO_MINDEX(base, field, index) \
  (base).mutable_##field(index)


#define IMP_RMF_PROTO_INDEX_2(base, field, index, field2, index2)       \
  IMP_RMF_PROTO_INDEX(IMP_RMF_PROTO_INDEX(base, field, index),\
                      field2, index2)

#define IMP_RMF_PROTO_MINDEX_2(base, field, index, field2, index2)      \
  IMP_RMF_PROTO_MINDEX(*IMP_RMF_PROTO_MINDEX(base, field, index),\
                       field2, index2)


namespace RMF {


  namespace internal {

    template <class V>
    struct ProtoTraits {
      template <class T>
      static V get(const T &t) {
        return t.value();
      }
      template <class T>
      static void set(const T &t, V tt) {
        t->set_value(tt);
      }
      template <class T>
      static bool get_has(const T &t) {
        return t.has_value();
      }
    };

    template<>
    struct ProtoTraits<NodeID> {
      template <class T>
      static NodeID get(const T &t) {
        return NodeID(t.value());
      }
      template <class T>
      static void set(const T &t, NodeID tt) {
        t->set_value(tt.get_index());
      }
      template <class T>
      static bool get_has(const T &t) {
        return t.has_value();
      }
    };

    template <class C>
    struct ProtoTraits<vector<C> > {
      typedef vector<C> V;
      template <class T>
      static V get(const T &t) {
        V ret(t.value_size());
        for (unsigned int i=0; i< ret.size(); ++i) {
          ret[i]= IMP_RMF_PROTO_INDEX(t, value,i);
        }
        return ret;
      }
      template <class T>
      static void set(const T &t, V tt) {
        t->clear_value();
        for( unsigned int i=0; i< tt.size(); ++i) {
          t->add_value(tt[i]);
        }
      }
      template <class T>
      static bool get_has(const T &t) {
        return t.value_size()>0;
      }
    };

    template <>
    struct ProtoTraits<vector<NodeID> > {
      typedef vector<NodeID> V;
      template <class T>
      static V get(const T &t) {
        V ret(t.value_size());
        for (unsigned int i=0; i< ret.size(); ++i) {
          ret[ret.size()-i-1]= NodeID(IMP_RMF_PROTO_INDEX(t, value, i));
        }
        return ret;
      }
      template <class T>
      static void set(const T &t, V tt) {
        for( unsigned int i=0; i< tt.size(); ++i) {
          t->add_value(tt[i].get_index());
        }
      }
      template <class T>
      static bool get_has(const T &t) {
        return t.value_size()>0;
      }
    };


#define IMP_RMF_PROTO_SHARED_PICKER(lcname, Ucname, PassValue,          \
                                          ReturnValue,                  \
                                          PassValues, ReturnValues)     \
    template <>                                                         \
    struct DataPicker<Ucname##Traits, true> {                           \
      typedef CategoryProto::PerFrame##Ucname Proto;                    \
      static const Proto* get(const CategoryProto &d,  int key_index) { \
        if (d.per_frame_##lcname##_list_size() <= key_index) {          \
          return NULL;                                                  \
        }                                                               \
        else return &d.per_frame_##lcname##_list(key_index);            \
      }                                                                 \
      static unsigned int get_size(const CategoryProto &d) {            \
        return d.per_frame_##lcname##_list_size();                      \
      }                                                                 \
      static  Proto& get_always( CategoryProto &d,  int key_index) {    \
        while (d.per_frame_##lcname##_list_size() <= key_index) {       \
          d.add_per_frame_##lcname##_list();                            \
        }                                                               \
        return *d.mutable_per_frame_##lcname##_list(key_index);         \
      }                                                                 \
    };                                                                  \
    template <>                                                         \
    struct DataPicker<Ucname##Traits, false> {                          \
      typedef CategoryProto::Ucname Proto;                              \
      static const Proto* get(const CategoryProto &d,  int key_index) { \
        if (d.lcname##_list_size() <= key_index) {                      \
          return NULL;                                                  \
        }                                                               \
        else return &d.lcname##_list(key_index);                        \
      }                                                                 \
      static unsigned int get_size(const CategoryProto &d) {            \
        return d.lcname##_list_size();                                  \
      }                                                                 \
      static Proto& get_always( CategoryProto &d,  int key_index) {     \
        while (d.lcname##_list_size() <= key_index) {                   \
          d.add_##lcname##_list();                                      \
        }                                                               \
        return *d.mutable_##lcname##_list(key_index);                   \
      }                                                                 \
    };                                                                  \



#define IMP_RMF_PROTO_SHARED_TYPE_ARITY(lcname, Ucname, PassValue,      \
                                        ReturnValue,                    \
                                        PassValues, ReturnValues, Arity) \
    Ucname##Traits::Type get_value(unsigned int node,                   \
                                   Key<Ucname##Traits,Arity> k,         \
                                   unsigned int frame) const {          \
      return get_value_impl(node, k, frame);                            \
    }                                                                   \
    void set_value(unsigned int node,                                   \
                   Key<Ucname##Traits, Arity> k,                        \
                        Ucname##Traits::Type v, unsigned int frame) {   \
      return set_value_impl(node, k, v, frame);                         \
    }                                                                   \
    Key<Ucname##Traits, Arity> add_##lcname##_key_##Arity(int category_id, \
                                                          std::string name, \
                                                          bool per_frame) { \
      return add_key_impl<Ucname##Traits, Arity>(category_id,           \
                                                 name, per_frame);      \
    }                                                                   \
    unsigned int                                                        \
    get_number_of_##lcname##_keys_##Arity(int category_id,              \
                                          bool per_frame) const {       \
      return get_number_of_keys_impl<Ucname##Traits, Arity>(category_id, \
                                                            per_frame); \
    }                                                                   \
    std::string get_name(Key<Ucname##Traits, Arity> k) const {          \
      return get_name_impl(k);                                          \
    }

#define IMP_RMF_PROTO_SHARED_TYPE(lcname, Ucname, PassValue, ReturnValue, \
                            PassValues, ReturnValues)               \
    IMP_RMF_PROTO_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                              PassValues, ReturnValues, 1);           \
    IMP_RMF_PROTO_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                              PassValues, ReturnValues, 2);           \
    IMP_RMF_PROTO_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                              PassValues, ReturnValues, 3);           \
    IMP_RMF_PROTO_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                              PassValues, ReturnValues, 4)

 template <class T, bool PF> struct DataPicker{};
    IMP_RMF_FOREACH_TYPE(IMP_RMF_PROTO_SHARED_PICKER);

    class RMFEXPORT ProtoBufSharedData: public SharedData {
      // indexed first by per frame, then by
      // TypeInfo::get_index() then by ID
      // then by key.get_index()
      RMFProto proto_;
      std::string name_;

      template <class TypeTraits, class Data>
        typename TypeTraits::Type get_value_per_frame( int node,
                                                       int frame,
                                                      const Data *d) const {
        if (!d) {
          return TypeTraits::get_null_value();
        }
        if (d->entry_size() <= node) {
          return TypeTraits::get_null_value();
        } else {
          if (IMP_RMF_PROTO_INDEX(*d, entry, node).frame_size() <= frame) {
            return TypeTraits::get_null_value();
          } else if (!ProtoTraits<typename TypeTraits::Type>
                ::get_has(IMP_RMF_PROTO_INDEX_2((*d), entry,node,
                                                frame,frame))) {
            return TypeTraits::get_null_value();
          } else {
            return ProtoTraits<typename TypeTraits::Type>
              ::get(IMP_RMF_PROTO_INDEX_2((*d), entry, node, frame, frame));
          }
        }
      }

      template <class TypeTraits, class Data>
        typename TypeTraits::Type get_value_constant( int node,
                                                     const Data *d) const {
        if (!d) {
          return TypeTraits::get_null_value();
        }
        if (d->entry_size() <= node) {
          return TypeTraits::get_null_value();
        }  else if (!ProtoTraits<typename TypeTraits::Type>
                    ::get_has(IMP_RMF_PROTO_INDEX(*d, entry,node))) {
          return TypeTraits::get_null_value();
        } else {
          return ProtoTraits<typename TypeTraits::Type>
            ::get(IMP_RMF_PROTO_INDEX(*d, entry,node));
        }
      }
      template <class TypeTraits>
        const typename DataPicker<TypeTraits, true>::Proto*
        get_per_frame_data(const CategoryProto& data,
                           unsigned int key_index) const {
        return DataPicker<TypeTraits, true>::get(data, key_index);
      }
      template <class TypeTraits>
        const typename DataPicker<TypeTraits, false>::Proto*
        get_data(const CategoryProto& data, unsigned int key_index) const {
        return DataPicker<TypeTraits, false>::get(data, key_index);
      }

      const CategoryProto *get_category_data( int Arity,
                                          int category_id) const {
        if (proto_.arity_size() < Arity) {
          return NULL;
        }
        const RMFProto::ArityData &arity_data
          = IMP_RMF_PROTO_INDEX(proto_, arity, Arity-1);
        if (arity_data.category_size() <= category_id) {
          return NULL;
        }
        return &IMP_RMF_PROTO_INDEX(arity_data, category,category_id);
      }

      template <class TypeTraits, int Arity>
        typename TypeTraits::Type get_value_impl(unsigned int node,
                                                 Key<TypeTraits,Arity> k,
                                                 unsigned int frame) const {
        const CategoryProto *category_data
          = get_category_data(Arity, k.get_category().get_index());
        if (!category_data) return TypeTraits::get_null_value();
        if (k.get_is_per_frame()) {
          return get_value_per_frame<TypeTraits>(node, frame,
                                                 get_per_frame_data<TypeTraits>
                                                 (*category_data,
                                                  k.get_index()));
        } else {
          return get_value_constant<TypeTraits>(node,
                                                get_data<TypeTraits>
                                                (*category_data,
                                                 k.get_index()));
        }
      }


      template <class TypeTraits, class Data>
        void
        set_value_per_frame( int node,
                             int frame,
                            Data &d,
                            typename TypeTraits::Type v) {
        while (d.entry_size() <= node) {
          d.add_entry();
        }
        while (IMP_RMF_PROTO_INDEX(d, entry,node).frame_size() <= frame) {
          IMP_RMF_PROTO_MINDEX(d, entry,node)->add_frame();
        }
        ProtoTraits<typename TypeTraits::Type>
          ::set(IMP_RMF_PROTO_MINDEX_2(d, entry,node,
                                       frame,frame), v);
      }

      template <class TypeTraits, class Data>
        void
        set_value_constant( int node,
                           Data &d,
                           typename TypeTraits::Type v) {
        while (d.entry_size() <= node) {
          d.add_entry();
        }
        ProtoTraits<typename TypeTraits::Type>
          ::set(IMP_RMF_PROTO_MINDEX(d, entry,node), v);
      }
      template <class TypeTraits>
        typename DataPicker<TypeTraits, true>::Proto&
        get_per_frame_data_always( CategoryProto& data,
                                  unsigned int key_index) {
        return DataPicker<TypeTraits, true>::get_always(data, key_index);
      }
      template <class TypeTraits>
        typename DataPicker<TypeTraits, false>::Proto&
        get_data_always( CategoryProto& data, unsigned int key_index) {
        return DataPicker<TypeTraits, false>::get_always(data, key_index);
      }

      CategoryProto &get_category_data_always( int Arity,
                                           int category_id) {
        while (proto_.arity_size() < Arity) {
          proto_.add_arity();
        }
        RMFProto::ArityData *arity_data
          = IMP_RMF_PROTO_MINDEX(proto_, arity, Arity-1);
        while (arity_data->category_size() <= category_id) {
          arity_data->add_category();
        }
        return *IMP_RMF_PROTO_MINDEX(*arity_data, category,category_id);

      }

      template <class TypeTraits, int Arity>
        void set_value_impl(unsigned int node, Key<TypeTraits, Arity> k,
                       typename TypeTraits::Type v, unsigned int frame) {
        CategoryProto &category_data
          = get_category_data_always(Arity,
                                     k.get_category().get_index());
        if (k.get_is_per_frame()) {
          set_value_per_frame<TypeTraits>(node, frame,
                                          get_per_frame_data_always<TypeTraits>
                                          (category_data, k.get_index()), v);
        } else {
          set_value_constant<TypeTraits>(node,
                                         get_data_always<TypeTraits>
                                         (category_data, k.get_index()), v);
        }
      }


      template <class TypeTraits, int Arity>
        Key<TypeTraits, Arity> add_key_impl(int category_id,
                                            std::string name,
                                            bool per_frame) {
        int id= get_number_of_keys_impl<TypeTraits, Arity>(category_id,
                                                           per_frame);
        CategoryProto &cat=get_category_data_always(Arity, category_id);
        if (per_frame) {
          DataPicker<TypeTraits, true>
            ::get_always(cat, id).set_name(name);
        } else {
          DataPicker<TypeTraits, false>
            ::get_always(cat, id).set_name(name);
        }
        return Key<TypeTraits, Arity>(CategoryD<Arity>(category_id),
                                      id, per_frame);
      }
      template <class TypeTraits, int Arity>
        unsigned int
        get_number_of_keys_impl(int category_id, bool per_frame) const {
        const CategoryProto *cat=get_category_data(Arity, category_id);
        if (!cat) return 0;
        if (per_frame) {
          return DataPicker<TypeTraits, true>::get_size(*cat);
        } else {
          return DataPicker<TypeTraits, false>::get_size(*cat);
        }
      }
      template <class TypeTraits, int Arity>
        std::string get_name_impl(Key<TypeTraits, Arity> k) const {
        const CategoryProto *cat=get_category_data(Arity,
                                               k.get_category().get_index());
        if (k.get_is_per_frame()) {
          return DataPicker<TypeTraits, true>::get(*cat, k.get_index())->name();
        } else {
          return DataPicker<TypeTraits, false>::get(*cat,
                                                    k.get_index())->name();
        }
      }
    public:
      IMP_RMF_FOREACH_TYPE(IMP_RMF_PROTO_SHARED_TYPE);

      void flush() const;
      std::string get_file_name() const {
        return name_;
      }

      ProtoBufSharedData(std::string name, bool create);
      ~ProtoBufSharedData();
      std::string get_name(unsigned int node) const;
      unsigned int get_type(unsigned int Arity, unsigned int node) const;
      int add_child(int node, std::string name, int t);
      Ints get_children(int node) const;
      void save_frames_hint(int i);
      unsigned int get_number_of_frames() const;
      unsigned int get_number_of_sets(int arity) const;
      unsigned int add_set( Indexes nis, int t);
      unsigned int get_set_member(int Arity, unsigned int index,
                                    int member_index) const;
      int add_category(int Arity, std::string name);
      unsigned int get_number_of_categories(int Arity) const;
      std::string get_category_name(int Arity, unsigned int kc) const;

      std::string get_description() const;
      void set_description(std::string str);

      void set_frame_name(unsigned int frame, std::string str);
      std::string get_frame_name(unsigned int frame) const;
      void reload();
    };
  } // namespace internal
} /* namespace RMF */
#endif

#endif /* IMPLIBRMF_INTERNAL_PROTO_BUF_SHARED_DATA_H */
