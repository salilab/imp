/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ctype.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <iostream>
#include <fstream>
#include <map>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include "Compiler.hh"
#include "ValidSchema.hh"
#include "NodeImpl.hh"

using std::ostream;
using std::ifstream;
using std::ofstream;
using std::map;
using std::set;
using std::string;
using std::vector;
using internal_avro::NodePtr;
using internal_avro::resolveSymbol;

using boost::lexical_cast;

using internal_avro::ValidSchema;
using internal_avro::compileJsonSchema;

struct PendingSetterGetter {
  string structName;
  string type;
  string name;
  size_t idx;

  PendingSetterGetter(const string& sn, const string& t, const string& n,
                      size_t i)
      : structName(sn), type(t), name(n), idx(i) {}
};

struct PendingConstructor {
  string structName;
  string memberName;
  bool initMember;
  PendingConstructor(const string& sn, const string& n, bool im)
      : structName(sn), memberName(n), initMember(im) {}
};

class CodeGen {
  size_t unionNumber_;
  std::ostream& os_;
  bool inNamespace_;
  const std::string ns_;
  const std::string schemaFile_;
  const std::string headerFile_;
  const std::string includePrefix_;
  const bool noUnion_;
  const std::string guardString_;
  boost::mt19937 random_;

  vector<PendingSetterGetter> pendingGettersAndSetters;
  vector<PendingConstructor> pendingConstructors;

  map<NodePtr, string> done;
  set<NodePtr> doing;

  std::string guard();
  std::string fullname(const string& name) const;
  std::string generateEnumType(const NodePtr& n);
  std::string cppTypeOf(const NodePtr& n);
  std::string generateRecordType(const NodePtr& n);
  std::string unionName();
  std::string generateUnionType(const NodePtr& n);
  std::string generateType(const NodePtr& n);
  std::string generateDeclaration(const NodePtr& n);
  std::string doGenerateType(const NodePtr& n);
  void generateEnumTraits(const NodePtr& n);
  void generateTraits(const NodePtr& n);
  void generateRecordTraits(const NodePtr& n);
  void generateUnionTraits(const NodePtr& n);
  void emitCopyright();

 public:
  CodeGen(std::ostream& os, const std::string& ns,
          const std::string& schemaFile, const std::string& headerFile,
          const std::string& guardString, const std::string& includePrefix,
          bool noUnion)
      : unionNumber_(0),
        os_(os),
        inNamespace_(false),
        ns_(ns),
        schemaFile_(schemaFile),
        headerFile_(headerFile),
        includePrefix_(includePrefix),
        noUnion_(noUnion),
        guardString_(guardString),
        random_(static_cast<uint32_t>(::time(0))) {}
  void generate(const ValidSchema& schema);
};

static string decorate(const internal_avro::Name& name) {
  return name.simpleName();
}

string CodeGen::fullname(const string& name) const {
  return ns_.empty() ? name : (ns_ + "::" + name);
}

string CodeGen::generateEnumType(const NodePtr& n) {
  string s = decorate(n->name());
  os_ << "enum " << s << " {\n";
  size_t c = n->names();
  for (size_t i = 0; i < c; ++i) {
    os_ << "    " << n->nameAt(i) << ",\n";
  }
  os_ << "};\n\n";
  return s;
}

