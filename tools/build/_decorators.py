#!/usr/bin/env python

import os.path
import sys


def replace(msg, to_replace):
    for k in to_replace:
        msg = msg.replace(k[0], k[1])
    return msg


class Base:

    def __init__(self, name, data_type, return_type):
        self.names = [("NAME", name.replace(" ", "_")),
                      ("DATA", data_type)]
        self.names.extend([("GET_BOTH", "get_value"),
                           ("SET_BOTH", "set_value"),
                           ("GET_STATIC", "get_static_value"),
                           ("SET_STATIC", "set_static_value"),
                           ("GET_FRAME", "get_frame_value"),
                           ("SET_FRAME", "set_frame_value")])
        if return_type.endswith("s"):
            self.names.append(("TYPES", return_type + "List"))
        elif return_type.endswith("x"):
            self.names.append(("TYPES", return_type + "es"))
        else:
            self.names.append(("TYPES", return_type + "s"))
        self.names.append(("TYPE", return_type))
        self.get_methods = ""
        self.set_methods = ""
        self.bulk_get_methods = ""
        self.bulk_set_methods = ""
        self.helpers = ""
        self.check = ""

    def get_data_members(self):
        return replace("DATA NAME_;", self.names)

    def get_get_set_methods(self, const):
        if not const:
            ret = replace(self.set_methods, self.names)
        else:
            ret = replace(self.get_methods, self.names)
        return ret

    def get_helpers(self):
        return replace(self.helpers, self.names)

    def get_data_arguments(self):
        return replace("DATA NAME", self.names)

    def get_data_pass(self, member):
        if member:
            return replace("NAME_", self.names)
        else:
            return replace("NAME", self.names)

    def get_data_saves(self):
        return replace("NAME_(NAME)", self.names)

    def get_data_initialize(self):
        return (
            replace("NAME_(" + self.data_initialize + ")", self.names)
        )

    def get_check(self):
        return replace(self.check, self.names)


class Attribute(Base):

    def __init__(
        self,
        name,
        attribute_type,
            function_name=None):
        if not function_name:
            function_name = name.replace(" ", "_")
        Base.__init__(self, name, attribute_type +
                      "Key", attribute_type)
        self.get_methods = """
  TYPE get_%s() const {
    try {
      return get_node().GET_BOTH(NAME_);
    } RMF_DECORATOR_CATCH( );
  }
  TYPE get_frame_%s() const {
    try {
      return get_node().GET_FRAME(NAME_);
    } RMF_DECORATOR_CATCH( );
  }
  TYPE get_static_%s() const {
    try {
      return get_node().GET_STATIC(NAME_);
    } RMF_DECORATOR_CATCH( );
  }
""" % (function_name, function_name, function_name)
        self.set_methods = """
  void set_%s(TYPE v) {
    try {
      get_node().SET_BOTH(NAME_, v);
    } RMF_DECORATOR_CATCH( );
  }
  void set_frame_%s(TYPE v) {
    try {
      get_node().SET_FRAME(NAME_, v);
    } RMF_DECORATOR_CATCH( );
  }
  void set_static_%s(TYPE v) {
    try {
      get_node().SET_STATIC(NAME_, v);
    } RMF_DECORATOR_CATCH( );
  }
""" % (function_name, function_name, function_name)
        self.check = "!nh.GET(NAME_).get_is_null()"
        self.data_initialize = "fh.get_key<TYPETag>(cat_, \"%s\")" % name


class NodeAttribute(Attribute):

    def __init__(self, name):
        Attribute.__init__(self, name, "Int", True)
        self.get_methods = """
  NodeConstHandle get_NAME() const {
    try {
      int id = get_node().GET_BOTH(NAME_);
      return get_node().get_file().get_node(NodeID(id));
    } RMF_DECORATOR_CATCH( );
  }
"""
        self.set_methods = """
  void set_NAME(NodeConstHandle v) {
    try {
      get_node().SET_BOTH(NAME_, v.get_id().get_index());
    } RMF_DECORATOR_CATCH( );
  }
"""


