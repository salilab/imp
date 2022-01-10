/**
 *  \file RMF/signature.cpp
 *  \brief Return a (long) string describing a file that can be compared.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/utility.h"

#include <boost/shared_ptr.hpp>
#include <algorithm>
#include <iostream>
#include <string>

#include "RMF/signature.h"
#include "RMF/decorator/alias.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/sequence.h"
#include "RMF/decorator/feature.h"
#include "RMF/decorator/bond.h"
#include "RMF/decorator/reference.h"
#include "RMF/decorator/provenance.h"
#include "RMF/decorator/uncertainty.h"
#include "RMF/decorator/shape.h"

RMF_ENABLE_WARNINGS

namespace RMF {

namespace {
std::string get_static_signature(
    FileConstHandle file, decorator::BondFactory bdcf,
    decorator::ColoredFactory ccf, decorator::ParticleFactory pcf,
    decorator::IntermediateParticleFactory ipcf,
    decorator::RigidParticleFactory rpcf, decorator::ScoreFactory scf,
    decorator::BallFactory bcf, decorator::CylinderFactory cycf,
    decorator::SegmentFactory segcf, decorator::ResidueFactory rcf,
    decorator::AtomFactory acf, decorator::ChainFactory chaincf,
    decorator::DomainFactory fragcf, decorator::CopyFactory copycf,
    decorator::DiffuserFactory diffusercf, decorator::TypedFactory typedcf,
    decorator::ReferenceFactory refcf,
    decorator::ScaleFactory scalecf) {
  std::ostringstream ret;
  ret << "hierarchy [\n";
  RMF_FOREACH(NodeID n, file.get_node_ids()) {
    NodeConstHandle nh = file.get_node(n);
    ret << n << ":";
    RMF_FOREACH(NodeConstHandle c, nh.get_children()) {
      ret << " " << c.get_id();
    }
    ret << "\n";
  }
  ret << "]\n";
  ret << "static [\n";
  RMF_FOREACH(NodeID n, file.get_node_ids()) {
    NodeConstHandle nh = file.get_node(n);
    ret << n << ":";
    if (bdcf.get_is_static(nh)) ret << " bond";
    if (ccf.get_is_static(nh)) ret << " color";
    if (pcf.get_is_static(nh))
      ret << " particle";
    else if (ipcf.get_is_static(nh))
      ret << " iparticle";
    if (rpcf.get_is_static(nh)) ret << " rigid";
    if (scf.get_is_static(nh)) ret << " score";
    if (bcf.get_is_static(nh)) ret << " ball";
    if (cycf.get_is_static(nh)) ret << " cylinder";
    if (segcf.get_is_static(nh)) ret << " segment";
    if (rcf.get_is_static(nh)) ret << " residue";
    if (acf.get_is_static(nh)) ret << " atom";
    if (chaincf.get_is_static(nh)) ret << " chain";
    if (fragcf.get_is_static(nh)) ret << " domain";
    if (copycf.get_is_static(nh)) ret << " copy";
    if (typedcf.get_is_static(nh)) ret << " typed";
    if (diffusercf.get_is_static(nh)) ret << " diffuser";
    if (refcf.get_is_static(nh)) ret << " reference";
    if (scalecf.get_is_static(nh)) ret << " scale";
    ret << "\n";
  }
  ret << "]\n";
  return ret.str();
}

std::string get_frame_signature(
    FileConstHandle file, decorator::BondFactory bdcf,
    decorator::ColoredFactory ccf, decorator::ParticleFactory pcf,
    decorator::IntermediateParticleFactory ipcf,
    decorator::RigidParticleFactory rpcf, decorator::ScoreFactory scf,
    decorator::BallFactory bcf, decorator::CylinderFactory cycf,
    decorator::SegmentFactory segcf, decorator::ResidueFactory rcf,
    decorator::AtomFactory acf, decorator::ChainFactory chaincf,
    decorator::DomainFactory fragcf, decorator::CopyFactory copycf,
    decorator::DiffuserFactory diffusercf, decorator::TypedFactory typedcf,
    decorator::ReferenceFactory refcf,
    decorator::ScaleFactory scalecf) {
  std::ostringstream ret;
  ret << file.get_current_frame() << " [\n";
  RMF_FOREACH(NodeID n, file.get_node_ids()) {
    NodeConstHandle nh = file.get_node(n);
    ret << n << ":";
    if (bdcf.get_is(nh)) ret << " bond";
    if (ccf.get_is(nh)) ret << " color";
    if (pcf.get_is(nh))
      ret << " particle";
    else if (ipcf.get_is(nh))
      ret << " iparticle";
    if (rpcf.get_is(nh)) ret << " rigid";
    if (scf.get_is(nh)) ret << " score";
    if (bcf.get_is(nh)) ret << " ball";
    if (cycf.get_is(nh)) ret << " cylinder";
    if (segcf.get_is(nh)) ret << " segment";
    if (rcf.get_is(nh)) ret << " residue";
    if (acf.get_is(nh)) ret << " atom";
    if (chaincf.get_is(nh)) ret << " chain";
    if (fragcf.get_is(nh)) ret << " domain";
    if (copycf.get_is(nh)) ret << " copy";
    if (typedcf.get_is(nh)) ret << " typed";
    if (diffusercf.get_is(nh)) ret << " diffuser";
    if (refcf.get_is(nh)) ret << " reference";
    if (scalecf.get_is(nh)) ret << " scale";
    ret << "\n";
  }
  ret << "]\n";
  return ret.str();
}
}

std::string get_signature_string(FileConstHandle file) {
  decorator::BondFactory bdf(file);
  decorator::ColoredFactory ccf(file);
  decorator::ParticleFactory pcf(file);
  decorator::IntermediateParticleFactory ipcf(file);
  decorator::RigidParticleFactory rpcf(file);
  decorator::ScoreFactory scf(file);
  decorator::BallFactory bcf(file);
  decorator::CylinderFactory cycf(file);
  decorator::SegmentFactory segcf(file);
  decorator::ResidueFactory rcf(file);
  decorator::AtomFactory acf(file);
  decorator::ChainFactory chaincf(file);
  decorator::DomainFactory fragcf(file);
  decorator::CopyFactory copycf(file);
  decorator::DiffuserFactory diffusercf(file);
  decorator::TypedFactory typedcf(file);
  decorator::ReferenceFactory refcf(file);
  decorator::ScaleFactory scalecf(file);

  std::string ret = get_static_signature(file, bdf, ccf, pcf, ipcf, rpcf, scf,
                                         bcf, cycf, segcf, rcf, acf, chaincf,
                                         fragcf, copycf, diffusercf, typedcf,
                                         refcf, scalecf);
  RMF_FOREACH(FrameID frame, file.get_frames()) {
    file.set_current_frame(frame);
    ret += std::string("\n") + get_frame_signature(file, bdf, ccf, pcf, ipcf,
                                                   rpcf, scf, bcf, cycf, segcf,
                                                   rcf, acf, chaincf, fragcf,
                                                   copycf, diffusercf, typedcf,
                                                   refcf, scalecf);
  }
  return ret;
}

} /* namespace RMF */

RMF_DISABLE_WARNINGS