string CodeGen::cppTypeOf(const NodePtr& n) {
  switch (n->type()) {
    case internal_avro::AVRO_STRING:
      return "std::string";
    case internal_avro::AVRO_BYTES:
      return "std::vector<uint8_t>";
    case internal_avro::AVRO_INT:
      return "int32_t";
    case internal_avro::AVRO_LONG:
      return "int64_t";
    case internal_avro::AVRO_FLOAT:
      return "float";
    case internal_avro::AVRO_DOUBLE:
      return "double";
    case internal_avro::AVRO_BOOL:
      return "bool";
    case internal_avro::AVRO_RECORD:
    case internal_avro::AVRO_ENUM: {
      string nm = decorate(n->name());
      return inNamespace_ ? nm : fullname(nm);
    }
    case internal_avro::AVRO_ARRAY:
      return "std::vector<" + cppTypeOf(n->leafAt(0)) + " >";
    case internal_avro::AVRO_MAP:
      return "std::map<std::string, " + cppTypeOf(n->leafAt(1)) + " >";
    case internal_avro::AVRO_FIXED:
      return "std::array<uint8_t, " + lexical_cast<string>(n->fixedSize()) +
             ">";
    case internal_avro::AVRO_SYMBOLIC:
      return cppTypeOf(resolveSymbol(n));
    case internal_avro::AVRO_UNION:
      return fullname(done[n]);
    default:
      return "$Undefined$";
  }
}

static string cppNameOf(const NodePtr& n) {
  switch (n->type()) {
    case internal_avro::AVRO_NULL:
      return "null";
    case internal_avro::AVRO_STRING:
      return "string";
    case internal_avro::AVRO_BYTES:
      return "bytes";
    case internal_avro::AVRO_INT:
      return "int";
    case internal_avro::AVRO_LONG:
      return "long";
    case internal_avro::AVRO_FLOAT:
      return "float";
    case internal_avro::AVRO_DOUBLE:
      return "double";
    case internal_avro::AVRO_BOOL:
      return "bool";
    case internal_avro::AVRO_RECORD:
    case internal_avro::AVRO_ENUM:
    case internal_avro::AVRO_FIXED:
      return decorate(n->name());
    case internal_avro::AVRO_ARRAY:
      return "array";
    case internal_avro::AVRO_MAP:
      return "map";
    case internal_avro::AVRO_SYMBOLIC:
      return cppNameOf(resolveSymbol(n));
    default:
      return "$Undefined$";
  }
}

string CodeGen::generateRecordType(const NodePtr& n) {
  size_t c = n->leaves();
  vector<string> types;
  for (size_t i = 0; i < c; ++i) {
    types.push_back(generateType(n->leafAt(i)));
  }

  map<NodePtr, string>::const_iterator it = done.find(n);
  if (it != done.end()) {
    return it->second;
  }

  os_ << "struct " << decorate(n->name()) << " {\n";
  if (!noUnion_) {
    for (size_t i = 0; i < c; ++i) {
      if (n->leafAt(i)->type() == internal_avro::AVRO_UNION) {
        os_ << "    typedef " << types[i] << ' ' << n->nameAt(i) << "_t;\n";
      }
    }
  }
  for (size_t i = 0; i < c; ++i) {
    if (!noUnion_ && n->leafAt(i)->type() == internal_avro::AVRO_UNION) {
      os_ << "    " << n->nameAt(i) << "_t";
    } else {
      os_ << "    " << types[i];
    }
    os_ << ' ' << n->nameAt(i) << ";\n";
  }
  os_ << "};\n\n";
  return decorate(n->name());
}

void makeCanonical(string& s, bool foldCase) {
  for (string::iterator it = s.begin(); it != s.end(); ++it) {
    if (isalpha(*it)) {
      if (foldCase) {
        *it = toupper(*it);
      }
    } else if (!isdigit(*it)) {
      *it = '_';
    }
  }
}

string CodeGen::unionName() {
  string s = schemaFile_;
  string::size_type n = s.find_last_of("/\\");
  if (n != string::npos) {
    s = s.substr(n);
  }
  makeCanonical(s, false);

  return s + "_Union__" + boost::lexical_cast<string>(unionNumber_++) + "__";
}