class PathAttribute(Attribute):

    def __init__(self, name):
        Attribute.__init__(self, name, "String")
        self.get_methods = """
  String get_NAME() const {
    try {
      String relpath = get_node().GET_BOTH(NAME_);
      String filename = get_node().get_file().get_path();
      return internal::get_absolute_path(filename, relpath);
    } RMF_DECORATOR_CATCH( );
  }
"""
        self.set_methods = """
  void set_NAME(String path) {
   try {
     String filename = get_node().get_file().get_path();
     String relpath = internal::get_relative_path(filename, path);
     get_node().SET_BOTH(NAME_, relpath);
   } RMF_DECORATOR_CATCH( );
  }
"""


class AttributePair(Base):

    def __init__(
            self,
            name,
        data_type,
        return_type,
        begin,
            end):
        Base.__init__(self, name, "boost::array<%sKey, 2>" %
                      data_type, return_type)
        self.helpers = """  template <class H> DATA get_NAME_keys(H fh) const {
     DATA ret;
     ret[0] = fh.template get_key<%sTag>(cat_, "%s");
     ret[1] = fh.template get_key<%sTag>(cat_, "%s");
     return ret;
    }
""" % (data_type, begin, data_type, end)
        self.check = "!nh.GET(NAME_[0]).get_is_null() && !nh.GET(NAME_[1]).get_is_null()"
        self.data_initialize = "get_NAME_keys(fh)"


class SingletonRangeAttribute(AttributePair):

    def __init__(self, name, data_type, begin, end):
        AttributePair.__init__(
            self, name, data_type, data_type, begin, end)
        self.get_methods = """
  TYPE get_NAME() const {
    try {
      return get_node().GET_BOTH(NAME_[0]);
    } RMF_DECORATOR_CATCH( );
  }
  TYPE get_frame_NAME() const {
    try {
      return get_node().GET_FRAME(NAME_[0]);
    } RMF_DECORATOR_CATCH( );
  }
  TYPE get_static_NAME() const {
    try {
      return get_node().GET_STATIC(NAME_[0]);
    } RMF_DECORATOR_CATCH( );
  }
"""
        self.set_methods = """
  void set_NAME(TYPE v) {
    try {
      get_node().SET_BOTH(NAME_[0], v);
      get_node().SET_BOTH(NAME_[1], v);
    } RMF_DECORATOR_CATCH( );
  }
  void set_frame_NAME(TYPE v) {
    try {
      get_node().SET_FRAME(NAME_[0], v);
      get_node().SET_FRAME(NAME_[1], v);
    } RMF_DECORATOR_CATCH( );
  }
  void set_static_NAME(TYPE v) {
    try {
      get_node().SET_STATIC(NAME_[0], v);
      get_node().SET_STATIC(NAME_[1], v);
    } RMF_DECORATOR_CATCH( );
  }
"""
        self.check = "!nh.GET(NAME_[0]).get_is_null() && !nh.GET(NAME_[1]).get_is_null() && nh.GET_BOTH(NAME_[0]) == nh.GET_BOTH(NAME_[1])"


class RangeAttribute(AttributePair):

    def __init__(self, name, data_type, begin, end):
        AttributePair.__init__(
            self, name, data_type, data_type + "Range", begin, end)
        self.get_methods = """
  TYPE get_NAME() const {
    try {
      TYPE ret;
      ret[0] = get_node().GET_BOTH(NAME_[0]);
      ret[1] = get_node().GET_BOTH(NAME_[1]);
      return ret;
    } RMF_DECORATOR_CATCH( );
  }
  TYPE get_static_NAME() const {
    try {
      TYPE ret;
      ret[0] = get_node().GET_STATIC(NAME_[0]);
      ret[1] = get_node().GET_STATIC(NAME_[1]);
      return ret;
    } RMF_DECORATOR_CATCH( );
  }
  TYPE get_frame_NAME() const {
    try {
    TYPE ret;
      ret[0] = get_node().GET_FRAME(NAME_[0]);
      ret[1] = get_node().GET_FRAME(NAME_[1]);
      return ret;
    } RMF_DECORATOR_CATCH( );
  }
"""
        self.set_methods = """
  void set_NAME(%s v0, %s v1) {
    try {
      get_node().SET_BOTH(NAME_[0], v0);
      get_node().SET_BOTH(NAME_[1], v1);
    } RMF_DECORATOR_CATCH( );
  }
  void set_frame_NAME(%s v0, %s v1) {
    try {
      get_node().SET_FRAME(NAME_[0], v0);
      get_node().SET_FRAME(NAME_[1], v1);
    } RMF_DECORATOR_CATCH( );
  }
  void set_static_NAME(%s v0, %s v1) {
    try {
      get_node().SET_STATIC(NAME_[0], v0);
      get_node().SET_STATIC(NAME_[1], v1);
    } RMF_DECORATOR_CATCH( );
  }
""" % (data_type, data_type, data_type, data_type, data_type, data_type)
        self.check = "!nh.GET(NAME_[0]).get_is_null() && !nh.GET(NAME_[1]).get_is_null() && nh.GET_BOTH(NAME_[0]) < nh.GET_BOTH(NAME_[1])"

        self.check = "!nh.GET(NAME_[0]).get_is_null() && !nh.GET(NAME_[1]).get_is_null() && nh.GET_BOTH(NAME_[0]) < nh.GET_BOTH(NAME_[1])"


