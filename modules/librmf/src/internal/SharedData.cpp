/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/internal/SharedData.h>
#include <RMF/NodeHandle.h>
#include <RMF/NodeSetHandle.h>
#include <RMF/Validator.h>
#include <RMF/internal/set.h>
namespace RMF {
  namespace internal {



    SharedData::~SharedData() {
      valid_=-66666;
    }

    void SharedData::audit_key_name(std::string name) const {
      if (name.empty()) {
        IMP_RMF_THROW("Empty key name", UsageException);
      }
      static const char *illegal="\\:=()[]{}\"'";
      const char *cur=illegal;
      while (*cur != '\0') {
        if (name.find(*cur) != std::string::npos) {
          IMP_RMF_THROW(get_error_message("Key names can't contain ",
                                          *cur), UsageException);
        }
        ++cur;
      }
      if (name.find("  ") != std::string::npos) {
        IMP_RMF_THROW("Key names can't contain two consecutive spaces",
                      UsageException);
      }
    }

    void SharedData::audit_node_name(std::string name) const {
      if (name.empty()) {
        IMP_RMF_THROW("Empty key name", UsageException);
      }
      static const char *illegal="\"";
      const char *cur=illegal;
      while (*cur != '\0') {
        if (name.find(*cur) != std::string::npos) {
          IMP_RMF_THROW(get_error_message("Node names names can't contain \"",
                                          *cur,
                                          "\", but \"", name, "\" does."),
                        UsageException);
        }
        ++cur;
      }
    }


  void SharedData::add_bond( int ida,  int idb, int type) {
    int bond=get_category(2, "bond");
    if (bond==-1) {
      bond=add_category(2, "bond");
    }
    IMP_RMF_USAGE_CHECK(ida>=0 && idb>=0,
                        get_error_message("Invalid bond ",
                                          ida, " ", idb));
    RMF::Indexes tp(2);
    tp[0]=ida;
    tp[1]=idb;
    int ind=add_set(tp, BOND);
    PairIndexKey pik=get_index_key_2(bond, "type", false);
    if (pik==PairIndexKey()) {
      pik= add_index_key_2(bond, "type", false);
    }
    IMP_RMF_IF_CHECK{
      flush();
      for ( int i=0; i< ind; ++i) {
        /*boost::tuple<int,int,int> bd=*/ get_bond(i);
      }
    }
    IMP_RMF_USAGE_CHECK(type != -1, get_error_message("Invalid type passed: ",
                                                      type));
    set_value(ind, pik, type, -1);
    IMP_RMF_IF_CHECK{
      flush();
      for ( int i=0; i< ind+1; ++i) {
        /*boost::tuple<int,int,int> bd=*/ get_bond(i);
      }
    }
  }

    unsigned int SharedData::get_number_of_bonds() const {
      // not really right
      return get_number_of_sets(2);
    }
    boost::tuple<int,int,int> SharedData::get_bond(unsigned int i) const {
      int bond=get_category(2, "bond");
      int na= get_set_member(2, i, 0);
      int nb= get_set_member(2, i, 1);
      PairIndexKey pik=get_index_key_2(bond, "type", false);
      int t= get_value(i, pik, -1);
      return boost::tuple<int,int,int>(na, nb, t);
    }



  void SharedData::validate() const {
    Creators cs= get_validators();
    for (unsigned int i=0; i< cs.size(); ++i) {
      boost::scoped_ptr<Validator>
          ptr(cs[i]->create(FileHandle(const_cast<SharedData*>(this))));
      ptr->write_errors(std::cerr);
    }
  }

  } // namespace internal
} /* namespace RMF */
