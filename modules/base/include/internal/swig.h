/**
 *  \file internal/swig.h
 *  \brief Functions for use in swig wrappers
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPBASE_INTERNAL_SWIG_H
#define IMPBASE_INTERNAL_SWIG_H

#include <IMP/base/base_config.h>
#include "graph_utility.h"
#include "../VersionInfo.h"
#include "../Object.h"
#include "../types.h"
#include "../utility_macros.h"
#include "../file.h"
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/copy.hpp>


IMPBASE_BEGIN_INTERNAL_NAMESPACE



template <class BG, class Label>
class BoostDigraph: public IMP::base::Object {
  BG bg_;
  typedef typename boost::graph_traits<BG> Traits;
  typedef typename boost::property_map<BG,
                              boost::vertex_name_t>::type VertexMap;
  VertexMap vm_;
  // keep vertex indices stable under removals
  Ints index_map_;
  template <class It>
  int distance(std::pair<It, It> r) const {
    return std::distance(r.first, r.second);
  }
  int get_vertex(int v) const{
    IMP_USAGE_CHECK(index_map_.size() > static_cast<unsigned int>(v),
                    "Out of range: " << v);
    IMP_USAGE_CHECK(index_map_[v] >=0, "Removed vertex requested: " << v);
    return boost::vertex(index_map_[v], bg_);
  }
public:
  BoostDigraph(): Object("Graph%1%"){
    vm_= boost::get(boost::vertex_name, bg_);
  }
#ifndef SWIG
  BoostDigraph(const BG& bg): Object("Graph"){
    boost::copy_graph(bg, bg_);
    vm_= boost::get(boost::vertex_name, bg_);
    for (unsigned int i=0; i< boost::num_vertices(bg_); ++i) {
      IMP_INTERNAL_CHECK(distance(boost::out_edges(i, bg_))
                         == distance(boost::out_edges(i, bg)),
                         "Edge counts don't match "
                         << distance(boost::out_edges(i, bg_))
                         << " vs "
                         << distance(boost::out_edges(i, bg)));
    }
    std::pair<typename Traits::vertex_iterator,
      typename Traits::vertex_iterator> be= boost::vertices(bg_);
    index_map_=Ints(be.first, be.second);
    IMP_INTERNAL_CHECK(get_vertices().size() ==
                      static_cast<unsigned int>(distance(boost::vertices(bg_))),
                       "Vertices don't match " << get_vertices().size()
                       << " vs " << distance(boost::vertices(bg_)));
  }
#endif
#ifndef SWIG
  BG &access_graph() {
    return bg_;
  }
#endif
  const BG &get_graph() const {
    return bg_;
  }
  typedef int VertexDescriptor;
  typedef Ints VertexDescriptors;
  typedef Label VertexName;
  VertexDescriptors get_vertices() const {
    set_was_used(true);
    IMP_CHECK_OBJECT(this);
    Ints ret;
    for (unsigned int i=0; i< index_map_.size(); ++i) {
      if (index_map_[i] >=0) {
        ret.push_back(i);
      }
    }
    return ret;
  }

  VertexName get_vertex_name(VertexDescriptor i) const {
    set_was_used(true);
    IMP_USAGE_CHECK(static_cast<unsigned int>(i) < boost::num_vertices(bg_),
                    "Out of range vertex " << i
                    << " " << boost::num_vertices(bg_));
    return boost::get(vm_, get_vertex(i));
  }
  VertexDescriptors get_in_neighbors(VertexDescriptor v) const {
    set_was_used(true);
    IMP_USAGE_CHECK(static_cast<unsigned int>(v) < boost::num_vertices(bg_),
                    "Out of range vertex " << v
                    << " " << boost::num_vertices(bg_));
    typedef typename Traits::in_edge_iterator IEIt;
    std::pair<IEIt, IEIt> be= boost::in_edges(get_vertex(v), bg_);
    Ints ret;
    for (; be.first != be.second; ++be.first) {
      ret.push_back(boost::source(*be.first, bg_));
    }
    return ret;
  }
  VertexDescriptors get_out_neighbors(VertexDescriptor v) const {
    set_was_used(true);
    IMP_USAGE_CHECK(static_cast<unsigned int>(v) < boost::num_vertices(bg_),
                    "Out of range vertex " << v
                    << " " << boost::num_vertices(bg_));
    typedef typename Traits::out_edge_iterator IEIt;
    std::pair<IEIt, IEIt> be= boost::out_edges(get_vertex(v), bg_);
    IMP_INTERNAL_CHECK(std::distance(be.first, be.second)< 10000,
                       "Insane number of neighbors "
                       << std::distance(be.first, be.second));
    Ints ret;
    for (; be.first != be.second; ++be.first) {
      ret.push_back(boost::target(*be.first, bg_));
    }
    return ret;
  }
  void do_show(std::ostream &out) const {
    set_was_used(true);
    show_as_graphviz(bg_, out);
  }
  void show_graphviz(std::ostream &out=std::cout) const {
    set_was_used(true);
    show_as_graphviz(bg_, out);
  }
  std::string get_graphviz_string() const {
    set_was_used(true);
    std::ostringstream oss;
    show_as_graphviz(bg_, oss);
    return oss.str();
  }
  std::string get_type_name() const {return "Graph";}
  VersionInfo get_version_info() const {
    return VersionInfo("IMP", get_module_version());
  }

  void add_edge(VertexDescriptor v0, VertexDescriptor v1) {
    boost::add_edge(v0, v1, bg_);
  }
  VertexDescriptor add_vertex(VertexName l) {
    VertexDescriptor v=boost::add_vertex(bg_);
    boost::put(vm_, v, l);
    index_map_.push_back(v);
    return index_map_.size()-1;
  }
  void remove_vertex(VertexDescriptor l) {
    boost::clear_vertex(get_vertex(l), bg_);
    boost::remove_vertex(get_vertex(l), bg_);
    index_map_[l]=-1;
    for (unsigned int i=l+1; i < index_map_.size(); ++i) {
      --index_map_[i];
    }
  }
};

#if 0
inline void bad_pass(FloatKey*) {}
inline void bad_pass(FloatKeys*) {}
#endif


class _Protection {
protected:
  _Protection(){}
  double get_one(){return 1.0;}
 public:
  IMP_SHOWABLE_INLINE(_Protection, IMP_UNUSED(out));
};


IMPBASEEXPORT void _test_log();

IMP_VALUES(_Protection, _Protections);


IMPBASEEXPORT int _test_intranges(const IntRanges &ips);

IMPBASEEXPORT IntRange _test_intrange(const IntRange &ips);

// for overload
IMPBASEEXPORT IntRange _test_intrange();

IMPBASEEXPORT std::string _test_ifile(base::TextInput a);
IMPBASEEXPORT std::string _test_ofile(base::TextOutput a);
// overload
IMPBASEEXPORT std::string _test_ifile_overloaded(base::TextInput a, int i);
IMPBASEEXPORT std::string _test_ofile_overloaded(base::TextOutput a, int i);
IMPBASEEXPORT std::string
_test_ifile_overloaded(base::TextInput a, std::string st);
IMPBASEEXPORT std::string _test_ofile_overloaded(base::TextOutput a,
                                             std::string st);

class IMPBASEEXPORT _TestValue {
  int i_;
 public:
  _TestValue(int i): i_(i){}
  IMP_SHOWABLE_INLINE(_TestValue, out << i_;);
  IMP_COMPARISONS_1(_TestValue, i_);
  int get() const {return i_;}
};

IMP_VALUES(_TestValue, _TestValues);

IMPBASEEXPORT FloatPair _pass_plain_pair( FloatPair p);


IMPBASEEXPORT Strings _pass_overloaded_strings(const Strings &a,
                                 int);
IMPBASEEXPORT Strings _pass_overloaded_strings(const Strings &a);

IMPBASEEXPORT DerivativePair
_pass_pair(const DerivativePair &p);

IMPBASEEXPORT Floats _pass_floats(const Floats& input);
IMPBASEEXPORT Ints _pass_ints( Ints input);
IMPBASEEXPORT IntsList _pass_ints_list(const IntsList &input);
IMPBASEEXPORT IntsLists _pass_ints_lists(const IntsLists &input);
IMPBASEEXPORT const Strings& _pass_strings(const Strings& input);

class _TestObject: public Object {
public:
  _TestObject(): Object("TestObject%1%") {}
  IMP_OBJECT_METHODS(_TestObject);
};

IMPBASE_END_INTERNAL_NAMESPACE

#endif  /* IMPBASE_INTERNAL_SWIG_H */