decorator = """

  /** See also NAME and NAMEFactory.
    */
    class NAMEConst: public Decorator {
    friend class NAMEFactory;
  protected:
    DATA_MEMBERS
    NAMEConst(NodeConstHandle nh,
              DATA_ARGUMENTS):
       DATA_SAVES {
    }
  public:
    CONSTMETHODS
    static std::string get_decorator_type_name() {
         return "NAMEConst";
    }
    RMF_SHOWABLE(NAMEConst, "NAME: " << get_node());
  };
   /** See also NAMEFactory.
    */
    class NAME: public NAMEConst {
    friend class NAMEFactory;
    NAME(NodeHandle nh,
              DATA_ARGUMENTS):
       NAMEConst(nh, DATA_PASS_ARGUMENTS) {
    }
  public:
    NONCONSTMETHODS
    static std::string get_decorator_type_name() {
         return "NAME";
    }
  };

"""


factory = """
  /** Create decorators of type NAME.
    */
  class NAMEFactory: public Factory {
    Category cat_;
DATA_MEMBERS
HELPERS
  public:
    NAMEFactory(FileConstHandle fh):
    cat_(fh.get_category("CATEGORY")),
    DATA_INITIALIZE {
    }
     NAMEFactory(FileHandle fh):
    cat_(fh.get_category("CATEGORY")),
    DATA_INITIALIZE {
    }
    /** Get a NAMEConst for nh.*/
    NAMEConst get(NodeConstHandle nh) const {
      CREATE_CHECKS
      return NAMEConst(nh, DATA_PASS);
    }
    /** Get a NAME for nh.*/
    NAME get(NodeHandle nh) const {
      CREATE_CHECKS
      return NAME(nh, DATA_PASS);
    }
    /** Check whether nh has all the attributes required to be a
        NAMEConst.*/
    bool get_is(NodeConstHandle nh) const {
      return FRAME_CHECKS;
    }
    bool get_is_static(NodeConstHandle nh) const {
      return STATIC_CHECKS;
    }
    RMF_SHOWABLE(NAMEFactory, "NAMEFactory");
  };
  #ifndef RMF_DOXYGEN
struct NAMEConstFactory: public NAMEFactory {
    NAMEConstFactory(FileConstHandle fh):
    NAMEFactory(fh) {
    }
    NAMEConstFactory(FileHandle fh):
    NAMEFactory(fh) {
    }

};
  #endif

"""


