#!/usr/bin/python
def get_string(type, name, const):
    return """fh.get_key<%(type)sTraits>(cat,
          \"%(name)s\")"""%{ "name":name,
                                 "type": type}

def gets_string(type, name, const):
   return """fh.get_keys<%(type)sTraits>(cat,
                    %(name)s)"""%{ "name":name,
                                  "type": type}
class Children:
    def __init__(self, nice_name):
        self.nice_name=nice_name
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
            ret.append("void set_"+self.nice_name+"(NodeConstHandles v) {")
            ret.append("  try{")
            ret.append("   for (unsigned int i=0; i< v.size(); ++i) {")
            ret.append("       internal::add_child_alias("+self.nice_name+"_, get_node(), v[i]);")
            ret.append("   }")
            ret.append("  } RMF_DECORATOR_CATCH( );")
            ret.append("}")
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
    def __init__(self, tt, nice_name, attribute_name):
        self.type=tt
        if tt.endswith("s"):
            self.plural_type=tt+"List"
        elif tt.endswith("x"):
            self.plural_type=tt+"es"
        else:
            self.plural_type=tt+"s"
        self.nice_name=nice_name
        self.attribute_name=attribute_name
    def get_key_members(self, const):
        return [self.type+"Key "+self.nice_name+"_;"]
    def get_methods(self, const):
        ret=[]
        ret.extend([self.type+" get_"+self.nice_name+"() const {",
                    "  try{",
                    "  return P::get_value("+self.nice_name+"_);",
                    "  } RMF_DECORATOR_CATCH( );",
                   "}"])
        ret.extend([self.plural_type+" get_all_"+self.nice_name+"s() const {",
                    "  try {",
                    "  return P::get_all_values("+self.nice_name+"_);",
                    "  } RMF_DECORATOR_CATCH( );",
                   "}"])
        if not const:
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
        ret.extend([nht+" get_"+self.nice_name+"() const {",
                    "  try {",
                    "  NodeID id;",
                    "   id= get_node().get_value("+self.nice_name+"_);",
                    "  return get_node().get_file().get_node_from_id(id);",
                    "  } RMF_DECORATOR_CATCH( );",
                    "}"])
        if not const:
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
        ret.extend(["String get_"+self.nice_name+"() const {",
                    " try {",
                    " String relpath;"
                    "   relpath= get_node().get_value("+self.nice_name+"_);",
                    "  String filepath=get_node().get_file().get_path();",
                    "  return internal::get_absolute_path(filepath, relpath);",
                    "  } RMF_DECORATOR_CATCH( );",
                    "}"])
        if not const:
            ret.extend(["void set_"+self.nice_name+"(String path) {",
                        " try {",
                        "  String filename= get_node().get_file().get_path();",
                        "  String relpath= internal::get_relative_path(filename, path);",
                        "    return get_node().set_value("+self.nice_name+"_, relpath);",
                        "  } RMF_DECORATOR_CATCH( );",
                        "}"])
        return ret