static void generateGetterAndSetter(ostream& os, const string& structName,
                                    const string& type, const string& name,
                                    size_t idx) {
  string sn = " " + structName + "::";

  os << "inline\n";

  os << type << sn << "get_" << name << "() const {\n"
     << "    if (idx_ != " << idx << ") {\n"
     << "        throw internal_avro::Exception(\"Invalid type for "
     << "union\");\n"
     << "    }\n"
     << "    return boost::any_cast<" << type << " >(value_);\n"
     << "}\n\n";

  os << "inline\n"
     << "void" << sn << "set_" << name << "(const " << type << "& v) {\n"
     << "    idx_ = " << idx << ";\n"
     << "    value_ = v;\n"
     << "}\n\n";
}

static void generateConstructor(ostream& os, const string& structName,
                                bool initMember, const string& type) {
  os << "inline " << structName << "::" << structName << "() : idx_(0)";
  if (initMember) {
    os << ", value_(" << type << "())";
  }
  os << " { }\n";
}

/**
 * Generates a type for union and emits the code.
 * Since unions can encounter names that are not fully defined yet,
 * such names must be declared and the inline functions deferred until all
 * types are fully defined.
 */
string CodeGen::generateUnionType(const NodePtr& n) {
  size_t c = n->leaves();
  vector<string> types;
  vector<string> names;

  set<NodePtr>::const_iterator it = doing.find(n);
  if (it != doing.end()) {
    for (size_t i = 0; i < c; ++i) {
      const NodePtr& nn = n->leafAt(i);
      types.push_back(generateDeclaration(nn));
      names.push_back(cppNameOf(nn));
    }
  } else {
    doing.insert(n);
    for (size_t i = 0; i < c; ++i) {
      const NodePtr& nn = n->leafAt(i);
      types.push_back(generateType(nn));
      names.push_back(cppNameOf(nn));
    }
    doing.erase(n);
  }
  if (done.find(n) != done.end()) {
    return done[n];
  }

  const string result = unionName();

  os_ << "struct " << result << " {\n"
      << "private:\n"
      << "    size_t idx_;\n"
      << "    boost::any value_;\n"
      << "public:\n"
      << "    size_t idx() const { return idx_; }\n";

  for (size_t i = 0; i < c; ++i) {
    const NodePtr& nn = n->leafAt(i);
    if (nn->type() == internal_avro::AVRO_NULL) {
      os_ << "    bool is_null() const {\n"
          << "        return (idx_ == " << i << ");\n"
          << "    }\n"
          << "    void set_null() {\n"
          << "        idx_ = " << i << ";\n"
          << "        value_ = boost::any();\n"
          << "    }\n";
    } else {
      const string& type = types[i];
      const string& name = names[i];
      os_ << "    " << type << " get_" << name << "() const;\n"
                                                  "    void set_" << name
          << "(const " << type << "& v);\n";
      pendingGettersAndSetters.push_back(
          PendingSetterGetter(result, type, name, i));
    }
  }

  os_ << "    " << result << "();\n";
  pendingConstructors.push_back(PendingConstructor(
      result, types[0], n->leafAt(0)->type() != internal_avro::AVRO_NULL));
  os_ << "};\n\n";

  return result;
}

/**
 * Returns the type for the given schema node and emits code to os.
 */
string CodeGen::generateType(const NodePtr& n) {
  NodePtr nn =
      (n->type() == internal_avro::AVRO_SYMBOLIC) ? resolveSymbol(n) : n;

  map<NodePtr, string>::const_iterator it = done.find(nn);
  if (it != done.end()) {
    return it->second;
  }
  string result = doGenerateType(nn);
  done[nn] = result;
  return result;
}

string CodeGen::doGenerateType(const NodePtr& n) {
  switch (n->type()) {
    case internal_avro::AVRO_STRING:
    case internal_avro::AVRO_BYTES:
    case internal_avro::AVRO_INT:
    case internal_avro::AVRO_LONG:
    case internal_avro::AVRO_FLOAT:
    case internal_avro::AVRO_DOUBLE:
    case internal_avro::AVRO_BOOL:
    case internal_avro::AVRO_NULL:
    case internal_avro::AVRO_FIXED:
      return cppTypeOf(n);
    case internal_avro::AVRO_ARRAY:
      return "std::vector<" + generateType(n->leafAt(0)) + " >";
    case internal_avro::AVRO_MAP:
      return "std::map<std::string, " + generateType(n->leafAt(1)) + " >";
    case internal_avro::AVRO_RECORD:
      return generateRecordType(n);
    case internal_avro::AVRO_ENUM:
      return generateEnumType(n);
    case internal_avro::AVRO_UNION:
      return generateUnionType(n);
    default:
      break;
  }
  return "$Undefuned$";
}

