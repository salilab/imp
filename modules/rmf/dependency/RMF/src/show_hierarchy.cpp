/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/show_hierarchy.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/Nullable.h"
#include "RMF/compiler_macros.h"
#include "RMF/decorator/feature.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/sequence.h"
#include "RMF/decorator/bond.h"
#include "RMF/decorator/representation.h"
#include "RMF/decorator/alternatives.h"
#include "RMF/decorator/shape.h"
#include "RMF/decorator/reference.h"
#include "RMF/decorator/provenance.h"
#include "RMF/decorator/uncertainty.h"
#include "RMF/enums.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/types.h"

RMF_ENABLE_WARNINGS

namespace RMF {

namespace {

template <class TypeT>
std::vector<ID<TypeT> > get_keys(FileConstHandle f) {
  Categories kcs = f.get_categories();
  std::vector<ID<TypeT> > ret;
  for (unsigned int i = 0; i < kcs.size(); ++i) {
    std::vector<ID<TypeT> > curp = f.get_keys<TypeT>(kcs[i]);
    ret.insert(ret.end(), curp.begin(), curp.end());
  }
  return ret;
}

template <class Traits>
void show_data(NodeConstHandle n, std::ostream& out,
               const std::vector<ID<Traits> >& ks, std::string prefix) {
  using std::operator<<;
  FileConstHandle f = n.get_file();
  for(ID<Traits> k : ks) {
    if (n.get_file().get_current_frame() != FrameID() &&
        !n.get_frame_value(k).get_is_null()) {
      out << std::endl << prefix << n.get_file().get_name(k) << ": "
          << Showable(n.get_frame_value(k));
    } else {
      Nullable<typename Traits::Type> ts = n.get_static_value(k);
      if (!ts.get_is_null()) {
        out << std::endl << prefix << f.get_name(k) << " ("
            << f.get_name(f.get_category(k)) << ")"
            << ": " << Showable(ts.get()) << "(s)";
      }
    }
  }
}

template <class Show>
void print_tree(std::ostream& out, RMF::NodeConstHandle start, Show show) {
  decorator::AlternativesFactory altcf(start.get_file());
  typedef boost::tuple<std::string, std::string, std::string, NodeConstHandle>
      QI;
  std::vector<QI> queue;
  queue.push_back(QI(std::string(), std::string(), std::string(), start));
  do {
    const QI& back = queue.back();
    NodeConstHandle n = back.get<3>();
    std::string prefix0 = back.get<0>();
    std::string prefix1 = back.get<1>();
    std::string node_suffix = back.get<2>();
    queue.pop_back();
    if (altcf.get_is(n)) {
      decorator::AlternativesConst ad = altcf.get(n);
      {
        NodeConstHandles alts = ad.get_alternatives(GAUSSIAN_PARTICLE);
        std::reverse(alts.begin(), alts.end());
        for(NodeConstHandle cur : alts) {
          RMF_INTERNAL_CHECK(cur != n, "Node can't be a gaussian one");
          std::ostringstream oss;
          oss << "[G" << decorator::get_resolution(cur) << "]";
          queue.push_back(QI(prefix0, prefix1, oss.str(), cur));
        }
      }
      {
        NodeConstHandles alts = ad.get_alternatives(PARTICLE);
        RMF_INTERNAL_CHECK(alts.front() == n,
                           "The node itself is not in front");
        std::reverse(alts.begin(), alts.end());
        for(NodeConstHandle cur : alts) {
          if (cur == n) continue;
          std::ostringstream oss;
          oss << "[" << decorator::get_resolution(cur) << "]";
          queue.push_back(QI(prefix0, prefix1, oss.str(), cur));
        }
        std::ostringstream oss;
        oss << "[" << decorator::get_resolution(n) << "]";
        node_suffix = oss.str();
      }
    }
    out << prefix0;
    NodeConstHandles children = n.get_children();
    if (children.size() > 0)
      out << " + ";
    else
      out << " - ";
    show(n, prefix1, node_suffix, out);
    out << std::endl;
    for (int i = static_cast<int>(children.size()) - 1; i >= 0; --i) {
      queue.push_back(
          QI(prefix1 + "  ", prefix1 + "  ", node_suffix, children[i]));
    }
  } while (!queue.empty());
}

struct LessName {
  FileConstHandle fh_;
  LessName(FileConstHandle fh) : fh_(fh) {}
  template <class Traits>
  bool operator()(ID<Traits> a, ID<Traits> b) const {
    return fh_.get_name(a) < fh_.get_name(b);
  }
};

void show_frames_impl(FileConstHandle fh, FrameID root, std::string prefix,
                      std::ostream& out) {
  FrameIDs ch = fh.get_children(root);
  out << prefix;
  if (ch.empty()) {
    out << " - ";
  } else {
    out << " + ";
  }
  out << fh.get_name(root) << " [" << fh.get_type(root) << "]" << std::endl;
  for(FrameID id : ch) { show_frames_impl(fh, id, prefix + "   ", out); }
}

void simple_show_node(NodeConstHandle n, std::string /*prefix*/,
                      std::string node_suffix, std::ostream& out) {
  using std::operator<<;
  out << "\"" << n.get_name() << "\"" << node_suffix << " [" << n.get_type()
      << "]";
}
void show_node(NodeConstHandle n, std::string node_suffix, std::ostream& out,
               FloatKeys fks, FloatsKeys fsks, IntKeys iks, IntsKeys isks,
               StringKeys sks, StringsKeys ssks, Vector3Keys v3ks,
               Vector4Keys v4ks, Vector3sKeys v3sks, std::string prefix) {
  using std::operator<<;
  if (true) {
    simple_show_node(n, "", node_suffix, out);
    show_data(n, out, fks, prefix + "  ");
    show_data(n, out, iks, prefix + "  ");
    show_data(n, out, sks, prefix + "  ");
    show_data(n, out, fsks, prefix + "  ");
    show_data(n, out, isks, prefix + "  ");
    show_data(n, out, ssks, prefix + "  ");
    show_data(n, out, v3ks, prefix + "  ");
    show_data(n, out, v4ks, prefix + "  ");
    show_data(n, out, v3sks, prefix + "  ");
  }
}

void show_node_decorators(
    NodeConstHandle n, std::string node_suffix, std::ostream& out,
    decorator::BondFactory bdcf, decorator::ColoredFactory ccf,
    decorator::ParticleFactory pcf, decorator::IntermediateParticleFactory ipcf,
    decorator::RigidParticleFactory rpcf, decorator::ScoreFactory scf,
    decorator::RepresentationFactory repcf, decorator::BallFactory bcf,
    decorator::CylinderFactory cycf, decorator::SegmentFactory segcf,
    decorator::ResidueFactory rcf, decorator::AtomFactory acf,
    decorator::ChainFactory chaincf, decorator::DomainFactory fragcf,
    decorator::CopyFactory copycf, decorator::DiffuserFactory diffusercf,
    decorator::TypedFactory typedcf, decorator::ReferenceFactory refcf,
    decorator::ScaleFactory scalecf,
    decorator::StructureProvenanceFactory strucpcf,
    decorator::SampleProvenanceFactory samppcf,
    decorator::CombineProvenanceFactory combpcf,
    decorator::FilterProvenanceFactory filtpcf,
    decorator::ClusterProvenanceFactory clustpcf,
    decorator::ScriptProvenanceFactory scriptpcf,
    decorator::SoftwareProvenanceFactory softpcf,
    std::string) {
  using std::operator<<;
  out << "\"" << n.get_name() << "\"" << node_suffix << " [" << n.get_type()
      << ":";
  if (bdcf.get_is_static(n)) out << " bond(s)";
  else if (bdcf.get_is(n)) out << " bond";
  if (ccf.get_is_static(n)) out << " color(s)";
  else if (ccf.get_is(n)) out << " color";
  if (pcf.get_is_static(n))
    out << " particle(s)";
  else if (ipcf.get_is_static(n))
    out << " iparticle(s)";
  else if (pcf.get_is(n))
    out << " particle";
  else if (ipcf.get_is(n))
    out << " iparticle";
  if (rpcf.get_is_static(n)) out << " rigid(s)";
  else if (rpcf.get_is(n)) out << " rigid";
  if (scf.get_is_static(n)) out << " score(s)";
  else if (scf.get_is(n)) out << " score";
  if (repcf.get_is_static(n)) out << " representation(s)";
  else if (repcf.get_is(n)) out << " representation";
  if (bcf.get_is_static(n)) out << " ball(s)";
  else if (bcf.get_is(n)) out << " ball";
  if (cycf.get_is_static(n)) out << " cylinder(s)";
  else if (cycf.get_is(n)) out << " cylinder";
  if (segcf.get_is_static(n)) out << " segment(s)";
  else if (segcf.get_is(n)) out << " segment";
  if (rcf.get_is_static(n)) out << " residue(s)";
  else if (rcf.get_is(n)) out << " residue";
  if (acf.get_is_static(n)) out << " atom(s)";
  else if (acf.get_is(n)) out << " atom";
  if (chaincf.get_is_static(n)) out << " chain(s)";
  else if (chaincf.get_is(n)) out << " chain";
  if (fragcf.get_is_static(n)) out << " domain(s)";
  else if (fragcf.get_is(n)) out << " domain";
  if (copycf.get_is_static(n)) out << " copy(s)";
  else if (copycf.get_is(n)) out << " copy";
  if (typedcf.get_is_static(n)) out << " typed(s)";
  else if (typedcf.get_is(n)) out << " typed";
  if (diffusercf.get_is_static(n)) out << " diffuser(s)";
  else if (diffusercf.get_is(n)) out << " diffuser";
  if (refcf.get_is_static(n)) out << " reference(s)";
  else if (refcf.get_is(n)) out << " reference";
  if (scalecf.get_is_static(n)) out << " scale(s)";
  else if (scalecf.get_is(n)) out << " scale";
  if (strucpcf.get_is_static(n)) out << " structure provenance(s)";
  else if (strucpcf.get_is(n)) out << " structure provenance";
  if (samppcf.get_is_static(n)) out << " sample provenance(s)";
  else if (samppcf.get_is(n)) out << " sample provenance";
  if (combpcf.get_is_static(n)) out << " combine provenance(s)";
  else if (combpcf.get_is(n)) out << " combine provenance";
  if (filtpcf.get_is_static(n)) out << " filter provenance(s)";
  else if (filtpcf.get_is(n)) out << " filter provenance";
  if (clustpcf.get_is_static(n)) out << " cluster provenance(s)";
  else if (clustpcf.get_is(n)) out << " cluster provenance";
  if (scriptpcf.get_is_static(n)) out << " script provenance(s)";
  else if (scriptpcf.get_is(n)) out << " script provenance";
  if (softpcf.get_is_static(n)) out << " software provenance(s)";
  else if (softpcf.get_is(n)) out << " software provenance";
  out << "]";
}

struct ShowValues {
  FloatKeys fks;
  IntKeys iks;
  StringKeys sks;
  FloatsKeys fsks;
  IntsKeys isks;
  StringsKeys ssks;
  Vector3Keys v3ks;
  Vector4Keys v4ks;
  Vector3sKeys v3sks;
  ShowValues(FileConstHandle fh) {
    fks = get_keys<FloatTraits>(fh);
    std::sort(fks.begin(), fks.end(), LessName(fh));
    iks = get_keys<IntTraits>(fh);
    std::sort(iks.begin(), iks.end(), LessName(fh));
    sks = get_keys<StringTraits>(fh);
    std::sort(sks.begin(), sks.end(), LessName(fh));
    fsks = get_keys<FloatsTraits>(fh);
    std::sort(fsks.begin(), fsks.end(), LessName(fh));
    isks = get_keys<IntsTraits>(fh);
    std::sort(isks.begin(), isks.end(), LessName(fh));
    ssks = get_keys<StringsTraits>(fh);
    std::sort(ssks.begin(), ssks.end(), LessName(fh));
    v3ks = get_keys<Vector3Traits>(fh);
    std::sort(v3ks.begin(), v3ks.end(), LessName(fh));
    v4ks = get_keys<Vector4Traits>(fh);
    std::sort(v4ks.begin(), v4ks.end(), LessName(fh));
    v3sks = get_keys<Vector3sTraits>(fh);
    std::sort(v3sks.begin(), v3sks.end(), LessName(fh));
  }
  void operator()(NodeConstHandle cur, std::string prefix, std::string suffix,
                  std::ostream& out) {
    show_node(cur, suffix, out, fks, fsks, iks, isks, sks, ssks, v3ks, v4ks,
              v3sks, prefix + "   ");
  }
};

struct ShowDecorators {
  decorator::BondFactory bdf;
  decorator::ColoredFactory ccf;
  decorator::ParticleFactory pcf;
  decorator::IntermediateParticleFactory ipcf;
  decorator::RigidParticleFactory rpcf;
  decorator::ScoreFactory scf;
  decorator::RepresentationFactory repcf;
  decorator::BallFactory bcf;
  decorator::CylinderFactory cycf;
  decorator::SegmentFactory segcf;
  decorator::ResidueFactory rcf;
  decorator::AtomFactory acf;
  decorator::ChainFactory chaincf;
  decorator::DomainFactory fragcf;
  decorator::CopyFactory copycf;
  decorator::DiffuserFactory diffusercf;
  decorator::TypedFactory typedcf;
  decorator::ReferenceFactory refcf;
  decorator::ScaleFactory scalecf;
  decorator::StructureProvenanceFactory strucpcf;
  decorator::SampleProvenanceFactory samppcf;
  decorator::CombineProvenanceFactory combpcf;
  decorator::FilterProvenanceFactory filtpcf;
  decorator::ClusterProvenanceFactory clustpcf;
  decorator::ScriptProvenanceFactory scriptpcf;
  decorator::SoftwareProvenanceFactory softpcf;
  ShowDecorators(FileConstHandle fh)
      : bdf(fh),
        ccf(fh),
        pcf(fh),
        ipcf(fh),
        rpcf(fh),
        scf(fh),
        repcf(fh),
        bcf(fh),
        cycf(fh),
        segcf(fh),
        rcf(fh),
        acf(fh),
        chaincf(fh),
        fragcf(fh),
        copycf(fh),
        diffusercf(fh),
        typedcf(fh),
        refcf(fh),
        scalecf(fh),
        strucpcf(fh),
        samppcf(fh),
        combpcf(fh),
        filtpcf(fh),
        clustpcf(fh),
        scriptpcf(fh),
        softpcf(fh) {}
  void operator()(NodeConstHandle cur, std::string prefix, std::string suffix,
                  std::ostream& out) {
    show_node_decorators(cur, suffix, out, bdf, ccf, pcf, ipcf, rpcf, scf,
                         repcf, bcf, cycf, segcf, rcf, acf, chaincf, fragcf,
                         copycf, diffusercf, typedcf, refcf, scalecf, strucpcf,
                         samppcf, combpcf, filtpcf, clustpcf, scriptpcf,
                         softpcf, prefix + "   ");
  }
};
}

void show_hierarchy(NodeConstHandle root, std::ostream& out) {
  print_tree(out, root, simple_show_node);
}

void show_hierarchy(NodeConstHandle root) {
  print_tree(std::cout, root, simple_show_node);
}

void show_hierarchy_with_values(NodeConstHandle root, std::ostream& out) {
  print_tree(out, root, ShowValues(root.get_file()));
}

void show_hierarchy_with_values(NodeConstHandle root) {
  print_tree(std::cout, root, ShowValues(root.get_file()));
}

void show_hierarchy_with_decorators(NodeConstHandle root, bool,
                                    std::ostream& out) {
  print_tree(out, root, ShowDecorators(root.get_file()));
}

void show_frames(FileConstHandle fh, std::ostream& out) {
  for(FrameID fr : fh.get_root_frames()) {
    show_frames_impl(fh, fr, std::string(), out);
  }
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
