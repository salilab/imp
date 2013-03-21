#!/usr/bin/env python

import os.path

def get_string(type, name, const):
    return """fh.get_key<%(type)sTraits>(cat,
        \"%(name)s\")"""%{ "name":name,
                               "type": type}

def gets_string(type, name, const):
    return """fh.get_keys<%(type)sTraits>(cat,
        %(name)s)"""%{ "name":name,
                      "type": type}
class Children:
    def __init__(self, nice_name, doc):
        self.nice_name=nice_name
        self.doc=doc
    def get_key_members(self, const):
        if (const):
            return ["AliasConstFactory "+self.nice_name+"_;"]
        else:
            return ["AliasFactory "+self.nice_name+"_;"]
    def get_methods(self, const):
        ret=[]
        if const:
            nht="NodeConstHandle"
        else:
            nht="NodeHandle"
        ret.append("/** "+self.doc+" */")
        ret.append(nht+"s get_"+self.nice_name+"() const {")
        ret.append("  try{")
        ret.append("  "+nht+"s typed=get_node().get_children();")
        ret.append("  "+nht+"s ret;")
        ret.append("  for (unsigned int i=0; i< typed.size(); ++i) {")
        ret.append("     if ("+self.nice_name+"_.get_is(typed[i])) {")
        ret.append("        ret.push_back("+self.nice_name+"_.get(typed[i]).get_aliased());")
        ret.append("     }");
        ret.append("  }");
        ret.append("  return ret;")
        ret.append("  } RMF_DECORATOR_CATCH( );")
        ret.append("}")
        if not const:
            ret.append("/** "+self.doc+" */")
            ret.append("void set_"+self.nice_name+"(NodeConstHandles v) {")
            ret.append("  try{")
            ret.append("   for (unsigned int i=0; i< v.size(); ++i) {")
            ret.append("       internal::add_child_alias("+self.nice_name+"_, get_node(), v[i]);")
            ret.append("   }")
            ret.append("  } RMF_DECORATOR_CATCH( );")
            ret.append("}")
            ret.append("/** "+self.doc+" */")
            ret.append("void set_"+self.nice_name+"(NodeHandles v) {")
            ret.append("  try{")
            ret.append("   for (unsigned int i=0; i< v.size(); ++i) {")
            ret.append("       internal::add_child_alias("+self.nice_name+"_, get_node(), v[i]);")
            ret.append("   }")
            ret.append("  } RMF_DECORATOR_CATCH( );")
            ret.append("}")
        return ret
    def get_key_arguments(self, const):
        if (const):
            return ["AliasConstFactory "+self.nice_name+""]
        else:
            return ["AliasFactory "+self.nice_name+""]
    def get_key_pass(self, const):
        return [self.nice_name+"_"]
    def get_key_saves(self, const):
        return [self.nice_name+"_("+self.nice_name+")"]
    def get_initialize(self, const):
        if (const):
            return [self.nice_name+"_(fh)"]
        else:
            return [self.nice_name+"_(fh)"]
    def get_construct(self, const):
        return []
    def get_check(self, const):
        return []


class Attribute:
    def __init__(self, tt, nice_name, attribute_name, doc):
        self.type=tt
        if tt.endswith("s"):
            self.plural_type=tt+"List"
        elif tt.endswith("x"):
            self.plural_type=tt+"es"
        else:
            self.plural_type=tt+"s"
        self.nice_name=nice_name
        self.attribute_name=attribute_name
        self.doc=doc
    def get_key_members(self, const):
        return [self.type+"Key "+self.nice_name+"_;"]
    def get_methods(self, const):
        ret=[]
        ret.append("/** "+self.doc+" */")
        ret.extend([self.type+" get_"+self.nice_name+"() const {",
                    "  try{",
                    "  return P::get_value("+self.nice_name+"_);",
                    "  } RMF_DECORATOR_CATCH( );",
                   "}"])
        ret.append("/** "+self.doc+" */")
        ret.extend([self.plural_type+" get_all_"+self.nice_name+"s() const {",
                    "  try {",
                    "  return P::get_all_values("+self.nice_name+"_);",
                    "  } RMF_DECORATOR_CATCH( );",
                   "}"])
        if not const:
            ret.append("/** "+self.doc+" */")
            ret.extend(["void set_"+self.nice_name+"("+self.type+" v) {",
                        "  try {",
                        "  P::set_value("+self.nice_name+"_,",
                        "            v);",
                        "  } RMF_DECORATOR_CATCH( );",
                        "}"])
        return ret
    def get_key_arguments(self, const):
        return [self.type+"Key "+self.nice_name]
    def get_key_pass(self, const):
        return [self.nice_name+"_"]
    def get_key_saves(self, const):
        return [self.nice_name+"_("+self.nice_name+")"]
    def get_construct(self, const):
        return [self.nice_name+"_="+get_string(self.type, self.attribute_name, const)+";"]
    def get_initialize(self, const):
        return []
    def get_check(self, const):
        return ["""nh.get_has_value(%(nn)s_)"""%{"nn":self.nice_name}]