string CodeGen::generateDeclaration(const NodePtr& n) {
  NodePtr nn =
      (n->type() == internal_avro::AVRO_SYMBOLIC) ? resolveSymbol(n) : n;
  switch (nn->type()) {
    case internal_avro::AVRO_STRING:
    case internal_avro::AVRO_BYTES:
    case internal_avro::AVRO_INT:
    case internal_avro::AVRO_LONG:
    case internal_avro::AVRO_FLOAT:
    case internal_avro::AVRO_DOUBLE:
    case internal_avro::AVRO_BOOL:
    case internal_avro::AVRO_NULL:
    case internal_avro::AVRO_FIXED:
      return cppTypeOf(nn);
    case internal_avro::AVRO_ARRAY:
      return "std::vector<" + generateDeclaration(nn->leafAt(0)) + " >";
    case internal_avro::AVRO_MAP:
      return "std::map<std::string, " + generateDeclaration(nn->leafAt(1)) +
             " >";
    case internal_avro::AVRO_RECORD:
      os_ << "struct " << cppTypeOf(nn) << ";\n";
      return cppTypeOf(nn);
    case internal_avro::AVRO_ENUM:
      return generateEnumType(nn);
    case internal_avro::AVRO_UNION:
      // FIXME: When can this happen?
      return generateUnionType(nn);
    default:
      break;
  }
  return "$Undefuned$";
}

void CodeGen::generateEnumTraits(const NodePtr& n) {
  string fn = fullname(decorate(n->name()));
  os_ << "template<> struct codec_traits<" << fn << "> {\n"
      << "    static void encode(Encoder& e, " << fn << " v) {\n"
      << "        e.encodeEnum(v);\n"
      << "    }\n"
      << "    static void decode(Decoder& d, " << fn << "& v) {\n"
      << "        v = static_cast<" << fn << ">(d.decodeEnum());\n"
      << "    }\n"
      << "};\n\n";
}

void CodeGen::generateRecordTraits(const NodePtr& n) {
  size_t c = n->leaves();
  for (size_t i = 0; i < c; ++i) {
    generateTraits(n->leafAt(i));
  }

  string fn = fullname(decorate(n->name()));
  os_ << "template<> struct codec_traits<" << fn << "> {\n"
      << "    static void encode(Encoder& e, const " << fn << "& v) {\n";

  for (size_t i = 0; i < c; ++i) {
    os_ << "        internal_avro::encode(e, v." << n->nameAt(i) << ");\n";
  }

  os_ << "    }\n"
      << "    static void decode(Decoder& d, " << fn << "& v) {\n";

  for (size_t i = 0; i < c; ++i) {
    os_ << "        internal_avro::decode(d, v." << n->nameAt(i) << ");\n";
  }

  os_ << "    }\n"
      << "};\n\n";
}

