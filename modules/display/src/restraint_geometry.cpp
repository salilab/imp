/**
 *  \file restraint_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/restraint_geometry.h"
#include <IMP/core/PairRestraint.h>
#include <IMP/core/SingletonRestraint.h>
#include <IMP/core/ClosePairsPairScore.h>
#include <IMP/container/PairsRestraint.h>
#include <IMP/container/ListPairContainer.h>
#include <IMP/container/SingletonsRestraint.h>
#include <IMP/algebra/Segment3D.h>
#include <IMP/core/XYZ.h>

IMPDISPLAY_BEGIN_NAMESPACE

PairRestraintGeometry::PairRestraintGeometry(core::PairRestraint*r):
  Geometry(r->get_name()),
  r_(r) {
}

PairsRestraintGeometry::PairsRestraintGeometry(container::PairsRestraint*r):
  Geometry(r->get_name()),
  r_(r) {
}


IMP::display::Geometries PairRestraintGeometry::get_components() const {
  IMP_CHECK_OBJECT(r_);
  IMP::display::Geometries ret;
  bool non_empty=false;
  algebra::Vector3D mp;
  {
    ParticlePair pp= r_->get_argument();
    algebra::Vector3D v0=core::XYZ(pp[0]).get_coordinates();
    algebra::Vector3D v1=core::XYZ(pp[1]).get_coordinates();
    algebra::Segment3D s(v0, v1);
    ret.push_back(new SegmentGeometry(s));
    non_empty=true;
    mp=.5*(v0+v1);
  }
  double s= r_->evaluate(false);
  std::ostringstream oss;
  oss << s;
  ret.push_back(new LabelGeometry(mp, oss.str()));
  return ret;
}

void PairRestraintGeometry::do_show(std::ostream &out) const {
  out << "  restraint: " << r_->get_name() << std::endl;
}

IMP::display::Geometries PairsRestraintGeometry::get_components() const {
  IMP_CHECK_OBJECT(r_);
  IMP::display::Geometries ret;
  bool non_empty=false;
  algebra::Vector3D mp;
  IMP_FOREACH_PAIR(r_->get_container(), {
        algebra::Vector3D v0=core::XYZ(_1[0]).get_coordinates();
        algebra::Vector3D v1=core::XYZ(_1[1]).get_coordinates();
        algebra::Segment3D s(v0, v1);
        ret.push_back(new SegmentGeometry(s));
        non_empty=true;
        mp=.5*(v0+v1);
    });
  double s= r_->evaluate(false);
  std::ostringstream oss;
  oss << s;
  ret.push_back(new LabelGeometry(mp, oss.str()));
  return ret;
}

void PairsRestraintGeometry::do_show(std::ostream &out) const {
  out << "  restraint: " << r_->get_name() << std::endl;
}





ConnectivityRestraintGeometry
  ::ConnectivityRestraintGeometry(core::ConnectivityRestraint*r):
    Geometry(r->get_name()), r_(r) {
}


IMP::display::Geometries ConnectivityRestraintGeometry::get_components() const {
  IMP_CHECK_OBJECT(r_);
  IMP::display::Geometries ret;
  PairScore *ps= r_->get_pair_score();
  ParticlePairsTemp edges= r_->get_connected_pairs();
  ParticlesTemp all(edges.size()*2);
  for (unsigned int i=0; i< edges.size(); ++i) {
    all[2*i]= edges[i][0];
    all[2*i+1]= edges[i][1];
  }
  std::sort(all.begin(), all.end());
  all.erase(std::unique(all.begin(), all.end()), all.end());
  IMP_USAGE_CHECK(all.size() >0, "No particles in connectivity edges");
  for (unsigned int i=0; i< all.size(); ++i) {
    for (unsigned int j=0; j< i; ++j) {
      algebra::Segment3D s((core::XYZ(all[i]).get_coordinates()),
                           core::XYZ(all[j]).get_coordinates());
      ret.push_back(new SegmentGeometry(s));
      ret.back()->set_name(r_->get_name()+" clique");
    }
  }
  if (dynamic_cast<core::KClosePairsPairScore*>(ps)) {
    IMP_LOG(TERSE, "Special casing k close pairs" << std::endl);
    ParticlePairsTemp nedges;
    core::KClosePairsPairScore* kps
      = dynamic_cast<core::KClosePairsPairScore*>(ps);
    for (unsigned int i=0; i< edges.size(); ++i) {
      ParticlePairsTemp cur= kps->get_close_pairs(edges[i]);
      nedges.insert(nedges.end(), cur.begin(), cur.end());
    }
    IMP_USAGE_CHECK(nedges.size() == edges.size(),
                    "Num edges don't match");
    std::swap(nedges, edges);
  }
  IMP_USAGE_CHECK(edges.size() >0, "No connectivity edges");
  algebra::Vector3D mv;
  for (unsigned int i=0; i< edges.size(); ++i) {
    algebra::Segment3D s((core::XYZ(edges[i][0]).get_coordinates()),
                         core::XYZ(edges[i][1]).get_coordinates());
    mv= .5*(s.get_point(0)+ s.get_point(1));
    ret.push_back(new SegmentGeometry(s));
    ret.back()->set_name(r_->get_name()+" edges");
  }

  double s= r_->evaluate(false);
  std::ostringstream oss;
  oss << s;
  ret.push_back(new LabelGeometry(mv, oss.str()));
  ret.back()->set_name(r_->get_name()+" score");
  return ret;
}

void ConnectivityRestraintGeometry::do_show(std::ostream &out) const {
  out << "  restraint: " << r_->get_name() << std::endl;
}



Geometry* create_restraint_geometry(Restraint*r) {
  if (dynamic_cast<core::PairRestraint*>(r)) {
    return new PairRestraintGeometry(dynamic_cast<core::PairRestraint*>(r));
  } else if (dynamic_cast<container::PairsRestraint*>(r)) {
 return new PairsRestraintGeometry(dynamic_cast<container::PairsRestraint*>(r));
  } else if (dynamic_cast<core::ConnectivityRestraint*>(r)) {
    return new
   ConnectivityRestraintGeometry(dynamic_cast<core::ConnectivityRestraint*>(r));
  } else {
    IMP_THROW("Can't handle restraint " << r->get_name()
              << " of type " << r->get_type_name(), ValueException);
  }
}


IMPDISPLAY_END_NAMESPACE