class Decorator:

    def __init__(self, allowed_types, category, name,
                 attributes,
                 init_function="", check_all_attributes=False):
        self.name = name
        self.category = category
        self.allowed_types = allowed_types
        self.init_function = init_function
        self.attributes = attributes
        self.check_all_attributes = check_all_attributes

    def _get_data_members(self):
        ret = []
        for a in self.attributes:
            ret.append(a.get_data_members())
        return "\n".join(ret)

    def _get_methods(self, const):
        ret = []
        for a in self.attributes:
            ret.append(a.get_get_set_methods(const))
        return "\n".join(ret)

    def _get_bulk_methods(self, const):
        ret = []
        for a in self.attributes:
            ret.append(a.get_bulk_methods(const))
        return "\n".join(ret)

    def _get_helpers(self):
        ret = []
        for a in self.attributes:
            ret.append(a.get_helpers())
        return "\n".join(ret)

    def _get_data_arguments(self):
        ret = []
        for a in self.attributes:
            ret.append(a.get_data_arguments())
        return ",\n".join(ret)

    def _get_data_pass(self, member):
        ret = []
        for a in self.attributes:
            ret.append(a.get_data_pass(member))
        return ",\n".join(ret)

    def _get_data_saves(self):
        ret = []
        for a in self.attributes:
            ret.append(a.get_data_saves())
        return ",\n".join(["Decorator(nh)"] + ret)

    def _get_type_check(self):
        cret = []
        for t in self.allowed_types:
            cret.append("nh.get_type() == RMF::%s" % t)
        return "(" + "||".join(cret) + ")"

    def _get_checks(self, use_all=False):
        ret = [self._get_type_check()]
        if self.check_all_attributes or use_all:
            for a in self.attributes:
                ret.append(a.get_check())
        else:
        # for a in self.attributes:
            ret.append(self.attributes[0].get_check())
        return "\n    && ".join(x for x in ret if x != "")

    def _get_construct(self):
        ret = []
        # make handle missing later
        ret.append("Category cat = fh.get_category(\""
                   + self.category + "\");")
        ret.append("RMF_UNUSED(cat);")
        for a in self.attributes:
            ret.append(a.get_construct())
        return "\n".join(ret)

    def _get_data_initialize(self):
        ret = []
        for a in self.attributes:
            ret.append(a.get_data_initialize())
        return ", ".join(ret)

    def _get_list(self):
        ret = [("HELPERS", self._get_helpers()),
               ("DATA_MEMBERS", self._get_data_members()),
               ("NONCONSTMETHODS", self._get_methods(False)),
               ("CONSTMETHODS", self._get_methods(True)),
               ("DATA_ARGUMENTS", self._get_data_arguments()),
               ("DATA_SAVES", self._get_data_saves()),
               ("DATA_PASS_ARGUMENTS", self._get_data_pass(False)),
               ("DATA_PASS", self._get_data_pass(True)),
               ("DATA_INITIALIZE", self._get_data_initialize())]
        ret.append(("CREATE_CHECKS", """RMF_USAGE_CHECK(%s, std::string("Bad node type. Got \\\"")
                                      + boost::lexical_cast<std::string>(nh.get_type())
                                      + "\\\" in decorator type  %s");""" % (self._get_type_check(),
                                                                             self.name)))
        ret.append(
            ("FRAME_CHECKS", self._get_checks()
             .replace("GET", "get_value")))
        ret.append(
            ("STATIC_CHECKS", self._get_checks(use_all=True)
             .replace("GET", "get_static_value")))
        return ret

    def get(self):
        ret = ""
        common = [("NAME", self.name),
                  ("CATEGORY", self.category)]
        ret += replace(decorator, common + self._get_list())
        ret += replace(factory, common + self._get_list())
        return ret


def make_header(name, infos, deps):
    path = os.path.join("include", "RMF", "decorator")
    if not os.path.exists(path):
        os.makedirs(path)

    path = os.path.join("include", "RMF", "decorator", name + ".h")
    fl = open(path, "w")
    fl.write("""/**
 *  \\file RMF/decorator/%(name)s.h
 *  \\brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2015 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_%(NAME)s_DECORATORS_H
#define RMF_%(NAME)s_DECORATORS_H

#include <RMF/config.h>
#include <RMF/infrastructure_macros.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/Decorator.h>
#include <RMF/constants.h>
#include <RMF/Vector.h>
#include <RMF/internal/paths.h>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
""" % {"name": name, "NAME": name.upper()})
    for d in deps:
        fl.write('#include "%s.h"\n' % d)
    fl.write("""
RMF_ENABLE_WARNINGS
namespace RMF {
namespace decorator {
""" % {"name": name, "NAME": name.upper()})
    for i in infos:
        fl.write(i.get() + '\n')
    fl.write("""
} /* namespace decorator */
} /* namespace RMF */
RMF_DISABLE_WARNINGS

#endif /* RMF_%(NAME)s_DECORATORS_H */
""" % {"name": name, "NAME": name.upper()})

    del fl
    root = os.path.split(os.path.split(sys.argv[0])[0])[0]
    os.system(
        os.path.join(
            root,
            "dev_tools",
            "cleanup_code.py") +
        " " +
        path)