void CodeGen::generateUnionTraits(const NodePtr& n) {
  size_t c = n->leaves();

  for (size_t i = 0; i < c; ++i) {
    const NodePtr& nn = n->leafAt(i);
    generateTraits(nn);
  }

  string name = done[n];
  string fn = fullname(name);

  os_ << "template<> struct codec_traits<" << fn << "> {\n"
      << "    static void encode(Encoder& e, " << fn << " v) {\n"
      << "        e.encodeUnionIndex(v.idx());\n"
      << "        switch (v.idx()) {\n";

  for (size_t i = 0; i < c; ++i) {
    const NodePtr& nn = n->leafAt(i);
    os_ << "        case " << i << ":\n";
    if (nn->type() == internal_avro::AVRO_NULL) {
      os_ << "            e.encodeNull();\n";
    } else {
      os_ << "            internal_avro::encode(e, v.get_" << cppNameOf(nn)
          << "());\n";
    }
    os_ << "            break;\n";
  }

  os_ << "        }\n"
      << "    }\n"
      << "    static void decode(Decoder& d, " << fn << "& v) {\n"
      << "        size_t n = d.decodeUnionIndex();\n"
      << "        if (n >= " << c << ") { throw internal_avro::Exception(\""
                                     "Union index too big\"); }\n"
      << "        switch (n) {\n";

  for (size_t i = 0; i < c; ++i) {
    const NodePtr& nn = n->leafAt(i);
    os_ << "        case " << i << ":\n";
    if (nn->type() == internal_avro::AVRO_NULL) {
      os_ << "            d.decodeNull();\n"
          << "            v.set_null();\n";
    } else {
      os_ << "            {\n"
          << "                " << cppTypeOf(nn) << " vv;\n"
          << "                internal_avro::decode(d, vv);\n"
          << "                v.set_" << cppNameOf(nn) << "(vv);\n"
          << "            }\n";
    }
    os_ << "            break;\n";
  }
  os_ << "        }\n"
      << "    }\n"
      << "};\n\n";
}

void CodeGen::generateTraits(const NodePtr& n) {
  switch (n->type()) {
    case internal_avro::AVRO_STRING:
    case internal_avro::AVRO_BYTES:
    case internal_avro::AVRO_INT:
    case internal_avro::AVRO_LONG:
    case internal_avro::AVRO_FLOAT:
    case internal_avro::AVRO_DOUBLE:
    case internal_avro::AVRO_BOOL:
    case internal_avro::AVRO_NULL:
      break;
    case internal_avro::AVRO_RECORD:
      generateRecordTraits(n);
      break;
    case internal_avro::AVRO_ENUM:
      generateEnumTraits(n);
      break;
    case internal_avro::AVRO_ARRAY:
    case internal_avro::AVRO_MAP:
      generateTraits(n->leafAt(n->type() == internal_avro::AVRO_ARRAY ? 0 : 1));
      break;
    case internal_avro::AVRO_UNION:
      generateUnionTraits(n);
      break;
    case internal_avro::AVRO_FIXED:
      break;
    default:
      break;
  }
}

void CodeGen::emitCopyright() {
  os_ << "/**\n"
         " * Licensed to the Apache Software Foundation (ASF) under one\n"
         " * or more contributor license agreements.  See the NOTICE file\n"
         " * distributed with this work for additional information\n"
         " * regarding copyright ownership.  The ASF licenses this file\n"
         " * to you under the Apache License, Version 2.0 (the\n"
         " * \"License\"); you may not use this file except in compliance\n"
         " * with the License.  You may obtain a copy of the License at\n"
         " *\n"
         " *     http://www.apache.org/licenses/LICENSE-2.0\n"
         " *\n"
         " * Unless required by applicable law or agreed to in writing, "
         "software\n"
         " * distributed under the License is distributed on an "
         "\"AS IS\" BASIS,\n"
         " * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express "
         "or implied.\n"
         " * See the License for the specific language governing "
         "permissions and\n"
         " * limitations under the License.\n"
         " */\n\n\n";
}

string CodeGen::guard() {
  string h = headerFile_;
  makeCanonical(h, true);
  return h + "_" + lexical_cast<string>(random_()) + "__H_";
}