class NodeAttribute(Attribute):
    def __init__(self, *args):
        Attribute.__init__(self, *args)
    def get_methods(self, const):
        ret=[]
        if const:
            nht= "NodeConstHandle"
        else:
            nht= "NodeHandle"
        ret.append("/** "+self.doc+" */")
        ret.extend([nht+" get_"+self.nice_name+"() const {",
                    "  try {",
                    "  NodeID id;",
                    "   id= get_node().get_value("+self.nice_name+"_);",
                    "  return get_node().get_file().get_node_from_id(id);",
                    "  } RMF_DECORATOR_CATCH( );",
                    "}"])
        if not const:
            ret.append("/** "+self.doc+" */")
            ret.extend(["void set_"+self.nice_name+"(NodeConstHandle v) {",
                        "  try {",
                        "    get_node().set_value("+self.nice_name+"_, v.get_id());",
                        "  } RMF_DECORATOR_CATCH( );",
                        "}"])
        return ret

class PathAttribute(Attribute):
    def __init__(self, *args):
        Attribute.__init__(self, *args)
    def get_methods(self, const):
        ret=[]
        ret.append("/** "+self.doc+" */")
        ret.extend(["String get_"+self.nice_name+"() const {",
                    " try {",
                    " String relpath;"
                    "   relpath= get_node().get_value("+self.nice_name+"_);",
                    "  String filepath=get_node().get_file().get_path();",
                    "  return internal::get_absolute_path(filepath, relpath);",
                    "  } RMF_DECORATOR_CATCH( );",
                    "}"])
        if not const:
            ret.append("/** "+self.doc+" */")
            ret.extend(["void set_"+self.nice_name+"(String path) {",
                        " try {",
                        "  String filename= get_node().get_file().get_path();",
                        "  String relpath= internal::get_relative_path(filename, path);",
                        "    return get_node().set_value("+self.nice_name+"_, relpath);",
                        "  } RMF_DECORATOR_CATCH( );",
                        "}"])
        return ret

class SingletonRangeAttribute:
    def __init__(self, type, nice_name, attribute_name_begin, attribute_name_end, doc):
        self.type=type
        self.doc=doc
        self.nice_name=nice_name
        self.attribute_name_begin=attribute_name_begin
        self.attribute_name_end=attribute_name_end
    def get_key_members(self, const):
        return ["boost::array<"+self.type+"Key,2> "+self.nice_name+"_;"]
    def get_methods(self, const):
        ret=[]
        ret.append("/** "+self.doc+" */")
        ret.extend([self.type+" get_"+self.nice_name+"() const {",
                    "  try {",
                    "  return get_node().get_value("+self.nice_name+"_[0]);",
                    "  } RMF_DECORATOR_CATCH( );",
                    "}"])
        if not const:
            ret.append("/** "+self.doc+" */")
            ret.extend(["void set_"+self.nice_name+"("+self.type+" v) {",
                        "  try {",
                        "   get_node().set_value("+self.nice_name+"_[0], v);",
                        "   get_node().set_value("+self.nice_name+"_[1], v);",
                        "  } RMF_DECORATOR_CATCH( );",
                "}"])
        return ret
    def get_key_arguments(self, const):
        return ["boost::array<"+self.type+"Key, 2> "+self.nice_name]
    def get_key_pass(self, const):
        return [self.nice_name+"_"]
    def get_key_saves(self, const):
        return [self.nice_name+"_("+self.nice_name+")"]
    def get_initialize(self, const):
        return []
    def get_construct(self, const):
        return [self.nice_name+"_[0]="+get_string(self.type, self.attribute_name_begin, const)+\
            ";\n"+self.nice_name+"_[1]="+get_string(self.type, self.attribute_name_end, const)+";"]
    def get_check(self, const):
        return ["nh.get_has_value("+self.nice_name+"_[0])"+\
            "\n  && nh.get_has_value("+self.nice_name+"_[1])"+\
            "\n  && nh.get_value("+self.nice_name+"_[0])"\
            "\n   ==nh.get_value("+self.nice_name+"_[1])"]


