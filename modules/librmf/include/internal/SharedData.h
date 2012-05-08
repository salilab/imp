/**
 *  \file RMF/internal/SharedData.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_INTERNAL_SHARED_DATA_H
#define IMPLIBRMF_INTERNAL_SHARED_DATA_H

#include "../RMF_config.h"
#include "../Key.h"
#include "../types.h"
#include "../names.h"
#include "../infrastructure_macros.h"
#include "map.h"
#include "set.h"
#include <boost/tuple/tuple.hpp>
#include <boost/array.hpp>
#include <boost/any.hpp>
#include <algorithm>

#include <boost/shared_ptr.hpp>

namespace RMF {
  template <class P>
  inline void *get_void_pointer(const P &p) {
    return p;
  }
  template <class P>
  inline void *get_void_pointer(boost::shared_ptr<P> p) {
    return p.get();
  }

  namespace internal {



#define IMP_RMF_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                                  PassValues, ReturnValues, Arity)      \
    virtual Ucname##Traits::Type get_value_always(unsigned int node,    \
                                                  Key<Ucname##Traits,Arity> k, \
                                                  unsigned int frame) const=0; \
    virtual unsigned int get_number_of_frames(Key<Ucname##Traits,       \
                                              Arity> k) const =0;       \
    virtual Ucname##Traits::Type get_value(unsigned int node,           \
                                           Key<Ucname##Traits, Arity> k, \
                                           unsigned int frame) const =0; \
    virtual bool get_has_value(unsigned int node,                       \
                               Key<Ucname##Traits, Arity> k,            \
                               unsigned int frame) const =0;            \
    virtual void set_value(unsigned int node,                           \
                           Key<Ucname##Traits, Arity> k,                \
                           Ucname##Traits::Type v, unsigned int frame) =0; \
    virtual Key<Ucname##Traits, Arity>                                  \
    add_##lcname##_key_##Arity(int category_id,                         \
                               std::string name,                        \
                               bool per_frame) =0;                      \
    virtual vector<Key<Ucname##Traits, Arity> >                         \
    get_##lcname##_keys_##Arity(int category_id) const=0;               \
    virtual std::string get_name(Key<Ucname##Traits, Arity> k) const =0; \
    virtual Key<Ucname##Traits, Arity>                                  \
    get_##lcname##_key_##Arity(int category_id,                         \
                               std::string name) const=0

#define IMP_RMF_SHARED_TYPE(lcname, Ucname, PassValue, ReturnValue,     \
                            PassValues, ReturnValues)                   \
    IMP_RMF_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue,   \
                              PassValues, ReturnValues, 1);             \
    IMP_RMF_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue,   \
                              PassValues, ReturnValues, 2);             \
    IMP_RMF_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue,   \
                              PassValues, ReturnValues, 3);             \
    IMP_RMF_SHARED_TYPE_ARITY(lcname, Ucname, PassValue, ReturnValue,   \
                              PassValues, ReturnValues, 4)


    class RMFEXPORT SharedData: public boost::intrusive_ptr_object {
      vector<boost::any> association_;
      map<void*, int> back_association_;
      map<int, boost::any> user_data_;
      int valid_;

    protected:
      SharedData(): valid_(11111) {};
    public:
      template <class TypeTraits, int Arity>
      bool get_is_per_frame(Key<TypeTraits, Arity> k) const {
        return k.get_is_per_frame();
      }
      IMP_RMF_FOREACH_TYPE(IMP_RMF_SHARED_TYPE);
      void audit_key_name(std::string name) const;
      void audit_node_name(std::string name) const;
      template <class T>
        void set_user_data(int i, const T&d) {
        user_data_[i]=boost::any(d);
      }
      bool get_has_user_data(int i) const {
        return user_data_.find(i) != user_data_.end();
      }
      template <class T>
        T get_user_data(int i) const {
        IMP_RMF_USAGE_CHECK(user_data_.find(i)
                            != user_data_.end(),
                            "No such data found");
        return boost::any_cast<T>(user_data_.find(i)->second);
      }
      template <class T>
        void set_association(int id, const T& d, bool overwrite) {
        if (association_.size() <= static_cast<unsigned int>(id)) {
          association_.resize(id+1, boost::any());
        }
        IMP_RMF_USAGE_CHECK(overwrite || association_[id].empty(),
                            "Associations can only be set once");
        if (overwrite && !association_[id].empty()) {
          boost::any old=association_[id];
          void* v= get_void_pointer(boost::any_cast<T>(old));
          if (back_association_[v]==id) {
            back_association_.erase(v);
          }
        }
        void *v= get_void_pointer(d);
        association_[id]=boost::any(d);
        back_association_[v]=id;
      }
      bool get_has_association(void* v) const {
        return back_association_.find(v) != back_association_.end();
      }
      boost::any get_association(int id) const {
        IMP_RMF_USAGE_CHECK(static_cast<unsigned int>(id) < association_.size(),
                            "Unassociated id");
        return association_[id];
      }
      bool get_has_association(int id) const {
        if (id >= static_cast<int>(association_.size())) return false;
        return !association_[id].empty();
      }
      int get_association(void* d) const {
        return back_association_.find(d)->second;
      }

      virtual void flush() const=0;
      virtual std::string get_file_name() const=0;

      // backwards compat
      void add_bond( int ida,  int idb,  int type);
      unsigned int get_number_of_bonds() const;
      boost::tuple<int,int,int> get_bond(unsigned int i) const;


      //SharedData(HDF5Group g, bool create);
      virtual ~SharedData();
      virtual std::string get_name(unsigned int node) const=0;
      virtual void set_name(unsigned int node, std::string name)=0;
      virtual unsigned int get_type(unsigned int node) const=0;

      virtual int add_child(int node, std::string name, int t)=0;
      virtual Ints get_children(int node) const=0;
      virtual void save_frames_hint(int i)=0;

      virtual void check_set(int arity, unsigned int index) const=0;
      virtual unsigned int get_number_of_sets(int arity) const=0;
      virtual RMF::Indexes get_set_indexes(int Arity) const=0;
      virtual unsigned int add_set( RMF::Indexes nis, int t)=0;
      virtual unsigned int get_set_member(int Arity, unsigned int index,
                                          int member_index) const=0;
      virtual unsigned int get_set_type(int Arity, unsigned int index) const=0;

      virtual int add_category(int Arity, std::string name)=0;
      virtual int get_category(int Arity, std::string name) const=0;
      virtual Strings get_category_names(int Arity) const=0;
      virtual Ints get_categories(int Arity) const=0;
      virtual std::string get_category_name(int Arity, unsigned int kc) const=0;
      virtual std::string get_description() const=0;
      virtual void set_description(std::string str)=0;
      void validate() const;
    };

    template <class Traits, int Arity>
    class GenericSharedData {
    };
    template <class Traits, int Arity>
    class ConstGenericSharedData {
    };

#define IMP_RMF_GENERIC_SHARED_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                                     PassValues, ReturnValues, Arity)   \
    template <>                                                         \
    class ConstGenericSharedData<Ucname##Traits, Arity> {               \
    public:                                                             \
    typedef Key<Ucname##Traits, Arity> K;                               \
    typedef vector<K > Ks;                                              \
    static K get_key( const SharedData *p, int category_id,             \
                      std::string name) {                               \
      return p->get_##lcname##_key_##Arity(category_id, name);          \
    }                                                                   \
    static bool get_has_key( const SharedData *p, int category_id,      \
                             std::string name) {                        \
      return p->get_##lcname##_key_##Arity(category_id, name)           \
        != K();                                                         \
    }                                                                   \
    static Ks get_keys( const SharedData *p, int category_id) {         \
      return p->get_##lcname##_keys_##Arity(category_id);               \
    }                                                                   \
    };                                                                  \
    template <>                                                         \
    class GenericSharedData<Ucname##Traits, Arity> {                    \
    public:                                                             \
    typedef Key<Ucname##Traits, Arity> K;                               \
    typedef vector<K > Ks;                                              \
    static K add_key(SharedData *p_, int category_id,                   \
                     std::string name, bool mf) {                       \
      return p_->add_##lcname##_key_##Arity(category_id, name, mf);     \
    }                                                                   \
    };
#define IMP_RMF_GENERIC_SHARED(lcname, Ucname, PassValue, ReturnValue,  \
                               PassValues, ReturnValues)                \
    IMP_RMF_GENERIC_SHARED_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                                 PassValues, ReturnValues, 1);          \
    IMP_RMF_GENERIC_SHARED_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                                 PassValues, ReturnValues, 2);          \
    IMP_RMF_GENERIC_SHARED_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                                 PassValues, ReturnValues, 3);          \
    IMP_RMF_GENERIC_SHARED_ARITY(lcname, Ucname, PassValue, ReturnValue, \
                                 PassValues, ReturnValues, 4)

    IMP_RMF_FOREACH_TYPE(IMP_RMF_GENERIC_SHARED);
  } // namespace internal
} /* namespace RMF */


#endif /* IMPLIBRMF_INTERNAL_SHARED_DATA_H */
