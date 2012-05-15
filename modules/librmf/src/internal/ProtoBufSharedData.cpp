/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/ProtoBufSharedData.h>
#include <RMF/NodeHandle.h>
#include <RMF/NodeSetHandle.h>
#include <RMF/Validator.h>
#include <RMF/internal/set.h>
#include <fstream>
namespace RMF {
  namespace internal {

    ProtoBufSharedData::ProtoBufSharedData(std::string name, bool create):
      name_(name)
    {
      if (!create) {
        std::fstream in(name_.c_str(), std::ios::in | std::ios::binary);
        if (!in) {
          IMP_RMF_THROW("Could not open file", IOException);
        }
        proto_.ParseFromIstream(&in);
      } else {
        proto_.add_nodes();
        proto_.mutable_nodes(0)->set_name("root");
        proto_.mutable_nodes(0)->set_type(ROOT);
      }
    }

    ProtoBufSharedData::~ProtoBufSharedData() {
      add_ref();
      // flush validates
      flush();
      release();
    }

    std::string ProtoBufSharedData::get_name(unsigned int node) const {
      return proto_.nodes(node).name();
    }
    unsigned int ProtoBufSharedData::get_type(unsigned int Arity,
                                              unsigned int index) const {
      switch (Arity) {
      case 1:
        return proto_.nodes(index).type();
      case 2:
        return proto_.pairs(index).type();
      case 3:
        return proto_.triplets(index).type();
      case 4:
        return proto_.quads(index).type();
      default:
        return -1;
      }
    }
    void ProtoBufSharedData::save_frames_hint( int){}

    int ProtoBufSharedData::add_child(int node, std::string name, int t) {
      // add node
      int id= proto_.nodes_size();
      proto_.add_nodes();
      proto_.mutable_nodes(id)->set_name(name);
      proto_.mutable_nodes(id)->set_type(t);
      // add as child
      proto_.mutable_nodes(node)->set_children(proto_.nodes(node)
                                               .children_size(), id);
      return id;
    }

    Ints ProtoBufSharedData::get_children(int node) const {
      Ints ret(proto_.nodes(node).children_size());
      for (unsigned int i=0; i< ret.size(); ++i) {
        ret[i]= proto_.nodes(node).children(i);
      }
      return ret;
    }