class RangeAttribute:
    def __init__(self, type, nice_name, attribute_name_begin,
                 attribute_name_end, doc):
        self.type=type
        self.doc=doc
        self.nice_name=nice_name
        self.attribute_name_begin=attribute_name_begin
        self.attribute_name_end=attribute_name_end
    def get_key_members(self, const):
        return ["boost::array<"+self.type+"Key,2> "+self.nice_name+"_;"]
    def get_methods(self, const):
        ret=[]
        ret.append("/** "+self.doc+" */")
        ret.extend([self.type+"Range get_"+self.nice_name+"() const {",
                    "  try {",
                    "  return std::make_pair(get_node().get_value("+self.nice_name+"_[0]),",
                    "                        get_node().get_value("+self.nice_name+"_[1]));",
                    "  } RMF_DECORATOR_CATCH( );",
            "}"])
        if not const:
            ret.append("/** "+self.doc+" */")
            ret.extend(["void set_"+self.nice_name+"("+self.type+" v0, "+self.type+" v1) {",
                        " try {",
                        "   get_node().set_value("+self.nice_name+"_[0], v0);",
                        "   get_node().set_value("+self.nice_name+"_[1], v1);",
                        "  } RMF_DECORATOR_CATCH( );",
                "}"])

        return ret
    def get_key_arguments(self, const):
        return ["boost::array<"+self.type+"Key, 2> "+self.nice_name]
    def get_key_pass(self, const):
        return [self.nice_name+"_"]
    def get_key_saves(self, const):
        return [self.nice_name+"_("+self.nice_name+")"]
    def get_initialize(self, const):
        return []
    def get_construct(self, const):
        return [self.nice_name+"_[0]="+get_string(self.type, self.attribute_name_begin, const)+\
            ";\n"+self.nice_name+"_[1]="+get_string(self.type, self.attribute_name_end, const)+";"]
    def get_check(self, const):
        return ["nh.get_has_value("+self.nice_name+"_[0])"+\
            "\n  && nh.get_has_value("+self.nice_name+"_[1])"+\
            "\n  && nh.get_value("+self.nice_name+"_[0])"\
            "\n   <nh.get_value("+self.nice_name+"_[1])"]
class Attributes:
    def __init__(self, type, ptype, nice_name, attribute_names, doc):
        self.type=type
        self.doc=doc
        self.nice_name=nice_name
        self.ptype=ptype
        self.attribute_names=attribute_names
    def get_key_members(self, const):
        return [self.type+"Keys "+self.nice_name+"_;"]
    def get_methods(self, const):
        ret=[]
        if not const:
            ret.append("/** "+self.doc+" */")
            ret.append("""%(ptype)s get_%(name)s() const {
             return P::get_values(%(key)s);
           }"""%{"type":self.type,
            "ptype":self.ptype,
            "name":self.nice_name,
            "len":len(self.attribute_names),
            "key":self.nice_name+"_"})
            ret.append("""/** %(doc)s */
    void set_%(name)s(const %(ptype)s &v) {
           P::set_values(%(key)s, v);
        }"""%{"doc": self.doc,
              "type":self.type,
              "ptype":self.ptype,
              "name":self.nice_name,
              "len":len(self.attribute_names),
              "key":self.nice_name+"_"})
        else:
            ret.append("/** "+self.doc+" */")
            ret.append("""%(ptype)s get_%(name)s() const {
             return P::get_values(%(key)s);
           }"""%{"type":self.type,
            "ptype":self.ptype,
            "name":self.nice_name,
            "len":len(self.attribute_names),
            "key":self.nice_name+"_"})
        return ret
    def get_key_arguments(self, const):
        return [self.type+"Keys "+self.nice_name]
    def get_key_pass(self, const):
        return [self.nice_name+"_"]
    def get_key_saves(self, const):
        return [self.nice_name+"_("+self.nice_name+")"]
    def get_initialize(self, const):
        return []
    def get_construct(self, const):
        ret=[]
        ret.append("""        Strings  %(name)s_names;"""%{"name":self.nice_name})
        for nin in self.attribute_names:
            ret.append("""        %(name)s_names.push_back("%(thisname)s");"""%{"name":self.nice_name,
                                                                                "thisname":nin})
        ret.append("""      %(name)s_=%(get)s;"""%{"name":self.nice_name,
                                                   "get":gets_string(self.type,
                                                                     "%(name)s_names"%{"name":self.nice_name},
                                                                     const)})
        return ret
    def get_check(self, const):
        return ["""nh.get_has_value(%(nn)s_[0])"""%{"nn":self.nice_name}]