void CodeGen::generate(const ValidSchema& schema) {
  emitCopyright();

  string h = guardString_.empty() ? guard() : guardString_;

  os_ << "#ifndef " << h << "\n";
  os_ << "#define " << h << "\n\n\n";

  os_ << "#include \"boost/any.hpp\"\n"
      << "#include \"" << includePrefix_ << "Specific.hh\"\n"
      << "#include \"" << includePrefix_ << "Encoder.hh\"\n"
      << "#include \"" << includePrefix_ << "Decoder.hh\"\n"
      << "\n";

  if (!ns_.empty()) {
    os_ << "namespace " << ns_ << " {\n";
    inNamespace_ = true;
  }

  const NodePtr& root = schema.root();
  generateType(root);

  for (vector<PendingSetterGetter>::const_iterator it =
           pendingGettersAndSetters.begin();
       it != pendingGettersAndSetters.end(); ++it) {
    generateGetterAndSetter(os_, it->structName, it->type, it->name, it->idx);
  }

  for (vector<PendingConstructor>::const_iterator it =
           pendingConstructors.begin();
       it != pendingConstructors.end(); ++it) {
    generateConstructor(os_, it->structName, it->initMember, it->memberName);
  }

  if (!ns_.empty()) {
    inNamespace_ = false;
    os_ << "}\n";
  }

  os_ << "namespace internal_avro {\n";

  unionNumber_ = 0;

  generateTraits(root);

  os_ << "}\n";

  os_ << "#endif\n";
  os_.flush();
}

namespace po = boost::program_options;

static const string NS("namespace");
static const string OUT("output");
static const string IN("input");
static const string INCLUDE_PREFIX("include-prefix");
static const string NO_UNION_TYPEDEF("no-union-typedef");

static string readGuard(const string& filename) {
  std::ifstream ifs(filename.c_str());
  string buf;
  string candidate;
  while (std::getline(ifs, buf)) {
    boost::algorithm::trim(buf);
    if (candidate.empty()) {
      if (boost::algorithm::starts_with(buf, "#ifndef ")) {
        candidate = buf.substr(8);
      }
    } else if (boost::algorithm::starts_with(buf, "#define ")) {
      if (candidate == buf.substr(8)) {
        break;
      }
    } else {
      candidate.erase();
    }
  }
  return candidate;
}

int main(int argc, char** argv) {
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "produce help message")(
      "include-prefix,p", po::value<string>()->default_value("internal_avro"),
      "prefix for include headers, - for none, default: internal_avro")(
      "no-union-typedef,U", "do not generate typedefs for unions in records")(
      "namespace,n", po::value<string>(), "set namespace for generated code")(
      "input,i", po::value<string>(), "input file")(
      "output,o", po::value<string>(), "output file to generate");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help") || vm.count(IN) == 0 || vm.count(OUT) == 0) {
    std::cout << desc << std::endl;
    return 1;
  }

  string ns = vm.count(NS) > 0 ? vm[NS].as<string>() : string();
  string outf = vm.count(OUT) > 0 ? vm[OUT].as<string>() : string();
  string inf = vm.count(IN) > 0 ? vm[IN].as<string>() : string();
  string incPrefix = vm[INCLUDE_PREFIX].as<string>();
  bool noUnion = vm.count(NO_UNION_TYPEDEF) != 0;
  if (incPrefix == "-") {
    incPrefix.clear();
  } else if (*incPrefix.rbegin() != '/') {
    incPrefix += "/";
  }

  try {
    ValidSchema schema;

    if (!inf.empty()) {
      ifstream in(inf.c_str());
      compileJsonSchema(in, schema);
    } else {
      compileJsonSchema(std::cin, schema);
    }

    if (!outf.empty()) {
      string g = readGuard(outf);
      ofstream out(outf.c_str());
      CodeGen(out, ns, inf, outf, g, incPrefix, noUnion).generate(schema);
    } else {
      CodeGen(std::cout, ns, inf, outf, "", incPrefix, noUnion)
          .generate(schema);
    }
    return 0;
  }
  catch (std::exception& e) {
    std::cerr << "Failed to parse or compile schema: " << e.what() << std::endl;
    return 1;
  }
}
