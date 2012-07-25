/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/ProtoBufSharedData.h>
#ifdef RMF_USE_PROTOBUF
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
        reload();
      } else {
        proto_.add_node();
        proto_.mutable_node(0)->set_name("root");
        proto_.mutable_node(0)->set_type(ROOT);
      }
    }

    ProtoBufSharedData::~ProtoBufSharedData() {
      add_ref();
      // flush validates
      flush();
      release();
    }

    std::string ProtoBufSharedData::get_name(unsigned int node) const {
      return proto_.node(node).name();
    }
    unsigned int ProtoBufSharedData::get_type(unsigned int Arity,
                                              unsigned int index) const {
      switch (Arity) {
      case 1:
        return proto_.node(index).type();
      case 2:
        return proto_.pair(index).type();
      case 3:
        return proto_.triplet(index).type();
      case 4:
        return proto_.quad(index).type();
      default:
        return -1;
      }
    }
    void ProtoBufSharedData::save_frames_hint( int){}

    int ProtoBufSharedData::add_child(int node, std::string name, int t) {
      // add node
      int id= proto_.node_size();
      RMFProto::Node *n=proto_.add_node();
      n->set_name(name);
      n->set_type(t);
      // add as child
      RMFProto::Node *parent=IMP_RMF_PROTO_MINDEX(proto_, node,node);
      parent->add_children(id);
      Ints ch= get_children(node);
      Strings names;
      for (unsigned int i=0; i < ch.size(); ++i) {
        names.push_back(get_name(ch[i]));
      }
      return id;
    }

    Ints ProtoBufSharedData::get_children(int node) const {
      const RMFProto::Node &n= IMP_RMF_PROTO_INDEX(proto_, node, node);
      Ints ret(n.children_size());
      for (unsigned int i=0; i < ret.size(); ++i) {
        ret[i]= IMP_RMF_PROTO_INDEX(n,children, i);
      }
      return ret;
    }

    unsigned int ProtoBufSharedData::get_number_of_sets(int Arity) const {
      switch (Arity) {
      case 1:
        return proto_.node_size();
      case 2:
        return proto_.pair_size();
      case 3:
        return proto_.triplet_size();
      case 4:
        return proto_.quad_size();
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
      int get(const RMFProto::Pair&c, unsigned int member_index) {
        switch( member_index) {
        case 0:
          return get_0(c);
        case 1:
          return get_1(c);
        default:
          return -1;
        };
      }
      int get(const RMFProto::Triplet&c, unsigned int member_index) {
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
      int get(const RMFProto::Quad &c, unsigned int member_index) {
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
          int id= proto_.pair_size();
          RMFProto::Pair* p= proto_.add_pair();
          set_0(p, nis[0]);
          set_1(p, nis[1]);
          p->set_type(t);
          return id;
        }
      case 3:
        {
          int id= proto_.triplet_size();
          RMFProto::Triplet* p=proto_.add_triplet();
          set_0(p, nis[0]);
          set_1(p, nis[1]);
          set_2(p, nis[2]);
          p->set_type(t);
          return id;
        }
      case 4:
        {
          int id= proto_.quad_size();
          RMFProto::Quad *p=proto_.add_quad();
          set_0(p, nis[0]);
          set_1(p, nis[1]);
          set_2(p, nis[2]);
          set_3(p, nis[3]);
          p->set_type(t);
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
        return get(proto_.pair(index), member_index);
      case 3:
        return get(proto_.triplet(index), member_index);
      case 4:
        return get(proto_.quad(index), member_index);
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
      if (proto_.arity_size() < Arity) {
        return 0;
      }
      const RMFProto::ArityData &arity_data= proto_.arity(Arity-1);
      return arity_data.category_size();
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



  void ProtoBufSharedData::set_frame_name(unsigned int frame,
                                             std::string str) {
    while (proto_.frame_size() <= static_cast<int>(frame)) {
      proto_.add_frame();
    }
    proto_.mutable_frame(frame)->set_comment(str);
  }
  std::string ProtoBufSharedData::get_frame_name(unsigned int frame) const {
    if (proto_.frame_size() > static_cast<int>(frame)) {
      return proto_.frame(frame).comment();
    } else {
      return std::string();
    }
  }

  void ProtoBufSharedData::reload() {
    std::fstream in(name_.c_str(), std::ios::in | std::ios::binary);
    if (!in) {
      IMP_RMF_THROW("Could not open file", IOException);
    }
    proto_.ParseFromIstream(&in);
  }

  } // namespace internal
} /* namespace RMF */
#endif