# currently writing multiple plural attributes is not supported
class PluralAttributes(Attributes):
    def get_methods(self, const):
        ret=[]
        if not const:
            ret.append("/** "+self.doc+" */")
            ret.append("""%(ptype)s get_%(name)s() const {
         %(ptype)s ret(%(len)s);
           for (unsigned int i=0; i< %(len)s; ++i) {
            ret[i]=get_node().get_value(%(key)s[i]);
           }
         return ret;
      }"""%{"type":self.type,
            "ptype":self.ptype,
            "name":self.nice_name,
            "len":len(self.attribute_names),
            "key":self.nice_name+"_"})
            ret.append("/** "+self.doc+" */")
            ret.append("""void set_%(name)s(const %(ptype)s &v) {
             for (unsigned int i=0; i< %(len)s; ++i) {
                get_node().set_value(%(key)s[i], v[i]);
             }
        }"""%{"type":self.type,
              "ptype":self.ptype,
              "name":self.nice_name,
              "len":len(self.attribute_names),
              "key":self.nice_name+"_"})
        else:
            ret.append("/** "+self.doc+" */")
            ret.append("""%(ptype)s get_%(name)s() const {
         %(ptype)s ret(%(len)s);
           for (unsigned int i=0; i< %(len)s; ++i) {
            ret[i]=get_node().get_value(%(key)s[i]);
           }
         return ret;
      }"""%{"type":self.type,
            "ptype":self.ptype,
            "name":self.nice_name,
            "len":len(self.attribute_names),
            "key":self.nice_name+"_"})
        return ret