    unsigned int ProtoBufSharedData::get_number_of_sets(int Arity) const {
      switch (Arity) {
      case 1:
        return proto_.nodes_size();
      case 2:
        return proto_.pairs_size();
      case 3:
        return proto_.triplets_size();
      case 4:
        return proto_.quads_size();
      default:
        return 0;
      }
    }
    namespace {
      template <class C>
      void set_0(C *c, int i) {
        c->set_id0(i);
      }
      template <class C>
      void set_1(C *c, int i) {
        c->set_id1(i);
      }
      template <class C>
      void set_2(C *c, int i) {
        c->set_id2(i);
      }
      template <class C>
      void set_3(C *c, int i) {
        c->set_id3(i);
      }
      template <class C>
      int get_0(const C &c) {
        return c.id0();
      }
      template <class C>
      int get_1(const C &c) {
        return c.id1();
      }
      template <class C>
      int get_2(const C &c) {
        return c.id2();
      }
      template <class C>
      int get_3(const C &c) {
        return c.id3();
      }
      int get(const RMFProto_Pair&c, unsigned int member_index) {
        switch( member_index) {
        case 0:
          return get_0(c);
        case 1:
          return get_1(c);
        default:
          return -1;
        };
      }
      int get(const RMFProto_Triplet&c, unsigned int member_index) {
        switch( member_index) {
        case 0:
          return get_0(c);
        case 1:
          return get_1(c);
        case 2:
          return get_2(c);
        };
        return -1;
      }
      int get(const RMFProto_Quad &c, unsigned int member_index) {
        switch( member_index) {
        case 0:
          return get_0(c);
        case 1:
          return get_1(c);
        case 2:
          return get_2(c);
        case 3:
          return get_3(c);
        }
        return -1;
      }

    }
    unsigned int ProtoBufSharedData::add_set( RMF::Indexes nis, int t) {
      switch (nis.size()) {
      case 2:
        {
          int id= proto_.pairs_size();
          proto_.add_pairs();
          set_0(proto_.mutable_pairs(id), nis[0]);
          set_1(proto_.mutable_pairs(id), nis[1]);
          proto_.mutable_pairs(id)->set_type(t);
          return id;
        }
      case 3:
        {
          int id= proto_.triplets_size();
          proto_.add_triplets();
          set_0(proto_.mutable_triplets(id), nis[0]);
          set_1(proto_.mutable_triplets(id), nis[1]);
          set_2(proto_.mutable_triplets(id), nis[2]);
          proto_.mutable_triplets(id)->set_type(t);
          return id;
        }
      case 4:
        {
          int id= proto_.quads_size();
          proto_.add_quads();
          set_0(proto_.mutable_quads(id), nis[0]);
          set_1(proto_.mutable_quads(id), nis[1]);
          set_2(proto_.mutable_quads(id), nis[2]);
          set_3(proto_.mutable_quads(id), nis[3]);
          proto_.mutable_quads(id)->set_type(t);
          return id;
        }
      default:
        IMP_RMF_USAGE_CHECK(nis.size()>1 && nis.size()<5,
                            "Bad set size");
        return -1;
      };
    }
    unsigned int ProtoBufSharedData::get_set_member(int Arity,
                                                    unsigned int index,
                                                    int member_index) const {
      switch (Arity) {
      case 2:
        return get(proto_.pairs(index), member_index);
      case 3:
        return get(proto_.triplets(index), member_index);
      case 4:
        return get(proto_.quads(index), member_index);
      default:
        IMP_RMF_USAGE_CHECK(0,
                            "Bad set size");
        return -1;
      };
    }


    int ProtoBufSharedData::add_category(int Arity, std::string name) {
      int id=get_number_of_categories(Arity);
      get_category_data_always(Arity, id).set_name(name);
      return id;
    }
    unsigned int ProtoBufSharedData::get_number_of_categories(int Arity) const {
      if (proto_.arities_size() < Arity) {
        return 0;
      }
      const RMFProto::ArityData &arity_data= proto_.arities(Arity-1);
      return arity_data.categories_size();
    }
    std::string ProtoBufSharedData::get_category_name(int Arity,
                                                     unsigned int id) const {
      return get_category_data(Arity, id)->name();
    }

#define IMP_RMF_COUNT_FRAMES(lcname, Ucname, PassValue,                 \
                             ReturnValue,                               \
                             PassValues, ReturnValues)                  \
    for (unsigned int j=0;                                              \
         j< get_number_of_keys_impl<Ucname##Traits, 1>(i, true); ++j) { \
      ret=std::max(ret,                                                 \
                   DataPicker<Ucname##Traits, true>::get_size(*cat));   \
    }

    unsigned int ProtoBufSharedData::get_number_of_frames() const {
      unsigned int ret=0;
      for (unsigned int i=0; i< get_number_of_categories(1); ++i) {
        const CategoryProto *cat=get_category_data(1, i);
        IMP_RMF_FOREACH_TYPE(IMP_RMF_COUNT_FRAMES);
      }
      return ret;
    }


    std::string ProtoBufSharedData::get_description() const {
      return proto_.description();
    }
    void ProtoBufSharedData::set_description(std::string str) {
      proto_.set_description(str);
    }
    void ProtoBufSharedData::flush() const {
      std::fstream out(name_.c_str(), std::ios::out | std::ios::binary);
      if (!out) {
        IMP_RMF_THROW("Could not open file", IOException);
      }
      proto_.SerializeToOstream(&out);
    }

  } // namespace internal
} /* namespace RMF */
