/**
 *  \file particle_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/display/particle_geometry.h"

IMPDISPLAY_BEGIN_NAMESPACE

SingletonGeometry::SingletonGeometry(Particle *p)
    : Geometry(p->get_name() + " geometry"), p_(p) {}

SingletonsGeometry::SingletonsGeometry(SingletonContainerAdaptor pc)
    : Geometry(pc->get_name() + " geometry"), sc_(pc) {}

SingletonsGeometry::SingletonsGeometry(SingletonContainerAdaptor pc, Color c)
    : Geometry(c, pc->get_name() + " geometry"), sc_(pc) {}

PairGeometry::PairGeometry(const ParticlePair &p)
    : Geometry(p.get_name() + " geometry"),
      p0_(std::get<0>(p)), p1_(std::get<1>(p)) {}

PairsGeometry::PairsGeometry(PairContainer *pc)
    : Geometry(pc->get_name() + " geometry"), sc_(pc) {}

PairsGeometry::PairsGeometry(PairContainer *pc, Color c)
    : Geometry(c, pc->get_name() + " geometry"), sc_(pc) {}

IMPDISPLAY_END_NAMESPACE