class Decorator:
    def __init__(self, allowed_types, category, name, description,
                 attributes, internal_attributes=[],
                 init_function=[],
                 check_function=[]):
        self.name=name
        self.category=category
        self.allowed_types= allowed_types
        self.description=description
        self.init_function=init_function
        self.attributes= attributes
        self.internal_attributes = internal_attributes
        self.check_function= check_function
    def _get_key_members(self, const):
        ret=[]
        for a in self.attributes+self.internal_attributes:
          ret.extend(a.get_key_members(const))
        return "\n".join(ret)
    def _get_methods(self, const):
        ret=[]
        for a in self.attributes:
          ret.extend(a.get_methods(const))
        return "\n".join(ret)
    def _get_key_arguments(self, const):
        ret=[]
        for a in self.attributes+self.internal_attributes:
          ret.extend(a.get_key_arguments(const))
        return ",\n".join(ret)
    def _get_key_pass(self, const):
        ret=[]
        for a in self.attributes+self.internal_attributes:
          ret.extend(a.get_key_pass(const))
        return ",\n".join(ret)
    def _get_key_saves(self, const):
        ret=[]
        for a in self.attributes+self.internal_attributes:
          ret.extend(a.get_key_saves(const))
        return ",\n".join(["P(nh)"]+ret)
    def _get_type_check(self):
        cret=[]
        for t in self.allowed_types:
            cret.append("nh.get_type() == RMF::%s"%t)
        return "("+ "||".join(cret) +")"

    def _get_checks(self, const):
        ret=[self._get_type_check()]
        for a in self.attributes+self.internal_attributes:
          ret.extend(a.get_check(const))
        ret.extend(self.check_function)
        return "\n    && ".join(ret)
    def _get_construct(self, const):
        ret=[]
        # make handle missing later
        ret.append("Category cat = fh.get_category(\""\
                     +self.category+"\");")
        ret.append("RMF_UNUSED(cat);")
        for a in self.attributes+self.internal_attributes:
          ret.extend(a.get_construct(const))
        return "\n".join(ret)
    def _get_initialize(self, const):
        ret=[]
        for a in self.attributes+self.internal_attributes:
          ret.extend(a.get_initialize(const))
        if ret==[]:
            return "P()"
        else:
            return ", ".join(["P()"]+ret)
    def get(self):
        ret=[]
        classstr="""/** %(description)s

       See also %(name)s%(NOTCONST)s and %(name)s%(CONST)sFactory.
     */
    class %(name)s%(CONST)s:
        public Decorator<Node%(CONST)sHandle> {
    friend class %(name)s%(CONST)sFactory;
    std::string get_name() const {return \"%(name)s\";}
    private:
    typedef Decorator<Node%(CONST)sHandle> P;
    %(key_members)s
    %(name)s%(CONST)s(Node%(CONST)sHandle nh,
                  %(key_arguments)s):
       %(key_saves)s {
    %(init)s;
    }
    public:
    %(methods)s
    static std::string get_decorator_type_name() {
         return "%(name)s%(CONST)s";
    }
    };

    typedef std::vector<%(name)s%(CONST)s>
            %(name)s%(CONST)ss;
"""
        ret.append(classstr%{"description":self.description,
                             "name":self.name,
                             "key_members": self._get_key_members(True),
                             "methods": self._get_methods(True),
                             "key_arguments": self._get_key_arguments(True),
                             "key_saves": self._get_key_saves(True),
                             "CONST":"Const", "NOTCONST":"",
                             "init":""})
        ret.append(classstr%{"description":self.description,
                             "name":self.name,
                             "key_members": self._get_key_members(False),
                             "methods": self._get_methods(False),
                             "key_arguments": self._get_key_arguments(False),
                             "key_saves": self._get_key_saves(False),
                             "CONST":"", "NOTCONST":"Const",
                             "init":"\n".join(self.init_function)})

        factstr="""/** Create decorators of type %(name)s.

       See also %(name)s%(CONST)s and %(name)s%(NOTCONST)sFactory.
    */
    class %(name)s%(CONST)sFactory:
       public Factory<File%(CONST)sHandle>
                 {
    private:
    typedef Factory<File%(CONST)sHandle> P;
    %(key_members)s
    public:
    %(name)s%(CONST)sFactory(File%(CONST)sHandle fh):
      %(initialize)s{
    %(construct)s;
    }
    /** Get a %(name)s%(CONST)s for nh.*/
    %(name)s%(CONST)s get(Node%(CONST)sHandle nh) const {
      %(create_check)s;
      return %(name)s%(CONST)s(nh, %(key_pass)s);
    }
    /** Check whether nh has all the attributes required to be a
        %(name)s%(CONST)s.*/
    bool get_is(Node%(CONST)sHandle nh) const {
      return %(checks)s;
    }
    };

    typedef std::vector<%(name)s%(CONST)sFactory>
            %(name)s%(CONST)sFactories;
"""
        typecheck = """RMF_USAGE_CHECK(%s, std::string("Bad node type. Got \\\"")
                                      + boost::lexical_cast<std::string>(nh.get_type())
                                      + "\\\" in decorator type  %s");""" % (self._get_type_check(), self.name)
        ret.append(factstr%{"name":self.name,
                             "key_members": self._get_key_members(False),
                             "key_pass": self._get_key_pass(False),
                             "CONST":"", "NOTCONST":"Const",
                            "create_check":typecheck,
                            "construct": self._get_construct(False),
                            "initialize": self._get_initialize(False),
                            "checks":self._get_checks(False)});
        ret.append(factstr%{"name":self.name,
                             "key_members": self._get_key_members(True),
                             "key_pass": self._get_key_pass(True),
                             "create_check":typecheck,
                             "CONST":"Const", "NOTCONST":"",
                            "construct": self._get_construct(True),
                            "initialize": self._get_initialize(True),
                            "checks":self._get_checks(True)});
        return "\n".join(ret)


def make_header(name, infos, deps):
  fl = open(os.path.join("include", "RMF", name+"_decorators.h"), "w")
  print >> fl, """/**
 *  \\file RMF/%(name)s_decorators.h
 *  \\brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_%(NAME)s_DECORATORS_H
#define RMF_%(NAME)s_DECORATORS_H

#include <RMF/config.h>
#include <RMF/infrastructure_macros.h>
#include <RMF/NodeHandle.h>
#include <RMF/FileHandle.h>
#include <RMF/Decorator.h>
#include <RMF/Factory.h>
#include <RMF/constants.h>
#include <RMF/internal/utility.h>
#include <RMF/internal/paths.h>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>""" %{"name":name, "NAME":name.upper()}
  for d in deps:
    print >> fl, """#include "%s_decorators.h" """ %d
  print >> fl, """
RMF_ENABLE_WARNINGS
namespace RMF {
"""%{"name":name, "NAME":name.upper()}
  for i in infos:
    print >> fl, i.get()
  print >> fl, """} /* namespace RMF */
RMF_DISABLE_WARNINGS

#endif /* RMF_%(NAME)s_DECORATORS_H */"""%{"name":name, "NAME":name.upper()}