class SingletonRangeAttribute:
    def __init__(self, type, nice_name, attribute_name_begin, attribute_name_end):
        self.type=type
        self.nice_name=nice_name
        self.attribute_name_begin=attribute_name_begin
        self.attribute_name_end=attribute_name_end
    def get_key_members(self, const):
        return ["boost::array<"+self.type+"Key,2> "+self.nice_name+"_;"]
    def get_methods(self, const):
        ret=[]
        ret.extend([self.type+" get_"+self.nice_name+"() const {",
                    "  try {",
                    "  return get_node().get_value("+self.nice_name+"_[0]);",
                    "  } RMF_DECORATOR_CATCH( );",
                    "}"])
        if not const:
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
                 attribute_name_end):
        self.type=type
        self.nice_name=nice_name
        self.attribute_name_begin=attribute_name_begin
        self.attribute_name_end=attribute_name_end
    def get_key_members(self, const):
        return ["boost::array<"+self.type+"Key,2> "+self.nice_name+"_;"]
    def get_methods(self, const):
        ret=[]
        ret.extend([self.type+"Range get_"+self.nice_name+"() const {",
                    "  try {",
                    "  return std::make_pair(get_node().get_value("+self.nice_name+"_[0]),",
                    "                        get_node().get_value("+self.nice_name+"_[1]));",
                    "  } RMF_DECORATOR_CATCH( );",
            "}"])
        if not const:
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
    def __init__(self, type, ptype, nice_name, attribute_names):
        self.type=type
        self.nice_name=nice_name
        self.ptype=ptype
        self.attribute_names=attribute_names
    def get_key_members(self, const):
        return [self.type+"Keys "+self.nice_name+"_;"]
    def get_methods(self, const):
        ret=[]
        if not const:
            ret.append("""%(ptype)s get_%(name)s() const {
             return P::get_values(%(key)s);
           }"""%{"type":self.type,
            "ptype":self.ptype,
            "name":self.nice_name,
            "len":len(self.attribute_names),
            "key":self.nice_name+"_"})
            ret.append("""void set_%(name)s(const %(ptype)s &v) {
           P::set_values(%(key)s, v);
        }"""%{"type":self.type,
              "ptype":self.ptype,
              "name":self.nice_name,
              "len":len(self.attribute_names),
              "key":self.nice_name+"_"})
        else:
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

class DecoratorCategory:
    def __init__(self, category,
                 attributes, internal_attributes=[]):
        self.category=category
        self.attributes=attributes
        self.internal_attributes=internal_attributes

class Decorator:
    def __init__(self, name, description,
                 decorator_categories,
                 init_function=[],
                 check_function=[]):
        self.name=name
        self.description=description
        self.init_function=init_function
        self.categories= decorator_categories
        self.check_function= check_function
    def _get_key_members(self, const):
        ret=[]
        for cd in self.categories:
            for a in cd.attributes+cd.internal_attributes:
                ret.extend(a.get_key_members(const))
        return "\n".join(ret)
    def _get_methods(self, const):
        ret=[]
        for cd in self.categories:
            for a in cd.attributes:
                ret.extend(a.get_methods(const))
        return "\n".join(ret)
    def _get_key_arguments(self, const):
        ret=[]
        for cd in self.categories:
            for a in cd.attributes+cd.internal_attributes:
                ret.extend(a.get_key_arguments(const))
        return ",\n".join(ret)
    def _get_key_pass(self, const):
        ret=[]
        for cd in self.categories:
            for a in cd.attributes+cd.internal_attributes:
                ret.extend(a.get_key_pass(const))
        return ",\n".join(ret)
    def _get_key_saves(self, const):
        ret=[]
        for cd in self.categories:
            for a in cd.attributes+cd.internal_attributes:
                ret.extend(a.get_key_saves(const))
        return ",\n".join(["P(nh)"]+ret)
    def _get_checks(self, const):
        ret=[]
        for cd in self.categories:
            for a in cd.attributes+cd.internal_attributes:
                ret.extend(a.get_check(const))
        ret.extend(self.check_function)
        return "\n    && ".join(ret)
    def _get_construct(self, const):
        ret=[]
        for cd in self.categories:
            # make handle missing later
            lhs="{\n  Category cat="
            if const:
                rhs="fh.get_category(\""\
                    +cd.category+"\");"
            else:
                rhs="fh.get_category(\""+cd.category+"\");"
            ret.append(lhs+rhs)
            for a in cd.attributes+cd.internal_attributes:
                ret.extend(a.get_construct(const))
            ret.append("}")
        return "\n".join(ret)
    def _get_initialize(self, const):
        ret=[]
        for cd in self.categories:
            for a in cd.attributes+cd.internal_attributes:
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
    %(name)s%(CONST)s get(Node%(CONST)sHandle nh) const {
      %(create_check)s;
      return %(name)s%(CONST)s(nh, %(key_pass)s);
    }
    bool get_is(Node%(CONST)sHandle nh) const {
      return %(checks)s;
    }
    };

    typedef std::vector<%(name)s%(CONST)sFactory>
            %(name)s%(CONST)sFactories;
