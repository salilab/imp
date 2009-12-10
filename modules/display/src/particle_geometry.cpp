/**
 *  \file particle_geometry.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/display/particle_geometry.h"


IMPDISPLAY_BEGIN_NAMESPACE

SingletonGeometry::SingletonGeometry(Particle *p):
  Geometry(p->get_name()+" geometry"), p_(p){
}

SingletonsGeometry::SingletonsGeometry(SingletonContainer *pc):
  Geometry(pc->get_name()+" geometry"), sc_(pc){
}

SingletonsGeometry::SingletonsGeometry(SingletonContainer *pc,
                                     Color c):
  Geometry(c, pc->get_name()+" geometry"), sc_(pc){
}

PairGeometry::PairGeometry(Particle *p0,
                           Particle *p1):
  Geometry(p0->get_name()+" geometry"), p0_(p0), p1_(p1){
}

PairsGeometry::PairsGeometry(PairContainer *pc):
  Geometry(pc->get_name()+" geometry"), sc_(pc){
}

PairsGeometry::PairsGeometry(PairContainer *pc,
                             Color c):
  Geometry(c, pc->get_name()+" geometry"), sc_(pc){
}


IMPDISPLAY_END_NAMESPACE