"""
        ret.append(factstr%{"name":self.name,
                             "key_members": self._get_key_members(False),
                             "key_pass": self._get_key_pass(False),
                             "CONST":"", "NOTCONST":"Const",
                            "create_check":"",
                            "construct": self._get_construct(False),
                            "initialize": self._get_initialize(False),
                            "checks":self._get_checks(False)});
        ret.append(factstr%{"name":self.name,
                             "key_members": self._get_key_members(True),
                             "key_pass": self._get_key_pass(True),
                             "create_check":"",
                             "CONST":"Const", "NOTCONST":"",
                            "construct": self._get_construct(True),
                            "initialize": self._get_initialize(True),
                            "checks":self._get_checks(True)});
        return "\n".join(ret)


colored= Decorator("Colored", "These particles has associated color information.",
                   [DecoratorCategory("shape", [Attributes("Float", "Floats",
                                                              "rgb_color", ["rgb color red",
                                                                            "rgb color green",
                                                                            "rgb color blue"])])],
                   "")
particle= Decorator("Particle", "These particles has associated coordinates and radius information.",
                   [DecoratorCategory("physics", [Attributes("Float", "Floats",
                                                                "coordinates", ["cartesian x",
                                                                                "cartesian y",
                                                                                "cartesian z"]),
                                                     Attribute("Float", "radius", "radius"),
                                                     Attribute("Float", "mass", "mass")])],
                   "")
iparticle= Decorator("IntermediateParticle", "These particles has associated coordinates and radius information.",
                   [DecoratorCategory("physics", [Attributes("Float", "Floats",
                                                                "coordinates", ["cartesian x",
                                                                                "cartesian y",
                                                                                "cartesian z"]),
                                                     Attribute("Float", "radius", "radius")])],
                   "")
pparticle= Decorator("RigidParticle", "These particles has associated coordinates and orientation information.",
                   [DecoratorCategory("physics", [Attributes("Float", "Floats",
                                                                "orientation", ["orientation r",
                                                                                "orientation i",
                                                                                "orientation j",
                                                                                "orientation k"]),
                                                     Attributes("Float", "Floats",
                                                                "coordinates", ["cartesian x",
                                                                                "cartesian y",
                                                                                "cartesian z"])])],
                   "")
refframe= Decorator("ReferenceFrame", "Define a reference frame to the child particles. The orientation is a quaternion.",
                   [DecoratorCategory("physics",
                                      [Attributes("Float", "Floats",
                                                  "orientation", ["reference frame orientation r",
                                                                  "reference frame orientation i",
                                                                  "reference frame orientation j",
                                                                  "reference frame orientation k"]),
                                        Attributes("Float", "Floats",
                                                   "coordinates", ["reference frame cartesian x",
                                                                   "reference frame cartesian y",
                                                                   "reference frame cartesian z"])])],
    "")

score= Decorator("Score", "Associate a score with some set of particles.",
                   [DecoratorCategory("feature", [Children("representation"),
                                                     Attribute("Float", "score", "score")])],
                   "")

ball= Decorator("Ball", "A geometric ball.",
                   [DecoratorCategory("shape", [PluralAttributes("Float", "Floats",
                                                              "coordinates", ["cartesian x",
                                                                              "cartesian y",
                                                                              "cartesian z"]),
                                                   Attribute("Float", "radius", "radius")],
                                      internal_attributes=[Attribute("Index", "type", "type")])],
                   ["nh.set_value(type_, 0);"], ["nh.get_value(type_)==0"])
cylinder= Decorator("Cylinder", "A geometric cylinder.",
                   [DecoratorCategory("shape", [PluralAttributes("Floats", "FloatsList",
                                                              "coordinates", ["cartesian xs",
                                                                              "cartesian ys",
                                                                              "cartesian zs"]),
                                                   Attribute("Float", "radius", "radius")],
                                      internal_attributes=[Attribute("Index", "type", "type")])],
                   ["nh.set_value(type_, 1);"], ["nh.get_value(type_)==1"])

segment= Decorator("Segment", "A geometric line setgment.",
                   [DecoratorCategory("shape", [PluralAttributes("Floats", "FloatsList",
                                                              "coordinates", ["cartesian xs",
                                                                              "cartesian ys",
                                                                              "cartesian zs"])],
                                      internal_attributes=[Attribute("Index", "type", "type")])],
                    ["nh.set_value(type_, 1);"], ["nh.get_value(type_)==1"])

journal= Decorator("JournalArticle", "Information regarding a publication.",
                   [DecoratorCategory("publication", [Attribute("String", "title", "title"),
                                                         Attribute("String", "journal", "journal"),
                                                         Attribute("String", "pubmed_id", "pubmed id"),
                                                         Attribute("Int", "year", "year"),
                                                         Attribute("Strings", "authors", "authors"),])],
                   "")

atom= Decorator("Atom", "Information regarding an atom.",
                   [DecoratorCategory("physics", [Attributes("Float", "Floats",
                                                                "coordinates", ["cartesian x",
                                                                                "cartesian y",
                                                                                "cartesian z"]),
                                                     Attribute("Float", "radius", "radius"),
                                                     Attribute("Float", "mass", "mass"),
                                                     Attribute("Index", "element", "element")])],
                   "")


residue= Decorator("Residue", "Information regarding a residue.",
                   [DecoratorCategory("sequence", [SingletonRangeAttribute("Int", "index", "first residue index", "last residue index"),
                                                         Attribute("String", "type", "residue type")])],
                   "")

chain= Decorator("Chain", "Information regarding a chain.",
                   [DecoratorCategory("sequence", [Attribute("Index", "chain_id", "chain id")])],
                   "")

fragment= Decorator("Domain", "Information regarding a fragment of a molecule.",
                   [DecoratorCategory("sequence", [RangeAttribute("Int", "indexes", "first residue index", "last residue index")])],
                   "")

copy= Decorator("Copy", "Information regarding a copy of a molecule.",
                   [DecoratorCategory("sequence", [Attribute("Index", "copy_index", "copy index")])],
                   "")
diffuser= Decorator("Diffuser", "Information regarding diffusion coefficients.",
                   [DecoratorCategory("physics", [Attribute("Float", "diffusion_coefficient", "diffusion coefficient")])],
                   "")
typed= Decorator("Typed", "A numeric tag for keeping track of types of molecules.",
                   [DecoratorCategory("sequence", [Attribute("String", "type_name", "type name")])],
                   "")

salias= Decorator("Alias", "Store a reference to another node as an alias.",
                  [DecoratorCategory("alias", [NodeAttribute("NodeID", "aliased", "aliased")])],
                   "")
external= Decorator("External", "A reference to something in an external file.",
                   [DecoratorCategory("external", [PathAttribute("String", "path", "path")])],
                   "")


print """/**
 *  \\file RMF/decorators.h
 *  \\brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_DECORATORS_H
#define RMF_DECORATORS_H

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

#define RMF_DECORATOR_CATCH(extra_info) \\
catch (Exception &e) {\\
  RMF_RETHROW(Decorator(get_name()) extra_info, e);\\
}


namespace RMF {
"""
print colored.get()
print particle.get()
print iparticle.get()
print pparticle.get()
print ball.get()
print cylinder.get()
print segment.get()
print journal.get()
print residue.get()
print atom.get()
print chain.get()
print fragment.get()
print refframe.get()
print copy.get()
print diffuser.get()
print typed.get()
print salias.get()
print score.get()
print external.get()
print """} /* namespace RMF */

#endif /* RMF_DECORATORS_H */"""
